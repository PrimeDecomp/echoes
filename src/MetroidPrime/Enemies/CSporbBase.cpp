#include "MetroidPrime/Enemies/CSporbBase.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CSporbNeedle.hpp"
#include "MetroidPrime/Enemies/CSporbPowerBomb.hpp"
#include "MetroidPrime/Enemies/CSporbProjectile.hpp"
#include "MetroidPrime/Enemies/CSporbTop.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSporbBase.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"
#include "Weapons/CProjectileWeapon.hpp"
#include "rstl/math.hpp"

#include <float.h>

static EMaterialTypes skDamageSolid = kMT_Solid; // Guessed name

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::ShouldPatrol)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::ShouldAttack)},
    {"ShouldFire", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::ShouldFire)},
    {"ShouldSpit", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::ShouldSpit)},
    {"ShouldFlail", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::ShouldFlail)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::AnimOver)},
    {"AttackOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::AttackOver)},
    {"SpitOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::SpitOver)},
    {"AttackExitOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbBase::AttackExitOver)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Sleep", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Sleep)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Patrol)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Attack)},
    {"WakeUp", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::WakeUp)},
    {"GoToSleep", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::GoToSleep)},
    {"Fire", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Fire)},
    {"ContinueFire", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::ContinueFire)},
    {"AttackExit", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::AttackExit)},
    {"FakeDeath", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::FakeDeath)},
    {"FakeDead", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::FakeDead)},
    {"Flail", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Flail)},
    {"ContinueFlail",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::ContinueFlail)},
    {"Spit", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Spit)},
    {"ContinueSpit", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::ContinueSpit)},
    {"SpitExit", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::SpitExit)},
    {"Flinch", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbBase::Flinch)},
};

static EMaterialTypes skRayInclude = kMT_Solid;                            // Guessed name
static EMaterialTypes skRayExcludePassthrough = kMT_ProjectilePassthrough; // Guessed name
static const EMaterialTypes skRayExcludeCharacter = kMT_Character;         // Guessed name

// Guessed name; the Sporb currently holding the player, shared by every Sporb.
static TUniqueId sGrabbingSporb = kInvalidUniqueId;

const char* const CSporbBase::skConnectLocator = "Skeleton_Root_2_SDK";

// Guessed class: the tendril particle system, which carries its own bounds.
class CTendrilGen : public CElementGen {
public:
  CTendrilGen(const TToken< CGenDescription >& description)
  : CElementGen(description, kMOT_Normal, kOSF_One), mBounds(CAABox::MakeMaxInvertedBox()) {}

  rstl::optional_object< CAABox > GetBounds() override { return mBounds; }

  CAABox mBounds; // Guessed name
};

rstl::optional_object< CAABox > CSporbBase::GetTouchBounds() const { return GetBoundingBox(); }

CSporbBase::~CSporbBase() {}

CSporbBase::CSporbBase(
    const TUniqueId& uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& modelData, const CPatternedInfo& patternedInfo,
    const CActorParameters& actorParams, float minTimeBetweenAttacks, float maxTimeBetweenAttacks,
    float minTimeBetweenShots, float maxTimeBetweenShots, float shotAngleVariance,
    float grabberOutAcceleration, float grabberInAcceleration, float initialGrabberOutSpeed,
    uchar minShots, uchar maxShots, const CVector3f& attackAimOffset, float initialGrabberInSpeed,
    float grabberAttachTime, float minGrabberGrabTime, float maxGrabberGrabTime, float spitForce,
    CAssetId tendrilParticleEffect, ushort fireSound, ushort flightSound, ushort hitPlayerSound,
    ushort hitWorldSound, ushort retractSound, ushort retractMissedPlayerSound,
    ushort morphballSpitSound, ushort explosionSound, ushort ballEscapeSound,
    ushort needleTelegraphSound, ushort grabberTelegraphSound, float spitDamage, float grabDamage,
    float flailDamage, float maxGrabberGrabRange, float minGrabberGrabRange,
    bool isPowerBombGuardian, CAssetId powerBombProjectileParticleEffect,
    const CDamageInfo& powerBombProjectileDamage, float maxPowerBombProjectileHeight,
    float powerBombProjectileFuseTime, ushort powerBombProjectileSound,
    float powerBombEmitterMaxDistance, float powerBombEmitterDistanceComp, float startDamageTime,
    float endDamageTime,
    const rstl::vector< rstl::ownership_transfer< CPowerBombGuardianStageData > >& stages,
    float maxTimerScale, float aimPredictionTimeScale, float aimSpreadRadius, float damageWaitTime)
: CPatterned(kPAI_SporbBase, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_Floater, actorParams)
, mTopId(kInvalidUniqueId)
, mProjectileId(kInvalidUniqueId)
, mForwardLeanAnim(0)
, mLeftLeanAnim(0)
, mRightLeanAnim(0)
, mBackLeanAnim(0)
, mFlinchAnim(0)
, mDetectionDistance(patternedInfo.GetDetectionRange())
, mMaxAttackDistance(patternedInfo.GetMaxAttackRange())
, mMinAttackDistance(patternedInfo.GetMinAttackRange())
, x7e8_(1.f)
, mForwardLean(0.f)
, mBackLean(0.f)
, mLeftLean(0.f)
, mRightLean(0.f)
, x7fc_(0.1f)
, mTargetPlayerId(kInvalidUniqueId)
, mSpitDamage(CWeaponMode(kWT_AI), spitDamage, 0.f, 0.f, false, false)
, mGrabDamage(CWeaponMode(kWT_AI), grabDamage, 0.f, 0.f, false, false)
, mFlailDamage(CWeaponMode(kWT_AI), 0.f, 0.f, 0.f, true)
, mShotTimer(minTimeBetweenShots)
, x85c_(0.f)
, x860_(0.f)
, mTimeBetweenAttacks(minTimeBetweenAttacks)
, mTimeBetweenShots(minTimeBetweenShots)
, mShotsInBurst(minShots)
, mMinTimeBetweenAttacks(minTimeBetweenAttacks)
, mMaxTimeBetweenAttacks(maxTimeBetweenAttacks)
, mMinTimeBetweenShots(minTimeBetweenShots)
, mMaxTimeBetweenShots(maxTimeBetweenShots)
, x880_(0.f)
, mMinShots(minShots)
, mMaxShots(maxShots)
, mShotsFired(0)
, mShooting(false)
, mReadyToFire(true)
, x889_(false)
, x88a_(false)
, x88b_(false)
, x88c_(false)
, mGrabFinished(false)
, mBallEscaped(false)
, x890_(CSfxManager::kInternalInvalidSfxId)
, mNeedleSound(CSfxManager::kInternalInvalidSfxId)
, mTimeSinceDetection(mPlayerLeashTime)
, mNeedleSpawnerId(kInvalidEditorId)
, mShotAngleVariance(shotAngleVariance)
, mAttackAimOffset(attackAimOffset)
, mTendrilParticleCount(0)
, mGrabberOutSpeed(initialGrabberOutSpeed)
, mGrabberInSpeed(initialGrabberInSpeed)
, mGrabberOutAcceleration(grabberOutAcceleration)
, mGrabberInAcceleration(grabberInAcceleration)
, mRetractPosition(CVector3f::Zero())
, mHoldTimer(0.f)
, mHoldDuration(0.05f)
, mAttachTimer(0.f)
, mAttachDuration(grabberAttachTime)
, mInitialGrabberOutSpeed(initialGrabberOutSpeed)
, mInitialGrabberInSpeed(initialGrabberInSpeed)
, mMinGrabTime(minGrabberGrabTime)
, mMaxGrabTime(maxGrabberGrabTime)
, mGrabTime(minGrabberGrabTime)
, mSpitTimer(0.f)
, mSpitForce(spitForce)
, x8f8_(false)
, mReleaseTimer(0.f)
, mMaxGrabRange(maxGrabberGrabRange)
, mMinGrabRange(minGrabberGrabRange)
, mCanSpit(false)
, mAlerted(true)
, mAttackType(kAT_Invalid)
, mGrabberState(kGS_Idle)
, mAimTarget(CVector3f::Zero())
, mProjectileRotation(CQuaternion::NoRotation())
, mTendrilDesc(tendrilParticleEffect == kInvalidAssetId
                   ? nullptr
                   : rs_new TCachedToken< CGenDescription >(
                         gpSimplePool->GetObj(SObjectTag('PART', tendrilParticleEffect)), true))
