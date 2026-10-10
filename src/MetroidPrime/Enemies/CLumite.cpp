#include "MetroidPrime/Enemies/CLumite.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrLumite.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"InSmallShotRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::InSmallShotRange)},
    {"InBigShotRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::InBigShotRange)},
    {"InSunlight", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::InSunlight)},
    {"FacingPlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::FacingPlayer)},
    {"CanAttack", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::CanAttack)},
    {"ReadyToMove", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::ReadyToMove)},
    {"Landed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::Landed)},
    {"IsHeckler", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::IsHeckler)},
    {"CanTaunt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::CanTaunt)},
    {"FacingHopPoint",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::FacingHopPoint)},
    {"ClearLineOfFire",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CLumite::ClearLineOfFire)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Null", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::Null)},
    {"Pause", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::Pause)},
    {"Hop", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::Hop)},
    {"SmallShot", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::SmallShot)},
    {"BigShot", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::BigShot)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::Taunt)},
    {"Turn", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::Turn)},
    {"TurnToHopPoint",
     static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::TurnToHopPoint)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CLumite::Dead)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"StickToNearestSurface",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CLumite::StickToNearestSurface)},
};

// Guessed name; speed curve of a hop, accelerating towards the middle and slowing at the end.
static float GetHopSpeedScale(float t) {
  if (t < 0.5f) {
    const float f = 2.f * t;
    return 5.f * (1.f - f) + 25.f * f;
  }
  const float f = 2.f * (t - 0.5f);
  return 25.f * (1.f - f) + 15.f * f;
}

CLumite::SShotAttack::SShotAttack(CAssetId projectile, const CDamageInfo& damage, float minRange,
                                  float maxRange)
: mProjectile(projectile, damage), mMinRange(minRange), mMaxRange(maxRange) {
  mProjectile.Token().Lock();
}

CLumite::SSurfaceTarget::SSurfaceTarget()
: mPlane(0.f, CVector3f::Up())
, mClosestPoint(CVector3f::Zero())
, mKind(-1)
, mZoneId(kInvalidUniqueId) {}

CLumite::SHopPlan::SHopPlan(float minDistance, float maxDistance)
: mTarget(CVector3f::Zero())
, mLaunchDirection(CVector3f::Zero())
, mHintId(kInvalidUniqueId)
, x20_(CVector3f::Zero())
, mPlane(0.f, CVector3f::Up())
, mMinDistance(minDistance)
, mMaxDistance(maxDistance)
, mChosenHintId(kInvalidUniqueId)
, mChosenScore(0.f)
, mIgnoreInUse(false)
, mHopSelected(false)
, mHopFailed(false) {
  Reset();
}

void CLumite::SHopPlan::Reset() {
  mNextHopTime = -1000.f;
  mLaunchDirection = CVector3f::Zero();
  mTarget = mLaunchDirection;
  mLaunched = mFinished = mLanded = mDidSideTaunt = mAborted = mSunlightHop = false;
  mPlane = CPlane(0.f, CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes));
  mSpeedScale = 1.f;
  mStartTime = 0.f;
  mAngleDiff = 0.f;
  mDistance = 0.f;
  mCandidates.clear();
  mChosenHintId = kInvalidUniqueId;
  mChosenScore = 0.f;
  mIgnoreInUse = false;
  mHopSelected = false;
  mHopFailed = false;
}

CLumite::SSunlightEffect::SSunlightEffect(CAssetId id)
: mId(id)
, mParticle(nullptr)
, mToken(gpSimplePool->GetObj(SObjectTag('PART', id)))
, mExplosionId(kInvalidUniqueId) {}

CLumite::CLumite(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& modelData,
                 const CPatternedInfo& patternedInfo, CAssetId stateMachine2, float attackTimeMin,
                 float attackTimeMax, float smallShotMinRange, float smallShotMaxRange,
                 float bigShotMinRange, float bigShotMaxRange, float minHopDistance,
                 float maxHopDistance, CAssetId smallShotProjectile,
                 const CDamageInfo& smallShotDamage, CAssetId bigShotProjectile,
                 const CDamageInfo& bigShotDamage, CAssetId trailEffect, CAssetId sunlightEffect,
                 ushort phaseInSound, ushort phaseOutSound, const CActorParameters& actorParams)
: CPatterned(kPAI_Lumite, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mElapsedTime(0.f)
, mSpitPosition(CVector3f::Zero())
, mSurfaceAlign()
, mStateMachine2(gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine2)))
, mAttackTimeMin(attackTimeMin)
, mAttackTimeMax(attackTimeMax)
, mNextAttackTime(-1000.f)
, mSmallShot(smallShotProjectile, smallShotDamage, smallShotMinRange, smallShotMaxRange)
, mBigShot(bigShotProjectile, bigShotDamage, bigShotMinRange, bigShotMaxRange)
, mAttackState(-1)
, mTurnActive(false)
, mTurnLeft(false)
, mSurface()
, mKneeMask(0)
, mHop(minHopDistance, maxHopDistance)
, mInSunlight(false)
, mTransitioning(false)
, mLastSunlightPosition(CVector3f::Zero())
, mAlphaFactor(1.f)
, x968_(-1000.f)
, mSunlightEffect(sunlightEffect)
, mTauntType(pas::kTT_Invalid)
, mTauntStepDir(pas::kSD_Invalid)
, mLastTauntTime(-1000.f)
, mTeamAiMgrId(kInvalidUniqueId)
, mSpinRate(0.f)
, mBounceSpeed(7.f)
, mPhaseInSound(phaseInSound)
, mPhaseOutSound(phaseOutSound) {
  mSurfaceAlign.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
}

void CLumite::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  mStateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  mStateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

rstl::optional_object< CAABox > CLumite::GetTouchBounds() const { return GetBoundingBox(); }

CVector3f CLumite::GetOrbitPosition(const CStateManager& mgr) const {
  const rstl::optional_object< CAABox >& bounds = GetTouchBounds();
  return GetTranslation() + CVector3f(0.f, 0.f, 0.5f * bounds->GetDepth());
}

CVector3f CLumite::GetAimPosition(const CStateManager& mgr, float dt) const {
  const rstl::optional_object< CAABox >& bounds = GetTouchBounds();
  return GetTranslation() + CVector3f(0.f, 0.f, 0.5f * bounds->GetDepth());
}

