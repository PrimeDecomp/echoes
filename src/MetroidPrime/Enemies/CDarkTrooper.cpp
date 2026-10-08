#include "MetroidPrime/Enemies/CDarkTrooper.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDarkTrooper.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

#include "float.h"

static EMaterialTypes MeleeSolidMaterial = kMT_Solid;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"Alerted", static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::Alerted)},
    {"BreakMissileAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::BreakMissileAttack)},
    {"BreakSmallShotAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::BreakSmallShotAttack)},
    {"CanMeleeAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::CanMeleeAttack)},
    {"CanMissileAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::CanMissileAttack)},
    {"CanSmallShotAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::CanSmallShotAttack)},
    {"ClearLineOfFire",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::ClearLineOfFire)},
    {"DonePausing",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::DonePausing)},
    {"FacingPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::FacingPlayer)},
    {"InMeleeRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::InMeleeRange)},
    {"InMissileRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::InMissileRange)},
    {"InSmallShotRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::InSmallShotRange)},
    {"IsSleeper", static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::IsSleeper)},
    {"ReadyToRumble",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::ReadyToRumble)},
    {"ShouldPause",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CDarkTrooper::ShouldPause)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::Dead)},
    {"MeleeAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::MeleeAttack)},
    {"MissileAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::MissileAttack)},
    {"Null", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::Null)},
    {"Pause", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::Pause)},
    {"Pursue", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::Pursue)},
    {"Rise", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::Rise)},
    {"Sleep", static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::Sleep)},
    {"SmallShotAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CDarkTrooper::SmallShotAttack)},
};

static EMaterialTypes SolidMaterial = kMT_Solid;
static EMaterialTypes CharacterMaterial = kMT_Character;
static EMaterialTypes PlayerMaterial = kMT_Player;
static EMaterialTypes CollisionActorMaterial = kMT_CollisionActor;
static EMaterialTypes NoPlatformCollisionMaterial = kMT_ProjectilePassthrough;
static EMaterialTypes ExcludeFromLineOfSightMaterial = kMT_ExcludeFromLineOfSightTest;

static const char* const skBoneTrackingName = "Head_1";
static const char* const skGunLocatorName = "GUN_Particle_LCTR";
static const char* const skHeadLocatorName = "Head_1";
static const char* const skRootLocatorName = "Skeleton_Root";
static const char* const skBossName = "BossMissileTrooper";

static const float skRagDollParticleRadii[] = {0.23f, 0.23f, 0.15f, 0.15f, 0.23f, 0.15f, 0.15f,
                                               0.1f,  0.15f, 0.17f, 0.2f,  0.15f, 0.17f, 0.2f};

CDarkTrooper::CDarkTrooper(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& modelData,
                           const CPatternedInfo& patternedInfo, CAssetId stateMachine2,
                           float meleeMinRange, float meleeMaxRange, float attackCooldown,
                           float rangedMinRange, float rangedMaxRange, bool flotsam,
                           bool avoidDownFrames, int initialAnim, const CDamageInfo& meleeDamage,
                           CAssetId rangedProjectile, const CDamageInfo& rangedDamage,
                           bool firesMissiles, CAssetId missileProjectile,
                           const CDamageInfo& missileDamage, ushort ragdollImpactSound,
                           CAssetId scannableInfoWhenAttacking, const CActorParameters& actorParams)
: CPatterned(kPAI_DarkTrooper, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mPathFindSearch(nullptr, 1, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mElapsedTime(0.f)
, mPathDestination(CVector3f::Zero())
, mGunTransform(CTransform4f::Identity())
, mSavedBounds(CAABox::MakeNullBox())
, mAttackScanInfo(nullptr)
, mHeadDirection(CVector3f::Zero())
, mRootPosition(CVector3f::Zero())
, mMarkedRegionIndex(-1)
, mAlerted(false)
, mFiresMissiles(firesMissiles)
, mUpdatingAnimation(false)
, mAvoidDownFrames(avoidDownFrames)
, mGunSegId(AnimationData()->GetLocatorSegId(rstl::string_l(skGunLocatorName)))
, mHeadSegId(AnimationData()->GetLocatorSegId(rstl::string_l(skHeadLocatorName)))
, mRootSegId(AnimationData()->GetLocatorSegId(rstl::string_l(skRootLocatorName)))
, mBoneTracking(*AnimationData(), rstl::string_l(skBoneTrackingName), 0.5235988f, 3.1415927f,
                kBTF_NoParent)
, mLineOfSightTracker(GetUniqueId(), mHeadSegId, 0.2f, 0.05f)
, mPreviousAttackState(-1)
, mAttackState(-1)
, mStateMachine2(gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine2)))
, mRagDollHolder(flotsam, ragdollImpactSound)
, mMeleeAttack(meleeMinRange, meleeMaxRange, meleeDamage)
, mRangedAttackMinRange(rangedMinRange)
, mRangedAttackMaxRange(rangedMaxRange)
, mSmallShotAttack(attackCooldown, rangedProjectile, rangedDamage)
, mMissileAttack(attackCooldown, missileProjectile, missileDamage)
, mPauseEndTime(0.f)
, mShouldPause(false)
, mLocomotionType(pas::kLT_Relaxed)
, mIsSleeper(false)
, mTransitioning(false) {
  mDefaultTurnSpeed = mBodyController->GetTurnSpeed();

  const CPASDatabase& pasDatabase = AnimationData()->GetPASDatabase();
  const CPASAnimParmData sleeperParms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                      CPASAnimParm::FromEnum(7));
  if (pasDatabase.FindBestAnimation(sleeperParms, -1).second == initialAnim) {
    mIsSleeper = true;
    mLocomotionType = pas::kLT_Internal7;
  } else {
    const CPASAnimParmData otherSleeperParms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                             CPASAnimParm::FromEnum(9));
    if (pasDatabase.FindBestAnimation(otherSleeperParms, -1).second == initialAnim) {
      mIsSleeper = true;
      mLocomotionType = pas::kLT_Internal9;
    }
  }

  if (scannableInfoWhenAttacking != kInvalidAssetId) {
    mAttackScanInfo = rs_new TLockedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', scannableInfoWhenAttacking)));
  }

  if (flotsam) {
    SetDrawShadow(false);
    SetHighlightedInDarkVisor(false);
  }

  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(SolidMaterial, CharacterMaterial),
      CMaterialList(PlayerMaterial, CollisionActorMaterial, NoPlatformCollisionMaterial,
                    ExcludeFromLineOfSightMaterial));
  mLineOfSightTracker.SetRayFilter(filter);
  KnockBackController().SetHurlVelocityEnabled(false);
  KnockBackController().SetPhysicsImpulseMagnitude(0.8f);
}