, mTendrilGen(mTendrilDesc.get() ? rs_new CTendrilGen(*mTendrilDesc.get()) : nullptr)
, mFireSound(fireSound)
, mFlightSound(flightSound)
, mHitPlayerSound(hitPlayerSound)
, mHitWorldSound(hitWorldSound)
, mRetractSound(retractSound)
, mRetractMissedPlayerSound(retractMissedPlayerSound)
, mMorphballSpitSound(morphballSpitSound)
, mExplosionSound(explosionSound)
, mBallEscapeSound(ballEscapeSound)
, mNeedleTelegraphSound(needleTelegraphSound)
, mGrabberTelegraphSound(grabberTelegraphSound)
, mFiring(false)
, mTopHit(false)
, mTopAttachPosition(CVector3f::Zero())
, mTopBlend(1.f)
, mProjectilePosition(CVector3f::Zero())
, mSavedForwardLean(0.f)
, mSavedBackLean(0.f)
, mSavedLeftLean(0.f)
, mSavedRightLean(0.f)
, mSavedVulnerability(CDamageVulnerability::ImmuneVulnerabilty())
, mIsPowerBombGuardian(isPowerBombGuardian)
, mProjectileInfo(powerBombProjectileParticleEffect, powerBombProjectileDamage)
, mMaxPowerBombHeight(maxPowerBombProjectileHeight)
, mPowerBombFuseTime(powerBombProjectileFuseTime)
, mPowerBombSound(powerBombProjectileSound)
, mPowerBombEmitterMaxDistance(powerBombEmitterMaxDistance)
, mPowerBombEmitterDistanceComp(powerBombEmitterDistanceComp)
, mStartDamageTime(startDamageTime)
, mEndDamageTime(endDamageTime)
, mStageIndex(0)
, mMaxTimerScale(isPowerBombGuardian ? maxTimerScale : 1.f)
, mTimerScale(1.f)
, mAttacksSinceDoubleShot(0)
, mAttacksPerDoubleShot(0)
, mDamageWaitTime(damageWaitTime)
, mPowerBombsFired(0)
, mStages(stages)
, mPowerBombEmitters(SPowerBombEmitter())
, mPowerBombAimPosition(CVector3f::Zero())
, mAimPredictionTimeScale(aimPredictionTimeScale)
, mAimSpreadRadius(aimSpreadRadius)
, mFireGenerateType(pas::kGType_Two)
, mGrabberPosition(CVector3f::Zero())
, mActive(true)
, mAimLocked(false)
, mTimerScaleMaxed(false) {
  mProjectileInfo.Token().Lock();

  const rstl::pair< float, int > forward = GetAnimationData()->GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(5)), -1);
  if (forward.first > FLT_EPSILON) {
    mForwardLeanAnim = forward.second;
  }
  const rstl::pair< float, int > back = GetAnimationData()->GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(6)), -1);
  if (back.first > FLT_EPSILON) {
    mBackLeanAnim = back.second;
  }
  const rstl::pair< float, int > left = GetAnimationData()->GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(4)), -1);
  if (left.first > FLT_EPSILON) {
    mLeftLeanAnim = left.second;
  }
  const rstl::pair< float, int > right = GetAnimationData()->GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(7)), -1);
  if (right.first > FLT_EPSILON) {
    mRightLeanAnim = right.second;
  }
  const rstl::pair< float, int > flinch = GetAnimationData()->GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_AdditiveFlinch, CPASAnimParm::FromEnum(7)), -1);
  if (flinch.first > FLT_EPSILON) {
    mFlinchAnim = flinch.second;
  }

  SetDrawShadow(false);

  if (mMinGrabRange < mMinAttackDistance) {
    mMinGrabRange = mMinAttackDistance;
  }
  if (mMaxGrabRange > mMaxAttackDistance) {
    mMaxGrabRange = mMaxAttackDistance;
  }
  if (mMaxGrabRange > 30.f) {
    mMaxGrabRange = 30.f;
  }

  mWaypointIds.reserve(32);
  SetStage(mStageIndex);

  const int damageOnCount = GetNumUserEventsForAnimation(
      CPASAnimParmData(pas::kAS_Generate, CPASAnimParm::FromEnum(7)), kUE_DamageOn);
  if (damageOnCount != 0) {
    mFlailDamage.SetDamage(flailDamage / damageOnCount);
  }
}

void CSporbBase::SetStage(uchar stage) {
  if (mIsPowerBombGuardian && mStages.size() > stage) {
    mMinTimeBetweenAttacks = mStages[stage]->mMinTimeBetweenAttacks;
    mMaxTimeBetweenAttacks = mStages[stage]->mMaxTimeBetweenAttacks;
    mMinTimeBetweenShots = mStages[stage]->mMinTimeBetweenShots;
    mMaxTimeBetweenShots = mStages[stage]->mMaxTimeBetweenShots;
    mMinShots = mStages[stage]->mMinShotsInABurst;
    mMaxShots = mStages[stage]->mMaxShotsInABurst;
  }
}

void CSporbBase::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CSporbBase::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
    for (; it != GetConnectionList().end(); ++it) {
      const TUniqueId id = mgr.GetIdForScript(it->objId);
      if (it->state == kSS_Approach) {
        if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(id))) {
          mTopId = id;
          mSavedVulnerability = *top->GetDamageVulnerability();
          *top->DamageVulnerability() = CDamageVulnerability::ImmuneVulnerabilty();
          top->SetImmune(true);
          top->SetBaseId(GetUniqueId());
        } else if (TCastToPtr< CSporbProjectile >(mgr.ObjectById(id))) {
          mProjectileId = id;
        } else if (TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
          mWaypointIds.push_back_unsafe(id);
        }
      } else if (it->state == kSS_GRNT) {
        mNeedleSpawnerId = it->objId;
      }
    }
    sGrabbingSporb = kInvalidUniqueId;
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetTopId(mTopId);
    }
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetProjectileId(mProjectileId);
    }
    break;
  }
  case kSM_Create:
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Crouch);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
    *DamageVulnerability() = mIsPowerBombGuardian ? CDamageVulnerability::ReflectVulnerabilty()
                                                  : CDamageVulnerability::ImmuneVulnerabilty();
    RemoveMaterial(kMT_Target, mgr);
    AddMaterial(kMT_Unknown54, mgr);
    if (mIsPowerBombGuardian && !mStages.empty() && mStageIndex < mStages.size()) {
      const CPowerBombGuardianStageData& stage = *mStages[mStageIndex];
      const int range = stage.mMaxAttacksPerDoubleShot - stage.mMinAttacksPerDoubleShot;
      mAttacksPerDoubleShot =
          stage.mMinAttacksPerDoubleShot + (range != 0 ? mgr.Random()->Next() % (range + 1) : 0);
    }
    if (mTendrilGen.get()) {
      mTendrilGen->SetTranslation(GetTranslation());
      mTendrilGen->ForceParticleCreation(static_cast< uint >(mMaxGrabRange / 0.15f) + 2);
    }
    break;
  case kSM_Increment:
    if (mIsPowerBombGuardian) {
      mStageIndex = rstl::max_val(mStageIndex - 1, 0);
      SetStage(mStageIndex);
    }
    break;
  case kSM_Decrement:
    if (mIsPowerBombGuardian) {
      ++mStageIndex;
      SetStage(mStageIndex);
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      mStateMachine->SetState(mgr, *this, rstl::string_l("Flinch"));
      const float remaining = static_cast< float >(mStages.size() - mStageIndex);
      HealthInfo()->SetHP(remaining / static_cast< float >(mStages.size()) *
                          HealthInfo()->GetInitialHP());
    }
    break;
  case kSM_Alert:
    mTimeSinceDetection = 0.f;
    mAlerted = true;
    break;
  case kSM_Reset:
    mAlerted = false;
    mTimeSinceDetection = mPlayerLeashTime;
    mTopHit = false;
    break;
  case kSM_Start:
    mActive = true;
    break;
  case kSM_Stop:
    mActive = false;
    break;
  case kSM_SetToMax:
    mTimerScale = mMaxTimerScale;
    mTimerScaleMaxed = true;
    break;
  case kSM_SetToZero:
    mTimerScale = 1.f;
    mTimerScaleMaxed = false;
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