void CLumite::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

void CLumite::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

CScannableObjectInfo* CLumite::GetScannableObjectInfo() const {
  if (mAlphaFactor == 0.f) {
    return nullptr;
  }
  return CPatterned::GetScannableObjectInfo();
}

CProjectileInfo* CLumite::ProjectileInfo() {
  if (mAttackState == 1) {
    return &mSmallShot.mProjectile;
  }
  if (mAttackState == 2) {
    return &mBigShot.mProjectile;
  }
  return nullptr;
}

void CLumite::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (!mStateMachine->HasState()) {
    SetupStateMachineHelper(mgr);
    return;
  }

  if (!mAlive) {
    mSurfaceAlign.Update(*this, mgr, dt);
    CPatterned::Think(dt, mgr);
    return;
  }

  mElapsedTime += dt;
  if (!BodyController()->GetIsActive()) {
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    DisableGroundCollision(mgr);
  }

  CPatterned::Think(dt, mgr);
  if ((mAttackState == 0 && mHop.mLanded) || (mAttackState != 0 && mAttackState != 3)) {
    StickToSurface(dt);
  }
  UpdateSunlight(mgr, dt);
  DamageSafeZoneOccupant(mgr, dt);
}

void CLumite::DisableGroundCollision(CStateManager& mgr) {
  mVerticalMovement = true;
  mOnGround = false;
  RemoveMaterial(kMT_GroundCollider, mgr);
}

void CLumite::StickToSurface(float dt) {
  mSurface.mClosestPoint = mSurface.mPlane.GetClosestPoint(GetTranslation());
  if (mSurface.mKind == 2) {
    mSurface.mClosestPoint += -0.2f * mSurface.mPlane.GetNormal();
  }
  const CVector3f delta = mSurface.mClosestPoint - GetTranslation();
  if (delta.Magnitude() <= 0.1f) {
    SetVelocityWR(CVector3f::Zero());
  } else {
    SetVelocityWR(20.f * delta);
  }
  mSurfaceAlign.OrientToSurfaceNormal(*this, mSurface.mPlane.GetNormal(), dt);
}

void CLumite::StickToNearestSurface(CStateManager& mgr, float dt) {
  SetVelocityWR(CVector3f::Zero());
  mSurfaceAlign.Update(*this, mgr, 1000.f);
  mSurface.mPlane = mSurfaceAlign.GetSurface().GetPlane();
  mSurface.mClosestPoint = mSurface.mPlane.GetClosestPoint(GetTranslation());
}

void CLumite::PlaySound(CStateManager& mgr, ushort sfx) {
  CAudioSys::C3DEmitterParmData parms(50.f, 0.1f, 1, 127, 20);
  parms.mPos = GetTranslation();
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = sfx;
  CSfxManager::AddEmitter(parms, GetCurrentAreaId().Value(), false, false,
                          CSfxManager::kMedPriority);
}

void CLumite::UpdateSunlight(CStateManager& mgr, float dt) {
  if (!mAlive) {
    return;
  }

  const CVector3f moved = mLastSunlightPosition - GetTranslation();
  if (!mTransitioning || moved.MagSquared() > 0.09f) {
    const bool touching = IsTouchingSunTrigger(mgr);
    if (touching != mInSunlight || !mTransitioning) {
      mInSunlight = touching;
      if (mInSunlight) {
        RemoveMaterial(kMT_ExcludeFromRadar, mgr);
        AddMaterial(kMT_Scannable, mgr);
        PlaySound(mgr, mPhaseInSound);
      } else {
        AddMaterial(kMT_ExcludeFromRadar, mgr);
        RemoveMaterial(kMT_Scannable, mgr);
        PlaySound(mgr, mPhaseOutSound);
      }
      CExplosion* explosion = rs_new CExplosion(
          mSunlightEffect.mToken, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("Lumite Sunlight Transition Effect"), GetTransform(), 0,
          GetModelData()->GetScale(), CColor::White(), -1);
      mgr.AddObject(explosion);
      mSunlightEffect.mExplosionId = explosion->GetUniqueId();
    }
    mLastSunlightPosition = GetTranslation();
  }

  if (mSunlightEffect.mExplosionId != kInvalidUniqueId) {
    if (CExplosion* explosion =
            TCastToPtr< CExplosion >(mgr.ObjectById(mSunlightEffect.mExplosionId))) {
      explosion->SetTransform(GetTransform());
    } else {
      mSunlightEffect.mExplosionId = kInvalidUniqueId;
    }
  }

  if (!mInSunlight) {
    mAlphaFactor -= dt / 0.6f;
    if (mAlphaFactor < 0.f || !mTransitioning) {
      mAlphaFactor = 0.f;
    }
  } else {
    mAlphaFactor += dt / 0.6f;
    if (mAlphaFactor > 1.f || !mTransitioning) {
      mAlphaFactor = 1.f;
    }
  }

  if (mgr.GetPlayerState(0)->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    mColor.SetAlpha(1.f);
    Shadow()->SetUserAlpha(0.f);
  } else {
    if (!mInSunlight) {
      RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    } else {
      AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    }
    mColor.SetAlpha(mAlphaFactor);
    Shadow()->SetUserAlpha(mAlphaFactor);
  }

  mTransitioning = true;
  mAlphaDelta = 0.f;
}

bool CLumite::IsTouchingSunTrigger(CStateManager& mgr) const {
  if (mAttackState != 0 && mSurface.mKind == 2) {
    return true;
  }

  const CAABox box(GetTranslation() - CVector3f(1.f, 1.f, 1.f),
                   GetTranslation() + CVector3f(1.f, 1.f, 1.f));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Trigger)),
                    nullptr);
  for (const TUniqueId* it = nearList.begin(); it != nearList.end(); ++it) {
    const CScriptTrigger* trigger = TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(*it));
    if (trigger != nullptr && trigger->GetActive() &&
        (trigger->GetTriggerFlags() & kTFL_LumiteSunlight)) {
      if (const CScriptTriggerOrientated* orientated =
              TCastToConstPtr< CScriptTriggerOrientated >(trigger)) {
        if (orientated->GetOBBox().IntersectsAABox(box)) {
          return true;
        }
      } else if (trigger->GetTriggerBoundsWR().DoBoundsOverlap(box)) {
        return true;
      }
    }
  }
  return false;
}