CDarkTrooper::~CDarkTrooper() {}

CScannableObjectInfo* CDarkTrooper::GetScannableObjectInfo() const {
  if (mAlive && GetActive() && (!mTransitioning || mAttackState == 3) && mAttackScanInfo.get()) {
    return **mAttackScanInfo;
  }
  return CPatterned::GetScannableObjectInfo();
}

CProjectileInfo* CDarkTrooper::ProjectileInfo() {
  switch (mAttackState) {
  case 5:
    return &mSmallShotAttack.mProjectile.data();
  case 1:
    return &mMissileAttack.mProjectile.data();
  default:
    return nullptr;
  }
}

void CDarkTrooper::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  bool handled = false;
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (mFiresMissiles) {
      mgr.SetBossParams(GetUniqueId(), GetHealthInfo()->GetInitialHP(),
                        gpStringTable->GetStringIndex(skBossName));
    }
    break;
  case kSM_Alert:
    if (mFiresMissiles) {
      mgr.SetBossParams(GetUniqueId(), GetHealthInfo()->GetInitialHP(),
                        gpStringTable->GetStringIndex(skBossName));
    }
    mAlerted = true;
    break;
  case kSM_Decrement:
    if (mRagDollHolder.mRagDoll.get()) {
      mRagDollHolder.mRagDoll->SetNoOverTimer(false);
      mRagDollHolder.mRagDoll->SetContinueSmallMovements(false);
    }
    handled = true;
    break;
  case kSM_Delete:
    UnmarkPathRegion(mgr);
    break;
  case kSM_AreaLoaded:
    if (mRagDollHolder.mFlotsam) {
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
      mAlive = false;
      HealthInfo()->SetHP(-1.f);
      CreateRagDoll(mgr);
    } else {
      if (mIsSleeper) {
        RemoveMaterial(kMT_Solid, kMT_GroundCollider, mgr);
        AddMaterial(kMT_ProjectilePassthrough, mgr);
      } else {
        AddMaterial(kMT_GroundCollider, mgr);
      }
      SetPathArea(mgr);
      mLineOfSightTracker.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    }
    break;
  case kSM_XXDG:
    mHitByPlayerProjectile = true;
    break;
  case kSM_Damage:
    if (mAlive && mTransitioning) {
      return;
    }
    mPauseEndTime = -1000.f;
    mHitByPlayerProjectile = true;
    if (CWeapon* weapon = TCastToPtr< CWeapon >(mgr.ObjectById(msg.GetSenderId()))) {
      mRagDollHolder.mLastDamage =
          weapon->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability());
    }
    break;
  case kSM_Create:
  case kSM_Deactivate:
  case kSM_AIUpdateDisabled:
  case kSM_Landed:
  case kSM_Launching:
  case kSM_Falling:
    break;
  }

  if (!handled) {
    CPatterned::AcceptScriptMsg(mgr, msg);
  }
}

const CDamageVulnerability* CDarkTrooper::GetDamageVulnerability() const {
  if (mAlive && mTransitioning) {
    return &CDamageVulnerability::ImmuneVulnerabilty();
  }
  return CPatterned::GetDamageVulnerability();
}