void CSporbBase::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);

  CPlayer* player = mgr.GetPlayer(0);
  if (mIsPowerBombGuardian && !mTimerScaleMaxed) {
    const CVector3f offset = PredictAimOffset(mMaxPowerBombHeight, mgr, GetLocatorPosition());
    mPowerBombAimPosition = player->GetAimPosition(mgr, 0.f) + offset;
  }

  if (IsWithinRange(mDetectionDistance, *player)) {
    mTimeSinceDetection = 0.f;
  } else if (!IsWithinRange(mMaxAttackDistance, *player) &&
             mTimeSinceDetection < mPlayerLeashTime) {
    mTimeSinceDetection += dt;
  }

  if (mIsPowerBombGuardian && !mStages.empty() &&
      mAttacksSinceDoubleShot >= mAttacksPerDoubleShot) {
    mShotsInBurst = 2;
  }

  const CTransform4f connectXf =
      GetTransform() * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));

  if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
    if (top->GetAlive()) {
      top->SetTransform(connectXf);
    } else if (mState != kS_FakeDeath && mState != kS_FakeDead) {
      mStateMachine->SetState(mgr, *this, rstl::string_l("FakeDeath"));
    }
    if (mIsPowerBombGuardian) {
      top->SetImmune(true);
    }

    const CVector3f headPosition =
        connectXf *
        top->GetScaledLocatorTransform(rstl::string_l("lockon_target_LCTR")).GetTranslation();
    if (top->GetHitByPlayerProjectile() && !top->IsImmune()) {
      mTopHit = true;
      top->SetHitByPlayerProjectile(false);
      top->AnimationData()->AddAdditiveAnimation(mFlinchAnim, 1.f, false, true);
      if (mTopBlend >= 1.f) {
        mTopAttachPosition =
            GetTransform() *
            GetScaledLocatorTransform(rstl::string_l("lockon_target_LCTR")).GetTranslation();
        mSavedForwardLean = mForwardLean;
        mSavedBackLean = mBackLean;
        mSavedLeftLean = mLeftLean;
        mSavedRightLean = mRightLean;
      }
      mTopBlend = 0.f;
    } else {
      if (top->IsImmune()) {
        top->SetHitByPlayerProjectile(false);
      }
      mTopBlend = rstl::min_val(1.f, mTopBlend + dt);
    }
    top->SetOrbitPosition(mTopAttachPosition * (1.f - mTopBlend) + headPosition * mTopBlend);

    if (top->GetAlive() && top->BodyController()->IsFrozen() && !BodyController()->IsFrozen()) {
      Freeze(mgr, CVector3f::Zero(), CUnitVector3f(CVector3f::Forward()), top->GetFreezeDuration(),
             -1.f);
      if (CSporbProjectile* projectile =
              TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
        projectile->Freeze(mgr, CVector3f::Zero(), CUnitVector3f(CVector3f::Forward()),
                           top->GetFreezeDuration(), -1.f);
      }
      ResetShotTimers(mgr);
    }
  }

  if (CSporbProjectile* projectile =
          TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
    if (projectile->IsPassable() && (mState != kS_FakeDeath || mState != kS_FakeDead)) {
      if (!x8f8_) {
        x8f8_ = true;
      }
      CPlayer* target = mgr.GetPlayer(0);
      if (!target->GetMorphBall()->IsBoosting()) {
        target->Stop();
        target->SetAngularVelocityWR(CAxisAngle::Identity());
        target->SetVelocityWR(CVector3f::Zero());
      }
      target->EnableLeaveMorphBall(false);
    }
    if (mGrabberState == kGS_Idle || mGrabberState == kGS_Launch) {
      projectile->SetTransform(connectXf);
    }
    mProjectilePosition = projectile->GetTranslation();
  }

  switch (mState) {
  case kS_Sleeping:
  case kS_Patrolling:
    DecayAim();
    ResetShotTimers(mgr);
    break;
  case kS_Firing:
  case kS_ContinueFire:
  case kS_Flailing:
  case kS_Spitting:
  case kS_SpitExit: {
    float threshold = 0.8f;
    if (mIsPowerBombGuardian) {
      threshold = mShotsInBurst > 1 ? 2.433333f : 1.3f;
    }
    if (x860_ > threshold) {
      if (mIsPowerBombGuardian && !mStages.empty() && mStageIndex < mStages.size() &&
          mAttacksSinceDoubleShot >= mAttacksPerDoubleShot && mShotsInBurst > 1) {
        mAttacksSinceDoubleShot = 0;
        const CPowerBombGuardianStageData& stage = *mStages[mStageIndex];
        const int range = stage.mMaxAttacksPerDoubleShot - stage.mMinAttacksPerDoubleShot;
        mAttacksPerDoubleShot =
            stage.mMinAttacksPerDoubleShot + (range != 0 ? mgr.Random()->Next() % (range + 1) : 0);
      }
      if (mIsPowerBombGuardian) {
        UpdateAim(mgr);
      }
      UpdateAttack(dt, mgr);
    } else {
      if (!mIsPowerBombGuardian || !mAimLocked) {
        mAimTarget = PickTargetPosition(mgr, mState == kS_Firing || mState == kS_ContinueFire);
        mAimLocked = true;
      }
      UpdateAim(mgr);
      x860_ += dt;
    }
    break;
  }
  default:
    if (!mIsPowerBombGuardian || !mAimLocked) {
      mAimTarget = PickTargetPosition(mgr, false);
      mAimLocked = true;
    }
    UpdateAim(mgr);
    break;
  }
  if (mState == kS_Flailing || mState == kS_FakeDeath || mState == kS_FakeDead ||
      mState == kS_Flinching) {
    DecayAim();
  }

  for (uint i = 0; i < mPowerBombEmitters.size(); ++i) {
    SPowerBombEmitter& emitter = mPowerBombEmitters[i];
    if (const CEnergyProjectile* projectile =
            TCastToConstPtr< CEnergyProjectile >(mgr.GetObjectById(emitter.mProjectileId))) {
      CSfxManager::UpdateEmitter(emitter.mHandle, projectile->GetTranslation(), CVector3f::Zero(),
                                 0x80);
    } else {
      CSfxManager::RemoveEmitter(emitter.mHandle);
      emitter.mHandle = CSfxHandle();
      emitter.mProjectileId = kInvalidUniqueId;
    }
  }

  AnimationData()->AddAdditiveAnimation(mForwardLeanAnim, mForwardLean, false, false);
  AnimationData()->AddAdditiveAnimation(mBackLeanAnim, mBackLean, false, false);
  AnimationData()->AddAdditiveAnimation(mLeftLeanAnim, mLeftLean, false, false);
  AnimationData()->AddAdditiveAnimation(mRightLeanAnim, mRightLean, false, false);
}

void CSporbBase::DecayAim() {
  mForwardLean = rstl::max_val(0.f, mForwardLean - 0.025f);
  mBackLean = rstl::max_val(0.f, mBackLean - 0.025f);
  mLeftLean = rstl::max_val(0.f, mLeftLean - 0.025f);
  mRightLean = rstl::max_val(0.f, mRightLean - 0.025f);
}

void CSporbBase::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (mTendrilGen.get()) {
    mTendrilGen->Render();
  }
}

void CSporbBase::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CSporbBase::PreRenderAllViewports(CStateManager& mgr) {
  CPatterned::PreRenderAllViewports(mgr);
  if (mState != kS_FakeDeath && mState != kS_FakeDead) {
    CAABox bounds = GetOtherBounds();
    bounds.AccumulateBounds(mProjectilePosition);
    if (mTendrilGen.get()) {
      rstl::optional_object< CAABox > tendrilBounds = mTendrilGen->GetBounds();
      if (tendrilBounds) {
        CAABox box = *tendrilBounds;
        if (!box.Invalid()) {
          bounds.AccumulateBounds(box.GetMinPoint());
          bounds.AccumulateBounds(box.GetMaxPoint());
        }
      }
    }
    SetOtherBounds(bounds);
    SetRenderBounds(bounds);
  }
}

void CSporbBase::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
  if (mTendrilGen.get()) {
    rstl::optional_object< CAABox > tendrilBounds = mTendrilGen->GetBounds();
    if (tendrilBounds) {
      if (!tendrilBounds->Invalid()) {
        gpRender->AddParticleGen(*mTendrilGen);
      }
    }
  }
}

void CSporbBase::Sleep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_Sleeping;
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    mFiring = false;
    mAlerted = true;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbBase::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mState = kS_Patrolling;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetState(CSporbTop::kS_Patrol);
    }
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Patrolling);
    }
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CSporbBase::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mState = kS_Attacking;
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetState(CSporbTop::kS_Attack);
    }
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Attacking);
    }
    mReadyToFire = true;
    x860_ = 0.f;
    mAimLocked = false;

    const CPlayer* player = mgr.GetPlayer(0);
    const bool inRange =
        IsWithinRange(mMaxGrabRange, *player) && !IsWithinRange(mMinGrabRange, *player);
    if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
             ? player->GetMorphballTransitionState()
             : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed &&
        inRange && sGrabbingSporb == kInvalidUniqueId && !mIsPowerBombGuardian) {
      mAttackType = kAT_Grab;
      mGrabberState = kGS_Idle;
    } else {
      mAttackType = kAT_Shoot;
    }
    if (mIsPowerBombGuardian && !mStages.empty() && mStageIndex < mStages.size() &&
        mShotsInBurst != 2 && mStages[mStageIndex]->mDoubleShotChance > 0.f) {
      mAttacksSinceDoubleShot =
          rstl::min_val(static_cast< uchar >(mAttacksSinceDoubleShot + 1), mAttacksPerDoubleShot);
    }
    mPowerBombsFired = 0;
    break;
  }
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbBase::Fire(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    if (mShotsInBurst > 1 && mIsPowerBombGuardian) {
      mFireGenerateType = pas::kGType_Five;
    } else {
      mFireGenerateType = pas::kGType_Two;
    }
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetState(CSporbTop::kS_Fire);
      if (!mIsPowerBombGuardian) {
        top->AnimationData()->SetEffectState(rstl::string_l("telegraph"), true, mgr);
      }
      if (mFireGenerateType == pas::kGType_Two) {
        top->SetFireGenerateType(pas::kGType_Three);
      } else {
        top->SetFireGenerateType(pas::kGType_Five);
      }
    }
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Fire);
    }
    mState = kS_Firing;
    mShooting = true;
    if (mAttackType == kAT_Grab) {
      mGrabberState = kGS_Launch;
    }
    mFiring = true;
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(mFireGenerateType, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbBase::Spit(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    mState = kS_Spitting;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    }
    if (mStateMachine->GetTime() > 0.5f) {
      if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
        top->SetState(CSporbTop::kS_Spit);
      }
      if (CSporbProjectile* projectile =
              TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
        projectile->SetState(CSporbProjectile::kS_Spit);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbBase::Flail(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Seven, -1));
    mState = kS_Flailing;
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      mGrabberPosition = projectile->GetTranslation();
    }
    SendScriptMsgs(kSS_MaxReached, mgr, mgr.GetPlayer(0)->GetUniqueId(), kSM_None);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Seven, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mCanSpit = true;
    break;
  }
}

void CSporbBase::ContinueFlail(CStateManager& mgr, EStateMsg msg, float dt) {}

void CSporbBase::ContinueSpit(CStateManager& mgr, EStateMsg msg, float dt) {}

void CSporbBase::AttackExit(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_AttackExit;
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetState(CSporbTop::kS_Reload);
      if (!mIsPowerBombGuardian) {
        top->AnimationData()->SetEffectState(rstl::string_l("telegraph"), false, mgr);
      }
    }
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Reload);
    }
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    ResetShotTimers(mgr);
    break;
  }
}