void CLumite::DamageSafeZoneOccupant(CStateManager& mgr, float dt) {
  if (mSurface.mKind == 2 && mSurface.mZoneId != kInvalidUniqueId) {
    CScriptSafeZone* zone = TCastToPtr< CScriptSafeZone >(mgr.ObjectById(mSurface.mZoneId));
    if (zone != nullptr && zone->GetActive() && zone->IsHurtful()) {
      zone->DamageActor(mgr, GetUniqueId(), dt);
    }
  }
}

void CLumite::SpawnZoneImpact(CStateManager& mgr, const CVector3f& position, float scale) {
  if (mSurface.mKind == 2 && mSurface.mZoneId != kInvalidUniqueId) {
    const CVector3f closest = mSurface.mPlane.GetClosestPoint(position);
    const CVector3f impactPosition = closest + -0.2f * mSurface.mPlane.GetNormal();
    CScriptSafeZone* zone = TCastToPtr< CScriptSafeZone >(mgr.ObjectById(mSurface.mZoneId));
    if (zone == nullptr) {
      mSurface.mZoneId = kInvalidUniqueId;
    } else {
      zone->SpawnImpactEffect(impactPosition, scale);
    }
  }
}

void CLumite::PreRender(CStateManager& mgr) {
  if (!mTransitioning) {
    UpdateSunlight(mgr, 100.f);
  }
  CPatterned::PreRender(mgr);
  mSpitPosition = GetLctrTransform(rstl::string_l("spit_LCTR")).GetTranslation();
  if (mKneeMask != 0) {
    for (int i = 0; i < 6; ++i) {
      if (mKneeMask & (1 << i)) {
        const CTransform4f kneeXf = GetLctrTransform(GetKneeName(i));
        SpawnZoneImpact(mgr, kneeXf.GetTranslation(), 0.4f);
      }
    }
    mKneeMask = 0;
  }
}

int CLumite::GetKneeIndex(const rstl::string& name) const {
  if (name.size() > 0 && name[0] == 'L') {
    if (name == rstl::string_l("L_front_knee")) {
      return 2;
    }
    if (name == rstl::string_l("L_back_knee")) {
      return 0;
    }
    if (name == rstl::string_l("L_middle_knee")) {
      return 1;
    }
  } else {
    if (name == rstl::string_l("R_front_knee")) {
      return 5;
    }
    if (name == rstl::string_l("R_back_knee")) {
      return 3;
    }
    if (name == rstl::string_l("R_middle_knee")) {
      return 4;
    }
  }
  return -1;
}

rstl::string CLumite::GetKneeName(int index) const {
  switch (index) {
  case 0:
    return rstl::string_l("L_back_knee");
  case 1:
    return rstl::string_l("L_middle_knee");
  case 2:
    return rstl::string_l("L_front_knee");
  case 3:
    return rstl::string_l("R_back_knee");
  case 4:
    return rstl::string_l("R_middle_knee");
  case 5:
    return rstl::string_l("R_front_knee");
  default:
    return rstl::string_l("");
  }
}

void CLumite::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                              float dt) {
  switch (type) {
  case kUE_Projectile:
    if (mAttackState == 1) {
      FireSmallShot(mgr);
      SpawnZoneImpact(mgr, GetTranslation(), 1.1f);
    } else if (mAttackState == 2) {
      FireBigShot(mgr);
      SpawnZoneImpact(mgr, GetTranslation(), 1.1f);
    }
    break;
  case kUE_BeginAction:
    if (mSurface.mKind == 2) {
      const int index = GetKneeIndex(node.GetLocatorName());
      if (index != -1) {
        mKneeMask |= static_cast< uchar >(1 << index);
      }
    }
    break;
  case kUE_EffectOn:
    if (mAttackState == 4) {
      mTurnActive = true;
    }
    break;
  case kUE_EffectOff:
    if (mAttackState == 4) {
      mTurnActive = false;
    }
    break;
  }
}

void CLumite::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                           CStateManager& mgr) {
  if (mAlive) {
    CPatterned::CollidedWith(id, list, mgr);
    return;
  }
  for (const CCollisionInfo* it = list.Begin(); it != list.End(); ++it) {
    if (it->GetMaterialLeft().HasMaterial(kMT_Floor)) {
      if (mBounceSpeed > 1.f) {
        const CVector3f horizontalVelocity(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f);
        SetVelocityWR(0.5f * horizontalVelocity + mBounceSpeed * CVector3f::Up());
        mBounceSpeed *= 0.5f;
      } else {
        SetVelocityWR(CVector3f::Zero());
        mOnStaticGround = true;
        mPrevOnGround = true;
      }
      return;
    }
  }
  CPatterned::CollidedWith(id, list, mgr);
}

void CLumite::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Delete: {
    QuitTeam(mgr);
    if (mHop.mHintId != kInvalidUniqueId) {
      if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHop.mHintId))) {
        hint->SetInUse(false);
        hint->SetTimeRemaining(0.f);
      }
      mHop.mHintId = kInvalidUniqueId;
    }
    if (mHop.mChosenHintId != kInvalidUniqueId) {
      if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHop.mChosenHintId))) {
        hint->SetInUse(false);
        hint->SetTimeRemaining(0.f);
      }
      mHop.mChosenHintId = kInvalidUniqueId;
    }
    if (mSunlightEffect.mExplosionId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mSunlightEffect.mExplosionId);
      mSunlightEffect.mExplosionId = kInvalidUniqueId;
    }
    CPatterned::AcceptScriptMsg(mgr, msg);
    break;
  }
  case kSM_Launching:
    mHop.mLaunched = true;
    mSurface.mPlane = mHop.mPlane;
    CPatterned::AcceptScriptMsg(mgr, msg);
    break;
  case kSM_Landed:
  case kSM_LandedOnStaticGround:
    break;
  case kSM_OffGround:
    if (!mVerticalMovement && BodyController()->GetPercentageFrozen() == 0.f) {
      SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
      RemoveMaterial(kMT_GroundCollider, mgr);
    }
    mOnGround = false;
    mOnStaticGround = false;
    break;
  case kSM_AreaLoaded: {
    if (GetActive() && mTeamAiMgrId == kInvalidUniqueId) {
      mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
      JoinTeam(mgr);
    }
    CCollisionSurface surface(CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), -1);
    if (mSurfaceAlign.FindNearestSurface(mgr, GetTranslation(), 4.f, surface)) {
      mSurface.mPlane = surface.GetPlane();
    }
    CPatterned::AcceptScriptMsg(mgr, msg);
    break;
  }
  case kSM_Damage:
  case kSM_ReflectedDamage:
    mHitByPlayerProjectile = true;
    CPatterned::AcceptScriptMsg(mgr, msg);
    break;
  default:
    CPatterned::AcceptScriptMsg(mgr, msg);
    break;
  }
}