void CDarkTrooper::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mRagDollHolder.mFlotsam && mRagDollHolder.mRagDoll.get()) {
    const float power = info.GetDamageInfo().GetKnockBackPower(*GetDamageVulnerability(), 0.f);
    mRagDollHolder.mRagDoll->TorsoImpulse() += (20.f * power) * info.GetDirection();
  }

  if (!mRagDollHolder.mRagDoll.get()) {
    if (GetHealthInfo()->GetHP() <= 0.f) {
      const float magnitude = info.GetDirection().Magnitude();
      CVector3f direction = info.GetDirection();
      const float scatter = 0.2f * magnitude;
      direction.SetX(direction.GetX() + scatter * (mgr.Random()->Float() - 0.5f));
      direction.SetY(direction.GetY() + scatter * (mgr.Random()->Float() - 0.5f));
      const CKnockBackInfo scatteredInfo(magnitude * direction.AsNormalized(), info.GetSourceId(),
                                         info.GetOwnerId(), info.GetDamageInfo(), info.IsDirect());
      KnockBackController().SetPhysicsImpulseMagnitude(0.5f * mgr.Random()->Float() + 1.5f);
      CPatterned::KnockBack(mgr, scatteredInfo);
    } else {
      CPatterned::KnockBack(mgr, info);
    }
  }
}

void CDarkTrooper::PreRender(CStateManager& mgr) {
  if (mRagDollHolder.mRagDoll.get() && mRagDollHolder.mRagDoll->IsPrimed()) {
    mRagDollHolder.mRagDoll->PreRender(GetTranslation(), *ModelData());
  }
  CPatterned::PreRender(mgr);
  PreRenderBoneTracking(mgr);
  mGunTransform = GetLctrTransform(mGunSegId);
  if (mDamageCooldownTimer == 0.f) {
    const CTransform4f headTransform = GetLctrTransform(mHeadSegId);
    const CTransform4f rootTransform = GetLctrTransform(mRootSegId);
    mRootPosition = rootTransform.GetTranslation();
    mHeadDirection = headTransform.GetTranslation() - mRootPosition;
    if (mHeadDirection.CanBeNormalized()) {
      mHeadDirection.Normalize();
    }
  }
}

void CDarkTrooper::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

CVector3f CDarkTrooper::GetAimPosition(const CStateManager&, float dt) const {
  CVector3f offset = CVector3f::Zero();
  if (dt > 0.f) {
    offset = PredictMotion(dt).GetTranslation();
  }

  if (mLockOnTarget.val() != 0xff) {
    const CTransform4f locatorTransform =
        GetModelData()->GetAnimationData()->GetLocatorTransform(mLockOnTarget, nullptr);
    const CVector3f scale = GetModelData()->GetScale();
    offset +=
        GetTransform() * CVector3f::ByElementMultiply(scale, locatorTransform.GetTranslation());
  } else {
    offset += GetBoundingBox().GetCenterPoint();
  }
  return offset;
}

void CDarkTrooper::FireSmallShot(CStateManager& mgr) {
  if (mSmallShotAttack.mProjectile) {
    CVector3f target = GetTargetPosition(mgr);
    if (mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      target += CVector3f(0.f, 0.f, 2.5f);
    }
    target += CVector3f(mgr.Random()->Range(-0.3f, 0.3f), mgr.Random()->Range(-0.3f, 0.3f),
                        mgr.Random()->Range(-0.3f, 0.3f));
    CVector3f toTarget = mGunTransform.GetTranslation() - target;
    toTarget.SetZ(0.f);
    if (toTarget.CanBeNormalized()) {
      toTarget.Normalize();
      CVector3f forward = mGunTransform.GetColumn(kDY);
      forward.SetZ(0.f);
      if (forward.CanBeNormalized() && CVector3f::GetAngleDiff(forward, toTarget) < 0.7853982f) {
        LaunchProjectile(CTransform4f::LookAt(mGunTransform.GetTranslation(), target), mgr, 5,
                         kPA_None, false, CImpactVisorEffect::None(), CVector3f(1.f, 1.f, 1.f));
      }
    }
    if (!FacingPlayer(mgr, CTriggerData(0.f)) ||
        !InRange(mgr, mRangedAttackMinRange, mRangedAttackMaxRange)) {
      EndSmallShotAttack();
    }
  }
}

void CDarkTrooper::FireMissile(CStateManager& mgr) {
  if (mMissileAttack.mProjectile) {
    CVector3f target = GetTargetPosition(mgr);
    if (mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      target += CVector3f(0.f, 0.f, 2.5f);
    }
    target += CVector3f(mgr.Random()->Range(-0.3f, 0.3f), mgr.Random()->Range(-0.3f, 0.3f),
                        mgr.Random()->Range(-0.3f, 0.3f));
    CVector3f toTarget = mGunTransform.GetTranslation() - target;
    toTarget.SetZ(0.f);
    if (toTarget.CanBeNormalized()) {
      toTarget.Normalize();
      CVector3f forward = mGunTransform.GetColumn(kDY);
      forward.SetZ(0.f);
      if (forward.CanBeNormalized() && CVector3f::GetAngleDiff(forward, toTarget) < 0.7853982f &&
          mLineOfSightTracker.HasLineOfSight()) {
        LaunchProjectile(CTransform4f::LookAt(mGunTransform.GetTranslation(), target), mgr, 5,
                         kPA_None, false, CImpactVisorEffect::None(), CVector3f(1.f, 1.f, 1.f));
      }
    }
    if (!FacingPlayer(mgr, CTriggerData(0.f)) ||
        !InRange(mgr, mRangedAttackMinRange, mRangedAttackMaxRange)) {
      EndSmallShotAttack();
    }
  }
}