void CSporbBase::SpitExit(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_SpitExit;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbBase::ContinueFire(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Internal6);
    mState = kS_ContinueFire;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbBase::Flinch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, -1));
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->StartFlinch(mgr);
    }
    mState = kS_Flinching;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbBase::WakeUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetState(CSporbTop::kS_Patrol);
      *top->DamageVulnerability() = mSavedVulnerability;
      top->SetImmune(false);
    }
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Patrolling);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbBase::GoToSleep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      top->SetState(CSporbTop::kS_Sleeping);
      mSavedVulnerability = *top->GetDamageVulnerability();
      *top->DamageVulnerability() = CDamageVulnerability::ImmuneVulnerabilty();
      top->SetImmune(true);
    }
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Sleeping);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbBase::FakeDeath(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mState = kS_FakeDeath;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Six, -1));
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetPassable(false);
      mgr.DeleteObjectRequest(projectile->GetUniqueId());
    }
    StopTendril();
    SendScriptMsgs(kSS_Exited, mgr, mgr.GetPlayer(0)->GetUniqueId(), kSM_None);
    if (sGrabbingSporb == GetUniqueId()) {
      mgr.GetPlayer(0)->EnableLeaveMorphBall(true);
      mgr.GetPlayer(0)->GetMorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
      sGrabbingSporb = kInvalidUniqueId;
    }
    BodyController()->UnFreeze();
    mColor = CColor(CColor::Black().GetRedu8(), CColor::Black().GetGreenu8(),
                    CColor::Black().GetBlueu8(), mColor.GetAlphau8());
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Six, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbBase::FakeDead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_FakeDead;
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

bool CSporbBase::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

bool CSporbBase::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() ==
      CGameArea::kOS_Occluded) {
    return false;
  }
  if (!mAlerted) {
    return false;
  }
  if (sGrabbingSporb == kInvalidUniqueId) {
    const CPlayer* player = mgr.GetPlayer(0);
    const bool inRange =
        IsWithinRange(mMaxAttackDistance, *player) && !IsWithinRange(mMinAttackDistance, *player);
    if (TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
      return inRange || mTopHit;
    }
    return inRange;
  }
  return false;
}

bool CSporbBase::ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const {
  return mTimeSinceDetection < mPlayerLeashTime && mAlerted;
}

bool CSporbBase::ShouldFire(CStateManager& mgr, const CTriggerData& data) const {
  if (!mAlerted) {
    return false;
  }
  if (!mActive) {
    return false;
  }
  if (mFiring) {
    return mReadyToFire;
  }
  return mReadyToFire && mStateMachine->GetTime() > mTimeBetweenAttacks;
}

bool CSporbBase::ShouldSpit(CStateManager& mgr, const CTriggerData& data) const { return mCanSpit; }

bool CSporbBase::ShouldFlail(CStateManager& mgr, const CTriggerData& data) const {
  return mState == kS_Flailing;
}

bool CSporbBase::AttackExitOver(CStateManager& mgr, const CTriggerData& data) const {
  return mStateMachine->GetTime() > mTimeBetweenAttacks;
}

bool CSporbBase::AttackOver(CStateManager& mgr, const CTriggerData& data) const {
  if (mAttackType == kAT_Shoot) {
    return !mShooting;
  }
  if (mAttackType == kAT_Grab) {
    return mGrabFinished;
  }
  return false;
}

bool CSporbBase::SpitOver(CStateManager& mgr, const CTriggerData& data) const { return true; }

void CSporbBase::FindTargetPlayer(CStateManager& mgr) {
  mTargetPlayerId = kInvalidUniqueId;
  float best = FLT_MAX;
  const CVector3f forward = GetTransform().GetForward();
  const float scale = mMaxAttackDistance * mMaxAttackDistance / M_PIF;
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    if (IsWithinRange(mMaxAttackDistance, *player) && !IsWithinRange(mMinAttackDistance, *player)) {
      const CVector3f diff = player->GetTranslation() - GetTranslation();
      const float angle = CVector3f::GetAngleDiff(diff, forward);
      const float score = angle * scale + diff.MagSquared();
      if (score < best) {
        best = score;
        mTargetPlayerId = player->GetUniqueId();
      }
    }
  }
}

bool CSporbBase::IsWithinRange(float range, const CActor& other) const {
  return (other.GetTranslation() - GetTranslation()).MagSquared() < range * range;
}

float CSporbBase::GetAimAngle(const CStateManager& mgr) const {
  const CTransform4f& xf = GetTransform();
  const CTransform4f locatorXf(GetScaledLocatorTransform(rstl::string_l(skConnectLocator)));
  const CVector3f up = xf.GetUp();
  const CVector3f position = xf * locatorXf.GetTranslation();
  const CVector3f direction = (mAimTarget - position).AsNormalized();
  return CVector3f::GetAngleDiff(up, direction);
}

void CSporbBase::UpdateAttack(float dt, CStateManager& mgr) {
  switch (mAttackType) {
  case kAT_Grab:
    UpdateGrabber(dt, mgr);
    break;
  case kAT_Shoot:
    UpdateShooting(dt, mgr);
    break;
  }
}

