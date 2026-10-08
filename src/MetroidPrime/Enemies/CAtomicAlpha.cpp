#include "MetroidPrime/Enemies/CAtomicAlpha.hpp"

#include "Kyoto/Graphics/CModelFlags.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAtomicAlpha.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

#include "float.h"
#include "rstl/math.hpp"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"AggressionCheck",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CAtomicAlpha::AggressionCheck)},
    {"InMaxRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CAtomicAlpha::InMaxRange)},
    {"PathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CAtomicAlpha::PathOver)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CAtomicAlpha::Patrol)},
    {"Pathfind", static_cast< CPatterned::StateMachine::StateFunc >(&CAtomicAlpha::Pathfind)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CAtomicAlpha::Attack)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CAtomicAlpha::Dead)},
};

CAtomicAlpha::CAtomicAlpha(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CActorParameters& actParms, const CPatternedInfo& pInfo,
                           CAssetId bombWeapon, const CDamageInfo& bombDamage, float bombDropDelay,
                           float bombReappearDelay, float bombRappearTime, CAssetId cmdl,
                           bool invisible, bool applyBeamAttraction)
: CPatterned(kPAI_AtomicAlpha, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, actParms)
, mInRange(false)
, mInvisible(invisible)
, mApplyBeamAttraction(applyBeamAttraction)
, mBombDropDelay(bombDropDelay)
, mBombReappearDelay(bombReappearDelay)
, mBombRappearTime(bombRappearTime)
, mBombTime(0.f)
, mCurBomb(0)
, mPathFind(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mSteeringBehaviors()
, mBombProjectile(bombWeapon, bombDamage)
, mBombModel(CStaticRes(cmdl, mData.GetScale()))
, mWaypointId(kInvalidUniqueId)
, mTouchRadius(pInfo.GetHalfExtent() * GetModelData()->GetScale().GetX()) {
  mBombProjectile.Token().Lock();
  mBombLocators.push_back(SBomb(rstl::string_l("bomb1_LCTR"), pas::kLT_Internal10, 3.4028235e38f));
  mBombLocators.push_back(SBomb(rstl::string_l("bomb2_LCTR"), pas::kLT_Internal11, 3.4028235e38f));
  mBombLocators.push_back(SBomb(rstl::string_l("bomb3_LCTR"), pas::kLT_Internal12, 3.4028235e38f));
  mBombLocators.push_back(SBomb(rstl::string_l("bomb4_LCTR"), pas::kLT_Internal13, 3.4028235e38f));
}

CAtomicAlpha::~CAtomicAlpha() {}

void CAtomicAlpha::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    break;
  case kSM_XENF:
    if (GetAlive()) {
      mPendingDeath = true;
    }
    break;
  case kSM_AreaLoaded: {
    const TAreaId areaId = GetCurrentAreaId();
    mPathFind.SetArea(mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea);
  } break;
  }
}

void CAtomicAlpha::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  if (!GetActive()) {
    return;
  }

  mBombTime += dt;

  for (int i = 0; i < mBombLocators.size(); ++i) {
    mBombLocators[i].mScaleTime += dt;
  }
}

void CAtomicAlpha::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  if (GetAlive()) {
    if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(id))) {
      if (mCurDamageRemTime <= 0.f) {
        const uint playerIdx = mgr.MaskUIdNumPlayers(id);
        mgr.PlayerState(playerIdx)->StaticInterference().AddSource(GetUniqueId(), 0.5f, 0.25f);

        for (int i = 0; i < mBombLocators.size(); ++i) {
          mBombLocators[i].mScaleTime = 0.f;
        }
      }
    }
  }

  CPatterned::CollidedWith(id, list, mgr);
}

rstl::optional_object< CAABox > CAtomicAlpha::GetTouchBounds() const {
  const CVector3f extent = mTouchRadius * CVector3f::One();
  return rstl::optional_object< CAABox >(
      CAABox(GetTranslation() - extent, GetTranslation() + extent));
}

void CAtomicAlpha::Render(const CStateManager& mgr) const {
  if (mInvisible) {
    return;
  }
  CPatterned::Render(mgr);

  const float damageLerp = rstl::min_val(1.f, mDamageCooldownTimer / skDamageHitTime);
  CModelFlags flags = CModelFlags::Normal();
  if (damageLerp > 0.f) {
    flags = CModelFlags::ColorModulate(CColor::Lerp(CColor::White(), CColor::Red(), damageLerp));
  }

  for (int i = 0; i < mBombLocators.size(); ++i) {
    const float scale = rstl::min_val(
        rstl::max_val(0.f, mBombLocators[i].mScaleTime - mBombReappearDelay) /
            mBombReappearDelay,
        1.f);

    const CTransform4f locatorXf = GetTransform() *
                                   GetScaledLocatorTransform(mBombLocators[i].mLocatorName) *
                                   CTransform4f::Scale(scale);
    mBombModel.Render(mgr, locatorXf, GetActorLights(), flags);
  }

  if (damageLerp > 0.f) {
    const CModelData* modelData = GetModelData();
    modelData->RenderSolid(
        CModelData::GetRenderingModel(mgr), GetTransform(), false,
        CModelFlags::AdditiveRGB(CColor::Lerp(CColor::Black(), CColor::Red(), damageLerp))
            .DepthCompareUpdate(true, false));
  }
}

