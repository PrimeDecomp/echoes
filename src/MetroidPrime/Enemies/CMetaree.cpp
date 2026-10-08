#include "MetroidPrime/Enemies/CMetaree.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMetaree.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "REL/REL_Setup.h"

static EMaterialTypes SolidMaterial = kMT_Solid;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"InRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMetaree::InRange)},
    {"DropDelay", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMetaree::DropDelay)},
    {"AttackDelay", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMetaree::AttackDelay)},
    {"ShouldAttack", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMetaree::ShouldAttack)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"InActive", static_cast< CPatterned::StateMachine::StateFunc >(&CMetaree::InActive)},
    {"Active", static_cast< CPatterned::StateMachine::StateFunc >(&CMetaree::Active)},
    {"InActiveReady", static_cast< CPatterned::StateMachine::StateFunc >(&CMetaree::InActiveReady)},
    {"Halt", static_cast< CPatterned::StateMachine::StateFunc >(&CMetaree::Halt)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CMetaree::Attack)},
    {"Explode", static_cast< CPatterned::StateMachine::StateFunc >(&CMetaree::Explode)},
};

static EMaterialTypes skCollideMaterial = kMT_Player;
static EMaterialTypes skDeadMaterial = kMT_Player;

CMetaree::CMetaree(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                   const CDamageInfo& damageInfo, float dropHeight, const CVector3f& offset,
                   float attackSpeed, float delay, float haltDelay, float launchSpeed,
                   const SLdrAudioPlaybackParms& attackSound)
: CPatterned(kPAI_Metaree, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_Zero, kBT_Flyer, actorParams)
, mDelay(delay)
, mHaltDelay(haltDelay)
, mDropHeight(dropHeight)
, mLaunchSpeed(launchSpeed)
, mOffset(offset)
, mAttackSpeed(attackSpeed)
, mLookPos(CVector3f::Zero())
, mProjectileDelta(0.f, 0.f, 0.f)
, mVelocity(CVector3f::Zero())
, mFleeState(0)
, mDamageInfo(damageInfo)
, mAttackSound(attackSound)
, x83c_24_(true)
, mStarted(false)
, mDropped(false) {}

void CMetaree::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    break;
  case kSM_Start:
    mStarted = true;
    break;
  case kSM_Activate:
  default:
    break;
  }
}

void CMetaree::ThinkAboutMove(float) {}

void CMetaree::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                            CStateManager& mgr) {
  if (mAlive && list.GetCount() > 0) {
    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), mDamageInfo,
                           CMaterialFilter::MakeInclude(CMaterialList(skCollideMaterial)));
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    MassiveDeath(mgr);
  }
}

void CMetaree::Touch(CActor& actor, CStateManager& mgr) {
  if (!mAlive) {
    return;
  }

  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    if (projectile->GetOwnerId() != mgr.GetPlayer(0)->GetUniqueId()) {
      return;
    }

    mHitByPlayerProjectile = true;
    mProjectileDelta = projectile->GetTranslation() - projectile->GetPreviousPos();
  }
}

bool CMetaree::ShouldAttack(CStateManager&, const CTriggerData&) const {
  return GetTranslation().GetZ() < mLookPos.GetZ();
}

bool CMetaree::InRange(CStateManager& mgr, const CTriggerData&) const {
  if (mStarted) {
    return true;
  }
  const float magSq = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
  const float range = 0.5f * (mMinAttackRange + mMaxAttackRange);
  return magSq < range * range;
}

bool CMetaree::DropDelay(CStateManager&, const CTriggerData&) const {
  return mStateMachine->GetTime() > mHaltDelay;
}

bool CMetaree::AttackDelay(CStateManager&, const CTriggerData&) const {
  return mStateMachine->GetTime() > mDelay;
}

void CMetaree::InActive(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  }
}

void CMetaree::InActiveReady(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
  }
}

void CMetaree::Active(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mDropped = true;
    mHitByPlayerProjectile = false;
    mLookPos = GetTranslation() - CVector3f(0.f, 0.f, mDropHeight);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, mLookPos, true));
    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    ApplyImpulseWR(CVector3f(0.f, 0.f, -mLaunchSpeed) * GetMass(), CAxisAngle::Identity());
    break;
  }
  case kStateMsg_Update:
    BodyController()->CommandMgr().SetTargetVector(
        (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).AsNormalized());
    break;
  case kStateMsg_Deactivate:
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

void CMetaree::Halt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    Stop();
    SetVelocityWR(CVector3f::Zero());
    SetMomentumWR(CVector3f::Zero());
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    mLookPos = mgr.GetPlayer(0)->GetTranslation() + mOffset;
    SetTransform(CTransform4f::LookAt(GetTranslation(), mLookPos));
    mStateMachine->SetDelay(mHaltDelay);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetaree::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mFleeState = 0;
    CVector3f dir = (mLookPos - GetTranslation()).AsNormalized();
    SetVelocityWR(mAttackSpeed * dir);
    PlayCustomSound(GetTranslation(), dir, mAttackSound, false);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mVelocity = mAttackSpeed * dir;
    break;
  }
  case kStateMsg_Update:
    if (GetBodyController()->GetPercentageFrozen() == 0.f) {
      SetVelocityWR(mVelocity);
    } else {
      Stop();
      SetVelocityWR(CVector3f::Zero());
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetaree::Dead(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.ApplyDamageToWorld(
        GetUniqueId(), *this, GetTranslation(), mDamageInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(skDeadMaterial), CMaterialList()));
    DeathDelete(mgr);
    break;
  default:
    break;
  }
}

void CMetaree::Flee(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f ang =
        GetMass() * CVector3f(mProjectileDelta.GetX(), mProjectileDelta.GetY(), 0.f) * 5.f;
    ApplyImpulseWR(ang, CAxisAngle::Identity());

    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    SetTransform(CTransform4f::Translate(GetTranslation()));
    mFleeState = 0;
    break;
  }
  case kStateMsg_Update: {
    switch (mFleeState) {
    case 0:
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LieOnGround) {
        mFleeState = 1;
      } else {
        BodyController()->CommandMgr().DeliverCmd(
            CBCKnockDownCmd(CVector3f(0.f, 1.f, 0.f), pas::kS_Zero));
      }
      break;
    default:
      break;
    }

    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetaree::Explode(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.ApplyDamage(
        GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), mDamageInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
        CVector3f::Zero());
    MassiveDeath(mgr);
    break;
  default:
    break;
  }
}

void CMetaree::Think(float dt, CStateManager& mgr) {
  SetValidTarget(0, (mgr.GetPlayerState(0)->GetCurrentVisor() == CPlayerState::kPV_Dark ||
                     mgr.GetPlayerState(0)->GetCurrentVisor() == CPlayerState::kPV_Scan) ||
                        mDropped);
  CPatterned::Think(dt, mgr);
}

void CMetaree::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

CEntity* LoadMetaree(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrMetaree sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrMetaree.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CMetaree(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), LdrToDamageInfo(sldrThis.radiusDamage),
      sldrThis.dropHeight, sldrThis.collisionOffset0, sldrThis.attackSpeed, sldrThis.haltDelay,
      sldrThis.dropDelay, sldrThis.launchSpeed, sldrThis.turnSound);
}

static void SetFuncPtrs() {
  static SMetaree_FuncPtrs funcPtrs;
  funcPtrs.mLoadMetaree = &LoadMetaree;
  SetSMetaree_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSMetaree_FuncPtrs(nullptr); }