void CSporbBase::UpdateGrabber(float dt, CStateManager& mgr) {
  bool attached = false;
  CAABox tendrilBounds = CAABox::MakeMaxInvertedBox();
  const CTransform4f connectXf =
      GetTransform() * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
  const CVector3f connectPos = connectXf.GetTranslation();

  if (mProjectileId != kInvalidUniqueId) {
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      switch (mGrabberState) {
      case kGS_Idle:
        projectile->SetSolidPhase(CSporbProjectile::kSP_Solid);
        break;
      case kGS_Launch: {
        mGrabberState = kGS_Extend;
        projectile->SetState(CSporbProjectile::kS_Launch);
        const CMatrix3f rotation = CMatrix3f::RotateX(CRelAngle::FromRadians(-M_PIF / 2.f));
        const CTransform4f lookXf = BuildLookAtTransform(mgr, connectPos);
        mProjectileRotation = CQuaternion::FromMatrix(lookXf.BuildMatrix3f() * rotation);
        ProcessSoundEvent(mFlightSound | 0xa0000000, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20,
                          127, GetDistanceToCamera(mgr), GetTranslation(),
                          mgr.GetNextAreaId().Value(), mgr, true);
        ProcessSoundEvent(mFireSound, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                          GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                          mgr, true);
        break;
      }
      case kGS_Extend:
        if (projectile->GetSolidPhase() == CSporbProjectile::kSP_Landed ||
            sGrabbingSporb != kInvalidUniqueId) {
          mGrabberState = kGS_Hold;
          StopLoopedSounds();
          ProcessSoundEvent(mHitWorldSound, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                            GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                            mgr, true);
          projectile->Stop();
          projectile->SetState(CSporbProjectile::kS_Close);
        } else if (projectile->GetSolidPhase() == CSporbProjectile::kSP_BallInside) {
          mAimTarget = PickTargetPosition(mgr, false);
          const CTransform4f& projectileXf = projectile->GetTransform();
          const CVector3f ballAttach =
              projectile->GetScaledLocatorTransform(rstl::string_l("ball_attach_LCTR"))
                  .GetTranslation();
          const CVector3f attachWorld = projectileXf * ballAttach;
          const CTransform4f lookXf = BuildLookAtTransform(mgr, attachWorld);
          const CVector3f delta = mAimTarget - attachWorld;
          const CVector3f velocity = lookXf * CVector3f(0.f, mGrabberOutSpeed, 0.f);
          const CVector3f forward = lookXf * CVector3f::Forward();
          const float dot = CVector3f::Dot(forward, delta);
          const CVector3f next = attachWorld + dt * velocity;
          const float nextDistSq = (next - GetTranslation()).MagSquared();
          const float aimDistSq = (mAimTarget - GetTranslation()).MagSquared();
          if (dot > 0.f && CMath::AbsF(dot) >= 1e-5f && nextDistSq <= aimDistSq) {
            projectile->SetVelocityWR(velocity);
          } else {
            projectile->SetTranslation(mAimTarget - (attachWorld - projectile->GetTranslation()));
            mGrabberState = kGS_Hold;
            projectile->Stop();
            projectile->SetState(CSporbProjectile::kS_Close);
            StopLoopedSounds();
            ProcessSoundEvent(mHitPlayerSound, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                              GetDistanceToCamera(mgr), projectile->GetTranslation(),
                              mgr.GetNextAreaId().Value(), mgr, true);
            sGrabbingSporb = GetUniqueId();
            mgr.GetPlayer(0)->GetMorphBall()->StopParticleWakes();
            mgr.ApplyDamage(
                GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), mGrabDamage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageSolid), CMaterialList()),
                CVector3f::Zero());
          }
        } else {
          if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
            if (!mIsPowerBombGuardian) {
              top->AnimationData()->SetEffectState(rstl::string_l("telegraph"), false, mgr);
            }
          }
          const CVector3f direction = projectile->GetTranslation() - connectPos;
          const CTransform4f lookXf =
              CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up());
          const float distance = rstl::min_val(direction.Magnitude(), mMaxGrabRange);
          mTendrilParticleCount = static_cast< uint >(distance / 0.15f) + 2;
          float maxSize = 0.f;
          for (int i = 0; i < mTendrilParticleCount; ++i) {
            const float sinArg = 0.15f * i - 0.15f * mTendrilParticleCount;
            const CVector3f offset(
                CMath::FastSinR(sinArg) *
                    (static_cast< float >(i) / static_cast< float >(mTendrilParticleCount)),
                0.15f * i, 0.f);
            const CVector3f position = connectPos + lookXf * offset;
            if (mTendrilGen.get() && i < mTendrilGen->GetParticleCount()) {
              CElementGen::CParticle& particle = mTendrilGen->mParticles[i];
              particle.mPos = position;
              particle.mPrevPos = position;
              maxSize = rstl::max_val(maxSize, particle.mLineLengthOrSize);
            }
            tendrilBounds.AccumulateBounds(position);
          }
          tendrilBounds =
              CAABox(tendrilBounds.GetMinPoint() - CVector3f(maxSize, maxSize, maxSize),
                     tendrilBounds.GetMaxPoint() + CVector3f(maxSize, maxSize, maxSize));

          const CTransform4f xf = BuildLookAtTransform(mgr, connectPos);
          mGrabberOutSpeed = rstl::max_val(0.f, mGrabberOutSpeed + mGrabberOutAcceleration * dt);
          projectile->SetVelocityWR(xf * CVector3f(0.f, mGrabberOutSpeed, 0.f));
          const CVector3f step = dt * CVector3f(0.f, mGrabberInSpeed, 0.f);
          if ((projectile->GetTranslation() + step - connectPos).MagSquared() >
              mMaxGrabRange * mMaxGrabRange) {
            const CTransform4f clampXf = BuildLookAtTransform(mgr, connectPos);
            clampXf* CVector3f(0.f, mMaxGrabRange, 0.f);
            mGrabberState = kGS_Hold;
            StopLoopedSounds();
            projectile->Stop();
            projectile->SetState(CSporbProjectile::kS_Close);
          }
        }
        break;
      case kGS_Hold: {
        mHoldTimer = rstl::min_val(mHoldDuration, mHoldTimer + dt);
        const CVector3f direction = projectile->GetTranslation() - connectPos;
        const CTransform4f lookXf =
            CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up());
        const float distance = rstl::min_val(direction.Magnitude(), mMaxGrabRange);
        mTendrilParticleCount = static_cast< uint >(distance / 0.15f) + 2;
        const float amplitude = 1.f - mHoldTimer / mHoldDuration;
        const float last = 0.15f * (mTendrilParticleCount - 1);
        float maxSize = 0.f;
        for (int i = 0; i < mTendrilParticleCount; ++i) {
          const float sinArg = 0.15f * i - last;
          const CVector3f offset(
              amplitude * CMath::FastSinR(sinArg) *
                  (static_cast< float >(i) / static_cast< float >(mTendrilParticleCount)),
              0.15f * i, 0.f);
          const CVector3f position = connectPos + lookXf * offset;
          if (mTendrilGen.get() && i < mTendrilGen->GetParticleCount()) {
            CElementGen::CParticle& particle = mTendrilGen->mParticles[i];
            particle.mEndFrame = 0;
            particle.mPos = position;
            particle.mPrevPos = position;
            maxSize = rstl::max_val(maxSize, particle.mLineLengthOrSize);
          }
          tendrilBounds.AccumulateBounds(position);
        }
        tendrilBounds = CAABox(tendrilBounds.GetMinPoint() - CVector3f(maxSize, maxSize, maxSize),
                               tendrilBounds.GetMaxPoint() + CVector3f(maxSize, maxSize, maxSize));
        if (mTendrilGen.get()) {
          const CVector3f restPosition = GetTranslation();
          for (int i = 0; i < mTendrilGen->GetParticleCount() - mTendrilParticleCount; ++i) {
            CElementGen::CParticle& particle = mTendrilGen->mParticles[mTendrilParticleCount + i];
            particle.mPos = restPosition;
            particle.mPrevPos = restPosition;
          }
        }

        mAttachTimer = rstl::min_val(mAttachDuration, mAttachTimer + dt);
        if (mAttachTimer >= mAttachDuration) {
          mGrabberState = kGS_Retract;
          if (projectile->GetSolidPhase() == CSporbProjectile::kSP_BallInside) {
            projectile->SetCarryingBall(true);
            mgr.GetPlayer(0)->GetMorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostDisabled);
            SendScriptMsgs(kSS_Entered, mgr, mgr.GetPlayer(0)->GetUniqueId(), kSM_None);
          }
        } else if (projectile->GetSolidPhase() == CSporbProjectile::kSP_Solid ||
                   (sGrabbingSporb != kInvalidUniqueId && sGrabbingSporb != GetUniqueId())) {
          if (mHoldTimer == mHoldDuration) {
            mGrabberState = kGS_Retract;
          }
        }
        if (mGrabberState != kGS_Retract) {
          CPlayer* player = mgr.GetPlayer(0);
          if (player->GetMorphBall()->IsBoosting() && projectile->IsPassable()) {
            mGrabberState = kGS_Retract;
            x8f8_ = false;
            projectile->SetPassable(false);
            projectile->SetCarryingBall(false);
            projectile->SetState(CSporbProjectile::kS_Open);
            player->GetMorphBall()->SetAsProjectile(true);
            mBallEscaped = true;
            sGrabbingSporb = kInvalidUniqueId;
            const CTransform4f unusedXf =
                GetTransform() * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
            projectile->PlayBallEscapeEffect(mgr);
            ProcessSoundEvent(mBallEscapeSound, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                              GetDistanceToCamera(mgr), GetTranslation(),
                              mgr.GetNextAreaId().Value(), mgr, true);
          }
        }
        if (mGrabberState == kGS_Retract) {
          ProcessSoundEvent(
              (projectile->IsCarryingBall() ? mRetractSound : mRetractMissedPlayerSound) |
                  0x20000000,
              1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127, GetDistanceToCamera(mgr),
              GetTranslation(), mgr.GetNextAreaId().Value(), mgr, true);
        }
        projectile->Stop();
        break;
      }
      case kGS_Retract: {
        const CVector3f direction = projectile->GetTranslation() - connectPos;
        const CTransform4f lookXf =
            CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up());
        const float distance = rstl::min_val(direction.Magnitude(), mMaxGrabRange);
        mTendrilParticleCount = static_cast< uint >(distance / 0.15f) + 2;
        float maxSize = 0.f;
        for (int i = 0; i < mTendrilParticleCount; ++i) {
          const CVector3f offset(0.f, 0.15f * i, 0.f);
          const CVector3f position = connectPos + lookXf * offset;
          if (mTendrilGen.get() && i < mTendrilGen->GetParticleCount()) {
            CElementGen::CParticle& particle = mTendrilGen->mParticles[i];
            particle.mEndFrame = 0;
            particle.mPos = position;
            particle.mPrevPos = position;
            maxSize = rstl::max_val(maxSize, particle.mLineLengthOrSize);
          }
          tendrilBounds.AccumulateBounds(position);
        }
        tendrilBounds = CAABox(tendrilBounds.GetMinPoint() - CVector3f(maxSize, maxSize, maxSize),
                               tendrilBounds.GetMaxPoint() + CVector3f(maxSize, maxSize, maxSize));
        if (mTendrilGen.get()) {
          const CVector3f restPosition = GetTranslation();
          for (int i = 0; i < mTendrilGen->GetParticleCount() - mTendrilParticleCount; ++i) {
            CElementGen::CParticle& particle = mTendrilGen->mParticles[mTendrilParticleCount + i];
            particle.mPos = restPosition;
            particle.mPrevPos = restPosition;
          }
        }

        mGrabberInSpeed = rstl::min_val(0.f, mGrabberInSpeed + mGrabberInAcceleration * dt);
        const CVector3f velocity(0.f, mGrabberInSpeed, 0.f);
        if (direction.MagSquared() < (dt * velocity).MagSquared()) {
          projectile->Stop();
          projectile->SetTranslation(connectPos);
          if (!projectile->IsCarryingBall()) {
            ResetAfterGrab(mgr, connectPos);
            StopLoopedSounds();
          } else {
            mGrabberState = kGS_Attach;
            StopLoopedSounds();
            if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
              top->SetState(CSporbTop::kS_Close);
            }
            BodyController()->SetLocomotionType(pas::kLT_Lurk);
            mState = kS_Flailing;
          }
          StopTendril();
        } else {
          projectile->SetVelocityWR(lookXf * velocity);
        }
        if (projectile->IsCarryingBall()) {
          attached = true;
        }
        if (mBallEscaped && mReleaseTimer > 0.1f) {
          projectile->SetState(CSporbProjectile::kS_Close);
        } else {
          mReleaseTimer += dt;
        }
        break;
      }
      case kGS_Attach: {
        const CTransform4f locatorXf =
            GetTransform() * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
        mProjectileRotation = CQuaternion::FromMatrix(locatorXf.BuildMatrix3f());
        projectile->SetTranslation(locatorXf.GetTranslation());
        attached = true;
        if (mCanSpit) {
          mGrabberState = kGS_Spit;
        }
        break;
      }
      case kGS_Spit: {
        const CTransform4f locatorXf =
            GetTransform() * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
        mProjectileRotation = CQuaternion::FromMatrix(locatorXf.BuildMatrix3f());
        projectile->SetTranslation(locatorXf.GetTranslation());
        if (mSpitTimer > 0.6f) {
          SpitPlayer(mgr, mSpitForce);
          x8f8_ = false;
          projectile->SetPassable(false);
          projectile->SetCarryingBall(false);
          projectile->AddMaterial(kMT_Solid, mgr);
          projectile->SetSolidPhase(CSporbProjectile::kSP_Solid);
          mGrabberState = kGS_Idle;
          ResetAfterGrab(mgr, connectPos);
        } else {
          attached = true;
        }
        mSpitTimer += dt;
        break;
      }
      }

      if (mGrabberState != kGS_Attach && mGrabberState != kGS_Spit) {
        mGrabberPosition = projectile->GetTranslation();
      }
      const CVector3f position = projectile->GetTranslation();
      projectile->SetTransform(CTransform4f(mProjectileRotation.BuildTransform(), position));
      if (attached) {
        AttachPlayer(mgr);
      }
      if (mTendrilGen.get()) {
        mTendrilGen->mBounds = tendrilBounds;
      }
    }
  }
}