void CDarkTrooper::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = true;
  switch (type) {
  case kUE_Projectile:
    switch (mAttackState) {
    case 0:
      ApplyMeleeDamage(mgr);
      break;
    case 5:
      FireSmallShot(mgr);
      break;
    case 1:
      FireMissile(mgr);
      break;
    }
    break;
  case kUE_EggLay:
    if (mAttackState == 2) {
      mShouldPause = mgr.Random()->Range(0.f, 1.f) < 0.2f;
    }
    break;
  case kUE_DeGenerate:
  case kUE_BecomeRagDoll:
    if (!mFiresMissiles) {
      CreateRagDoll(mgr);
    }
    break;
  case kUE_Unknown37:
    mUpdatingAnimation = true;
    break;
  case kUE_Unknown38:
    mUpdatingAnimation = false;
    break;
  case kUE_EndAction:
    switch (mAttackState) {
    case 5:
      ++mSmallShotAttack.mShotsFired;
      if (!(mSmallShotAttack.mShotsFired < mSmallShotAttack.mShotLimit &&
            mLineOfSightTracker.HasLineOfSight())) {
        EndSmallShotAttack();
      }
      break;
    case 1:
      ++mMissileAttack.mShotsFired;
      if (!(mMissileAttack.mShotsFired < mMissileAttack.mShotLimit &&
            mLineOfSightTracker.HasLineOfSight())) {
        EndMissileAttack();
      }
      break;
    }
    break;
  case kUE_Landing:
    switch (mAttackState) {
    case 5:
      mSmallShotAttack.mBreakAttack = true;
      break;
    case 1:
      mMissileAttack.mBreakAttack = true;
      break;
    }
    break;
  case kUE_BreakLockOn:
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
    handled = false;
    break;
  case kUE_Activate:
    if (mAttackState >= 3 && mAttackState < 5) {
      mTransitioning = false;
    }
    break;
  default:
    handled = false;
    break;
  }

  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CDarkTrooper::EndSmallShotAttack() {
  mSmallShotAttack.mFinished = true;
  BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
  BodyController()->CommandMgr().DeliverAdditiveTargetVector(CVector3f::Zero());
}

void CDarkTrooper::EndMissileAttack() {
  mMissileAttack.mFinished = true;
  BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
  BodyController()->CommandMgr().DeliverAdditiveTargetVector(CVector3f::Zero());
}

void CDarkTrooper::ApplyMeleeDamage(CStateManager& mgr) {
  if (InRange(mgr, 0.f, mMeleeAttack.mMaxRange)) {
    mgr.ApplyDamage(
        GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), mMeleeAttack.mDamage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(MeleeSolidMaterial), CMaterialList()),
        CVector3f::Zero());
    PushPlayer(mgr, 12.f, 8.f);
  }
}

void CDarkTrooper::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  mStateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CDarkTrooper::MoveToTarget(CStateManager& mgr, float dt, const CVector3f& target) {
  mPathFindSearch.SetAvoidanceFilter(2);
  UnmarkPathRegion(mgr);
  mPathDestination = target;
  mPathFindNavigation.SetDestination(target);
  mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
  MarkPathRegion(mgr);
}

void CDarkTrooper::SetPathArea(CStateManager& mgr) { mPathFindSearch.SetArea(GetPathArea(mgr)); }

void CDarkTrooper::PreThink(float dt, CStateManager& mgr) {
  if (!mRagDollHolder.mRagDoll.get() || !mRagDollHolder.mRagDoll->IsPrimed()) {
    mBoneTracking.PreThink(*AnimationData());
  }
  CPatterned::PreThink(dt, mgr);
}

void CDarkTrooper::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (!mStateMachine->HasState()) {
    SetupStateMachineHelper(mgr);
    return;
  }

  if (!BodyController()->GetIsActive()) {
    if (mIsSleeper) {
      BodyController()->SetLocomotionType(mLocomotionType);
    }
    BodyController()->Activate(mgr, pas::kAS_Invalid);
  }

  if (mTransitioning && mAttackState == 4 && !mAlerted) {
    CActor::Think(dt, mgr);
    return;
  }

  if (mRagDollHolder.mRagDoll.get()) {
    if (!mRagDollHolder.mRagDoll->IsPrimed()) {
      if (!mRagDollHolder.mFlotsam) {
        CPatterned::Think(dt, mgr);
      } else {
        CActor::Think(dt, mgr);
      }
      mRagDollHolder.mRagDoll->Prime(mgr, GetTransform(), *ModelData());
      const CVector3f translation = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(translation);
      BodyController()->SetPlaybackRate(0.f);
    } else {
      CActor::Think(dt, mgr);
      ThinkRagDoll(dt, mgr);
      if (mFadeToDeath) {
        UpdateAlphaDelta(mgr, dt);
      }
    }
  } else {
    if (mAlive) {
      mElapsedTime += dt;
    }
    CPatterned::Think(dt, mgr);
    if (mAlive) {
      ThinkBoneTracking(dt, mgr);
    }
  }
}