// Team

void CLumite::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->JoinTeam(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Unknown,
                     CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CLumite::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
      }
    }
  }
}

CScriptTeamAiMgr* CLumite::GetTeamAiMgr(CStateManager& mgr) {
  return TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId));
}

const CScriptTeamAiMgr* CLumite::GetTeamAiMgr(const CStateManager& mgr) const {
  return TCastToConstPtr< CScriptTeamAiMgr >(mgr.GetObjectById(mTeamAiMgrId));
}

int CLumite::GetTeamRole(const CStateManager& mgr) const {
  const CScriptTeamAiMgr* team = GetTeamAiMgr(mgr);
  if (team == nullptr || team->GetRoleCount() < 2) {
    return 0;
  }
  return team->GetTeamRole(GetUniqueId());
}

void CLumite::StartTeamAction(CStateManager& mgr) {
  if (GetTeamAiMgr(mgr) != nullptr) {
    GetTeamAiMgr(mgr)->StartTeamAction(GetUniqueId(), CScriptTeamAiMgr::kTA_Hop);
  }
}

void CLumite::EndTeamAction(CStateManager& mgr) {
  if (GetTeamAiMgr(mgr) != nullptr) {
    GetTeamAiMgr(mgr)->EndTeamAction(GetUniqueId(), CScriptTeamAiMgr::kTA_Hop);
  }
}

// Triggers

bool CLumite::InSmallShotRange(CStateManager& mgr, const CTriggerData& data) const {
  const float distance = DistanceToPlayer(mgr);
  return distance > mSmallShot.mMinRange && distance < mSmallShot.mMaxRange;
}

bool CLumite::InBigShotRange(CStateManager& mgr, const CTriggerData& data) const {
  const float distance = DistanceToPlayer(mgr);
  return distance > mBigShot.mMinRange && distance < mBigShot.mMaxRange;
}

bool CLumite::InSunlight(CStateManager& mgr, const CTriggerData& data) const { return mInSunlight; }

bool CLumite::FacingPlayer(CStateManager& mgr, const CTriggerData& data) const {
  return FacingHint(mgr.GetPlayer(0)->GetTranslation());
}

bool CLumite::CanAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (IsHeckler(mgr, data)) {
    return false;
  }
  if (mNextAttackTime > mElapsedTime) {
    return false;
  }
  CTriggerData trigger(0.f);
  return InDetectionRange(mgr, trigger);
}

bool CLumite::ReadyToMove(CStateManager& mgr, const CTriggerData& data) const {
  if (mSurface.mKind == 2 && mSurface.mZoneId != kInvalidUniqueId) {
    const CScriptSafeZone* zone =
        TCastToConstPtr< CScriptSafeZone >(mgr.GetObjectById(mSurface.mZoneId));
    if (zone == nullptr || !zone->GetActive()) {
      return true;
    }
  }
  const CScriptTeamAiMgr* team = GetTeamAiMgr(mgr);
  if (team != nullptr && team->GetTeamActionCount(CScriptTeamAiMgr::kTA_Hop) >= 5) {
    return false;
  }
  return mHop.mNextHopTime < mElapsedTime;
}

bool CLumite::Landed(CStateManager& mgr, const CTriggerData& data) const {
  return mHop.mAborted == 1 || mHop.mLanded == 1;
}

bool CLumite::IsHeckler(CStateManager& mgr, const CTriggerData& data) const {
  return GetTeamRole(mgr) == CTeamAiRole::kTAR_Unknown;
}

bool CLumite::CanTaunt(CStateManager& mgr, const CTriggerData& data) const {
  if (IsHeckler(mgr, data)) {
    return mLastTauntTime + 2.5f < mElapsedTime;
  }
  return mLastTauntTime + 3.5f < mElapsedTime;
}

bool CLumite::FacingHopPoint(CStateManager& mgr, const CTriggerData& data) const {
  return mHop.mHopFailed || (mHop.mHopSelected && FacingHint(mHop.mTarget));
}

bool CLumite::ClearLineOfFire(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f start = GetTranslation() + GetTransform().GetUp();
  const CVector3f end = mgr.GetPlayer(0)->GetTranslation() + CVector3f(0.f, 0.f, 1.5f);
  const CVector3f delta = end - start;
  if (!delta.CanBeNormalized()) {
    return true;
  }
  static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CRayCastResult result = CGameCollision::RayStaticIntersection(
      mgr, start, delta.AsNormalized(), delta.Magnitude(), filter);
  return !result.IsValid();
}

float CLumite::DistanceToPlayer(const CStateManager& mgr) const {
  return (GetTranslation() - mgr.GetPlayer(0)->GetTranslation()).Magnitude();
}

bool CLumite::FacingHint(const CVector3f& point) const {
  const CVector3f closest = mSurface.mPlane.GetClosestPoint(point);
  CVector3f direction = closest - GetTranslation();
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  }
  return CVector3f::GetAngleDiff(direction, GetTransform().GetRight()) < (M_PIF / 4.f);
}

// Attack

void CLumite::SetAttackState(int state, EStateMsg msg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAttackState = state;
    break;
  case kStateMsg_Deactivate:
    mAttackState = -1;
    break;
  }
}

void CLumite::ResetAttackTimer(CStateManager& mgr) {
  mNextAttackTime = mElapsedTime;
  if (mgr.IsRandomAvailable()) {
    mNextAttackTime += mgr.Random()->Range(mAttackTimeMin, mAttackTimeMax);
  }
}

void CLumite::Null(CStateManager& mgr, EStateMsg msg, float dt) {}

void CLumite::Pause(CStateManager& mgr, EStateMsg msg, float dt) {}