void CSporbBase::AttachPlayer(CStateManager& mgr) {
  if (mProjectileId != kInvalidUniqueId) {
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      CPlayer* player = mgr.GetPlayer(0);
      const CVector3f playerPosition = player->GetTranslation();
      const CVector3f aimPosition = player->GetAimPosition(mgr, 0.f);
      const CTransform4f projectileXf = projectile->GetTransform();
      const CVector3f locator =
          projectile->GetScaledLocatorTransform(rstl::string_l("ball_attach_LCTR"))
              .GetTranslation();
      const CVector3f world = projectileXf * locator;
      player->SetTranslation(world - (aimPosition - playerPosition));
    }
  }
}

CVector3f CSporbBase::GetLocatorPosition() const {
  const CTransform4f connectXf =
      GetTransform() * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
  return connectXf.GetTranslation();
}

void CSporbBase::UpdateShooting(float dt, CStateManager& mgr) {
  if (mShooting && mShotTimer >= mTimeBetweenShots) {
    CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId));
    CSporbProjectile* projectile = TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId));
    if (top && (projectile || mIsPowerBombGuardian)) {
      mShooting = true;
      top->SetState(CSporbTop::kS_Fire);
      if (projectile) {
        projectile->SetState(CSporbProjectile::kS_Fire);
      }
      mShotTimer = 0.f;
      mTimeBetweenShots =
          mTimerScale * (mMinTimeBetweenShots +
                         (mMaxTimeBetweenShots - mMinTimeBetweenShots) * mgr.Random()->Float());
      ++mShotsFired;
      if (mShotsFired >= mShotsInBurst) {
        mTimeBetweenAttacks = mTimerScale * (mMinTimeBetweenAttacks +
                                             (mMaxTimeBetweenAttacks - mMinTimeBetweenAttacks) *
                                                 mgr.Random()->Float());
        mShotsFired = 0;
        mShotsInBurst = mMinShots + CCast::ToUint8(static_cast< float >(mMaxShots - mMinShots) *
                                                   mgr.Random()->Float());
        mShooting = false;
        x88c_ = true;
        x85c_ = 0.f;
      }

      const CVector3f topHead = GetTopHeadPosition(mgr);
      const CVector3f locatorPosition = GetLocatorPosition();
      if (!mIsPowerBombGuardian && mNeedleSpawnerId != kInvalidEditorId) {
        const CScriptObjectLoaderHelper::SGeneratedObject generated =
            mgr.ScriptObjectLoaderHelper().GenerateScriptObject(mNeedleSpawnerId, mgr);
        CPhysicsActor* needle = static_cast< CPhysicsActor* >(generated.mEntity);
        if (needle) {
          const CTransform4f lookXf = BuildLookAtTransform(mgr, locatorPosition);
          needle->SetTransform(lookXf);
          needle->SetTranslation(locatorPosition);
          const float speed = needle->GetVelocityWR().Magnitude();
          int pitchSign = -1;
          if (mgr.Random()->Next() % 2 == 0) {
            pitchSign = 1;
          }
          int yawSign = -1;
          if (mgr.Random()->Next() % 2 == 0) {
            yawSign = 1;
          }
          const float pitch = pitchSign * (mShotAngleVariance * mgr.Random()->Float());
          const float yaw = yawSign * (mShotAngleVariance * mgr.Random()->Float());
          const CTransform4f spread = CTransform4f::RotateX(CRelAngle::FromDegrees(pitch)) *
                                      CTransform4f::RotateZ(CRelAngle::FromDegrees(yaw));
          needle->SetVelocityWR(spread * (speed * lookXf.GetForward()));
          ProcessSoundEvent(mNeedleSound, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                            GetDistanceToCamera(mgr), topHead, mgr.GetNextAreaId().Value(), mgr,
                            true);
        }
      }
      SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
    }
  }
  mShotTimer += dt;
}

void CSporbBase::ResetShotTimers(CStateManager& mgr) {
  mTimeBetweenAttacks =
      mTimerScale * (mMinTimeBetweenAttacks +
                     (mMaxTimeBetweenAttacks - mMinTimeBetweenAttacks) * mgr.Random()->Float());
  mTimeBetweenShots =
      mTimerScale * (mMinTimeBetweenShots +
                     (mMaxTimeBetweenShots - mMinTimeBetweenShots) * mgr.Random()->Float());
  mShotsInBurst = mMinShots + CCast::ToUint8(static_cast< float >(mMaxShots - mMinShots) *
                                             mgr.Random()->Float());
  mShotTimer = mTimeBetweenShots;
  mShotsFired = 0;
  mShooting = false;
  mReadyToFire = false;
  if (CSporbTop* top = TCastToPtr< CSporbTop >(mgr.ObjectById(mTopId))) {
    top->ResetAttack(mgr, false);
  }
  x88a_ = false;
  x85c_ = 0.f;
  x88c_ = true;
  x88b_ = true;
  mGrabFinished = false;
  x860_ = 0.f;
  mHoldTimer = 0.f;
  mAttachTimer = 0.f;
  mGrabberInSpeed = mInitialGrabberInSpeed;
  mGrabberOutSpeed = mInitialGrabberOutSpeed;
  mAimLocked = false;
}

CVector3f CSporbBase::GetTopHeadPosition(CStateManager& mgr) {
  if (const CSporbTop* top = TCastToConstPtr< CSporbTop >(mgr.GetObjectById(mTopId))) {
    const CVector3f topPosition = top->GetTransform().GetTranslation();
    const CTransform4f locator = top->GetScaledLocatorTransform(CSegId(1));
    return topPosition + top->GetTransform().Rotate(locator.GetTranslation());
  }
  return GetTranslation();
}

void CSporbBase::UpdateAim(CStateManager& mgr) {
  const CTransform4f inverse = GetTransform().GetQuickInverse();
  const CVector3f aimTarget = mAimTarget;
  const CVector3f localAim = inverse * aimTarget;
  const CVector3f direction = localAim.AsNormalized();
  const float angle = GetAimAngle(mgr);
  const float scale = rstl::min_val(120.f, CMath::AbsF(57.295776f * angle)) / 120.f;
  const CVector3f flat(direction.GetX(), direction.GetY(), 0.f);
  const CVector2f lean =
      CVector2f(CVector3f::Dot(CVector3f::Forward(), flat), CVector3f::Dot(CVector3f::Left(), flat))
          .AsNormalized();

  if (lean.GetX() >= 0.f) {
    const float forward = scale * CMath::AbsF(lean.GetX());
    if (forward > 0.f) {
      mForwardLean = forward > mForwardLean ? rstl::min_val(mForwardLean + 0.025f, forward)
                                            : rstl::max_val(forward, mForwardLean - 0.025f);
    } else {
      mForwardLean = rstl::max_val(0.f, mForwardLean - 0.025f);
    }
    mBackLean = rstl::max_val(0.f, mBackLean - 0.025f);
  } else {
    const float back = scale * CMath::AbsF(lean.GetX());
    if (back > 0.f) {
      mBackLean = back > mBackLean ? rstl::min_val(mBackLean + 0.025f, back)
                                   : rstl::max_val(back, mBackLean - 0.025f);
    } else {
      mBackLean = rstl::max_val(0.f, mBackLean - 0.025f);
    }
    mForwardLean = rstl::max_val(0.f, mForwardLean - 0.025f);
  }

  if (lean.GetY() >= 0.f) {
    const float left = scale * CMath::AbsF(lean.GetY());
    if (left > 0.f) {
      mLeftLean = left > mLeftLean ? rstl::min_val(mLeftLean + 0.025f, left)
                                   : rstl::max_val(left, mLeftLean - 0.025f);
    } else {
      mLeftLean = rstl::max_val(0.f, mLeftLean - 0.025f);
    }
    mRightLean = rstl::max_val(0.f, mRightLean - 0.025f);
  } else {
    const float right = scale * CMath::AbsF(lean.GetY());
    if (right > 0.f) {
      mRightLean = right > mRightLean ? rstl::min_val(mRightLean + 0.025f, right)
                                      : rstl::max_val(right, mRightLean - 0.025f);
    } else {
      mRightLean = rstl::max_val(0.f, mRightLean - 0.025f);
    }
    mLeftLean = rstl::max_val(0.f, mLeftLean - 0.025f);
  }
}

CTransform4f CSporbBase::BuildLookAtTransform(CStateManager& mgr, const CVector3f& from) {
  const CUnitVector3f direction(mAimTarget - from);
  const CVector3f& forward = CVector3f::Forward();
  const CQuaternion rotation =
      CQuaternion::LookAt(CUnitVector3f(forward), direction, CRelAngle::FromRadians(2.f * M_PIF));
  return rotation.BuildTransform4f();
}

void CSporbBase::ResetAfterGrab(CStateManager& mgr, const CVector3f& position) {
  mGrabberState = kGS_Idle;
  mTendrilParticleCount = 0;
  mRetractPosition = position;
  if (mProjectileId != kInvalidUniqueId) {
    if (CSporbProjectile* projectile =
            TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
      projectile->SetState(CSporbProjectile::kS_Reload);
    }
  }
  mGrabFinished = true;
  mCanSpit = false;
  mSpitTimer = 0.f;
  mReleaseTimer = 0.f;
  mBallEscaped = false;
}