void CDarkTrooper::Sleep(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
    mTransitioning = true;
    if (mLocomotionType != pas::kLT_Relaxed) {
      BodyController()->SetLocomotionType(mLocomotionType);
    } else {
      BodyController()->SetLocomotionType(
          mgr.Random()->Range(0.f, 1.f) < 0.5f ? pas::kLT_Internal7 : pas::kLT_Internal9);
    }
    KnockBackController().EnableAllAnimReactions(false);
  } else if (msg == kStateMsg_Deactivate) {
    KnockBackController().EnableAllAnimReactions(true);
    AddMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
  }

  SetAttackState(4, msg);

  if (msg != kStateMsg_Update) {
    CAABox box = CAABox::MakeNullBox();
    if (msg == kStateMsg_Activate) {
      box = GetBaseBoundingBox();
    } else {
      box = mSavedBounds;
    }

    const CVector3f offset = GetPrimitiveOffset();
    CAABox newBox = CAABox::MakeNullBox();
    float maxZ = box.GetMaxPoint().GetZ();
    if (msg == kStateMsg_Activate) {
      maxZ = 1.f + box.GetMinPoint().GetZ();
      mSavedBounds = GetBaseBoundingBox();
    }
    newBox = CAABox(box.GetMinPoint() + offset,
                    CVector3f(box.GetMaxPoint().GetX() + offset.GetX(),
                              box.GetMaxPoint().GetY() + offset.GetY(), maxZ + offset.GetZ()));
    SetBoundingBox(newBox);

    CMaterialList materials = GetMaterialList();
    materials.Add(kMT_Solid);
    materials.Add(kMT_Character);
    materials.Remove(kMT_ProjectilePassthrough);
    SetCollisionPrimitive(CCollidableAABox(newBox, materials));
    SetTransformDirty();
    mgr.UpdateActorInSortedLists(this);
  }
}

void CDarkTrooper::Rise(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mTransitioning = true;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    SetIngPossessed(true, 2.5f, mgr);
    SendScriptMsgs(kSS_Attack, mgr);
    AddMaterial(kMT_Solid, kMT_GroundCollider, mgr);
    RemoveMaterial(kMT_ProjectilePassthrough, mgr);
  }
  SetAttackState(3, msg);
}

void CDarkTrooper::Null(CStateManager& mgr, EStateMsg msg, float dt) {}

void CDarkTrooper::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);
  if (mRagDollHolder.mRagDoll.get() && mRagDollHolder.mRagDoll->IsPrimed()) {
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(&actor)) {
      if (trigger->GetActive() && (trigger->GetTriggerFlags() & kTFL_DetectAI) &&
          trigger->GetForceMagnitude() > 0.f) {
        mRagDollHolder.mRagDoll->TorsoImpulse() += trigger->GetForceField();
      }
    }
  }
}

void CDarkTrooper::PreRenderAllViewports(CStateManager& mgr) {
  if (mRagDollHolder.mRagDoll.get() && mRagDollHolder.mRagDoll->IsPrimed()) {
    mRagDollHolder.mRagDoll->PreRenderAllViewports(*this, 0.2f);
    UpdatePortalSystemState(mgr);
  } else {
    CPatterned::PreRenderAllViewports(mgr);
  }
}

void CDarkTrooper::ThinkRagDoll(float dt, CStateManager& mgr) {
  if (mRagDollHolder.mRagDoll.get()) {
    float waterTop = -FLT_MAX;
    if (InFluidId() != kInvalidUniqueId) {
      CScriptWater* water = TCastToPtr< CScriptWater >(mgr.ObjectById(InFluidId()));
      if (water && water->GetActive()) {
        waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
      }
    }
    mRagDollHolder.mRagDoll->Update(mgr, dt * GetDeathTimeScale(), waterTop);
    ModelData()->AdvanceParticles(GetTransform(), dt, mgr);

    if (mRagDollHolder.mRagDoll->IsOver()) {
      if (!mRagDollHolder.mRagDoll->WillContinueSmallMovements()) {
        SetVelocityWR(CVector3f::Zero());
        Stop();
        if (!mAvoidDownFrames && !mFadeToDeath) {
          mFadeToDeath = true;
          mAlphaDelta = -0.33333334f;
        }
      }
    } else if (mUpdatingAnimation) {
      UpdateAnimation(0.f, mgr, mBodyController->GetPercentageFrozen() != 1.f);
    }
  }
}

void CDarkTrooper::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    UnmarkPathRegion(mgr);
    if (GetScannableObjectInfo()) {
      AddMaterial(kMT_Scannable, mgr);
    }
  }
  RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  if (mAvoidDownFrames) {
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Die));
    mStateControlledMassiveDeath = false;
  } else {
    CPatterned::Dead(mgr, msg, dt);
  }
}