void CLumite::SmallShot(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(1, msg);
  if (msg == kStateMsg_Deactivate) {
    ResetAttackTimer(mgr);
  }
  DeliverCommand(msg, pas::kAS_ProjectileAttack,
                 CBCProjectileAttackCmd(pas::kS_Zero, mgr.GetPlayer(0)->GetTranslation(), false));
}

void CLumite::BigShot(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(2, msg);
  if (msg == kStateMsg_Deactivate) {
    ResetAttackTimer(mgr);
  }
  DeliverCommand(msg, pas::kAS_ProjectileAttack,
                 CBCProjectileAttackCmd(pas::kS_Two, mgr.GetPlayer(0)->GetTranslation(), false));
}

CVector3f CLumite::RandomVector(CStateManager& mgr, float scale) {
  if (!mgr.IsRandomAvailable()) {
    return CVector3f::Zero();
  }
  return CVector3f(mgr.Random()->Range(-scale, scale), mgr.Random()->Range(-scale, scale),
                   mgr.Random()->Range(-scale, scale));
}

CTransform4f CLumite::BuildProjectileTransform(CStateManager& mgr, float spread) {
  CVector3f target = mgr.GetPlayer(0)->GetTranslation() + CVector3f(0.f, 0.f, 1.5f);
  target += RandomVector(mgr, spread);
  CTransform4f xf = GetTransform();
  xf.SetTranslation(mSpitPosition);
  return CTransform4f::LookAt(mSpitPosition, target, xf.GetUp());
}

void CLumite::FireSmallShot(CStateManager& mgr) {
  const CImpactVisorEffect visorEffect = CImpactVisorEffect::None();
  const CVector3f scale(1.f, 1.f, 1.f);
  const CTransform4f xf = BuildProjectileTransform(mgr, 0.75f);
  LaunchProjectile(xf, mgr, 5, 0, false, visorEffect, scale);
}

void CLumite::FireBigShot(CStateManager& mgr) {
  const CImpactVisorEffect visorEffect = CImpactVisorEffect::None();
  const CVector3f scale(1.f, 1.f, 1.f);
  const CTransform4f xf = BuildProjectileTransform(mgr, 0.75f);
  LaunchProjectile(xf, mgr, 5, 0, false, visorEffect, scale);
}

void CLumite::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(3, msg);
  if (msg == kStateMsg_Activate) {
    mTauntType = pas::kTT_Invalid;
    mTauntStepDir = pas::kSD_Invalid;
    mLastTauntTime = -1000.f;
    if (mSurface.mKind != 0) {
      mTauntType = pas::kTT_One;
    } else {
      const float r = mgr.Random()->Float();
      if (r < 0.6f) {
        if (mHop.mDidSideTaunt) {
          mTauntType = pas::kTT_Zero;
        } else {
          if (r < 0.2f) {
            mTauntStepDir = pas::kSD_Forward;
          } else if (r < 0.4f) {
            mTauntStepDir = pas::kSD_Left;
          } else {
            mTauntStepDir = pas::kSD_Right;
          }
          mHop.mDidSideTaunt = true;
        }
      } else if (r < 0.75f) {
        mTauntType = pas::kTT_Zero;
      } else {
        mTauntType = pas::kTT_One;
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    mLastTauntTime = mElapsedTime;
    if (mSurface.mKind == 0) {
      CCollisionSurface surface(CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), -1);
      if (mSurfaceAlign.FindNearestSurface(mgr, GetTranslation(), 4.f, surface) &&
          !surface.IsDegenerate()) {
        mSurface.mPlane = surface.GetPlane();
      }
    }
  }

  if (mTauntType != pas::kTT_Invalid) {
    DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(mTauntType));
  } else {
    DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(mTauntStepDir, pas::kStep_Normal));
  }
}

void CLumite::Turn(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(4, msg);
  TurnTowards(msg, mgr.GetPlayer(0)->GetTranslation());
}

void CLumite::TurnTowards(EStateMsg msg, const CVector3f& target) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mTurnActive = false;
    mTurnLeft = false;
    const CVector3f closest = mSurface.mPlane.GetClosestPoint(target);
    const float distance = (closest - GetTranslation()).Magnitude();
    CTransform4f xfA = GetTransform();
    xfA.RotateLocalZ(CRelAngle::FromDegrees(-45.f));
    CTransform4f xfB = GetTransform();
    xfB.RotateLocalZ(CRelAngle::FromDegrees(45.f));
    const CVector3f pointA = GetTranslation() + (0.5f * distance) * xfA.GetForward();
    const CVector3f pointB = GetTranslation() + (0.5f * distance) * xfB.GetForward();
    mTurnLeft = (closest - pointA).MagSquared() > (closest - pointB).MagSquared();
    break;
  }
  case kStateMsg_Update:
    if (mTurnActive) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    break;
  case kStateMsg_Deactivate:
    mTurnActive = false;
    mTurnLeft = false;
    break;
  }
  DeliverCommand(msg, pas::kAS_Step,
                 CBCStepCmd(mTurnLeft ? pas::kSD_Left : pas::kSD_Right, pas::kStep_RollDodge));
}

// Hopping

void CLumite::TurnToHopPoint(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(4, msg);
  switch (msg) {
  case kStateMsg_Activate:
    if (mHop.mChosenHintId != kInvalidUniqueId) {
      if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHop.mChosenHintId))) {
        hint->SetInUse(false);
        hint->SetTimeRemaining(0.f);
      }
    }
    mHop.Reset();
    StartTeamAction(mgr);
    break;
  case kStateMsg_Update:
    if (!mHop.mHopSelected && !mHop.mHopFailed) {
      if (ChooseHop(mgr)) {
        if (mHop.mHopSelected) {
          SelectHopTarget(mgr, mHop.mChosenHintId);
          TurnTowards(kStateMsg_Activate, mHop.mTarget);
        } else if (mHop.mHopFailed) {
          mHop.mAborted = true;
        }
      }
    }
    if (mHop.mHopSelected) {
      TurnTowards(msg, mHop.mTarget);
    }
    break;
  case kStateMsg_Deactivate:
    EndTeamAction(mgr);
    TurnTowards(msg, mHop.mTarget);
    break;
  }
}