CVector3f CSporbBase::PickWaypointPosition(CStateManager& mgr) {
  const uint waypointCount = mWaypointIds.size();
  rstl::vector< TUniqueId > ids;
  ids.reserve(waypointCount);
  for (uint i = 0; i < waypointCount; ++i) {
    const CEntity* waypoint = mgr.GetObjectById(mWaypointIds[i]);
    if (waypoint != nullptr && waypoint->GetActive()) {
      ids.push_back_unsafe(mWaypointIds[i]);
    }
  }
  const uint idCount = ids.size();
  if (idCount != 0) {
    uint index = 0;
    if (idCount > 1) {
      index = mgr.Random()->Next() % idCount;
    }
    const CActor* waypoint = static_cast< const CActor* >(mgr.GetObjectById(ids[index]));
    if (waypoint != nullptr) {
      return waypoint->GetTranslation();
    }
  }
  return mAimTarget;
}

void CSporbBase::SpitPlayer(CStateManager& mgr, float force) {
  CPlayer* player = mgr.GetPlayer(0);
  player->EnableLeaveMorphBall(true);
  if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
           ? player->GetMorphballTransitionState()
           : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
    const CVector3f target = PickWaypointPosition(mgr);
    const CVector3f direction = target - player->GetTranslation();
    player->Stop();
    player->SetVelocityWR(CVector3f::Zero());
    player->ApplyImpulseWR(force * (player->GetMass() * direction.AsNormalized()),
                           CAxisAngle::Identity());
    player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    mgr.ApplyDamage(
        GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mSpitDamage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageSolid), CMaterialList()),
        CVector3f::Zero());
    player->GetMorphBall()->SetAsProjectile(true);
    player->GetMorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
    const ushort sfx = mMorphballSpitSound;
    ProcessSoundEvent(sfx, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                      GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(), mgr,
                      true);
    sGrabbingSporb = kInvalidUniqueId;
    SendScriptMsgs(kSS_Exited, mgr, mgr.GetPlayer(0)->GetUniqueId(), kSM_None);
    if (mProjectileId != kInvalidUniqueId) {
      if (CSporbProjectile* projectile =
              TCastToPtr< CSporbProjectile >(mgr.ObjectById(mProjectileId))) {
        projectile->PlayBallSpitEffect(mgr);
      }
    }
  }
}

void CSporbBase::StopTendril() {
  mTendrilParticleCount = 0;
  ClearTendrilParticles();
}

CVector3f CSporbBase::PickTargetPosition(CStateManager& mgr, bool trackPlayer) {
  if (mIsPowerBombGuardian) {
    CPlayer* player = mgr.GetPlayer(0);
    const CVector3f playerPosition = player->GetTranslation();
    const uint waypointCount = mWaypointIds.size();
    rstl::vector< CVector3f > positions;
    positions.reserve(waypointCount);
    for (uint i = 0; i < waypointCount; ++i) {
      if (const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mWaypointIds[i]))) {
        if (waypoint->GetActive()) {
          const CVector3f waypointPosition = waypoint->GetTranslation();
          float radius = 0.f;
          const uint stageCount = mStages.size();
          if (stageCount != 0 && mStageIndex < stageCount) {
            radius = mStages[mStageIndex]->mWaypointTargetSpreadRadius;
          }
          radius *= mgr.Random()->Float();
          const float angle = 2.f * M_PIF * mgr.Random()->Float();
          const CVector3f offset =
              CMatrix3f::RotateZ(CRelAngle::FromRadians(angle)) * (radius * CVector3f::Forward());
          positions.push_back_unsafe(waypointPosition + offset);
        }
      }
    }
    CVector3f result = playerPosition;
    const uint positionCount = positions.size();
    if (positionCount != 0) {
      if (positionCount > 1) {
        result = positions[mgr.Random()->Next() % positionCount];
      } else {
        result = positions[0];
      }
    }
    return result;
  }

  CPlayer* player = mgr.GetPlayer(0);
  const CVector3f base = (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                              ? player->GetMorphballTransitionState()
                              : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed
                             ? CVector3f::Zero()
                             : mAttackAimOffset;
  const CVector3f predicted = trackPlayer
                                  ? PredictAimOffset(mMaxPowerBombHeight, mgr, GetLocatorPosition())
                                  : CVector3f::Zero();
  return player->GetAimPosition(mgr, 0.f) + predicted + base;
}

CDamageInfo CSporbBase::GetContactDamage() const {
  if (mState == kS_FakeDeath || mState == kS_FakeDead) {
    return CDamageInfo();
  }
  return CPatterned::GetContactDamage();
}

void CSporbBase::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                 float dt) {
  switch (type) {
  case kUE_SoundPlay:
    switch (mAttackType) {
    case kAT_Grab: {
      const ushort sfx = mGrabberTelegraphSound;
      ProcessSoundEvent(sfx, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                        GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                        mgr, true);
      break;
    }
    case kAT_Shoot:
      if (mState != kS_Flinching) {
        const ushort sfx = mNeedleTelegraphSound;
        ProcessSoundEvent(sfx, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                          GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                          mgr, true);
      }
      break;
    }
    break;
  case kUE_DamageOn:
    if (mState == kS_Flailing) {
      CPlayer* player = mgr.GetPlayer(0);
      mgr.ApplyDamage(
          GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mFlailDamage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageSolid), CMaterialList()),
          CVector3f::Zero());
    }
    break;
  case kUE_Projectile: {
    if (mIsPowerBombGuardian) {
      if (!mTimerScaleMaxed) {
        mAimTarget = mPowerBombAimPosition;
      } else if (mPowerBombsFired != 0) {
        mAimTarget = PickTargetPosition(mgr, false);
      }
    }
    const CVector3f locatorPosition = GetLocatorPosition();
    ShootPowerBomb(mMaxPowerBombHeight, locatorPosition, mgr, 4, mAimTarget);
    ++mPowerBombsFired;
    break;
  }
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

CVector3f CSporbBase::GetLeanDirection(float forward, float back, float left, float right) {
  return CVector3f(back - left, forward - right, 0.f).AsNormalized();
}

CVector3f CSporbBase::GetTopAttachPosition(const CStateManager& mgr) const {
  if (mTopId != kInvalidUniqueId) {
    if (const CSporbTop* top = TCastToConstPtr< CSporbTop >(mgr.GetObjectById(mTopId))) {
      if (top->GetAlive()) {
        const CVector3f direction =
            GetLeanDirection(mForwardLean, mBackLean, mLeftLean, mRightLean);
        const CQuaternion rotation = CQuaternion::LookAt(
            CUnitVector3f(CVector3f(mTopAttachPosition.GetX(), mTopAttachPosition.GetY(), 0.f)),
            CUnitVector3f(direction), CRelAngle::FromRadians(2.f * M_PIF));
        return GetTranslation() +
               rotation.BuildTransform() * (top->OrbitPosition() - GetTranslation());
      }
    }
  }
  return CVector3f::Zero();
}

CSporbPowerBomb* CSporbBase::CreatePowerBomb(CStateManager& mgr,
                                             const TToken< CWeaponDescription >& token,
                                             const CTransform4f& xf, const CDamageInfo& damage) {
  const TUniqueId uid = mgr.AllocateUniqueId();
  CSporbPowerBomb* bomb = rs_new CSporbPowerBomb(
      true, token, kWT_AI, xf, skRayExcludeCharacter, damage, uid, GetCurrentAreaId(),
      GetUniqueId(), kInvalidUniqueId, 0, false, CVector3f::One(), CImpactVisorEffect::None(),
      false, true, mPowerBombFuseTime, mStartDamageTime, mEndDamageTime, mDamageWaitTime);

  CAudioSys::C3DEmitterParmData parms(mPowerBombEmitterMaxDistance, mPowerBombEmitterDistanceComp,
                                      1, 0x80, 0x14);
  parms.mPos = xf.GetTranslation();
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = mPowerBombSound;
  const CSfxHandle handle = CSfxManager::AddEmitter(parms, GetCurrentAreaId().Value(), true, false,
                                                    CSfxManager::kMedPriority);

  for (uint i = 0; i < mPowerBombEmitters.size(); ++i) {
    SPowerBombEmitter& emitter = mPowerBombEmitters[i];
    if (!emitter.mHandle && emitter.mProjectileId == kInvalidUniqueId) {
      emitter.mHandle = handle;
      emitter.mProjectileId = uid;
      break;
    }
  }
  return bomb;
}

CVector3f CSporbBase::PredictAimOffset(float height, CStateManager& mgr,
                                       const CVector3f& from) const {
  CPlayer* player = mgr.GetPlayer(0);

  const CTransform4f playerXf = (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                                     ? player->GetMorphballTransitionState()
                                     : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed
                                    ? CTransform4f(player->GetMorphBall()->GetSurfaceToWorld())
                                    : player->GetTransform();
  const float radius = mAimSpreadRadius * mgr.Random()->Float();
  const float angle = 2.f * M_PIF * mgr.Random()->Float();
  const CVector3f spread =
      CQuaternion::AxisAngle(CUnitVector3f(playerXf.GetUp()), CRelAngle::FromRadians(angle))
          .BuildTransform() *
      (radius * playerXf.GetForward());

  if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
           ? player->GetMorphballTransitionState()
           : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed &&
      !player->GetMorphBall()->IsBoosting()) {
    const CVector3f aim = player->GetAimPosition(mgr, 0.f);
    const float gravity = GetGravity();
    const float time = PredictFlightTime(height, gravity, from, aim);
    const CVector3f delta = player->PredictMotion(time * mAimPredictionTimeScale).GetTranslation();
    const CVector3f predicted = aim + delta;

    static CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(skRayInclude), CMaterialList(skRayExcludePassthrough, skRayExcludeCharacter));

    const float distance = (predicted - from).Magnitude();
    const CVector3f direction = (predicted - from).AsNormalized();
    TUniqueId hitId = kInvalidUniqueId;
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, from, direction, distance, filter, this);
    const CRayCastResult result =
        mgr.RayWorldIntersection(hitId, from, direction, distance, filter, nearList);
    if (result.IsValid()) {
      return (result.GetPoint() - aim) + spread;
    }
    return delta + spread;
  }
  return spread;
}