void CDarkTrooper::CreateRagDoll(CStateManager& mgr) {
  if (!mRagDollHolder.mRagDoll.get()) {
    UpdateHitDamageTime(1000.f);
    const rstl::reserved_vector< float, 14 > radii(
        skRagDollParticleRadii, skRagDollParticleRadii + ARRAY_SIZE(skRagDollParticleRadii));
    const uint flags = mRagDollHolder.mFlotsam ? 3 : 0;
    mRagDollHolder.mRagDoll =
        rs_new CPirateRagDoll(mgr, this, mRagDollHolder.mImpactSound, flags, 75.f, -3.f, radii);
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    RemoveMaterial(kMT_GroundCollider, kMT_Solid, kMT_AIBlock, mgr);
  }
}

CVector3f CDarkTrooper::GetTargetPosition(CStateManager& mgr) const {
  return mgr.GetPlayer(0)->GetTranslation();
}

void CDarkTrooper::SetAttackState(int state, EStateMsg msg) {
  if (msg == kStateMsg_Deactivate) {
    if (mAttackHistory.size() == 3) {
      mAttackHistory.erase(mAttackHistory.begin());
    }
    mAttackHistory.push_back(mAttackState);
    mAttackState = -1;
  } else {
    mAttackState = state;
  }
}

void CDarkTrooper::MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(0, msg);
  if (msg == kStateMsg_Activate) {
    mMeleeAttack.mNextAttackTime = mgr.Random()->Range(0.8f, 2.f);
  } else if (msg == kStateMsg_Deactivate) {
    mMeleeAttack.mNextAttackTime += mElapsedTime;
  }
  RotateToPoint(GetTargetPosition(mgr), dt, 1.5707964f);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_One));
}

void CDarkTrooper::Pursue(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(2, msg);
  const CVector3f target = GetTargetPosition(mgr);
  if (msg == kStateMsg_Activate) {
    mShouldPause = false;
    MoveToTarget(mgr, dt, target);
  } else if (msg == kStateMsg_Update) {
    mLineOfSightTracker.Update(dt, mgr);
    mSmallShotAttack.mCooldown -= dt;
    mMissileAttack.mCooldown -= dt;

    float turnSpeed = mDefaultTurnSpeed;
    if (InRange(mgr, 0.f, 6.f)) {
      turnSpeed *= 3.f;
    } else if (InRange(mgr, 0.f, 9.f)) {
      turnSpeed *= 1.5f;
    }
    BodyController()->SetTurnSpeed(turnSpeed);

    if (InRange(mgr, 0.f, 3.f)) {
      BodyController()->CommandMgr().ClearLocomotionCmds();
      RotateToPoint(GetTargetPosition(mgr), dt, 0.87266463f);
    } else {
      if ((target - mPathDestination).MagSquared() > 16.f) {
        MoveToTarget(mgr, dt, target);
      }
      if (!PathShagged(mgr, CTriggerData(0.f)) && !mPathFindSearch.IsOver()) {
        mPathFindNavigation.PathFind(mgr, msg, dt, *this);
        BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
            BodyController()->CommandMgr().GetMoveVector(), CVector3f::Zero(), 1.f));
      } else {
        BodyController()->CommandMgr().ClearLocomotionCmds();
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    BodyController()->SetTurnSpeed(mDefaultTurnSpeed);
  }
}

bool CDarkTrooper::InMeleeRange(CStateManager& mgr, const CTriggerData&) const {
  return InRange(mgr, mMeleeAttack.mMinRange, mMeleeAttack.mMaxRange);
}

bool CDarkTrooper::InSmallShotRange(CStateManager& mgr, const CTriggerData&) const {
  return InRange(mgr, mRangedAttackMinRange, mRangedAttackMaxRange);
}

bool CDarkTrooper::InMissileRange(CStateManager& mgr, const CTriggerData&) const {
  return InRange(mgr, mRangedAttackMinRange, mRangedAttackMaxRange);
}

void CDarkTrooper::UnmarkPathRegion(CStateManager& mgr) {
  if (mMarkedRegionIndex != -1) {
    if (CPFArea* area = GetPathArea(mgr)) {
      if (CPFRegion* region = area->GetRegionPtr(mMarkedRegionIndex)) {
        region->Data()->SetAvoidanceFlags(region->Data()->GetAvoidanceFlags() & ~2);
      }
    }
    mMarkedRegionIndex = -1;
  }
}

CPFArea* CDarkTrooper::GetPathArea(CStateManager& mgr) const {
  return mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea;
}

void CDarkTrooper::MarkPathRegion(CStateManager& mgr) {
  const rstl::reserved_vector< CVector3f, 16 >& waypoints = mPathFindSearch.GetWaypoints();
  if (waypoints.size() != 0) {
    CPFArea* area = GetPathArea(mgr);
    const CPFRegion* lastRegion =
        area->FindClosestRegion(waypoints[waypoints.size() - 1], GetSearchPath()->GetRegionFlags(),
                                GetSearchPath()->GetCreatureMask(), 2.f);
    for (int i = waypoints.size() - 1; i > 0; --i) {
      const CVector3f midpoint = 0.5f * (waypoints[i] + waypoints[i - 1]);
      CPFRegion* region = area->FindClosestRegion(midpoint, GetSearchPath()->GetRegionFlags(),
                                                  GetSearchPath()->GetCreatureMask(), 2.f);
      if (region && region != lastRegion && region->Data()->GetAvoidanceFlags() == 0) {
        region->Data()->SetAvoidanceFlags(region->Data()->GetAvoidanceFlags() | 2);
        mMarkedRegionIndex = region->GetIndex();
        break;
      }
    }
  }
}