void CAtomicAlpha::AddToRenderer(const CStateManager& mgr) const {
  if (mInvisible) {
    return;
  }
  CPatterned::AddToRenderer(mgr);
}

void CAtomicAlpha::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mBombTime = 0.f;
    break;
  case kStateMsg_Update:
    if (mInRange) {
      if (CanDropBomb()) {
        BodyController()->SetLocomotionType(mBombLocators[mCurBomb].mLocomotionType);
      } else {
        BodyController()->SetLocomotionType(pas::kLT_Relaxed);
      }

      if (Leash(mgr, CTriggerData(0.f))) {
        mInRange = false;
      }
    } else {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
      if (InMaxRange(mgr, CTriggerData(0.f))) {
        mInRange = true;
      }
    }
    break;
  case kStateMsg_Deactivate:
    mInRange = false;
    break;
  }
}

void CAtomicAlpha::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f lctrXf = GetLctrTransform(node.GetLocatorName());
    const CVector3f origin = lctrXf.GetTranslation();
    const CTransform4f xf = CTransform4f::LookAt(origin, origin + CVector3f::Down());
    LaunchProjectile(xf, mgr, 4, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                     CVector3f(1.f, 1.f, 1.f));
    mBombTime = 0.f;
    mBombLocators[mCurBomb].mScaleTime = 0.f;
    mCurBomb = (mCurBomb + 1) % mBombLocators.size();
    handled = true;
    break;
  }
  }

  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

bool CAtomicAlpha::AggressionCheck(CStateManager& mgr, const CTriggerData& data) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    if (mApplyBeamAttraction && player->GetPlayerState()->GetChargeBeamFactor() > 0.1f) {
      return true;
    }
  }
  return false;
}

void CAtomicAlpha::Pathfind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mWaypointId = FindConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    if (mWaypointId != kInvalidUniqueId) {
      if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mWaypointId))) {
        CVector3f delta = waypoint->GetTranslation() - GetTranslation();
        if (delta.CanBeNormalized()) {
          delta = delta.Normalize() * 1.f;
        }
        mPathFindNavigation.SetDestination(waypoint->GetTranslation() - delta);
        mWaypointNavigation.SetLastDestination(
            waypoint->FindConnectedObject(mgr, kSS_Arrived, kSM_Next));
      } else {
        mWaypointId = kInvalidUniqueId;
      }
    }
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }

  if (mWaypointId != kInvalidUniqueId) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
  }
}

void CAtomicAlpha::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Internal8);
    break;
  case kStateMsg_Update:
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      const CPlayer* player = mgr.GetPlayer(i);
      if (player->GetPlayerState()->GetChargeBeamFactor() > 0.1f) {
        const CVector3f eyePos = player->GetEyePosition();
        const float distSq = eyePos.MagSquared();
        if (distSq > FLT_EPSILON) {
          BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
              mSteeringBehaviors.Seek(*this, eyePos), CVector3f::Zero(), 1.f / distSq));
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
}

EWeaponCollisionResponseTypes CAtomicAlpha::GetCollisionResponseType(const CVector3f& position,
                                                                     const CVector3f& direction,
                                                                     const CWeaponMode& mode,
                                                                     int attributes) const {
  return GetDamageVulnerability()->WeaponHurts(mode)
             ? kWCR_AtomicAlpha
             : static_cast< EWeaponCollisionResponseTypes >(103);
}

void CAtomicAlpha::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

CEntity* LoadAtomicAlpha(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrAtomicAlpha sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrAtomicAlpha.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CAtomicAlpha(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      LdrToTransform4f(sldrThis.editorProperties), *modelData,
      LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.bombWeapon,
      LdrToDamageInfo(sldrThis.bombDamage), sldrThis.bombDropDelay, sldrThis.bombReappearDelay,
      sldrThis.bombReappearTime, sldrThis.bombModel, sldrThis.invisible,
      sldrThis.homeWhileCharging);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SAtomicAlpha_FuncPtrs funcPtrs;
  funcPtrs.mLoadAtomicAlpha = &LoadAtomicAlpha;
  SetSAtomicAlpha_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSAtomicAlpha_FuncPtrs(nullptr); }
#endif