void CLumite::Hop(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(0, msg);
  if (mHop.mAborted) {
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    return;
  }

  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    StartTeamAction(mgr);
    mKneeMask = 0;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(CBCJumpCmd(
          mHop.mTarget, pas::kJT_Normal, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    if (mHop.mLaunched) {
      if (mAnimationState.IsOver()) {
        mHop.mLanded = true;
        mHop.mFinished = true;
      }
      if (!mHop.mLanded && mHop.mSunlightHop &&
          (mHop.mTarget - GetTranslation()).Magnitude() < 0.5f) {
        mHop.mLanded = true;
        mHop.mFinished = true;
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      }
      if (!mHop.mLanded) {
        const float t = CMath::Clamp(0.f, (mElapsedTime - mHop.mStartTime) / 1.5f, 1.f);
        CVector3f direction = mHop.mTarget - GetTranslation();
        if (direction.CanBeNormalized()) {
          direction.Normalize();
          const CVector3f velocity = mHop.mLaunchDirection * (1.f - t) + direction * t;
          SetVelocityWR(mHop.mSpeedScale * (GetHopSpeedScale(t) * velocity));
        }
        CVector3f orientation = mHop.mLaunchDirection;
        if (CVector3f::GetAngleDiff(mHop.mLaunchDirection, mHop.mPlane.GetNormal()) >
            (5.f * (M_PIF / 180.f))) {
          orientation = CVector3f::Slerp(mHop.mLaunchDirection, mHop.mPlane.GetNormal(),
                                         CRelAngle::FromRadians(mHop.mAngleDiff * t));
        }
        if (orientation.CanBeNormalized()) {
          orientation.Normalize();
          mSurfaceAlign.OrientToSurfaceNormal(*this, orientation, 3.f * dt);
        }
      }
    }
    break;
  case kStateMsg_Deactivate: {
    EndTeamAction(mgr);
    if (mHop.mSunlightHop) {
      mSurface.mKind = 2;
      mSurface.mPlane = mHop.mPlane;
      SpawnZoneImpact(mgr, GetTranslation(), 1.3f);
    } else {
      CCollisionSurface surface(CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), -1);
      if (mSurfaceAlign.FindNearestSurface(mgr, GetTranslation(), 4.f, surface)) {
        mSurface.mPlane = surface.GetPlane();
        if (!surface.IsDegenerate() &&
            CVector3f::GetAngleDiff(surface.GetNormal(), mHop.mPlane.GetNormal()) >
                (M_PIF / 12.f)) {
          mHop.mNextHopTime = -1000.f;
          return;
        }
      } else {
        mSurface.mPlane = mHop.mPlane;
      }
      mSurface.mKind = mSurface.mPlane.GetNormal().GetZ() > 0.7f ? 0 : 1;
    }
    mHop.mNextHopTime = mElapsedTime;
    if (mgr.IsRandomAvailable()) {
      mHop.mNextHopTime += mgr.Random()->Range(5.f, 9.f);
    }
    break;
  }
  }
}

void CLumite::SelectHopTarget(CStateManager& mgr, const TUniqueId& hintId) {
  if (mHop.mHintId != kInvalidUniqueId) {
    if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHop.mHintId))) {
      hint->SetInUse(false);
    }
    mHop.mHintId = kInvalidUniqueId;
  }

  CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(hintId));
  if (hint == nullptr) {
    mHop.mAborted = true;
    return;
  }
  hint->SetInUse(true);
  mHop.mHintId = hint->GetUniqueId();
  mSurface.mZoneId = kInvalidUniqueId;

  if (hint->GetHintType() == CScriptAIHint::kHT_SunlightHop) {
    if (TrySunlightHop(mgr, hint->GetTranslation(), mHop.mPlane)) {
      mHop.mSunlightHop = true;
    } else {
      mHop.mAborted = true;
      return;
    }
  } else {
    mHop.mSunlightHop = false;
    for (float radius = 4.f; radius < 16.f; radius += 4.f) {
      CCollisionSurface surface(CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), -1);
      if (mSurfaceAlign.FindNearestSurface(mgr, hint->GetTranslation(), radius, surface)) {
        mHop.mPlane = surface.GetPlane();
        break;
      }
      if (radius == 16.f) {
        mHop.mAborted = true;
        return;
      }
    }
  }

  mHop.mTarget = mHop.mPlane.GetClosestPoint(hint->GetTranslation());
  mHop.mLaunchDirection = GetTransform().GetUp();
  mHop.mStartTime = mElapsedTime;
  if (mHop.mDistance < 10.f) {
    mHop.mSpeedScale = 1.f;
  } else if (mHop.mDistance > 50.f) {
    mHop.mSpeedScale = 2.f;
  } else {
    mHop.mSpeedScale = 1.f + (mHop.mDistance - 10.f) / 40.f;
  }

  CVector3f cross = CVector3f::Cross(mHop.mLaunchDirection, mHop.mPlane.GetNormal());
  while (!cross.CanBeNormalized() && mgr.IsRandomAvailable()) {
    mHop.mLaunchDirection +=
        CVector3f(mgr.Random()->Range(-0.1f, 0.1f), mgr.Random()->Range(-0.1f, 0.1f),
                  mgr.Random()->Range(-0.1f, 0.1f));
    if (mHop.mLaunchDirection.CanBeNormalized()) {
      mHop.mLaunchDirection.Normalize();
      cross = CVector3f::Cross(mHop.mLaunchDirection, mHop.mPlane.GetNormal());
    }
  }
  mHop.mAngleDiff = CVector3f::GetAngleDiff(mHop.mLaunchDirection, mHop.mPlane.GetNormal());
  mHop.mDistance = (mHop.mTarget - GetTranslation()).Magnitude();
}