bool CDarkTrooper::FacingPlayer(CStateManager& mgr, const CTriggerData&) const {
  const CVector3f toTarget = GetTargetPosition(mgr) - GetTranslation();
  const CVector3f forward = GetTransform().GetColumn(kDY);
  return CVector2f::GetAngleDiff(CVector2f(forward.GetX(), forward.GetY()),
                                 CVector2f(toTarget.GetX(), toTarget.GetY())) < 0.6981317f;
}

bool CDarkTrooper::InRange(CStateManager& mgr, float minRange, float maxRange) const {
  const float distanceSquared = (GetTargetPosition(mgr) - GetTranslation()).MagSquared();
  return distanceSquared > minRange * minRange && distanceSquared < maxRange * maxRange;
}

void CDarkTrooper::SmallShotAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(5, msg);
  const CVector3f target = GetTargetPosition(mgr);
  if (msg == kStateMsg_Activate) {
    mSmallShotAttack.Reset();
    mSmallShotAttack.mShotLimit = mgr.Random()->Range(1, 2);
    mSmallShotAttack.mNextAttackTime = mgr.Random()->Range(2.f, 4.f);
  } else if (msg == kStateMsg_Update) {
    mLineOfSightTracker.Update(dt, mgr);
    BodyController()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
    if (mSmallShotAttack.mFinished) {
      EndSmallShotAttack();
    } else {
      BodyController()->CommandMgr().DeliverAdditiveTargetVector(
          GetTransform().TransposeRotate(target - GetTranslation()));
    }
  } else if (msg == kStateMsg_Deactivate) {
    mSmallShotAttack.mNextAttackTime += mElapsedTime;
    BodyController()->CommandMgr().DeliverAdditiveTargetVector(CVector3f::Zero());
  }
  RotateToPoint(target, dt, 0.87266463f);
  DeliverCommand(msg, pas::kAS_LoopAttack, CBCLoopAttackCmd(pas::kLAT_Zero));
}

void CDarkTrooper::MissileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(1, msg);
  const CVector3f target = GetTargetPosition(mgr);
  if (msg == kStateMsg_Activate) {
    mMissileAttack.Reset();
    mMissileAttack.mShotLimit = mgr.Random()->Range(2, 3);
    mMissileAttack.mNextAttackTime = mgr.Random()->Range(2.f, 4.f);
  } else if (msg == kStateMsg_Update) {
    mLineOfSightTracker.Update(dt, mgr);
    BodyController()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
    if (mMissileAttack.mFinished) {
      EndMissileAttack();
    } else {
      BodyController()->CommandMgr().DeliverAdditiveTargetVector(
          GetTransform().TransposeRotate(target - GetTranslation()));
    }
  } else if (msg == kStateMsg_Deactivate) {
    mMissileAttack.mNextAttackTime += mElapsedTime;
    BodyController()->CommandMgr().DeliverAdditiveTargetVector(CVector3f::Zero());
  }
  RotateToPoint(target, dt, 1.3962634f);
  DeliverCommand(msg, pas::kAS_LoopAttack, CBCLoopAttackCmd(pas::kLAT_One));
}

bool CDarkTrooper::BreakSmallShotAttack(CStateManager&, const CTriggerData&) const {
  return mSmallShotAttack.mBreakAttack;
}

bool CDarkTrooper::BreakMissileAttack(CStateManager&, const CTriggerData&) const {
  return mMissileAttack.mBreakAttack;
}

bool CDarkTrooper::ClearLineOfFire(CStateManager&, const CTriggerData&) const {
  return mLineOfSightTracker.HasLineOfSight();
}

bool CDarkTrooper::ShouldPause(CStateManager& mgr, const CTriggerData&) const {
  if (mFiresMissiles) {
    return false;
  }
  if (InRange(mgr, 0.f, 9.f)) {
    return false;
  }
  return mShouldPause;
}

void CDarkTrooper::Pause(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    BodyController()->CommandMgr().ClearLocomotionCmds();
    mPauseEndTime = mElapsedTime + mgr.Random()->Range(2.5f, 4.f);
  } else if (msg == kStateMsg_Update) {
    mSmallShotAttack.mCooldown -= dt;
    mMissileAttack.mCooldown -= dt;
  }
}

bool CDarkTrooper::DonePausing(CStateManager&, const CTriggerData&) const {
  return mPauseEndTime < mElapsedTime;
}

bool CDarkTrooper::ReadyToRumble(CStateManager&, const CTriggerData&) const {
  return !mTransitioning;
}

bool CDarkTrooper::IsSleeper(CStateManager&, const CTriggerData&) const { return mIsSleeper; }

bool CDarkTrooper::Alerted(CStateManager&, const CTriggerData&) const { return mAlerted; }

bool CDarkTrooper::CanMeleeAttack(CStateManager&, const CTriggerData&) const {
  return mMeleeAttack.mNextAttackTime < mElapsedTime;
}