float CSporbBase::PredictFlightTime(float height, float gravity, const CVector3f& from,
                                    const CVector3f& to) const {
  const float dz = from.GetZ() - to.GetZ();
  const float rise = dz > 0.f ? height : -dz + height;
  const float fall = dz > 0.f ? dz + height : height;
  return CMath::SqrtF(2.f * rise / gravity) + CMath::SqrtF(2.f * fall / gravity);
}

CProjectileInfo* CSporbBase::ProjectileInfo() {
  return mIsPowerBombGuardian ? &mProjectileInfo : nullptr;
}

void CSporbBase::ShootPowerBomb(float height, const CVector3f& from, CStateManager& mgr, int count,
                                const CVector3f& to) {
  static float sTickTime = CProjectileWeapon::GetTickTime(); // Guessed name
  CProjectileInfo* info = ProjectileInfo();
  if (info->Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, count)) {
    const float gravity = GetGravity();
    const float time = PredictFlightTime(height, gravity, from, to);
    const float inverseTime = 1.f / time;
    const CVector3f velocity((to.GetX() - from.GetX()) * inverseTime,
                             (to.GetY() - from.GetY()) * inverseTime,
                             -(from.GetZ() - to.GetZ()) / time + 0.5f * gravity * time);
    const CTransform4f xf = CTransform4f::Translate(from);
    CSporbPowerBomb* bomb =
        CreatePowerBomb(mgr, ProjectileInfo()->Token(), xf, ProjectileInfo()->GetDamage());
    if (bomb) {
      CProjectileWeapon& projectile = bomb->Projectile();
      projectile.SetVelocity(sTickTime * velocity);
      projectile.SetGravity(CVector3f(0.f, 0.f, -gravity * sTickTime));
      mgr.AddObject(*bomb);
    }
  }
}

float CSporbBase::GetGravity() const {
  float multiplier = 1.f;
  const uint stageCount = mStages.size();
  if (stageCount != 0 && mStageIndex < stageCount) {
    multiplier = mStages[mStageIndex]->mProjectileGravityMultiplier;
  }
  return 9.81f * multiplier;
}

CVector3f CSporbBase::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  if (const CSporbProjectile* projectile =
          TCastToConstPtr< CSporbProjectile >(mgr.GetObjectById(mProjectileId))) {
    if (mState == kS_Flailing || mState == kS_Spitting) {
      return mGrabberPosition;
    }
    return projectile->GetTranslation();
  }
  return GetTranslation();
}

CAABox CSporbBase::GetScanVisorRenderBounds(const CStateManager& mgr) const {
  CAABox bounds = GetModelBounds();
  if (mState != kS_FakeDead && mState != kS_Sleeping) {
    const CTransform4f locatorXf = GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
    const CTransform4f xf = CTransform4f::Translate(-GetTranslation()) * GetTransform() * locatorXf;
    if (const CSporbTop* top = TCastToConstPtr< CSporbTop >(mgr.GetObjectById(mTopId))) {
      const CAABox topBounds = top->GetModelBounds();
      bounds.Include(topBounds.GetTransformedAABox(CTransform4f::Translate(xf.GetTranslation())));
    }
  }
  return bounds;
}

void CSporbBase::ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                                 const CModelFlags& flags) const {
  const CTransform4f savedXf = GetTransform();
  const CModelFlags savedFlags = GetModelFlags();
  const_cast< CTransform4f& >(GetTransform()) = xf;
  const_cast< CSporbBase* >(this)->SetModelFlags(flags);
  const TUniqueId scanningId = mgr.GetPlayer(0)->GetScanningObject();
  CPatterned::Render(mgr);
  const_cast< CTransform4f& >(GetTransform()) = savedXf;
  const_cast< CSporbBase* >(this)->SetModelFlags(savedFlags);

  if (scanningId == GetUniqueId() && mState != kS_FakeDead && mState != kS_Sleeping) {
    const CTransform4f topXf = xf * GetScaledLocatorTransform(rstl::string_l(skConnectLocator));
    if (const CSporbTop* top = TCastToConstPtr< CSporbTop >(mgr.GetObjectById(mTopId))) {
      top->ScanVisorRender(mgr, topXf, flags);
    }
  }
}

CAABox CSporbBase::GetModelBounds() const {
  CAABox box = CAABox::MakeMaxInvertedBox();
  box = GetAnimationData()->CalcBoundingBoxFromModelVerts();
  box = box.GetTransformedAABox(CTransform4f::Translate(-GetTranslation()) * GetTransform() *
                                CTransform4f::Scale(GetModelData()->GetScale()));
  return box;
}

void CSporbBase::ClearTendrilParticles() {
  if (mTendrilGen.get()) {
    const CVector3f position = GetTranslation();
    const uint particleCount = mTendrilGen->GetParticleCount();
    for (uint i = 0; i < particleCount; ++i) {
      CElementGen::CParticle& particle = mTendrilGen->mParticles[i];
      particle.mPos = position;
      particle.mPrevPos = position;
    }
  }
}

CEntity* LoadSporbBase(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSporbBase sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSporbBase.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  rstl::vector< rstl::ownership_transfer< CPowerBombGuardianStageData > > stages;
  if (sldrThis.isPowerBombGuardian) {
    stages.reserve(4);
    stages.push_back_unsafe(rstl::ownership_transfer< CPowerBombGuardianStageData >(
        rs_new CPowerBombGuardianStageData(LdrToPowerBombGuardianStageData(sldrThis.stage1Data))));
    stages.push_back_unsafe(rstl::ownership_transfer< CPowerBombGuardianStageData >(
        rs_new CPowerBombGuardianStageData(LdrToPowerBombGuardianStageData(sldrThis.stage2Data))));
    stages.push_back_unsafe(rstl::ownership_transfer< CPowerBombGuardianStageData >(
        rs_new CPowerBombGuardianStageData(LdrToPowerBombGuardianStageData(sldrThis.stage3Data))));
    stages.push_back_unsafe(rstl::ownership_transfer< CPowerBombGuardianStageData >(
        rs_new CPowerBombGuardianStageData(LdrToPowerBombGuardianStageData(sldrThis.stage4Data))));
  }

  return rs_new CSporbBase(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.minTimeBetweenAttacks,
      sldrThis.maxTimeBetweenAttacks, sldrThis.minTimeBetweenShots, sldrThis.maxTimeBetweenShots,
      sldrThis.shotAngleVariance, sldrThis.grabberOutAcceleration, sldrThis.grabberInAcceleration,
      sldrThis.initialGrabberOutSpeed, sldrThis.minShotsInABurst, sldrThis.maxShotsInABurst,
      sldrThis.attackAimOffset, sldrThis.initialGrabberInSpeed, sldrThis.grabberAttachTime,
      sldrThis.minGrabberGrabTime, sldrThis.maxGrabberGrabTime, sldrThis.spitForce,
      sldrThis.tendrilParticleEffect, sldrThis.grabberFireSound, sldrThis.grabberFlightSound,
      sldrThis.grabberHitPlayerSound, sldrThis.grabberHitWorldSound, sldrThis.grabberRetractSound,
      sldrThis.grabberRetractMissedPlayerSound, sldrThis.morphballSpitSound,
      sldrThis.grabberExplosionSound, sldrThis.ballEscapeSound, sldrThis.needleTelegraphSound,
      sldrThis.grabberTelegraphSound, sldrThis.spitDamage, sldrThis.grabDamage,
      sldrThis.unknown_0x2cfade2c, sldrThis.maxGrabberGrabRange, sldrThis.minGrabberGrabRange,
      sldrThis.isPowerBombGuardian, sldrThis.powerBombProjectileParticleEffect,
      LdrToDamageInfo(sldrThis.powerBombProjectileDamage), sldrThis.maxPowerBombProjectileHeight,
      sldrThis.powerBombProjectileFuseTime, sldrThis.powerBombProjectileSound,
      sldrThis.unknown_0x48df4182, sldrThis.unknown_0xe39482ad,
      sldrThis.powerBombProjectileStartDamageTime, sldrThis.powerBombProjectileEndDamageTime,
      stages, sldrThis.unknown_0xdd8502cc, sldrThis.unknown_0x4ab8cf7d, sldrThis.unknown_0xf5e28404,
      sldrThis.powerBombProjectileDamageWaitTime);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SSporb_FuncPtrs funcPtrs;
  funcPtrs.mLoadNeedle = &LoadSporbNeedle;
  funcPtrs.mLoadProjectile = &LoadSporbProjectile;
  funcPtrs.mLoadBase = &LoadSporbBase;
  funcPtrs.mLoadTop = &LoadSporbTop;
  SetSSporb_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSporb_FuncPtrs(nullptr); }
#endif