bool CLumite::TrySunlightHop(CStateManager& mgr, const CVector3f& hintPosition, CPlane& outPlane) {
  outPlane = CPlane(0.f, CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes));
  CScriptSafeZone* zone = FindSafeZone(mgr, hintPosition);
  if (zone == nullptr) {
    return false;
  }

  const CVector3f toZone = zone->GetTranslation() - hintPosition;
  if (!toZone.CanBeNormalized()) {
    return false;
  }

  const CVector3f& zoneScale = zone->GetScale();
  const CSphere sphere(zone->GetTranslation(),
                       (zoneScale.GetX() + zoneScale.GetY() + zoneScale.GetZ()) / 3.f);
  const float distance = toZone.Magnitude();
  const CVector3f direction = toZone.AsNormalized();
  float t;
  CVector3f hitPoint = CVector3f::Zero();
  if (!CollisionUtil::RaySphereIntersection(sphere, hintPosition, direction, distance, t,
                                            hitPoint)) {
    return false;
  }

  CVector3f normal = hintPosition - hitPoint;
  if (!normal.CanBeNormalized()) {
    return false;
  }
  normal = normal.AsNormalized();
  outPlane = CPlane(hitPoint, normal);
  mSurface.mZoneId = zone->GetUniqueId();
  return true;
}

CScriptSafeZone* CLumite::FindSafeZone(CStateManager& mgr, const CVector3f& position) {
  const CSafeZoneManager* zoneMgr = mgr.GetSafeZoneManager();
  for (float radius = 0.5f; radius <= 8.f; radius *= 2.f) {
    const TUniqueId id = zoneMgr->SphereTouchingWhichSafeZone(mgr, CSphere(position, radius));
    if (id != kInvalidUniqueId) {
      CScriptSafeZone* zone = TCastToPtr< CScriptSafeZone >(mgr.ObjectById(id));
      if (zone != nullptr && zone->GetActive() &&
          (zone->GetTranslation() - position).CanBeNormalized() &&
          zone->GetScale().MagSquared() >= 1.f) {
        return zone;
      }
    }
  }
  return nullptr;
}

bool CLumite::ChooseHop(CStateManager& mgr) {
  if (mHop.mCandidates.empty()) {
    BuildHopCandidates(mgr);
  }

  if (!mHop.mCandidates.empty()) {
    const TUniqueId id = mHop.mCandidates.back();
    mHop.mCandidates.pop_back();
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(id));
    if (hint != nullptr && hint->GetActive() &&
        !(mHop.mIgnoreInUse ? hint->GetInUseIgnoreLock(kInvalidUniqueId)
                            : hint->GetInUse(kInvalidUniqueId))) {
      const CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
      float score = 1.f - toPlayer.Magnitude() / mHop.mMaxDistance;
      score = CMath::Clamp(0.f, score, 1.f);
      score = score * mgr.Random()->Float() + 0.01f;
      if (score > mHop.mChosenScore) {
        const CVector3f start = GetTranslation() + 0.5f * GetTransform().GetUp();
        const CVector3f toHint = hint->GetTranslation() - start;
        bool clear = true;
        if (toHint.CanBeNormalized()) {
          const float length = toHint.Magnitude() - 1.f;
          if (length > 0.f) {
            static const CMaterialFilter filter =
                CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
            clear = CGameCollision::RayStaticLineOfSightTest(mgr, start, toHint.AsNormalized(),
                                                             length, filter);
          }
        }
        if (clear) {
          if (mHop.mChosenHintId != kInvalidUniqueId) {
            if (CScriptAIHint* previous =
                    TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHop.mChosenHintId))) {
              previous->SetInUse(false);
              previous->SetTimeRemaining(0.f);
            }
          }
          mHop.mChosenScore = score;
          mHop.mChosenHintId = id;
          hint->SetInUse(true);
        }
      }
    }
  }

  if (mHop.mCandidates.empty()) {
    if (mHop.mChosenHintId != kInvalidUniqueId) {
      mHop.mHopSelected = true;
    } else if (!mHop.mIgnoreInUse) {
      mHop.mIgnoreInUse = true;
    } else {
      mHop.mHopFailed = true;
    }
  }
  return mHop.mHopSelected || mHop.mHopFailed;
}

void CLumite::BuildHopCandidates(CStateManager& mgr) {
  if (mHop.mChosenHintId != kInvalidUniqueId) {
    if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHop.mChosenHintId))) {
      hint->SetInUse(false);
    }
  }
  mHop.mCandidates.clear();
  mHop.mChosenHintId = kInvalidUniqueId;
  mHop.mChosenScore = 0.f;
  mHop.mHopSelected = false;
  mHop.mHopFailed = false;

  CObjectList& list = mgr.ObjectListById(kOL_All);
  for (int i = list.GetFirstObjectIndex(); i != -1 && mHop.mCandidates.size() < 16;
       i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint == nullptr || !hint->GetActive() || hint->GetCurrentAreaId() != GetCurrentAreaId()) {
      continue;
    }
    const CScriptAIHint::EHintType type = hint->GetHintType();
    if (type != CScriptAIHint::kHT_Hop && type != CScriptAIHint::kHT_SunlightHop) {
      continue;
    }
    if (mHop.mIgnoreInUse ? hint->GetInUseIgnoreLock(kInvalidUniqueId)
                          : hint->GetInUse(kInvalidUniqueId)) {
      continue;
    }
    const CVector3f referencePosition =
        mLatestLeashPosition == CVector3f::Zero() ? GetTranslation() : mLatestLeashPosition;
    const float distance = (hint->GetTranslation() - referencePosition).Magnitude();
    if (distance >= mHop.mMinDistance && distance <= mHop.mMaxDistance) {
      if (type == CScriptAIHint::kHT_SunlightHop) {
        const CScriptSafeZone* zone = FindSafeZone(mgr, hint->GetTranslation());
        if (zone == nullptr || !zone->GetActive() || zone->IsHurtful()) {
          continue;
        }
      }
      mHop.mCandidates.push_back(hint->GetUniqueId());
    }
  }
}

// Death

void CLumite::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    AddMaterial(kMT_ProjectilePassthrough, mgr);
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    const CWeaponMode deathWeapon = GetHealthInfo()->GetCauseOfDeathWeapon();
    if (!(deathWeapon.IsComboed() &&
          (deathWeapon.GetType() == kWT_Dark || deathWeapon.GetType() == kWT_Annihilator))) {
      mSurfaceAlign.SetMode(CSurfaceAlignmentHelper::kM_WorldUp);
      const float speed = mgr.Random()->Range(4.f, 9.f);
      SetVelocityWR(CVector3f(speed * GetTransform().GetUp().GetX(),
                              speed * GetTransform().GetUp().GetY(), speed * 0.f));
      mSpinRate = mgr.Random()->Range(-11.f, 11.f);
    }
    break;
  }
  case kStateMsg_Update:
    if (!mPrevOnGround) {
      if (GetVelocityWR().Magnitude() > 0.1f) {
        RotateInOneFrameOR(CQuaternion::ZRotation(CRelAngle::FromDegrees(mSpinRate)), dt);
      }
    } else {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Die));
    }
    if (!mFadeToDeath && BodyController()->GetBodyStateInfo().GetCurrentState()->IsDead()) {
      mFadeToDeath = true;
      mAlphaDelta = -1.f / GetFadeOnDeathTime();
    }
    break;
  }
}