bool CDarkTrooper::CanSmallShotAttack(CStateManager&, const CTriggerData&) const {
  return !mFiresMissiles && mSmallShotAttack.mCooldown <= 0.f &&
         mSmallShotAttack.mNextAttackTime < mElapsedTime;
}

bool CDarkTrooper::CanMissileAttack(CStateManager&, const CTriggerData&) const {
  return mFiresMissiles && mMissileAttack.mCooldown <= 0.f &&
         mMissileAttack.mNextAttackTime < mElapsedTime;
}

void CDarkTrooper::PushPlayer(CStateManager& mgr, float impulseScale, float verticalSpeed) {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f direction = player->GetTranslation() - GetTranslation();
  direction.SetZ(0.f);
  direction.Normalize();
  direction *= impulseScale;
  direction.SetZ(verticalSpeed);
  player->Stop();
  player->ApplyImpulseWR(player->GetMass() * direction, CAxisAngle::Identity());
  player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  mgr.RumbleManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
      ->Rumble(mgr, kRFX_CameraShake, 1.f, kRP_Two);
}

bool CDarkTrooper::ShouldTrack() const { return !mTransitioning && mAlive; }

void CDarkTrooper::ThinkBoneTracking(float dt, CStateManager& mgr) {
  if (!ShouldTrack()) {
    mBoneTracking.SetActive(false);
  } else {
    mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    mBoneTracking.SetActive(true);
    mBoneTracking.Think(dt);
  }
}

void CDarkTrooper::PreRenderBoneTracking(CStateManager& mgr) {
  if (ShouldTrack()) {
    mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->GetScale(),
                            *mBodyController);
  }
}

CDamageInfo CDarkTrooper::GetContactDamage() const {
  if (mTransitioning || !mAlive) {
    CDamageInfo damage = CPatterned::GetContactDamage();
    damage.SetDamage(0.f);
    damage.SetKnockBackPower(0.f);
    return damage;
  }
  return CPatterned::GetContactDamage();
}

CVector3f CDarkTrooper::GetIngSnatchingNormal(float) const {
  CVector3f normal = mHeadDirection * -1.f;
  normal.SetZ(0.f);
  if (normal.CanBeNormalized()) {
    normal.Normalize();
  }
  return normal;
}

CVector3f CDarkTrooper::GetIngSnatchingPoint(float t) const {
  const rstl::optional_object< CAABox > bounds = GetTouchBounds();
  const CVector3f center = bounds->GetCenterPoint();
  const CVector3f extent(bounds->GetMaxPoint().GetX() - center.GetX(),
                         bounds->GetMaxPoint().GetY() - center.GetY(), 0.f);
  const float radius = extent.Magnitude();
  const float t3 = t * t * t;
  return (mRootPosition - mHeadDirection * radius) * (1.f - t3) +
         (mRootPosition + mHeadDirection * radius) * t3;
}

bool CDarkTrooper::CanBeUnPossessed(CStateManager&) const { return false; }

rstl::optional_object< CAABox > CDarkTrooper::GetTouchBounds() const {
  if (mRagDollHolder.mRagDoll.get() && mRagDollHolder.mRagDoll->IsPrimed() &&
      mRagDollHolder.mRagDoll->IsRenderBoundsValid()) {
    return mRagDollHolder.mRagDoll->GetCachedRenderBounds();
  }
  return CPatterned::GetTouchBounds();
}

CRagDoll* CDarkTrooper::GetRagDoll() const {
  if (mRagDollHolder.mRagDoll.get() && mRagDollHolder.mRagDoll->IsPrimed() &&
      mRagDollHolder.mRagDoll->IsRenderBoundsValid()) {
    return mRagDollHolder.mRagDoll.get();
  }
  return nullptr;
}

CEntity* REL_LoadDarkTrooper(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDarkTrooper sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDarkTrooper.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CDarkTrooper(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
      sldrThis.patterned.stateMachine2, sldrThis.meleeAttackMinRange, sldrThis.meleeAttackMaxRange,
      sldrThis.unknown_0x2dca199d, sldrThis.rangedAttackMinRange, sldrThis.rangedAttackMaxRange,
      sldrThis.flotsam, sldrThis.avoidDownFrames,
      sldrThis.patterned.animationInformation.initial_anim,
      LdrToDamageInfo(sldrThis.meleeAttackDamage), sldrThis.rangedAttackProjectile,
      LdrToDamageInfo(sldrThis.rangedAttackDamage), sldrThis.firesMissiles,
      sldrThis.missileProjectile, LdrToDamageInfo(sldrThis.missileDamage),
      sldrThis.ragdollImpactSound, sldrThis.scannableInfoWhenAttacking,
      LdrToActorParameters(sldrThis.actorInformation));
}

static void SetFuncPtrs() {
  static SDarkTrooper_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadDarkTrooper;
  SetSDarkTrooper_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSDarkTrooper_FuncPtrs(nullptr); }

template < typename T >
void CDarkTrooper::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBodyController->CommandMgr().DeliverCmd(cmd);
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, state)) {
      mBodyController->CommandMgr().DeliverCmd(cmd);
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
  }
}
