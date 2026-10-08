#include "MetroidPrime/Enemies/CShredder.hpp"

#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShredder.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

#include <float.h>
#include <math.h>

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldWakeUp",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShredder::ShouldWakeUp)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CShredder::Lurk)},
    {"Generate", static_cast< CPatterned::StateMachine::StateFunc >(&CShredder::Generate)},
    {"AttackPlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CShredder::AttackPlayer)},
};

CShredder::CShredder(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& modelData,
                     const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                     const SLdrShredderData& data)
: CPatterned(static_cast< EPatternedAI >(0x4e), uid, name, static_cast< EFlavorType >(0), info, xf,
             modelData, patternedInfo, kMT_Flyer, kCT_Zero, kBT_Flyer, actorParams)
, mStartState(data.startState)
, mExplosionDamage(LdrToDamageInfo(data.explosionDamage))
, mMinHeight(data.minHeight)
, mMaxHeight(data.maxHeight)
, mMinDownHeight(data.minDownHeight)
, mMaxDownHeight(data.maxDownHeight)
, mSeparationDistance(data.separationDistance)
, mMinLifeTime(data.minLifeTime)
, mMaxLifeTime(data.maxLifeTime)
, mNormalKnockback(data.normalKnockback)
, mHeavyKnockback(data.heavyKnockback)
, mKnockbackDecline(data.knockbackDecline)
, mIsDarkShredder(data.isDarkShredder)
, mAttacking(false)
, mWokenUp(false)
, mLurking(true)
, mExploded(false)
, mDamageTaken(0.f)
, mFuseProgress(0.f)
, mFuseRate(0.f)
, mGenerateDuration(0.f)
, mGenerateSpeed(0.f)
, mDamageRate(0.f)
, mKnockbackSpeed(0.f)
, mDesiredDistance(data.desiredDistance)
, mReactionAnim(0) {
  const CPASDatabase& database = GetAnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(5));
  const rstl::pair< float, int > anim = database.FindBestAnimation(parms, -1);
  if (anim.first > FLT_EPSILON) {
    mReactionAnim = anim.second;
  }
}

CShredder::~CShredder() {}

void CShredder::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CShredder::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (msg.GetMessage()) {
  case 'XCRT': // Guessed message
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    break;
  case 'ALRT': // Guessed message
    mWokenUp = true;
    break;
  case 'XDMG': { // Guessed message
    if (CWeapon* weapon = TCastToPtr< CWeapon >(mgr.ObjectById(msg.GetSenderId()))) {
      HealthInfo()->SetCauseOfDeathWeapon(weapon->GetCurrentDamageInfo().GetWeaponMode(),
                                          kInvalidUniqueId, kInvalidUniqueId, false, false);
    }
    mHitByPlayerProjectile = true;
    Explode(mgr);
    break;
  }
  case 'XRDG': { // Guessed message
    if (CWeapon* weapon = TCastToPtr< CWeapon >(mgr.ObjectById(msg.GetSenderId()))) {
      mDamageTaken += weapon->GetCurrentDamageInfo().GetVulnerableDamage(*GetDamageVulnerability());
      HealthInfo()->SetCauseOfDeathWeapon(weapon->GetCurrentDamageInfo().GetWeaponMode(),
                                          kInvalidUniqueId, kInvalidUniqueId, false, false);
    }
    break;
  }
  }
}

void CShredder::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Deactivate:
    if (mgr.IsRandomAvailable()) {
      mDamageRate =
          GetHealthInfo()->GetInitialHP() / mgr.Random()->Range(mMinLifeTime, mMaxLifeTime);
      mFuseRate = 1.f + mgr.Random()->Range(0.f, 1.f);
    }
    mLurking = false;
    break;
  case kStateMsg_Update:
    break;
  }
}

void CShredder::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CMaterialFilter& filter = GetMaterialFilter();
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        filter.GetIncludeList(), filter.GetExcludeList().Union(CMaterialList(kMT_Floor))));
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    if (mStartState == 1 && mgr.GetPlayer(0)->GetEyePosition().GetZ() < GetTranslation().GetZ()) {
      mStartState = 3;
    }
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      switch (mStartState) {
      case 0:
        BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
        break;
      case 2:
        BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Four, -1));
        break;
      case 1:
        BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
        break;
      case 3:
        BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, -1));
        break;
      }
    } else if (mGenerateDuration == 0.f) {
      const CAnimData* animData = GetAnimationData();
      mGenerateDuration = animData->GetAnimationDuration(animData->GetCurrentAnimation());
      if (mGenerateDuration > 0.f) {
        if (mStartState == 3) {
          mGenerateSpeed =
              -(mgr.Random()->Range(mMinDownHeight, mMaxDownHeight) - 3.079f) / mGenerateDuration;
        } else {
          mGenerateSpeed =
              (mgr.Random()->Range(mMinHeight, mMaxHeight) - 4.421f) / mGenerateDuration;
        }
      }
    }
    MoveToInOneFrameWR(GetTranslation() + CVector3f(0.f, 0.f, mGenerateSpeed * dt), dt);
    break;
  case kStateMsg_Deactivate: {
    const CMaterialFilter& filter = GetMaterialFilter();
    CMaterialList exclude = filter.GetExcludeList();
    exclude.Remove(kMT_Floor);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(filter.GetIncludeList(), exclude));
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
  }
}