CEntity* LoadLumite(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrLumite sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrLumite.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  ushort phaseInSound = sldrThis.phaseInSound;
  if (sldrThis.phaseInSound == -1) {
    phaseInSound = CSfxManager::kInternalInvalidSfxId;
  }
  ushort phaseOutSound = sldrThis.phaseOutSound;
  if (sldrThis.phaseOutSound == -1) {
    phaseOutSound = CSfxManager::kInternalInvalidSfxId;
  }

  const float averageAttackTime = sldrThis.patterned.averageAttackTime;
  const float halfVariation = 0.5f * sldrThis.patterned.attackTimeVariation;
  return rs_new CLumite(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.patterned.stateMachine2,
      averageAttackTime - halfVariation, averageAttackTime + halfVariation,
      sldrThis.smallShotMinRange, sldrThis.smallShotMaxRange, sldrThis.bigShotMinRange,
      sldrThis.bigShotMaxRange, sldrThis.minHopDistance, sldrThis.maxHopDistance,
      sldrThis.smallShotProjectile, LdrToDamageInfo(sldrThis.smallShotDamage),
      sldrThis.bigShotProjectile, LdrToDamageInfo(sldrThis.bigShotDamage), sldrThis.trailEffect,
      sldrThis.sunlightEnterExitEffect, phaseInSound, phaseOutSound,
      LdrToActorParameters(sldrThis.actorInformation));
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SLumite_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadLumite;
  SetSLumite_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSLumite_FuncPtrs(nullptr); }
#endif

template < typename T >
void CLumite::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(cmd);
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*BodyController(), state)) {
      BodyController()->CommandMgr().DeliverCmd(cmd);
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
  }
}

// ---- Raw matching-decompiler output from a local tree (reference only, not cleaned up) ----

extern "C" void fn_39_4094();
extern "C" int fn_39_572C();
extern int lbl_39_data_434;
extern "C" void fn_39_4124(int, unsigned char*, unsigned char*, unsigned char*);
extern int lbl_39_data_428;
extern int lbl_39_data_41C;
extern int lbl_39_data_410;
extern int lbl_39_data_404;
extern "C" void fn_39_4260();
extern "C" void fn_39_7C0();
struct __mwdec_vt_0_fn_39_6E74 {
  virtual void _0(int);
};

extern "C" bool fn_39_14F4() { return false; }

extern "C" void fn_39_3F08() {
  void fn_39_3E24();
  fn_39_3E24();
}

extern "C" void fn_39_4074() { fn_39_4094(); }

extern "C" void fn_39_4218() {
  void fn_39_4238();
  fn_39_4238();
}

extern "C" void fn_39_778() {
  void fn_39_798();
  fn_39_798();
}

extern "C" int fn_39_5704() {
  u32 ret_0 = 0;
  ret_0 = (fn_39_572C() == 0) ? 1 : ret_0;
  return ret_0;
}

extern "C" int fn_39_3DE8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_39_43A0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_39_43DC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_39_690C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_39_6A48(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_39_CE8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_39_14FC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_39_data_434;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_39_40DC(int arg0) {
  void fn_39_4018(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_39_4124(arg0, stack_24, stack_14, stack_8);
  fn_39_4018(stack_24, -1);
}

extern "C" int fn_39_4F18(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_6494(int arg0, int arg1) {
  void fn_39_64EC(int, int);
  if (arg0) {
    fn_39_64EC(arg0 + 80, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_6544(int arg0, int arg1) {
  void fn_39_659C(int, int);
  if (arg0) {
    fn_39_659C(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_2D1C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_39_data_428;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_39_data_434;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_3724(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_39_data_41C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_39_data_434;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_3BA0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_39_data_410;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_39_data_434;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_4798(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_39_data_404;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_39_data_434;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_6F70(int arg0, int arg1) {
  if (arg0) {
    if (*(unsigned char*)(arg0 + 0x8)) {
      ((CToken*)arg0)->~CToken();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_A20(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x4c))) {
      ((CModelData*)arg0)->~CModelData();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_6C6C(int arg0, int arg1) {
  if (arg0) {
    if (arg0 && (*(unsigned char*)(arg0 + 0x8))) {
      ((CToken*)arg0)->~CToken();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_39_4238(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_39_4260();
  }
}

extern "C" void fn_39_798(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_39_7C0();
  }
}

extern "C" int fn_39_3FC4(int arg0, int arg1) {
  void fn_39_4018(int, int);
  if (arg0) {
    fn_39_4018(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_4EC0(int arg0, int arg1) {
  void fn_39_4F18(int, int);
  if (arg0) {
    fn_39_4F18(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_64EC(int arg0, int arg1) {
  void fn_39_6544(int, int);
  if (arg0) {
    fn_39_6544(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_4018(int arg0, int arg1) {
  void fn_39_4074();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_39_4074();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_6E74(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if ((*(unsigned char*)arg0)) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_39_6E74*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_A80(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 800))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 772))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_39_659C(int obj, const int val) {
  if (obj) {
    unsigned int ptr = *(int*)(0xc + (char*)obj);
    if (ptr != 0) {
      unsigned char* ptr2 = (unsigned char*)obj;
      if ((*ptr2) >> 5 & 1) {
        CMemory::Free((const void*)ptr);
      } else {
        FreeLockedCache((void*)ptr);
      }
      unsigned char val2 = *ptr2;
      *ptr2 = __rlwimi(val2, (val2 >> 2 & 3) - 1, 2, 28, 29);
      if ((*ptr2) >> 2 & 3) {
        *ptr2 = __rlwimi(*ptr2, *ptr2, 1, 26, 26);
      }
    }
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

// ---- End of raw matching-decompiler output ----