void CShredder::AttackPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SendScriptMsgs(static_cast< EScriptObjectState >('ATTK'), mgr, kSM_None); // Guessed state
    mAttacking = true;
    break;
  case kStateMsg_Update: {
    const CPlayer* player = mgr.GetPlayer(0);
    const CAABox bounds = GetCollisionPrimitive()->CalculateAABox(GetTransform());
    CVector3f toTarget(player->GetTranslation().GetX() - GetTranslation().GetX(),
                       player->GetTranslation().GetY() - GetTranslation().GetY(),
                       player->GetTranslation().GetZ() +
                           (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) + 1.2f -
                           GetTranslation().GetZ());
    if (!IsUnderWater(mgr) && toTarget.GetZ() > 0.f) {
      toTarget.SetZ(0.f);
    }
    if (toTarget.CanBeNormalized()) {
      const float distance = toTarget.Magnitude();
      const CVector3f direction = toTarget.AsNormalized();
      if (distance > mDesiredDistance) {
        BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(direction, direction, 1.f));
      } else if (distance < mDesiredDistance - 1.f) {
        BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(-direction, direction, 1.f));
      }
    }
    ApplySeparation(mgr);
    break;
  }
  case kStateMsg_Deactivate:
    mAttacking = false;
    break;
  }
}

void CShredder::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  if (mKnockbackSpeed > 0.f) {
    MoveToWR(GetTranslation() - GetTransform().GetForward() * mKnockbackSpeed * dt, dt);
    mKnockbackSpeed -= mKnockbackDecline * dt * (GetFluidCount() != 0 ? 0.5f : 1.f);
  }

  if (mAttacking) {
    mDamageTaken += mDamageRate * dt;
    mFuseProgress =
        CMath::Min(mFuseRate * dt + mFuseProgress, mDamageTaken / GetHealthInfo()->GetInitialHP());
    AddReactionAnimation();
    if (mFuseProgress >= 1.f) {
      Explode(mgr);
    }
  }

  if (GetFluidCount() != 0) {
    if (mLurking) {
      BodyController()->SetLocomotionType(mStartState == 2 ? pas::kLT_Crouch : pas::kLT_Internal7);
    } else {
      BodyController()->SetLocomotionType(pas::kLT_Internal14);
    }
  } else if (mLurking) {
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
  } else {
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  }
}

void CShredder::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                             CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (TCastToPtr< CPlayer >(mgr.ObjectById(id)) && mAttacking) {
    Explode(mgr);
  }
}

void CShredder::Explode(CStateManager& mgr) {
  if (!mExploded) {
    mgr.ApplyDamageToWorld(
        GetUniqueId(), *this, GetTranslation(), mExplosionDamage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()));
    mgr.InformListeners(GetTranslation(), static_cast< EListenNoiseType >(7)); // Guessed type
    MassiveDeath(mgr);
    mExploded = true;
  }
}

void CShredder::AddReactionAnimation() {
  AnimationData()->AddAdditiveAnimation(mReactionAnim, CMath::Min(mFuseProgress, 1.f), false,
                                        false);
}

void CShredder::Touch(CActor& other, CStateManager& mgr) {
  CPatterned::Touch(other, mgr);
  if (!mLurking) {
    if (CEnergyProjectile* projectile = TCastToPtr< CEnergyProjectile >(other)) {
      const CWeaponMode& mode = projectile->GetCurrentDamageInfo().GetWeaponMode();
      if (mode.GetType() != kWT_Missile) {
        mKnockbackSpeed =
            (mode.IsCharged() || mode.IsComboed()) ? mHeavyKnockback : mNormalKnockback;
      }
    }
  }
}

bool CShredder::IsListening() const { return true; }

bool CShredder::Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) {
  bool heard = false;
  if (type == 7) {
    const float radius = mExplosionDamage.GetRadius();
    if ((GetTranslation() - position).MagSquared() < radius * radius) {
      if (mLurking) {
        mWokenUp = true;
      } else {
        mDamageTaken = GetHealthInfo()->GetInitialHP();
      }
    }
    heard = true;
  }
  return heard;
}

bool CShredder::ShouldWakeUp(CStateManager& mgr, const CTriggerData& data) const {
  return mWokenUp || mHitByPlayerProjectile || InDetectionRange(mgr, data);
}

bool CShredder::IsUnderWater(const CStateManager& mgr) const {
  if (const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
    const float top = GetCollisionPrimitive()->CalculateAABox(GetTransform()).GetMaxPoint().GetZ();
    return water->GetTriggerBoundsWR().GetMaxPoint().GetZ() > top + 0.25f;
  }
  return false;
}

void CShredder::ApplySeparation(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CPatterned* other = TCastToConstPtr< CPatterned >(list[i]);
    if (other != nullptr && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      CVector3f separation = mSteeringBehaviors.Separation(
          *this, other->GetTranslation(), mSeparationDistance * GetModelData()->GetScale().GetX());
      separation.SetZ(0.f);
      if (separation.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(separation.AsNormalized(), CVector3f::Zero(), 0.5f));
      }
    }
  }
}

void CShredder::Render(const CStateManager& mgr) const {
  if (mAttacking) {
    const float progress = CMath::Min(mFuseProgress, 1.f);
    const float blink =
        0.5f * progress *
        (1.f + sin(2.f * M_PIF * (3.f * progress * progress + 0.01f) * mStateMachine->GetTime()));
    gpRender->SetAmbientColor(CColor(blink, blink, blink, 1.f));
  } else {
    gpRender->SetAmbientColor(CColor::Black());
  }
  CPatterned::Render(mgr);
}

CEntity* REL_LoadShredder(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrShredder sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrShredder.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CShredder(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                          LdrToEntityInfo(info, sldrThis.editorProperties),
                          LdrToTransform4f(sldrThis.editorProperties), *modelData,
                          LdrToActorParameters(sldrThis.actorInformation),
                          LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.data);
}

static void SetFuncPtrs() {
  static SShredder_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadShredder;
  SetSShredder_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSShredder_FuncPtrs(nullptr); }
