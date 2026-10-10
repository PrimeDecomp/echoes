#include "MetroidPrime/Enemies/CSandworm.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CGenericFSM2.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/Weapons/CBouncingBomb.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"
#include "rstl/algorithm.hpp"

static const char* const skBossName = "BossBombGuardian";          // Guessed name
static const char* const skSpitLocatorName = "acid_1_LCTR";        // Guessed name
static const char* const skFrontLeftClawName = "L_front_claw";     // Guessed name
static const char* const skFrontRightClawName = "R_front_claw";    // Guessed name
static const char* const skBackRightClawName = "R_back_claw";      // Guessed name
static const char* const skBackLeftClawName = "L_back_claw";       // Guessed name
static const char* const skTossLocatorName = "attach_LCTR";        // Guessed name
static const char* const skFrontEyeLocatorName = "eye_front_LCTR"; // Guessed name
static const char* const skBackEyeLocatorName = "eye_back_LCTR";   // Guessed name

const float CSandworm::skBlendWeightMin = 0.1f;
const float CSandworm::skBlendWeightMax = 0.5f;
const float CSandworm::skThresholdMin = 0.4f;
const float CSandworm::skThresholdMax = 1.2f;

static const rstl::string skRelName = rstl::string_l("Sandworm.rel");

static const CVector3f skSpitTargetOffset(0.f, 0.f, 1.5f);

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"Activated", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::Activated)},
    {"BeginUnderground",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::BeginUnderground)},
    {"CanAttack", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::CanAttack)},
    {"CanCharge", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::CanCharge)},
    {"CanDescend", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::CanDescend)},
    {"CanGrabMorphball",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::CanGrabMorphball)},
    {"CanSpitAgain",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::CanSpitAgain)},
    {"CanMeleeAgain",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::CanMeleeAgain)},
    {"ChargeOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::ChargeOver)},
    {"DoneLurking", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::DoneLurking)},
    {"DoneStraightening",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::DoneStraightening)},
    {"DoneSulking", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::DoneSulking)},
    {"FacingPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::FacingPlayer)},
    {"FacingPlayerForCharge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::FacingPlayerForCharge)},
    {"ForceUnderground",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::ForceUnderground)},
    {"GrabAttackFinished",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::GrabAttackFinished)},
    {"HitDuringWindUp",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::HitDuringWindUp)},
    {"InAttackRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::InAttackRange)},
    {"InChargeRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::InChargeRange)},
    {"InSpitRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::InSpitRange)},
    {"MeleeAttackRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::MeleeAttackRange)},
    {"OneEyeKilled",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::OneEyeKilled)},
    {"PickedBombFountain",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PickedBombFountain)},
    {"PickedBombToss",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PickedBombToss)},
    {"PickedBombSpread",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PickedBombSpread)},
    {"PickedSpitAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PickedSpitAttack)},
    {"PlayerHiding",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PlayerHiding)},
    {"PlayerIsMorphball",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PlayerIsMorphball)},
    {"PlayerIsOnPathMesh",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PlayerIsOnPathMesh)},
    {"PlayerReachable",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::PlayerReachable)},
    {"Primed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::Primed)},
    {"ReadyToCharge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::ReadyToCharge)},
    {"SequenceAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::SequenceAttack)},
    {"ShouldSulk", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::ShouldSulk)},
    {"SnatchStarted",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::SnatchStarted)},
    {"SnatchEnded", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::SnatchEnded)},
    {"SpitAngleOK", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSandworm::SpitAngleOK)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"BombFountainAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::BombFountainAttack)},
    {"BombSpreadAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::BombSpreadAttack)},
    {"BombTossAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::BombTossAttack)},
    {"ChargePlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::ChargePlayer)},
    {"ChargeWindUp", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::ChargeWindUp)},
    {"Deactivate", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Deactivate)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Dead)},
    {"Descend", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Descend)},
    {"EyeKilledReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::EyeKilledReaction)},
    {"GrabAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::GrabAttack)},
    {"LurkUnderground",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::LurkUnderground)},
    {"MeleeAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::MeleeAttack)},
    {"Null", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Null)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Patrol)},
    {"PickMediumAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::PickMediumAttack)},
    {"Prime", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Prime)},
    {"Pursue", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Pursue)},
    {"Rise", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Rise)},
    {"Snatch", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Snatch)},
    {"SpitAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::SpitAttack)},
    {"StopGrabAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::StopGrabAttack)},
    {"Straighten", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Straighten)},
    {"Sulk", static_cast< CPatterned::StateMachine::StateFunc >(&CSandworm::Sulk)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"Interrupt", static_cast< CPatterned::StateMachine::CodeFunc >(&CSandworm::Interrupt)},
};

static const SSpineSegment skSpineSegments[] = {
    {"front_spine_7", 2.4f, 1.f},  {"front_spine_6", 1.2f, 0.8f}, {"front_spine_5", 1.1f, 0.6f},
    {"front_spine_4", 1.1f, 0.6f}, {"front_spine_3", 1.1f, 0.6f}, {"front_spine_2", 1.1f, 0.6f},
    {"front_spine_1", 1.1f, 0.6f}, {"Skeleton_Root", 1.1f, 0.6f}, {"back_spine_1", 1.1f, 0.6f},
    {"back_spine_2", 1.1f, 0.6f},  {"back_spine_3", 1.1f, 0.6f},  {"back_spine_4", 1.1f, 0.6f},
    {"back_spine_5", 1.1f, 0.6f},  {"back_spine_6", 1.2f, 0.8f},  {"back_spine_7", 2.4f, 1.f},
};
static float GetGroundHeight(CStateManager& mgr, const CVector3f& position);
static float GetGroundHeight(CStateManager& mgr, const CVector3f& position);
static float GetGroundHeight(CStateManager& mgr, const CVector3f& position);

CSandworm::SBombFountainData::SBombFountainData(float damageThreshold, float unusedValue)
: mOrigin(CVector3f::Zero())
, mHitDuringWindUp(false)
, mBombLaunched(false)
, mEyeKilledDuringFountain(false)
, x10_(unusedValue)
, mStartHealth(0.f)
, mDamageThreshold(damageThreshold)
, mDamageTaken(0.f)
, mDamageFlashTimer(0.f)
, mThresholdHitTime(-1000.f) {}

CSandworm::CSandworm(
    TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
    const CModelData& modelData, const CPatternedInfo& patternedInfo, CAssetId stateMachine,
    float pincerScale, float spitAttackMinRange, float spitAttackMaxRange, float spitAimAngle,
    float chargeRangeMin, float chargeRangeMax, float chargeImpulseHorizontal,
    float chargeImpulseVertical, bool startsUnderground, CAssetId pincerL, CAssetId pincerR,
    ushort walkSound, ushort walkVocalSound, ushort meleeAttackSound, ushort eyeKilledSound,
    ushort bombBounceSound, ushort bombExplodeSound, CAssetId spitAttackVisorEffect,
    float morphballTossImpulseHorizontal, float morphballTossImpulseVertical,
    float meleeImpulseHorizontal, float meleeImpulseVertical, float sulkHealthDrop,
    float lurkUndergroundTimeMin, float lurkUndergroundTimeMax, float pursuitFrustrationRadius,
    float pursuitFrustrationTimer, CAssetId projectile, const CDamageInfo& projectileDamage,
    const CDamageInfo& morphballTossDamage, const CDamageInfo& pincerSwipeDamage, CAssetId eyeGlow,
    CAssetId particle1, CAssetId particle2, CAssetId bombEffect, CAssetId bombExplosionEffect,
    const CDamageInfo& bombDamage, float bombDropRate, float fountainDamageThreshold,
    float unusedFountainValue, const SLdrSandwormStruct& struct0, const SLdrSandwormStruct& struct1,
    const SLdrSandwormStruct& struct2, const SLdrSandwormStruct& struct3,
    const SLdrSandwormStruct& struct4, const CActorParameters& actorParams)
: CPatterned(kPAI_Sandworm, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mCollisionRadiusMode(0)
, mPathFindSearch(nullptr, 1 + (patternedInfo.GetIngPossessionData().isAnEncounter ? 0x300 : 0),
                  patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mTime(0.f)
, mPathDestination(CVector3f::Zero())
, mStartsUnderground(startsUnderground)
, mPatrolling(false)
, mFirstThink(true)
, mIsBoss(true)
, mBossEyeEffectsSpawned(false)
, mBossParamsSet(false)
, mLocatorsValid(false)
, mUnderground(false)
, mForceUnderground(false)
, mScanVisorActive(false)
, mSandwormStruct0(struct0)
, mSandwormStruct1(struct1)
, mSandwormStruct2(struct2)
, mSandwormStruct3(struct3)
, mSandwormStruct4(struct4)
, x964_(0)
, mSequenceStep(0)
, x96c_(-1000.f)
, mSpinePositions()
, mLastReverseTime(-1000.f)
, mAttackCooldown(0.f)
, mRiseTransform(CTransform4f::Identity())
, mPursueTime(0.f)
, mSpineCenterPosition(CVector3f::Zero())
, mSpineHeadPosition(CVector3f::Zero())
, mSpineTailPosition(CVector3f::Zero())
, mPursueStartPosition(CVector3f::Zero())
, mPincerTransforms(4, CTransform4f::Identity())
, mStateMachineToken(gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine)))
, mFrustrationTime(0.f)
, mPursuitFrustrationRadius(pursuitFrustrationRadius)
, mPursuitFrustrationTimer(pursuitFrustrationTimer)
, mDeathTime(0.f)
, mDying(false)
, mDeathDeleted(false)
, mHeadEyeId(kInvalidUniqueId)
, mTailEyeId(kInvalidUniqueId)
, mEyeKilledEffectsSpawned(0)
, mHeadEyeAnimation(-1)
, mTailEyeAnimation(-1)
, mEyeKillReactionDone(false)
, mChargeData(chargeRangeMin, chargeRangeMax, chargeImpulseHorizontal, chargeImpulseVertical)
, mSideStepTimer(0.f)
, mSideStepScale(0.f)
, mIsCharging(false)
, mSegmentRangeFirst(-1)
, mSegmentRangeLast(13)
, mSurfaced(false)
, mWasSurfaced(false)
, mSegmentBlendTarget(0.f)
, mSpitData(spitAttackMinRange, spitAttackMaxRange, spitAimAngle, projectile, projectileDamage,
            spitAttackVisorEffect)
, mMorphballTossData(morphballTossImpulseHorizontal, morphballTossImpulseVertical,
                     morphballTossDamage)
, mFrontEyePosition(CVector3f::Zero())
, mBackEyePosition(CVector3f::Zero())
, mMeleeData(pincerSwipeDamage, meleeImpulseHorizontal, meleeImpulseVertical)
, mSulkHealthDrop(sulkHealthDrop)
, mLurkUndergroundTimeMin(lurkUndergroundTimeMin)
, mLurkUndergroundTimeMax(lurkUndergroundTimeMax)
, mSulkHealthPercent(100.f)
, mSulkEndTime(0.f)
, mLastRiseTime(-1000.f)
, mSavedTurnSpeed(150.f)
, mTrackPlayerOnRise(false)
, mSpawnIndex(100000)
, mPincerL(pincerL)
, mPincerR(pincerR)
, mPincerScale(pincerScale)
, mSoundData(walkSound, walkVocalSound, meleeAttackSound, eyeKilledSound, bombBounceSound,
             bombExplodeSound)
, mFrontClawAngle(0.f)
, mBackClawAngle(0.f)
, mLastDamageTime(0.f)
, mCoverPointId(kInvalidUniqueId)
, mLastCoverPointId(kInvalidUniqueId)
, mGoToCover(false)
, mPlayerReachable(true)
, mPlayerReachableTime(-1000.f)
, mReachablePathSearch(nullptr, 1, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                       CPFRegion::kRP_Center)
, mBombData(bombEffect, bombExplosionEffect, bombDamage)
, mBombDropRate(bombDropRate)
, mBombDropTimer(0.f)
, mLastBombDropTime(-1000.f)
, mFountainData(fountainDamageThreshold, unusedFountainValue)
, mEyeEffects(eyeGlow, particle1, particle2)
, mCachedPlayerPosition(CVector3f::Zero())
, mCachedPlayerPositionTime(-1000.f)
, mDefaultTurnSpeed(0.f)
, mTurnSpeedTimer(0.f)
, mBombAimTimer(0.f)
, mCachedBounds(CAABox::MakeNullBox())
, mCachedBoundsTime(-1000.f) {
  Initialize();
}

CSandworm::SChargeData::SChargeData(float rangeMin, float rangeMax, float impulseH, float impulseV)
: mStartPosition(CVector3f::Zero())
, mRangeMin(rangeMin)
, mRangeMax(rangeMax)
, mImpulseHorizontal(impulseH)
, mImpulseVertical(impulseV)
, mTargetPosition(CVector3f::Zero())
, mSavedTurnSpeed(0.f)
, mStartTime(0.f)
, mReadyToCharge(false)
, mHitPlayer(false) {}

CSandworm::SSpitData::SSpitData(float minRange, float maxRange, float aimAngle, CAssetId projectile,
                                const CDamageInfo& damage, CAssetId visorEffect)
: mProjectileInfo(projectile, damage)
, mLocatorTransform(CTransform4f::Identity())
, mNextSpitTime(0.f)
, mMaxAimAngle(aimAngle)
, mMinRange(minRange)
, mMaxRange(maxRange)
, mOrigin(CVector3f::Zero())
, mVisorEffect(visorEffect != kInvalidAssetId
                   ? rstl::optional_object< TLockedToken< CGenDescription > >(
                         TLockedToken< CGenDescription >(
                             gpSimplePool->GetObj(SObjectTag('PART', visorEffect))))
                   : rstl::optional_object< TLockedToken< CGenDescription > >()) {}

CSandworm::SMorphballTossData::SMorphballTossData(float impulseH, float impulseV,
                                                  const CDamageInfo& damage)
: mLocatorTransform(CTransform4f::Identity())
, mDamage(damage)
, mImpulseHorizontal(impulseH)
, mImpulseVertical(impulseV)
, mLastTossTime(0.f)
, mHoldingPlayer(false)
, mTossFinished(false)
, mAttackFlagSet(false) {}

CSandworm::SMeleeData::SMeleeData(const CDamageInfo& damage, float impulseH, float impulseV)
: mDamage(damage)
, mDamageTimer(0.f)
, x20_(true)
, mImpulseHorizontal(impulseH)
, mImpulseVertical(impulseV) {}

CSandworm::SSpinePose::SSpinePose()
: mRootTranslation(CVector3f::Zero())
, mRootRotation(CQuaternion::NoRotation())
, mStoredTranslation(CVector3f::Zero())
, mStraightenBlend(0.f) {}

CSandworm::SSoundData::SSoundData(ushort walk, ushort walkVocal, ushort melee, ushort eyeKilled,
                                  ushort bombBounce, ushort bombExplode)
: mWalkSound(walk)
, mWalkVocalSound(walkVocal)
, mMeleeAttackSound(melee)
, mEyeKilledSound(eyeKilled)
, mBombBounceSound(bombBounce)
, mBombExplodeSound(bombExplode)
, mWalkSoundTimer(0.f)
, mWalkSoundInterval(0.f)
, mWalkHandle()
, mWalkVocalHandle()
, mMeleeHandle() {}

CSandworm::SBombFountainData::SBombFountainData()
: mOrigin(CVector3f::Zero())
, mHitDuringWindUp(false)
, mBombLaunched(false)
, mEyeKilledDuringFountain(false)
, x10_(0.f)
, mStartHealth(0.f)
, mDamageThreshold(0.f)
, mDamageTaken(0.f)
, mDamageFlashTimer(0.f)
, mThresholdHitTime(-1000.f) {}

CSandworm::CSandworm(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& modelData,
                     const CPatternedInfo& patternedInfo, CAssetId stateMachine, float pincerScale,
                     float spitAttackMinRange, float spitAttackMaxRange, float spitAimAngle,
                     float chargeRangeMin, float chargeRangeMax, float chargeImpulseHorizontal,
                     float chargeImpulseVertical, bool startsUnderground, CAssetId pincerL,
                     CAssetId pincerR, ushort walkSound, ushort walkVocalSound,
                     ushort meleeAttackSound, ushort eyeKilledSound, CAssetId spitAttackVisorEffect,
                     float morphballTossImpulseHorizontal, float morphballTossImpulseVertical,
                     float meleeImpulseHorizontal, float meleeImpulseVertical, float sulkHealthDrop,
                     float lurkUndergroundTimeMin, float lurkUndergroundTimeMax,
                     float pursuitFrustrationRadius, float pursuitFrustrationTimer,
                     CAssetId projectile, const CDamageInfo& projectileDamage,
                     const CDamageInfo& morphballTossDamage, const CDamageInfo& pincerSwipeDamage,
                     CAssetId eyeGlow, const CActorParameters& actorParams)
: CPatterned(kPAI_Sandworm, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mCollisionRadiusMode(0)
, mPathFindSearch(nullptr, 1 + (patternedInfo.GetIngPossessionData().isAnEncounter ? 0x300 : 0),
                  patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mTime(0.f)
, mPathDestination(CVector3f::Zero())
, mStartsUnderground(startsUnderground)
, mPatrolling(false)
, mFirstThink(true)
, mIsBoss(false)
, mBossEyeEffectsSpawned(true)
, mBossParamsSet(false)
, mLocatorsValid(false)
, mUnderground(false)
, mForceUnderground(false)
, mScanVisorActive(false)
, x964_(0)
, mSequenceStep(0)
, x96c_(-1000.f)
, mSpinePositions()
, mLastReverseTime(-1000.f)
, mAttackCooldown(0.f)
, mRiseTransform(CTransform4f::Identity())
, mPursueTime(0.f)
, mSpineCenterPosition(CVector3f::Zero())
, mSpineHeadPosition(CVector3f::Zero())
, mSpineTailPosition(CVector3f::Zero())
, mPursueStartPosition(CVector3f::Zero())
, mPincerTransforms(4, CTransform4f::Identity())
, mStateMachineToken(gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine)))
, mFrustrationTime(0.f)
, mPursuitFrustrationRadius(pursuitFrustrationRadius)
, mPursuitFrustrationTimer(pursuitFrustrationTimer)
, mDeathTime(0.f)
, mDying(false)
, mDeathDeleted(false)
, mHeadEyeId(kInvalidUniqueId)
, mTailEyeId(kInvalidUniqueId)
, mEyeKilledEffectsSpawned(0)
, mHeadEyeAnimation(-1)
, mTailEyeAnimation(-1)
, mEyeKillReactionDone(false)
, mChargeData(chargeRangeMin, chargeRangeMax, chargeImpulseHorizontal, chargeImpulseVertical)
, mSideStepTimer(0.f)
, mSideStepScale(0.f)
, mIsCharging(false)
, mSegmentRangeFirst(-1)
, mSegmentRangeLast(13)
, mSurfaced(false)
, mWasSurfaced(false)
, mSegmentBlendTarget(0.f)
, mSpitData(spitAttackMinRange, spitAttackMaxRange, spitAimAngle, projectile, projectileDamage,
            spitAttackVisorEffect)
, mMorphballTossData(morphballTossImpulseHorizontal, morphballTossImpulseVertical,
                     morphballTossDamage)
, mFrontEyePosition(CVector3f::Zero())
, mBackEyePosition(CVector3f::Zero())
, mMeleeData(pincerSwipeDamage, meleeImpulseHorizontal, meleeImpulseVertical)
, mSulkHealthDrop(sulkHealthDrop)
, mLurkUndergroundTimeMin(lurkUndergroundTimeMin)
, mLurkUndergroundTimeMax(lurkUndergroundTimeMax)
, mSulkHealthPercent(100.f)
, mSulkEndTime(0.f)
, mLastRiseTime(-1000.f)
, mSavedTurnSpeed(150.f)
, mTrackPlayerOnRise(false)
, mSpawnIndex(100000)
, mPincerL(pincerL)
, mPincerR(pincerR)
, mPincerScale(pincerScale)
, mSoundData(walkSound, walkVocalSound, meleeAttackSound, eyeKilledSound,
             CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId)
, mFrontClawAngle(0.f)
, mBackClawAngle(0.f)
, mLastDamageTime(0.f)
, mCoverPointId(kInvalidUniqueId)
, mLastCoverPointId(kInvalidUniqueId)
, mGoToCover(false)
, mPlayerReachable(true)
, mPlayerReachableTime(-1000.f)
, mReachablePathSearch(nullptr, 1, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                       CPFRegion::kRP_Center)
, mBombDropRate(0.f)
, mBombDropTimer(0.f)
, mLastBombDropTime(-1000.f)
, mEyeEffects(eyeGlow, kInvalidAssetId, kInvalidAssetId)
, mCachedPlayerPosition(CVector3f::Zero())
, mCachedPlayerPositionTime(-1000.f)
, mDefaultTurnSpeed(0.f)
, mTurnSpeedTimer(0.f)
, mBombAimTimer(0.f)
, mCachedBounds(CAABox::MakeNullBox())
, mCachedBoundsTime(-1000.f) {
  Initialize();
}

void CSandworm::Initialize() {
  SetDrawShadow(false);
  AnimationData()->SetKeepJSPose(true);
  CreateShadow();
  for (int i = 0; i < 12; ++i) {
    mSegmentLengths[i] = 0.f;
  }
  if (mPincerL != kInvalidAssetId) {
    mPincerModelL = CModelData(CStaticRes(mPincerL, GetModelData()->GetScale() * mPincerScale));
  }
  if (mPincerR != kInvalidAssetId) {
    mPincerModelR = CModelData(CStaticRes(mPincerR, GetModelData()->GetScale() * mPincerScale));
  }
  ClearFloatArray();
  mKnockBackController.EnableAllAnimReactions(false);
  mKnockBackController.EnableKnockBackPhysics(false);
  mKnockBackController.EnableBurn(false);
  for (int i = 0; i < 13; ++i) {
    mSpineSegIds.push_back(GetSpineSegId(i));
  }
  if (mChargeData.mImpulseHorizontal >= 50.f) {
    mChargeData.mImpulseHorizontal = 50.f;
  }
  if (mChargeData.mImpulseVertical >= 50.f) {
    mChargeData.mImpulseVertical = 50.f;
  }
  if (mMorphballTossData.mImpulseHorizontal >= 50.f) {
    mMorphballTossData.mImpulseHorizontal = 50.f;
  }
  if (mMorphballTossData.mImpulseVertical >= 50.f) {
    mMorphballTossData.mImpulseVertical = 50.f;
  }
  if (mMeleeData.mImpulseHorizontal >= 50.f) {
    mMeleeData.mImpulseHorizontal = 50.f;
  }
  if (mMeleeData.mImpulseVertical >= 50.f) {
    mMeleeData.mImpulseVertical = 50.f;
  }
}

CSandworm::~CSandworm() {}

void CSandworm::ClearFloatArray() {
  for (int i = 0; i < 13; ++i) {
    mSegmentBlendWeights[i] = 0.f;
  }
}

void CSandworm::SpawnEyeKilledEffects(CStateManager& mgr, bool headKilled) {
  float offset = mPincerScale * (0.5f * GetModelData()->GetScale().GetZ());
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state == kSS_SpawnResidue && it->msg == kSM_Attach) {
      TUniqueId id = mgr.GetIdForScript(it->objId);
      CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id));
      if (actor != nullptr) {
        if (mIsBoss == 1 && GetHealthInfo()->GetHP() > 0.5f * GetHealthInfo()->GetInitialHP()) {
          actor->SetTranslation(mBackEyePosition + CVector3f(0.f, 0.f, offset));
        } else {
          actor->SetTranslation(mFrontEyePosition + CVector3f(0.f, 0.f, offset));
        }
        CSfxManager::AddEmitter(mSoundData.mEyeKilledSound, actor->GetTranslation(),
                                GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
      }
    }
  }
  SendScriptMsgs(kSS_SpawnResidue, mgr, kSM_None);
}

void CSandworm::AddEyeAnimation(int index) {
  if (!(mPincerScale < 1.f) || index != 0) {
    uint* anim = index == 0 ? &mHeadEyeAnimation : &mTailEyeAnimation;
    if (*anim == -1) {
      int type = 10;
      if (index == 0) {
        type = 9;
      }
      const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(type));
      const CPASDatabase& database = BodyController()->GetPASDatabase();
      rstl::pair< float, int > best = database.FindBestAnimation(parms, -1);
      if (best.first > 0.f) {
        *anim = best.second;
        ModelData()->AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, false);
      }
    }
  }
}

void CSandworm::RemoveEyeAnimation(int index) {
  uint* anim = index == 0 ? &mHeadEyeAnimation : &mTailEyeAnimation;
  if (*anim != -1) {
    ModelData()->AnimationData()->DelAdditiveAnimation(*anim);
    *anim = -1;
  }
}

float CSandworm::GetHitAngle(const CTransform4f& xf) const {
  const CVector3f& tail = GetSpinePosition(12);
  const CVector3f& body = GetSpinePosition(11);
  CVector3f spineDirection = body - tail;
  CVector3f hitDirection = xf.GetForward();
  spineDirection.SetZ(0.f);
  hitDirection.SetZ(0.f);
  if (!hitDirection.CanBeNormalized() || spineDirection.CanBeNormalized() == 0) {
    return M_2PIF;
  }
  return CVector3f::GetAngleDiff(hitDirection.AsNormalized(), spineDirection.AsNormalized());
}

static EMaterialTypes sSolidMaterial = kMT_Solid; // Guessed name

void CSandworm::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_Create:
    if (mIsBoss == true) {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    } else {
      BodyController()->SetLocomotionType(pas::kLT_Internal7);
    }
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    RemoveMaterial(kMT_Orbit, mgr);
    SpawnEyes(mgr);
    mDefaultTurnSpeed = BodyController()->GetTurnSpeed();
    break;
  case kSM_Activate:
    if (mStartsUnderground == true) {
      mUnderground = true;
    }
    break;
  case kSM_Deactivate:
    if (mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->SetActive(mgr, false);
    }
    break;
  case kSM_Delete:
    if (mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->Destroy(mgr);
      mCollisionActorManager = nullptr;
    }
    if (mHeadEyeId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mHeadEyeId);
      mHeadEyeId = kInvalidUniqueId;
    }
    if (mTailEyeId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mTailEyeId);
      mTailEyeId = kInvalidUniqueId;
    }
    StopWalkVocalSound();
    break;
  case kSM_AreaLoaded:
    SetPathArea(mgr);
    while (mSpinePose.mRotations.size() < 13) {
      mSpinePose.mRotations.push_back(CQuaternion::NoRotation());
    }
    break;
  case kSM_ResistedDamage:
  case kSM_ReflectedDamage:
    if (TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId)) != nullptr) {
      mHitByPlayerProjectile = true;
    }
    break;
  case kSM_Damage:
    mHitByPlayerProjectile = true;
    if (!mSurfaced) {
      return;
    }
    const float health = GetHealthInfo()->GetHP();
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
    if (actor != nullptr) {
      uchar valid = 0;
      if (mAttackHistory.mRunningAttack == kA_BombFountain) {
        if (mFountainData.mBombLaunched == false) {
          valid = IsHeadCollisionActor(mgr, actor);
        }
      } else if (IsAnyEyeKilled(mgr) == 0) {
        if (IsIngControlled() == true) {
          valid = IsTailCollisionActor(mgr, actor);
        } else {
          valid = IsHeadCollisionActor(mgr, actor);
        }
      } else if (!IsIngControlled()) {
        valid = IsHeadCollisionActor(mgr, actor);
      }
      if (valid == true) {
        CHealthInfo* actorHealth = actor->HealthInfo();
        const TUniqueId weaponId = actor->GetLastTouchedObject();
        const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(weaponId));
        if (weapon != nullptr) {
          CDamageInfo damage = weapon->GetCurrentDamageInfo();
          damage.SetRadius(0.f);
          if (IsIngControlled() == true) {
            if (mAttackHistory.mRunningAttack == kA_BombFountain) {
              mgr.ApplyDamage(weaponId, GetUniqueId(), weaponId, damage,
                              CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial),
                                                                  CMaterialList()),
                              CVector3f::Zero());
            } else if (GetHitAngle(weapon->GetTransform()) < 75.f * (M_PIF / 180.f) &&
                       mAttackHistory.mRunningAttack == kA_Pursue) {
              const float amount = damage.GetDamage(*GetDamageVulnerability());
              mFountainData.mDamageTaken += amount;
              mFountainData.mDamageFlashTimer = CPatterned::skDamageHitTime;
              if (mFountainData.mDamageTaken > mFountainData.mDamageThreshold) {
                mAttackHistory.mCurrentAttack = kA_BombFountain;
              }
            }
          } else {
            if (IsAnyEyeKilled(mgr) == 0 &&
                health - damage.GetDamage(*GetDamageVulnerability()) <= 1.f) {
              damage.SetDamage(health - 1.f);
            }
            mgr.ApplyDamage(
                weaponId, GetUniqueId(), weaponId, damage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList()),
                CVector3f::Zero());
          }
          mLastDamageTime = mTime;
          if (IsAnyEyeKilled(mgr) == 0 && !IsIngControlled()) {
            const float maxHealth = GetHealthInfo()->GetInitialHP();
            if (GetHealthInfo()->GetHP() <= 0.5f * maxHealth) {
              const_cast< CSandwormEye* >(GetTailEye(mgr))->SetKilled();
              if (mAttackHistory.mRunningAttack == kA_BombFountain) {
                mFountainData.mEyeKilledDuringFountain = true;
              }
            }
          }
        } else {
          TakeDamage(CVector3f::Zero(), 10000.f - actorHealth->GetHP());
          mLastDamageTime = mTime;
        }
        actorHealth->SetHP(10000.f);
        if (mAttackHistory.mRunningAttack == kA_BombFountain &&
            mFountainData.mBombLaunched == false) {
          const float thresholds[3] = {120.f, 80.f, 40.f};
          for (int i = 0; i < 3; ++i) {
            if (mFountainData.mStartHealth >= thresholds[i]) {
              if (GetHealthInfo()->GetHP() < thresholds[i]) {
                mFountainData.mHitDuringWindUp = true;
                mFountainData.mThresholdHitTime = mTime;
                DeleteHeldBomb(mgr);
                break;
              }
            }
          }
        }
      }
    }
    break;
  case kSM_HitObject: {
    CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
    if (collisionActor != nullptr) {
      if (collisionActor->GetLastTouchedObject() == mgr.GetPlayer(0)->GetUniqueId()) {
        if (mCurDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
        if (mAttackHistory.LastEntriesEqual(kA_Charge, 1) == true) {
          ChargeHitPlayer(mgr);
        }
      }
    }
    break;
  }
  case kSM_InternalMessage0:
    if (mIsBoss == false) {
      mForceUnderground = true;
    }
    break;
  case kSM_Alert:
  case kSM_OffGround:
  case kSM_Launching:
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

void CSandworm::SetPathArea(CStateManager& mgr) {
  const TAreaId areaId = GetCurrentAreaId();
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(areaId);
  mPathFindSearch.SetArea(area.GetPostConstructed()->mPathArea);
  mReachablePathSearch.SetArea(area.GetPostConstructed()->mPathArea);
}

void CSandworm::Null(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    EnsureCollisionActors(mgr);
    mSurfaced = true;
    break;
  }
}

void CSandworm::Think(float dt, CStateManager& mgr) {
  mFirstThink = false;
  if (!mStateMachine->HasState()) {
    SetupStateMachineHelper(mgr);
  } else {
    if (mMorphballTossData.mHoldingPlayer == true) {
      TeleportPlayerToToss(mgr);
    }
    UpdateEyes(mgr, dt);
    if (GetActive()) {
      if (mCollisionActorManager.get() == nullptr) {
        CreateCollisionActors(mgr);
      }
      UpdateCollisionActors(mgr, dt);
      if (!mAlive) {
        mDeathTime += dt;
        UpdateEyeBlend(dt);
        UpdateSegmentWeights(dt);
        CPatterned::Think(dt, mgr);
      } else {
        mTime += dt;
        CPatterned::Think(dt, mgr);
        UpdatePose();
        UpdateSideStep(mgr, dt);
        UpdateAttackCooldown(dt);
        UpdateSegmentWeights(dt);
        UpdateMeleeDamage(mgr, dt);
        UpdateEyeBlend(dt);
        UpdateDamageFlash(dt);
        BoostNearbyProjectileHoming(mgr);
        UpdateSounds(mgr);
        if (mIsBoss == true) {
          if (mIngPossessionBlend > 0.99f) {
            UpdateBossState(mgr);
          }
        } else if (mPincerScale < 1.f) {
          float scale = 1.f;
          if (mLastReverseTime + 0.8f > mTime) {
            scale = (mTime - mLastReverseTime) / 0.8f;
          }
          mSpeed = scale;
        }
      }
    }
  }
}

void CSandworm::UpdatePose() {
  if (!mLocatorsValid) {
    ModelData()->AnimationData()->BuildPoseIfNecessary();
    UpdateLocators();
  }
}

void CSandworm::UpdateBossState(CStateManager& mgr) {
  if (!mBossEyeEffectsSpawned) {
    SpawnEyeKilledEffects(mgr, true);
    mBossEyeEffectsSpawned = true;
  }
  if (!mBossParamsSet) {
    mgr.SetBossParams(GetUniqueId(), GetHealthInfo()->GetInitialHP(),
                      gpStringTable->GetStringIndex(skBossName));
    mBossParamsSet = true;
  }
  mSpeed = GetPhaseStruct().moveSpeedMultiplier;
}

static EMaterialTypes sProjectileMaterial = kMT_Projectile; // Guessed name

void CSandworm::BoostNearbyProjectileHoming(CStateManager& mgr) {
  const CVector3f& position = GetTranslation();
  const CAABox bounds(position + CVector3f(-100.f, -100.f, -100.f),
                      position + CVector3f(100.f, 100.f, 100.f));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds,
                    CMaterialFilter::MakeInclude(CMaterialList(sProjectileMaterial)), this);
  for (const TUniqueId* it = nearList.begin(); it != nearList.end(); ++it) {
    CGameProjectile* projectile = TCastToPtr< CGameProjectile >(mgr.ObjectById(*it));
    if (projectile != nullptr) {
      projectile->SetHomingTurnRateScale(10.f);
    }
  }
}

void CSandworm::UpdateDamageFlash(float dt) {
  if (mFountainData.mDamageFlashTimer > 0.f) {
    mFountainData.mDamageFlashTimer = CMath::Max(0.f, mFountainData.mDamageFlashTimer - dt);
    const float t = CMath::Min(mFountainData.mDamageFlashTimer / CPatterned::skDamageHitTime, 1.f);
    const CColor& color = CColor::Lerp(CColor::Black(), CColor::Yellow(), t);
    mColor.SetRed(color.GetRedu8());
    mColor.SetGreen(color.GetGreenu8());
    mColor.SetBlue(color.GetBlueu8());
  }
}

void CSandworm::UpdateEyeBlend(float dt) {
  const float decay = 40.f * dt;
  mFrontClawAngle -= decay;
  if (mFrontClawAngle < 0.f) {
    mFrontClawAngle = 0.f;
  }
  mBackClawAngle -= decay;
  if (mBackClawAngle < 0.f) {
    mBackClawAngle = 0.f;
  }
}

float CSandworm::MoveToward(float current, float target, float step) {
  if (target < current) {
    return CMath::Max(target, current - step);
  }
  const float next = current + step;
  return next > target ? target : next;
}

void CSandworm::UpdateSegmentWeights(float dt) {
  const float step = 3.f * dt;
  for (int i = 0; i < 13; ++i) {
    if (i > mSegmentRangeFirst && i < mSegmentRangeLast) {
      mSegmentBlendWeights[i] = MoveToward(mSegmentBlendWeights[i], mSegmentBlendTarget, step);
    } else {
      mSegmentBlendWeights[i] = MoveToward(mSegmentBlendWeights[i], 0.f, step);
    }
  }
}

void CSandworm::UpdateSideStep(CStateManager& mgr, float dt) {
  mSideStepTimer += dt;
  const CVector3f destination = GetPatrolDestination(mgr);
  const float distance = CVector3f(destination.GetX() - GetTranslation().GetX(),
                                   destination.GetY() - GetTranslation().GetY(),
                                   destination.GetZ() - GetTranslation().GetZ())
                             .Magnitude();
  if (mIsCharging == true) {
    mSideStepScale = 1.f;
  } else if (distance > 25.f) {
    mSideStepScale = 1.f;
  } else {
    mSideStepScale = distance / 25.f;
  }
  const float threshold = GetSideStepThreshold();
  while (mSideStepTimer > threshold) {
    mSideStepTimer -= threshold;
  }
}

void CSandworm::UpdateSteeringSpeed() {
  const float remaining = mTime - mLastReverseTime;
  if (remaining > 1.2f) {
    BodyController()->CommandMgr().SetSteeringSpeedRange(0.f, 1.f);
  } else {
    BodyController()->CommandMgr().SetSteeringSpeedRange(0.f, remaining / 1.2f);
  }
}

CVector3f CSandworm::GetSegmentPosition(const CSegId& segId) const {
  const CTransform4f& xf = GetLctrTransform(segId);
  return CVector3f(xf.GetTranslation());
}

CVector3f CSandworm::GetSpineLocatorPosition(int index) const {
  CSegId segId;
  if (mLocatorSegIds.size() > 0) {
    segId = mLocatorSegIds[index + 1];
  } else {
    const CAnimData* animData = GetModelData()->GetAnimationData();
    const SSpineSegment* segment = &skSpineSegments[index + 1];
    segId = animData->GetLocatorSegId(rstl::string_l(segment->mName));
  }
  return GetSegmentPosition(segId);
}

CVector3f CSandworm::GetLocatorPosition(int index) const {
  CSegId segId;
  if (mLocatorSegIds.size() > 0) {
    segId = mLocatorSegIds[index];
  } else {
    const CAnimData* animData = GetModelData()->GetAnimationData();
    const SSpineSegment* segment = &skSpineSegments[index];
    segId = animData->GetLocatorSegId(rstl::string_l(segment->mName));
  }
  return GetSegmentPosition(segId);
}

float CSandworm::SumSegmentWeights(int count) const {
  float total = 0.f;
  for (int i = 0; i < count; ++i) {
    total += mSegmentLengths[i];
  }
  return total;
}

float CSandworm::GetTotalSegmentWeight() const { return SumSegmentWeights(12); }

int CSandworm::GetTrailIndexAtDistance(float distance) const {
  float traveled = 0.f;
  int index = mTrail.size() - 1;
  CVector3f previous = mTrail[index];
  while (traveled < distance) {
    if (index == 0) {
      return 0;
    }
    const CVector3f current = mTrail[index - 1];
    traveled += (current - previous).Magnitude();
    previous = current;
    --index;
  }
  return index;
}

CVector3f CSandworm::GetTrailPositionAtDistance(float distance) const {
  float traveled = 0.f;
  int index = mTrail.size() - 1;
  CVector3f previous = mTrail[index];
  while (traveled < distance) {
    if (index == 0) {
      return mTrail[0];
    }
    const CVector3f current = mTrail[index - 1];
    CVector3f offset = current - previous;
    const float length = offset.Magnitude();
    if (traveled + length > distance) {
      const float remaining = distance - traveled;
      offset.Normalize();
      offset *= remaining;
      return previous + offset;
    }
    previous = current;
    --index;
    traveled += length;
  }
  return previous;
}

CSandworm::STrailPoint CSandworm::FindTrailPoint(const STrailPoint& from, float distance) const {
  for (int i = from.mIndex - 1; i >= 0; --i) {
    if ((mTrail[i] - from.mPosition).Magnitude() > distance) {
      const int next = i + 1;
      const float nextX = mTrail[next].GetX();
      const float nextY = mTrail[next].GetY();
      const float nextZ = mTrail[next].GetZ();
      const float currentX = mTrail[i].GetX();
      const float currentY = mTrail[i].GetY();
      const float currentZ = mTrail[i].GetZ();
      float pointX = CVector3f::Zero().GetX();
      float pointY = CVector3f::Zero().GetY();
      float pointZ = CVector3f::Zero().GetZ();
      float t = 0.f;
      while (t <= 1.f) {
        const float inverse = 1.f - t;
        pointX = inverse * nextX + t * currentX;
        pointZ = inverse * nextZ + currentZ * t;
        pointY = inverse * nextY + t * currentY;
        const CVector3f offset(pointX - from.mPosition.GetX(), pointY - from.mPosition.GetY(),
                               pointZ - from.mPosition.GetZ());
        if (!(offset.Magnitude() > distance)) {
          t += 0.02f;
          continue;
        }
        break;
      }
      return STrailPoint(i, next, CVector3f(pointX, pointY, pointZ));
    }
  }
  return STrailPoint(0, 0, mTrail[0]);
}

CVector3f CSandworm::GetTrailPosition(int startIndex, int& outIndex, float distance) const {
  const CVector3f start = mTrail[startIndex];
  const float distanceSq = distance * distance;
  for (int i = startIndex - 1; i >= 0; --i) {
    CVector3f offset = mTrail[i] - start;
    if (distanceSq < offset.MagSquared()) {
      outIndex = i;
      offset.Normalize();
      offset *= distance;
      return start + offset;
    }
  }
  outIndex = 0;
  return mTrail[0];
}

CVector3f CSandworm::GetSpinePosition(int index) const {
  return mSpinePositions[index] + GetPincerOffset();
}

CVector3f CSandworm::GetPincerOffset() const {
  return CVector3f(0.f, 0.f, mPincerScale * GetAverageModelScale());
}

CVector3f CSandworm::GetLocalSpinePosition(int index) const {
  return GetTransform().TransposeMultiply(GetSpinePosition(index));
}

void CSandworm::UpdateTrail(CStateManager& mgr) {
  if (mSegmentRangeFirst <= 0 && !(mSegmentRangeLast < 12)) {
    const CVector3f& last = mTrail[mTrail.size() - 1];
    CVector3f delta = GetTranslation() - last;
    delta.SetZ(0.f);
    if (!(delta.Magnitude() < 0.001f)) {
      bool moved = false;
      mTrail.erase(mTrail.begin());
      const CVector3f& reference = mTrail[GetTrailIndexAtDistance(0.5f * GetTotalSegmentWeight())];
      const CVector3f toReference(GetTranslation().GetX() - reference.GetX(),
                                  GetTranslation().GetY() - reference.GetY(), 0.f);
      const float trailDistance =
          CVector3f(last.GetX() - reference.GetX(), last.GetY() - reference.GetY(), 0.f)
              .Magnitude();
      if (toReference.Magnitude() < trailDistance) {
        const float offsetX = GetTranslation().GetX() - last.GetX();
        const float offsetY = GetTranslation().GetY() - last.GetY();
        for (int i = mTrail.size() - 1; i >= 0; --i) {
          mTrail[i] += CVector3f(offsetX, offsetY, 0.f);
          moved = true;
        }
      }
      const CVector3f surface = GetSurfaceSamplePosition(mgr, GetTranslation());
      CVector3f point = surface;
      if (CMath::AbsF(point.GetZ() - last.GetZ()) > 0.05f) {
        if (point.GetZ() < last.GetZ()) {
          point.SetZ(last.GetZ() - 0.05f);
        } else {
          point.SetZ(0.05f + last.GetZ());
        }
      }
      mTrail.push_back(point);
      if (moved == true) {
        ResetSpineFacing();
      }
    }
  }
}

void CSandworm::ResetSpineFacing() {
  if (mSpinePositions.size() >= 3) {
    CVector3f forward = GetTransform().GetForward();
    const CVector3f head = GetSpinePosition(0);
    const CVector3f neck = GetSpinePosition(2);
    CVector3f direction = head - neck;
    forward.SetZ(0.f);
    direction.SetZ(0.f);
    if (direction.CanBeNormalized() && forward.CanBeNormalized()) {
      direction.Normalize();
      forward.Normalize();
      if (!(CVector3f::GetAngleDiff(direction, forward) < M_PIF / 4.f)) {
        SetTransform(CQuaternion::FromMatrix(
                         CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up()))
                         .BuildTransform4f(GetTranslation()));
      }
    }
  }
}

void CSandworm::UpdateSpinePositions() {
  while (mSpinePositions.size() < 13) {
    mSpinePositions.push_back(CVector3f::Zero());
  }
  mSpinePositions[0] = mTrail[mTrail.size() - 1];
  for (int i = 1; i < 13; ++i) {
    mSpinePositions[i] = GetTrailPositionAtDistance(SumSegmentWeights(i));
  }
}

void CSandworm::InitializeSpine(CStateManager& mgr) {
  if (mStartsUnderground != true) {
    UpdateSegmentLengths();
    while (mTrail.size() < 180) {
      mTrail.push_back(CVector3f::Zero());
    }
    CVector3f forward = GetTransform().GetForward();
    forward.SetZ(0.f);
    if (!forward.CanBeNormalized()) {
      forward = CVector3f::Forward();
    } else {
      forward.Normalize();
    }
    float speed;
    if (mIsBoss == true) {
      speed = 2.f;
    } else {
      speed = 1.1f;
    }
    const float length = speed * GetTotalSegmentWeight();
    const float step = length / 180.f;
    const float half = length * 0.5f;
    for (int i = 0; i < 180; ++i) {
      CVector3f position = GetTranslation() - forward * half;
      position += forward * (step * i);
      position -= GetPincerOffset();
      mTrail[i] = GetSurfaceSamplePosition(mgr, position);
    }
    mSurfaced = mWasSurfaced = true;
    mUnderground = false;
    UpdateSpine(mgr);
    UpdatePose();
  }
}

void CSandworm::Prime(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    InitializeSpine(mgr);
  }
}

void CSandworm::SetSegmentScale(const CSegId& segId, const CVector3f& scale) {
  AnimationData()->JointData().Scale(segId.val()) = scale;
}

bool CSandworm::Primed(CStateManager& mgr, const CTriggerData& data) const { return true; }

void CSandworm::UpdateSpine(CStateManager& mgr) {
  UpdateSegmentLengths();
  if (GetTotalSegmentWeight() != 0.f) {
    if (mSurfaced != mWasSurfaced) {
      if (mSurfaced == true) {
        BuildTrail(mgr);
        SetTranslation(GetSpineLocatorPosition(0) - GetPincerOffset());
      } else {
        const CVector3f position = mSpinePositions[6];
        if (0.f == mSpinePose.mStraightenBlend) {
          SetTranslation(position);
        }
      }
    }
    mWasSurfaced = mSurfaced;
    if (!mSurfaced) {
      if (mSpinePose.mStraightenBlend > 0.f) {
        ApplySpineRotations();
      }
    } else {
      UpdateTrail(mgr);
      UpdateSpinePositions();
      PoseSpine();
      StoreTranslation();
    }
  }
}

void CSandworm::ApplySpineRotations() {
  CAnimData* animData = ModelData()->AnimationData();
  const CCharLayoutInfo* layout = animData->GetCharLayoutInfo();
  CJointData_LinearStorage& joints = animData->JointData();
  const CSegId rootId = GetBodySegId(6);
  joints.Translation(rootId.val()) = mSpinePose.mRootTranslation;
  joints.Rotation(rootId.val()) = mSpinePose.mRootRotation;
  for (int i = 0; i < 13; ++i) {
    if (i <= 4 || i >= 8) {
      const CQuaternion rotation = CQuaternion::Slerp(
          CQuaternion::NoRotation(), mSpinePose.mRotations[i], mSpinePose.mStraightenBlend);
      joints.Rotation(GetBodySegId(i).val()) = rotation;
    }
  }
  animData->Pose().BuildPose(*layout, joints);
  animData->SetPoseBuilt(true);
  const CVector3f delta = mSpinePositions[6] - GetSpineLocatorPosition(6);
  SetTranslation(GetTranslation() + delta + GetPincerOffset());
}

void CSandworm::StoreTranslation() { mSpinePose.mStoredTranslation = GetTranslation(); }

void CSandworm::RotateAroundPivot(CStateManager& mgr, CVector3f target, CVector3f& position,
                                  float speed, float dt) {
  CVector3f toTarget = target - mSpineCenterPosition;
  toTarget.SetZ(0.f);
  CVector3f toCurrent = mSpineHeadPosition - mSpineCenterPosition;
  toCurrent.SetZ(0.f);
  if (toTarget.CanBeNormalized() && toCurrent.CanBeNormalized()) {
    toTarget.Normalize();
    toCurrent.Normalize();
    float angle = dt * ((M_PIF / 180.f) * speed);
    const CVector3f cross = CVector3f::Cross(toTarget, toCurrent);
    if (cross.GetZ() > 0.f) {
      angle = angle * -1.f;
    }
    const float diff = CVector3f::GetAngleDiff(toTarget, toCurrent);
    if (diff < 0.2f) {
      angle *= diff / 0.2f;
    }
    const CRelAngle rotation = CRelAngle::FromRadians(angle);
    const CTransform4f rotationXf = CTransform4f::RotateZ(rotation);
    for (int i = 0; i < mTrail.size(); ++i) {
      CVector3f offset = mTrail[i] - mSpineCenterPosition;
      offset.SetZ(0.f);
      const CVector3f& rotated = rotationXf.Rotate(offset);
      mTrail[i] = CVector3f(rotated.GetX(), rotated.GetY(),
                            mTrail[i].GetZ() - mSpineCenterPosition.GetZ()) +
                  mSpineCenterPosition;
    }
    CTransform4f orientation = GetTransform().GetRotation();
    orientation.RotateLocalZ(rotation);
    CVector3f newPosition = rotationXf.Rotate(GetTranslation() - mSpineCenterPosition);
    newPosition += mSpineCenterPosition;
    newPosition.SetZ(GetSurfaceHeight(mgr, newPosition));
    CTransform4f xf = GetTransform();
    xf.SetTranslation(newPosition);
    xf.SetRotation(orientation.GetRotation());
    SetTransform(xf);
    const CVector3f& rotatedPosition = rotationXf.Rotate(position - mSpineCenterPosition);
    position = rotatedPosition;
    position += mSpineCenterPosition;
  }
}

void CSandworm::PoseSpine() {
  rstl::reserved_vector< CVector3f, 13 > locals;
  for (int i = 0; i < 13; ++i) {
    locals.push_back(GetLocalSpinePosition(i));
  }
  CAnimData* animData = ModelData()->AnimationData();
  const CCharLayoutInfo* layout = animData->GetCharLayoutInfo();
  animData->BuildPoseIfNecessary();
  const float inverseScale = 1.f / ModelData()->GetScale().GetX();
  CJointData_LinearStorage& joints = animData->JointData();
  const CVector3f rootPosition(inverseScale * locals[6].GetX(), inverseScale * locals[6].GetY(),
                               inverseScale * locals[6].GetZ());
  joints.Translation(GetBodySegId(6).val()) = rootPosition;
  const CUnitVector3f toHead(CVector3f(locals[4].GetX() - locals[8].GetX(),
                                       locals[4].GetY() - locals[8].GetY(),
                                       locals[4].GetZ() - locals[8].GetZ()));
  const CUnitVector3f forward(static_cast< const CVector3f& >(CVector3f::Forward()));
  const CRelAngle rootAngle = CRelAngle::FromRadians(M_2PIF);
  const CQuaternion rootRotation = CQuaternion::LookAt(forward, toHead, rootAngle);
  joints.Rotation(GetBodySegId(6).val()) = rootRotation;
  SetSpineRotation(6, rootRotation);
  mSpinePose.mRootTranslation = GetLocalSpinePosition(6);
  mSpinePose.mRootRotation = rootRotation;

  CQuaternion accumulated = rootRotation;
  for (int i = 8; i < 13; ++i) {
    const CQuaternion inverse = accumulated.BuildInverted();
    int next = i;
    if (i < 12) {
      next = i + 1;
    }
    const CVector3f direction =
        inverse.Transform(CVector3f(locals[next].GetX() - locals[next - 1].GetX(),
                                    locals[next].GetY() - locals[next - 1].GetY(),
                                    locals[next].GetZ() - locals[next - 1].GetZ()));
    const CVector3f offset = layout->GetFromParentUnrotated(GetBodySegId(next));
    const CQuaternion rotation = CQuaternion::LookAt(
        CUnitVector3f(offset), CUnitVector3f(direction), CRelAngle::FromRadians(M_2PIF));
    const CSegId segId = GetBodySegId(i);
    const float weight = mSegmentBlendWeights[i];
    if (1.f == weight) {
      joints.Rotation(segId.val()) = rotation;
    } else if (weight > 0.f) {
      joints.Rotation(segId.val()) =
          CQuaternion::Slerp(joints.Rotation(segId.val()), rotation, weight);
    }
    SetSpineRotation(i, joints.Rotation(segId.val()));
    accumulated = accumulated * rotation;
  }

  accumulated = rootRotation;
  for (int i = 4; i >= 0; --i) {
    const CQuaternion inverse = accumulated.BuildInverted();
    int next = i;
    if (i > 0) {
      next = i - 1;
    }
    const CVector3f direction =
        inverse.Transform(CVector3f(locals[next].GetX() - locals[next + 1].GetX(),
                                    locals[next].GetY() - locals[next + 1].GetY(),
                                    locals[next].GetZ() - locals[next + 1].GetZ()));
    const CVector3f offset = layout->GetFromParentUnrotated(GetBodySegId(next));
    const CQuaternion rotation = CQuaternion::LookAt(
        CUnitVector3f(offset), CUnitVector3f(direction), CRelAngle::FromRadians(M_2PIF));
    const CSegId segId = GetBodySegId(i);
    const float weight = mSegmentBlendWeights[i];
    if (1.f == weight) {
      joints.Rotation(segId.val()) = rotation;
    } else if (weight > 0.f) {
      joints.Rotation(segId.val()) =
          CQuaternion::Slerp(joints.Rotation(segId.val()), rotation, weight);
    }
    SetSpineRotation(i, joints.Rotation(segId.val()));
    accumulated = accumulated * rotation;
  }
  if (mDying == true) {
    UpdateDeathScale();
  }
  animData->Pose().BuildPose(*layout, joints);
  animData->SetPoseBuilt(true);
}

void CSandworm::SetSpineRotation(int index, const CQuaternion& rotation) {
  mSpinePose.mRotations[index] = rotation;
}

CSegId CSandworm::GetBodySegId(int index) const { return mSpineSegIds[index]; }

CSegId CSandworm::GetSpineSegId(int index) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  const SSpineSegment* segment = &skSpineSegments[index + 1];
  return animData->GetLocatorSegId(rstl::string_l(segment->mName));
}

CVector3f CSandworm::GetMorphballTossPosition(float distance) const {
  CVector3f position = GetTransform().GetForward();
  position.Normalize();
  position *= distance;
  position += mMorphballTossData.mLocatorTransform.GetTranslation();
  return position;
}

void CSandworm::Render(const CStateManager& mgr) const {
  if (mDeathDeleted != true) {
    CPatterned::Render(mgr);
    if (mEyeEffects.mGlow.mGenerator.get() != nullptr &&
        mEyeEffects.mGlow.mGenerator->GetParticleEmission() == true) {
      mEyeEffects.mGlow.mGenerator->Render();
      if (mEyeEffects.mEffect1.mGenerator.get() != nullptr &&
          mEyeEffects.mEffect1.mGenerator->GetParticleEmission() == true) {
        mEyeEffects.mEffect1.mGenerator->Render();
      }
    } else {
      if (mEyeEffects.mEffect2.mGenerator.get() != nullptr &&
          mEyeEffects.mEffect2.mGenerator->GetParticleEmission() == true) {
        mEyeEffects.mEffect2.mGenerator->Render();
      }
      if (mEyeEffects.mEffect3.mGenerator.get() != nullptr &&
          mEyeEffects.mEffect3.mGenerator->GetParticleEmission() == true) {
        mEyeEffects.mEffect3.mGenerator->Render();
      }
    }
    const CPlane plane(GetTranslation() + CVector3f(0.f, 0.f, -1000.f), CVector3f::Up());
    GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), plane);
    if (IsRemainingEyeAlive(mgr) == true) {
      RenderPincer(mgr, *mPincerModelL, mPincerTransforms[0]);
      RenderPincer(mgr, *mPincerModelR, mPincerTransforms[1]);
    }
    if (IsActiveEyeAlive(mgr) == true) {
      RenderPincer(mgr, *mPincerModelL, mPincerTransforms[2]);
      RenderPincer(mgr, *mPincerModelR, mPincerTransforms[3]);
    }
  }
}

bool CSandworm::ShouldReactToEyeKill(const CStateManager& mgr) const {
  if (IsAnyEyeKilled(mgr) == 1) {
    if (!IsIngControlled()) {
      if (!mEyeKillReactionDone) {
        return true;
      }
    } else if (!mEyeKillReactionDone && mFountainData.mEyeKilledDuringFountain == 1) {
      return true;
    }
  }
  return false;
}

bool CSandworm::IsRemainingEyeAlive(const CStateManager& mgr) const {
  if (ShouldReactToEyeKill(mgr) == 1) {
    return !GetTailEye(mgr)->IsKilled();
  }
  return !GetHeadEye(mgr)->IsKilled();
}

bool CSandworm::IsActiveEyeAlive(const CStateManager& mgr) const {
  if (mIsBoss == 1 && (mBossEyeEffectsSpawned == 1 || IsIngControlled() == 1)) {
    return false;
  }
  if (ShouldReactToEyeKill(mgr) == 1) {
    return !GetHeadEye(mgr)->IsKilled();
  }
  return !GetTailEye(mgr)->IsKilled();
}

const CSandwormEye* CSandworm::GetHeadEye(const CStateManager& mgr) const {
  return TCastToConstPtr< CSandwormEye >(mgr.GetObjectById(mHeadEyeId));
}

const CSandwormEye* CSandworm::GetTailEye(const CStateManager& mgr) const {
  return TCastToConstPtr< CSandwormEye >(mgr.GetObjectById(mTailEyeId));
}

CSandwormEye* CSandworm::GetHeadEye(CStateManager& mgr) const {
  return TCastToPtr< CSandwormEye >(mgr.ObjectById(mHeadEyeId));
}

CSandwormEye* CSandworm::GetTailEye(CStateManager& mgr) const {
  return TCastToPtr< CSandwormEye >(mgr.ObjectById(mTailEyeId));
}

static float GetGroundHeight(CStateManager& mgr, const CVector3f& position) {
  static CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f start = position + CVector3f(0.f, 0.f, 5.f);
  const CVector3f direction(0.f, 0.f, -1.f);
  const CRayCastResult result =
      CGameCollision::RayStaticIntersection(mgr, start, direction, 20.f, filter);
  if (!result.IsValid()) {
    return position.GetZ();
  }
  return result.GetPoint().GetZ();
}

float CSandworm::GetClawGroundAngle(CStateManager& mgr, const char* name, bool isBack) const {
  if (mDying == true) {
    return 0.f;
  }
  CTransform4f xf = GetLctrTransform(rstl::string_l(name));
  if (isBack == true) {
    xf.RotateLocalZ(CRelAngle::FromRadians(M_PIF));
    return 0.f;
  }
  const float groundHeight = GetGroundHeight(mgr, xf.GetTranslation() + xf.GetForward() * 2.4f);
  const CVector3f& tip = xf.GetTranslation() + xf.GetForward() * 2.4f;
  const CVector3f& middle = xf.GetTranslation() + xf.GetForward() * 2.4f * 0.5f;
  float angle = 0.f;
  if (-0.6f + tip.GetZ() < groundHeight) {
    for (float i = 1.f; i < 30.f; i += 1.f) {
      xf.RotateLocalX(CRelAngle::FromDegrees(1.f));
      angle += 1.f;
      const CVector3f& nextTip = xf.GetTranslation() + xf.GetForward() * 2.4f;
      const CVector3f& nextMiddle = xf.GetTranslation() + xf.GetForward() * 2.4f * 0.5f;
      if (-0.6f + nextTip.GetZ() > groundHeight) {
        break;
      }
    }
  }
  return angle;
}

CRelAngle CSandworm::GetShakeAngle(float time, float frequency, float amplitude) const {
  const float swing = static_cast< float >(sin(time * frequency));
  return CRelAngle::FromDegrees(swing * ((0.7f - time) / 0.7f) * amplitude);
}

void CSandworm::RenderPincer(const CStateManager& mgr, const CModelData& model,
                             const CTransform4f& xf) const {
  model.Render(mgr, xf, GetActorLights(), GetModelFlags());
}

CTransform4f CSandworm::GetClawTransform(CStateManager& mgr, const char* name, bool isBack,
                                         bool isRight, float angle) const {
  CTransform4f xf = GetLctrTransform(rstl::string_l(name));
  if (isBack == true) {
    xf.RotateLocalZ(CRelAngle::FromRadians(M_PIF));
  }
  xf.RotateLocalX(CRelAngle::FromDegrees(angle));
  if (!GetAlive()) {
    float progress;
    if (mDeathTime > 3.f) {
      progress = 1.f;
    } else {
      progress = mDeathTime / 3.f;
    }
    xf.ScaleBy(0.6f * (1.f - progress) + 0.4f);
  } else {
    const float elapsed = mTime - mLastDamageTime;
    if (elapsed < 0.7f) {
      const float direction = isRight == true ? -1.f : 1.f;
      xf.RotateLocalZ(GetShakeAngle(elapsed, 40.f, 3.f * direction));
      xf.RotateLocalX(GetShakeAngle(elapsed, 60.f, 5.f * direction));
    } else if (isBack == true && CanDropBomb(mgr) == true) {
      const float rate = mBombDropRate;
      float twist = 0.f;
      float window = 0.2f;
      if (0.5 * rate < window) {
        window = 0.5f * rate;
      }
      const float sinceDrop = mTime - mLastBombDropTime;
      if (sinceDrop < window) {
        twist = 1.f - sinceDrop / window;
      } else {
        const float untilDrop = rate - mBombDropTimer;
        if (untilDrop < window) {
          twist = 1.f - untilDrop / window;
        }
      }
      if (twist > 0.f) {
        twist *= 90.f;
        if (isRight == true) {
          twist *= -1.f;
        }
        xf.RotateLocalZ(CRelAngle::FromDegrees(twist));
      }
    }
  }
  return xf;
}

float CSandworm::GetSurfaceHeight(CStateManager& mgr, const CVector3f& position) const {
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  CVector3f start(position.GetX(), position.GetY(), position.GetZ() + 2.f);
  CRayCastResult result =
      CGameCollision::RayStaticIntersection(mgr, start, CVector3f(0.f, 0.f, -1.f), 20.f, filter);
  if (result.IsValid() == true) {
    return result.GetPoint().GetZ();
  }
  start.SetZ(position.GetZ() + 6.f);
  result =
      CGameCollision::RayStaticIntersection(mgr, start, CVector3f(0.f, 0.f, -1.f), 40.f, filter);
  if (result.IsValid() == true) {
    return result.GetPoint().GetZ();
  }
  return position.GetZ();
}

const CGenericFSM2* CSandworm::GetStateMachine() const {
  if (!mStateMachineToken->IsLoaded()) {
    return nullptr;
  }
  TToken< CGenericFSM2 > machine(*mStateMachineToken);
  return *machine;
}

void CSandworm::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  mStateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  mStateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

void CSandworm::UpdateCollisionActors(CStateManager& mgr, float dt) {
  if (mCollisionActorManager.get() != nullptr) {
    if (GetActive() == false) {
    } else {
      SetCollisionRadiusMode(mgr, mgr.GetPlayer(0)->GetMorphballTransitionState() ==
                                      CPlayer::kMS_Morphed);
      CVector3f offset;
      if (mSpinePositions.size() > 6) {
        offset = GetSpinePosition(6) - GetSpineLocatorPosition(6);
      } else {
        offset = GetSpineLocatorPosition(6);
      }
      for (uint i = 0; i < 15; ++i) {
        CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(
            mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
        if (actor != nullptr) {
          actor->SetActive(true);
          if (mSurfaced == true) {
            actor->MoveToWR(GetLocatorPosition(i) + offset, dt);
          } else {
            actor->MoveToWR(GetLocatorPosition(i), dt);
          }
        }
      }
    }
  }
}

static EMaterialTypes sCollisionIncludeMaterial = kMT_Solid;           // Guessed name
static EMaterialTypes sCollisionExcludeMaterial0 = kMT_CollisionActor; // Guessed name
static EMaterialTypes sCollisionExcludeMaterial1 = kMT_AIPassthrough;  // Guessed name
static EMaterialTypes sCollisionExcludeMaterial2 = kMT_Player;         // Guessed name

void CSandworm::AddJointCollisions(const SSpineSegment* segments, int count,
                                   rstl::vector< CJointCollisionDescription >& descriptions) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId segId = animData->GetLocatorSegId(rstl::string_l(segments[i].mName));
    if (segId != CSegId::Invalid()) {
      CJointCollisionDescription description = CJointCollisionDescription::SphereCollision(
          segId, CVector3f::Zero(), segments[i].mRadius * GetAverageModelScale(),
          rstl::string_l(segments[i].mName), 10000.f);
      descriptions.push_back(description);
    }
  }
}

void CSandworm::CreateCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descriptions;
  descriptions.reserve(15);
  AddJointCollisions(skSpineSegments, 15, descriptions);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(sCollisionIncludeMaterial),
                                                        CMaterialList(sCollisionExcludeMaterial0,
                                                                      sCollisionExcludeMaterial1,
                                                                      sCollisionExcludeMaterial2)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  AddMaterial(kMT_RadarObject, mgr);
  mCollisionActorManager = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(),
                                                         descriptions, GetActive());
  for (uint i = 0; i < 15; ++i) {
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(
        mgr.ObjectById(mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    CHealthInfo* health = actor->HealthInfo();
    health->SetKnockbackResistance(100000.f);
    health->SetHP(10000.f);
    if (i == 14) {
      actor->SetDamageVulnerability(GetDamageVulnerability()->MakeIgnoreRadius());
    } else {
      actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
    }
  }
  SetMass(20000.f);
  RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (int i = 0; i < 15; ++i) {
    const CSegId segId = animData->GetLocatorSegId(rstl::string_l(skSpineSegments[i].mName));
    mLocatorSegIds.push_back(segId);
  }
}

void CSandworm::SetCollisionRadiusMode(CStateManager& mgr, int mode) {
  if (mCollisionActorManager.get() != nullptr && mIsBoss != true) {
    if (HasUnknown11Hint(mgr) != true && mode != mCollisionRadiusMode) {
      for (int i = 0; i < 15; ++i) {
        CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(
            mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
        if (actor != nullptr) {
          if (mode == 1) {
            actor->SetSphereRadius(skSpineSegments[i].mScale);
          } else {
            actor->SetSphereRadius(skSpineSegments[i].mRadius);
          }
        }
      }
      mCollisionRadiusMode = mode;
    }
  }
}

void CSandworm::SetEyeVulnerable(CStateManager& mgr, int index, uchar vulnerable) {
  if (mCollisionActorManager.get() != nullptr) {
    const CJointCollisionDescription& desc =
        mCollisionActorManager->GetCollisionDescFromIndex(index == 0 ? 0 : 14);
    CCollisionActor* actor =
        TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId()));
    if (vulnerable == true) {
      actor->SetDamageVulnerability(*GetDamageVulnerability());
    } else {
      actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
    }
  }
}

CVector3f CSandworm::GetSurfaceSamplePosition(CStateManager& mgr, const CVector3f& position) const {
  CVector3f forward = GetTransform().GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized() == true) {
    forward.Normalize();
  }
  const float forwardY = forward.GetY();
  const float forwardX = forward.GetX();
  const float right = -forwardX;
  const float frontHeight =
      GetSurfaceHeight(mgr, CVector3f(position.GetX() + -forwardY, position.GetY() + forwardX,
                                      position.GetZ() + 0.f));
  const float backHeight = GetSurfaceHeight(
      mgr, CVector3f(position.GetX() + forwardY, position.GetY() + right, position.GetZ() + 0.f));
  return CVector3f(position.GetX(), position.GetY(), (frontHeight + backHeight) * 0.5f);
}

void CSandworm::UpdateSplineSegments(CStateManager& mgr) {
  while (mTrail.size() < 180) {
    mTrail.push_back(CVector3f::Zero());
  }
  CVector3f position = GetTranslation();
  mTrail[mTrail.size() - 1] = position;
  for (int i = mTrail.size() - 2; i >= 0; --i) {
    mTrail[i] = GetSurfaceSamplePosition(mgr, position);
    position += CVector3f(0.f, -0.5f, 0.f);
  }
}

void CSandworm::UpdateSegmentLengths() {
  if (!(GetTotalSegmentWeight() > 0.f)) {
    const float scale = ModelData()->GetScale().GetX();
    const CCharLayoutInfo* layout = ModelData()->AnimationData()->GetCharLayoutInfo();
    for (int i = 0; i < 6; ++i) {
      mSegmentLengths[i] = scale * layout->GetFromParentUnrotated(GetBodySegId(i)).Magnitude();
    }
    for (int i = 7; i < 12; ++i) {
      mSegmentLengths[i] = scale * layout->GetFromParentUnrotated(GetBodySegId(i + 1)).Magnitude();
    }
  }
}

CTransform4f CSandworm::GetFarthestSpawnTransform(CStateManager& mgr, int& index) {
  const CPlayer* player = mgr.GetPlayer(0);
  float farthestDistance = 0.f;
  CActor* farthest = nullptr;
  int farthestIndex = 0;
  int i = 0;
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state == kSS_GeneratorConnection && it->msg == kSM_Next) {
      TUniqueId id = mgr.GetIdForScript(it->objId);
      CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id));
      if (actor != nullptr && i != index) {
        const float distance = (player->GetTranslation() - actor->GetTranslation()).MagSquared();
        if (distance > farthestDistance) {
          farthestDistance = distance;
          farthest = actor;
          farthestIndex = i;
        }
        ++i;
      }
    }
  }
  CTransform4f xf = mRiseTransform;
  if (farthest != nullptr) {
    xf.SetTranslation(farthest->GetTranslation());
    index = farthestIndex;
  }
  return xf;
}

void CSandworm::LurkUnderground(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->SetLocomotionType(pas::kLT_Internal7);
  if (msg == kStateMsg_Activate) {
    SetTransform(GetFarthestSpawnTransform(mgr, mSpawnIndex));
  }
}

bool CSandworm::BeginUnderground(CStateManager& mgr, const CTriggerData& data) const {
  return mStartsUnderground;
}

bool CSandworm::Activated(CStateManager& mgr, const CTriggerData& data) const {
  return GetActive();
}

bool CSandworm::InAttackRange(CStateManager& mgr, const CTriggerData& data) const {
  const float maxAttackRange = mMaxAttackRange;
  const float distSq = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
  return distSq > 0.f && distSq < maxAttackRange * maxAttackRange;
}

bool CSandworm::InSpitRange(CStateManager& mgr, const CTriggerData& data) const {
  const float rangeMin = mSpitData.mMinRange;
  float rangeMax = mSpitData.mMaxRange;
  const float distSq = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
  if (UpdatePlayerReachable(mgr) == 0) {
    rangeMax *= 2.f;
  }
  return distSq > rangeMin * rangeMin && distSq < rangeMax * rangeMax;
}

bool CSandworm::CanGrabMorphball(CStateManager& mgr, const CTriggerData& data) const {
  if (mMorphballTossData.mTossFinished == 1 || 1.2f + mMorphballTossData.mLastTossTime > mTime ||
      mAttackHistory.LastEntriesEqual(kA_GrabAttack, 3) == 1 || IsNearHint(mgr) == 1 ||
      mForceUnderground == 1) {
    return false;
  }
  if (!mIsBoss && HasUnknown11Hint(mgr) == 1) {
    return false;
  }
  if (0.3f + mLastReverseTime > mTime) {
    return false;
  }
  return IsPlayerInTossRange(mgr, 0.f);
}

bool CSandworm::IsPlayerInTossRange(CStateManager& mgr, float distance) const {
  CPlayer* player = mgr.GetPlayer(0);
  if ((player->GetSpawnedMorphballState() == 0 ? player->GetMorphballTransitionState()
                                               : CPlayer::kMS_Unmorphed) != CPlayer::kMS_Morphed) {
    return false;
  }
  const CVector3f tossPosition = GetMorphballTossPosition(distance);
  return !((mgr.GetPlayer(0)->GetTranslation() - tossPosition).MagSquared() > 6.25f);
}

void CSandworm::GrabAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_GrabAttack, msg);
  CPlayer* player = mgr.GetPlayer(0);
  if (msg == kStateMsg_Activate) {
    SetSegmentRange(3, 13);
    mSurfaced = true;
    mMorphballTossData.mHoldingPlayer = false;
    mMorphballTossData.mTossFinished = false;
    player->EnableLeaveMorphBall(false);
  } else if (msg == kStateMsg_Deactivate) {
    if (mMorphballTossData.mHoldingPlayer == true) {
      ReleasePlayer(mgr);
    }
    ClearTossAttackFlag(mgr);
    player->EnableLeaveMorphBall(true);
  }
  TryCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Three));
}

bool CSandworm::GrabAttackFinished(CStateManager& mgr, const CTriggerData& data) const {
  return mMorphballTossData.mTossFinished;
}

void CSandworm::StopGrabAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    ClearSegmentRange();
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
  } else if (msg == kStateMsg_Deactivate) {
    mMorphballTossData.mTossFinished = false;
    ResetAttackCooldown(mgr);
    mMorphballTossData.mLastTossTime = mTime;
  }
}

void CSandworm::BombTossAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_BombToss, msg);
  mSurfaced = true;
  if (msg == kStateMsg_Activate) {
    mBombTossData.mOrigin = GetTranslation();
    SetSegmentRange(3, 13);
    mBombAimTimer = 1.3f;
  } else if (msg == kStateMsg_Deactivate) {
    ResetAttackCooldown(mgr);
    if (mBombTossData.mHoldingBomb == true) {
      DeleteHeldBomb(mgr);
    }
    SetTranslation(mBombTossData.mOrigin);
    mBombTossData = SBombTossData();
  } else {
    PositionHeldBomb(mgr);
    if (mBombAimTimer > 0.f) {
      mBombAimTimer -= dt;
      RotateAroundPivot(mgr, mgr.GetPlayer(0)->GetTranslation(), mBombTossData.mOrigin, 100.f, dt);
    }
  }
  TryCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eight));
}

void CSandworm::BombSpreadAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_BombSpread, msg);
  mSurfaced = true;
  if (msg == kStateMsg_Activate) {
    mBombSpreadData.mOrigin = GetTranslation();
    SetSegmentRange(3, 13);
    mBombAimTimer = 1.3f;
  } else if (msg == kStateMsg_Deactivate) {
    ResetAttackCooldown(mgr);
    SetTranslation(mBombSpreadData.mOrigin);
    mBombSpreadData = SBombSpreadData();
  } else if (mBombAimTimer > 0.f) {
    mBombAimTimer -= dt;
    RotateAroundPivot(mgr, mgr.GetPlayer(0)->GetTranslation(), mBombSpreadData.mOrigin, 100.f, dt);
  }
  TryCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Nine));
}

void CSandworm::BombFountainAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_BombFountain, msg);
  mSurfaced = true;
  if (msg == kStateMsg_Activate) {
    mFountainData.mOrigin = GetTranslation();
    mFountainData.mHitDuringWindUp = false;
    mFountainData.mBombLaunched = false;
    mFountainData.mStartHealth = GetHealthInfo()->GetHP();
    SetSegmentRange(5, 13);
    if (mEyeEffects.mEffect3.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect3.mGenerator->SetParticleEmission(false);
    }
  } else if (msg == kStateMsg_Deactivate) {
    ResetAttackCooldown(mgr);
    if (mBombTossData.mHoldingBomb == true) {
      DeleteHeldBomb(mgr);
    }
    SetTranslation(mFountainData.mOrigin);
    mFountainData.mHitDuringWindUp = mFountainData.mBombLaunched = false;
    mFountainData.mDamageFlashTimer = 0.f;
    mFountainData.mDamageTaken = 0.f;
    mFountainData.mStartHealth = 0.f;
    mFountainData.mOrigin = CVector3f::Zero();
    mAttackHistory.mCurrentAttack = -1;
  } else {
    PositionHeldBomb(mgr);
  }
  TryCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Four));
}

void CSandworm::UpdateAttackHistory(EAttack attack, EStateMsg msg) {
  if (msg == kStateMsg_Activate) {
    mAttackHistory.AddAttack(attack);
    mAttackHistory.mRunningAttack = attack;
  } else if (msg == kStateMsg_Deactivate) {
    mAttackHistory.mRunningAttack = -1;
  }
}

void CSandworm::SpitAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_SpitAttack, msg);
  mSurfaced = true;
  if (msg == kStateMsg_Activate) {
    mSpitData.mOrigin = GetTranslation();
    SetSegmentRange(3, 13);
  } else if (msg == kStateMsg_Deactivate) {
    ResetAttackCooldown(mgr);
    SetTranslation(mSpitData.mOrigin);
  }
  TryCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eleven));
}

void CSandworm::Rise(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mForceUnderground = false;
    mRiseTransform = GetTransform();
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    SendScriptMsgs(kSS_GRNT, mgr, kInvalidUniqueId, kSM_None);
    mTrackPlayerOnRise = true;
    mSavedTurnSpeed = BodyController()->GetTurnSpeed();
    BodyController()->SetTurnSpeed(2000.f);
  } else if (msg == kStateMsg_Deactivate) {
    BodyController()->SetTurnSpeed(mSavedTurnSpeed);
    mSegmentBlendTarget = 1.f;
  }
  mLastRiseTime = mTime;
  if (mTrackPlayerOnRise == true) {
    BodyController()->CommandMgr().SetTargetVector(mgr.GetPlayer(0)->GetTranslation() -
                                                   GetTranslation());
  }
  TryCommand(msg, pas::kAS_Generate, CBCGenerateCmd(pas::kGType_Zero, -1));
}

void CSandworm::Descend(CStateManager& mgr, EStateMsg msg, float dt) {
  TryCommand(msg, pas::kAS_Generate, CBCGenerateCmd(pas::kGType_One, -1));
  if (msg == kStateMsg_Activate) {
    mSurfaced = false;
    SendScriptMsgs(kSS_DeGenerate, mgr, kInvalidUniqueId, kSM_None);
    mAttackHistory.Clear();
  } else if (msg == kStateMsg_Deactivate && mCollisionActorManager.get() != nullptr) {
    for (uint i = 0; i < 15; ++i) {
      const CJointCollisionDescription& desc = mCollisionActorManager->GetCollisionDescFromIndex(i);
      CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId()));
      if (actor != nullptr) {
        actor->SetActive(false);
      }
    }
  }
}

void CSandworm::BuildTrail(CStateManager& mgr) {
  while (mTrail.size() < 180) {
    mTrail.push_back(CVector3f::Zero());
  }
  int index = 179;
  for (int segment = 0; segment < 12; ++segment) {
    const CVector3f from = GetSpineLocatorPosition(segment);
    CVector3f position = from;
    const CVector3f to = GetSpineLocatorPosition(segment + 1);
    const CVector3f step((to.GetX() - from.GetX()) * (1.f / 15.f),
                         (to.GetY() - from.GetY()) * (1.f / 15.f),
                         (1.f / 15.f) * (to.GetZ() - from.GetZ()));
    for (int i = 0; i < 15; ++i) {
      if (index < 0) {
        break;
      }
      const CVector3f sample = position - GetPincerOffset();
      mTrail[index] = GetSurfaceSamplePosition(mgr, sample);
      position += step;
      --index;
    }
  }
}

bool CSandworm::ShouldReverseDirection(CStateManager& mgr) const {
  if (IsAnyEyeKilled(mgr) || mIsBoss == 1) {
    return false;
  }
  if (mAttackHistory.mRunningAttack == kA_GrabAttack) {
    return false;
  }
  if (mSpinePositions.size() < 13) {
    return false;
  }
  return 5.f + mLastReverseTime < mTime;
}

void CSandworm::ReverseDirection(CStateManager& mgr) {
  if (mSpinePositions.size() < 13) {
    return;
  }
  const CVector3f position = mSpinePositions[mSpinePositions.size() - 1];
  SetTranslation(position);
  CVector3f direction = position - mSpinePositions[mSpinePositions.size() - 2];
  if (!direction.CanBeNormalized()) {
    direction = position - mSpinePositions[mSpinePositions.size() - 3];
  }
  direction.SetZ(0.f);
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
  } else {
    return;
  }
  SetTransform(
      CQuaternion::FromMatrix(CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up()))
          .BuildTransform4f(GetTranslation()));
  const int removeCount = GetTrailIndexAtDistance(GetTotalSegmentWeight());
  for (int i = 0; i < removeCount; ++i) {
    mTrail.erase(mTrail.begin());
  }
  for (int i = 0; i < mTrail.size() / 2; ++i) {
    CVector3f first = mTrail[i];
    CVector3f last = mTrail[mTrail.size() - (i + 1)];
    mTrail[i] = last;
    mTrail[mTrail.size() - (i + 1)] = first;
  }
  const int count = mTrail.size();
  while (mTrail.size() < 180) {
    mTrail.push_back(CVector3f::Zero());
  }
  const CVector3f front = mTrail[0];
  for (int i = 0; i < 180; ++i) {
    const int source = count - (i + 1);
    if (source < 0) {
      mTrail[180 - (i + 1)] = front;
    } else {
      mTrail[180 - (i + 1)] = mTrail[source];
    }
  }
  mLastReverseTime = mTime;
  mFrustrationTime = 0.f;
  mLastDamageTime = mTime;
  UpdateEyes(mgr, 0.f);
}

void CSandworm::EnsureCollisionActors(CStateManager& mgr) {
  if (!mStartsUnderground) {
    UpdateSplineSegments(mgr);
  }
  if (mCollisionActorManager.get() == nullptr) {
    CreateCollisionActors(mgr);
  }
}

void CSandworm::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Clamped);
    StopWalkSound();
    PlayWalkVocalSound();
    EnsureCollisionActors(mgr);
    mPatrolling = true;
    mUnderground = false;
    ResetBombTimer();
    mSurfaced = true;
  } else if (msg == kStateMsg_Deactivate) {
    mPatrolling = false;
    StopWalkSound();
    StopWalkVocalSound();
    ResetBombTimer();
    mSurfaced = false;
  } else {
    UpdateBombDrop(mgr, dt);
    UpdateWalkSound(mgr, dt);
    mSurfaced = true;
  }
  CWaypointNavigation& navigation = mWaypointNavigation;
  const bool clockwise = mSideStepTimer > 0.5f * GetSideStepThreshold();
  navigation.ConfigureWobbleSteering(clockwise, GetBlendWeight());
  CPatterned::Patrol(mgr, msg, dt);
}

CVector3f CSandworm::GetPatrolDestination(CStateManager& mgr) const {
  if (mPatrolling == true) {
    return mWaypointNavigation.GetDestinationPosition();
  }
  return mgr.GetPlayer(0)->GetTranslation();
}

void CSandworm::ClearSegmentRange() { SetSegmentRange(-1, 13); }

void CSandworm::SetSegmentRange(int first, int last) {
  mSegmentRangeFirst = first;
  mSegmentRangeLast = last;
}

bool CSandworm::IsOnPathMesh(const CVector3f& position, float radius) const {
  for (float z = 0.f; z <= 3.f; z += 1.f) {
    if (mPathFindSearch.NearlyOnPath(position + CVector3f(0.f, 0.f, z), radius) ==
        CPathFindSearch::kR_Success) {
      return true;
    }
  }
  return false;
}

void CSandworm::CheckPathObstruction(CStateManager& mgr) {
  if (ShouldReverseDirection(mgr) && mSurfaced == true) {
    CVector3f ahead = GetTransform().GetForward();
    if (ahead.CanBeNormalized()) {
      ahead.Normalize();
    }
    ahead = ahead * 4.f;
    const CVector3f aheadPosition = GetTranslation() + ahead;
    if (!IsOnPathMesh(aheadPosition, 4.f)) {
      const CVector3f tail = GetSpinePosition(12);
      const CVector3f beforeTail = GetSpinePosition(11);
      CVector3f behind = tail - beforeTail;
      if (behind.CanBeNormalized()) {
        behind.Normalize();
      }
      behind = behind * 4.f;
      const CVector3f behindPosition = tail + behind;
      if (IsOnPathMesh(behindPosition, 4.f) == true) {
        ReverseDirection(mgr);
      }
    }
  }
}

void CSandworm::UpdateDeathScale() {
  CAnimData* animData = AnimationData();
  const float progress = mDeathTime > 6.f ? 1.f : mDeathTime / 6.f;
  if (!animData->HasAnimatedScale()) {
    AnimationData()->AddAnimatedScale();
  }
  float scaleZ = 1.f;
  const float limit = 0.2f + (0.8f - mDeathTime / 5.f);
  if (limit < scaleZ) {
    scaleZ = limit;
  }
  const float scaleX = 0.2f * progress + 0.9f;
  int i = -1;
  do {
    CSegId segId;
    if (i == -1) {
      segId = animData->GetLocatorSegId(rstl::string_l("front_spine_7"));
    } else if (i == 13) {
      segId = animData->GetLocatorSegId(rstl::string_l("back_spine_7"));
    } else {
      const SSpineSegment* segment = &skSpineSegments[i + 1];
      segId = animData->GetLocatorSegId(rstl::string_l(segment->mName));
    }
    SetSegmentScale(segId, CVector3f(scaleX, scaleZ, 1.f));
    ++i;
  } while (i <= 13);
}

void CSandworm::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CSandworm::UpdateLocators() {
  mSpitData.mLocatorTransform = GetLctrTransform(rstl::string_l(skSpitLocatorName));
  mMorphballTossData.mLocatorTransform = GetLctrTransform(rstl::string_l(skTossLocatorName));
  mFrontEyePosition = GetLctrTransform(rstl::string_l(skFrontEyeLocatorName)).GetTranslation();
  mBackEyePosition = GetLctrTransform(rstl::string_l(skBackEyeLocatorName)).GetTranslation();
  mSpineCenterPosition = GetSpineLocatorPosition(6);
  mSpineHeadPosition = GetSpineLocatorPosition(0);
  mSpineTailPosition = GetSpineLocatorPosition(12);
  mLocatorsValid = true;
}

CAABox CSandworm::CalculateBounds() const {
  CAABox box = CAABox::MakeMaxInvertedBox();
  if (!mSurfaced) {
    if (HasModelData()) {
      box = GetModelData()->GetBounds(GetTransform());
    } else {
      const CVector3f position = GetTranslation();
      box = CAABox(position, position);
    }
  } else {
    for (int i = 0; i < mSpinePositions.size(); ++i) {
      const CVector3f& spine = GetSpineLocatorPosition(i);
      box.AccumulateBounds(mSpinePositions[i] - CVector3f(3.f, 3.f, 1.5f));
      box.AccumulateBounds(mSpinePositions[i] + CVector3f(3.f, 3.f, 1.5f));
      box.AccumulateBounds(spine - CVector3f(3.f, 3.f, 1.5f));
      box.AccumulateBounds(spine + CVector3f(3.f, 3.f, 1.5f));
    }
  }
  AccumulateModelBounds(box, *mPincerModelL, mPincerTransforms[0]);
  AccumulateModelBounds(box, *mPincerModelR, mPincerTransforms[1]);
  AccumulateModelBounds(box, *mPincerModelL, mPincerTransforms[2]);
  AccumulateModelBounds(box, *mPincerModelR, mPincerTransforms[3]);
  return box;
}

void CSandworm::PreRenderAllViewports(CStateManager& mgr) {
  CPatterned::PreRenderAllViewports(mgr);
  if (mDying == true) {
    const float fade = 1.f - mDeathTime / 7.f;
    const CWeaponMode mode = GetHealthInfo()->GetCauseOfDeathWeapon();
    if (!mode.IsComboed() || (mode.GetType() != kWT_Dark && mode.GetType() != kWT_Annihilator)) {
      const CColor color(fade, fade, fade, 1.f);
      SetModelFlags(CModelFlags(CModelFlags::kT_Blend, color));
    }
  }
  UpdateSpine(mgr);
  CTransform4f* claws = &mPincerTransforms[0];
  claws[0] = GetClawTransform(mgr, skFrontLeftClawName, false, false, mFrontClawAngle);
  claws[1] = GetClawTransform(mgr, skFrontRightClawName, false, true, mFrontClawAngle);
  claws[2] = GetClawTransform(mgr, skBackRightClawName, true, false, mBackClawAngle);
  claws[3] = GetClawTransform(mgr, skBackLeftClawName, true, true, mBackClawAngle);
  if (mMorphballTossData.mHoldingPlayer == true) {
    TeleportPlayerToToss(mgr);
  }
  const float frontLeft = GetClawGroundAngle(mgr, skFrontLeftClawName, false);
  float frontAngle = GetClawGroundAngle(mgr, skFrontRightClawName, false);
  if (frontLeft > frontAngle) {
    frontAngle = frontLeft;
  }
  if (frontAngle > mFrontClawAngle) {
    mFrontClawAngle = frontAngle;
  }
  const float backRight = GetClawGroundAngle(mgr, skBackRightClawName, true);
  float backAngle = GetClawGroundAngle(mgr, skBackLeftClawName, true);
  if (backRight > backAngle) {
    backAngle = backRight;
  }
  if (backAngle > mBackClawAngle) {
    mBackClawAngle = backAngle;
  }
  UpdateLocators();
  UpdateEyes(mgr, 0.f);
  mScanVisorActive =
      mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan;
  ModelData()->AdvanceParticles(GetTransform(), 0.f, mgr);
  const CAABox& bounds = CalculateBounds();
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdateShadow(mgr);
  UpdatePortalSystemState(mgr);
}

void CSandworm::AccumulateModelBounds(CAABox& box, const CModelData& model,
                                      const CTransform4f& xf) const {
  box.Include(model.GetBounds(xf));
}

void CSandworm::CreateShadow() { mShadow = rs_new CProjectedShadow(128, 128, false, true); }

void CSandworm::UpdateShadow(CStateManager& mgr) {
  int count = 1;
  if (IsRemainingEyeAlive(mgr) == true) {
    count = 3;
  }
  if (IsActiveEyeAlive(mgr) == true) {
    count += 2;
  }
  const CModelData* models[] = {GetModelData(), &*mPincerModelL, &*mPincerModelR, &*mPincerModelL,
                                &*mPincerModelR};
  const CTransform4f* transforms[] = {&GetTransform(), &mPincerTransforms[0], &mPincerTransforms[1],
                                      &mPincerTransforms[2], &mPincerTransforms[3]};
  if (IsActiveEyeAlive(mgr) == true && IsRemainingEyeAlive(mgr) == false) {
    transforms[1] = transforms[3];
    transforms[2] = transforms[4];
  }
  mShadow->SetBounds(GetOtherBounds());
  mShadow->RenderShadowBuffer(mgr, count, models, transforms, 0, CVector3f::Zero(), 1.1f, 10.f);
  mShadow->SetOpacity(mUnderground == true ? 0.f : 0.5f);
}

void CSandworm::ChargeWindUp(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mChargeData.mReadyToCharge = false;
  }
  SetSegmentRange(3, 13);
  mSurfaced = true;
  TryCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_One));
}

void CSandworm::Snatch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mSpeed = 0.66f;
    mChargeData.mReadyToCharge = false;
    SetSegmentRange(3, 13);
    mSurfaced = true;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
    } else if (mAnimationState.IsOver()) {
      mSpeed = 1.f;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

float CSandworm::GetIngSnatchingModelOverlapSize() const { return 6.f; }

bool CSandworm::SnatchStarted(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsBoss == 1 && !IsIngControlled()) {
    return true;
  }
  return false;
}

bool CSandworm::SnatchEnded(CStateManager& mgr, const CTriggerData& data) const {
  return IsIngControlled();
}

bool CSandworm::ReadyToCharge(CStateManager& mgr, const CTriggerData& data) const {
  return mChargeData.mReadyToCharge;
}

void CSandworm::SpawnHeldBomb(CStateManager& mgr) {
  CBouncingBomb* bomb = rs_new CBouncingBomb(
      *mBombData.mBombEffect, *mBombData.mExplosionEffect, mgr.AllocateUniqueId(),
      GetCurrentAreaId(), GetUniqueId(), 10.f, 3.f, kWT_Bomb, 256, mSpitData.mLocatorTransform,
      mBombData.mDamage, 8.f, GetBombPlacementSound(), GetBombBounceSound(), GetBombExplodeSound(),
      1.f, 0.8f);
  mgr.AddObject(*bomb);
  mBombTossData.mBombId = bomb->GetUniqueId();
  mBombTossData.mHoldingBomb = true;
  PositionHeldBomb(mgr);
}

ushort CSandworm::GetBombPlacementSound() const { return CSfxManager::kInternalInvalidSfxId; }

ushort CSandworm::GetBombBounceSound() const { return mSoundData.mBombBounceSound; }

ushort CSandworm::GetBombExplodeSound() const { return mSoundData.mBombExplodeSound; }

void CSandworm::DeleteHeldBomb(CStateManager& mgr) {
  if (mBombTossData.mBombId != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mBombTossData.mBombId);
    mBombTossData.mBombId = kInvalidUniqueId;
  }
}

CBouncingBomb* CSandworm::GetHeldBomb(CStateManager& mgr) const {
  if (mBombTossData.mBombId == kInvalidUniqueId) {
    return nullptr;
  }
  return TCastToPtr< CBouncingBomb >(mgr.ObjectById(mBombTossData.mBombId));
}

const CBouncingBomb* CSandworm::GetHeldBomb(const CStateManager& mgr) const {
  if (mBombTossData.mBombId == kInvalidUniqueId) {
    return nullptr;
  }
  return TCastToConstPtr< CBouncingBomb >(mgr.GetObjectById(mBombTossData.mBombId));
}

void CSandworm::LaunchHeldBomb(CStateManager& mgr) {
  CBouncingBomb* bomb = GetHeldBomb(mgr);
  if (bomb != nullptr) {
    CVector3f direction =
        mgr.GetPlayer(0)->GetTranslation() + CVector3f(0.f, 0.f, -3.f) - bomb->GetTranslation();
    if (direction.CanBeNormalized() == true) {
      direction.Normalize();
    }
    direction *= 35.f;
    bomb->SetVelocity(direction);
    mBombTossData.mHoldingBomb = false;
  }
}

void CSandworm::PositionHeldBomb(CStateManager& mgr) {
  if (mBombTossData.mHoldingBomb) {
    CBouncingBomb* bomb = GetHeldBomb(mgr);
    if (bomb != nullptr) {
      CVector3f direction = mSpitData.mLocatorTransform.GetForward();
      if (direction.CanBeNormalized() == true) {
        direction.Normalize();
      }
      direction *= 0.9f;
      bomb->SetTranslation(mSpitData.mLocatorTransform.GetTranslation() + direction);
    }
  }
}

void CSandworm::TossBombAtPlayer(CStateManager& mgr) {
  CBouncingBomb* bomb = CreateBomb(mgr, 10.f, 5.f, 1.f, 0.65f);
  CVector3f direction = mSpitData.mLocatorTransform.GetForward();
  direction.SetZ(0.f);
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
  }
  direction.SetZ(0.5f);
  direction.Normalize();
  const float distance =
      CVector3f(GetTranslation() - mgr.GetPlayer(0)->GetTranslation()).Magnitude();
  float speedScale = 1.f;
  if (distance > 10.f) {
    if (distance > 40.f) {
      speedScale = 2.f;
    } else {
      speedScale = 1.f + (distance - 10.f) / 30.f;
    }
  }
  direction *= 15.f * speedScale;
  bomb->SetVelocity(direction);
  bomb->ApplyGravity();
}

CBouncingBomb* CSandworm::CreateBomb(CStateManager& mgr, float fuseTime, float touchRadius,
                                     float gravityScale, float bounceRestitution) {
  CBouncingBomb* bomb = rs_new CBouncingBomb(
      *mBombData.mBombEffect, *mBombData.mExplosionEffect, mgr.AllocateUniqueId(),
      GetCurrentAreaId(), GetUniqueId(), fuseTime, touchRadius, kWT_Bomb, 256,
      mSpitData.mLocatorTransform, mBombData.mDamage, 8.f, GetBombPlacementSound(),
      GetBombBounceSound(), GetBombExplodeSound(), gravityScale, bounceRestitution);
  mgr.AddObject(*bomb);
  return bomb;
}

void CSandworm::TossBombRandom(CStateManager& mgr) {
  CBouncingBomb* bomb = CreateBomb(mgr, 12.f, 3.f, 1.f, 0.7f);
  const float z = mgr.Random()->Range(1.f, 1.2f);
  const float y = mgr.Random()->Range(-1.f, 1.f);
  CVector3f direction(mgr.Random()->Range(-1.f, 1.f), y, z);
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
  } else {
    direction = CVector3f::Up();
  }
  direction *= 25.f;
  const float speed = mgr.Random()->Range(0.5f, 1.f);
  direction *= speed;
  bomb->SetVelocity(direction);
  bomb->ApplyGravity();
}

void CSandworm::FireSpit(CStateManager& mgr) {
  CVector3f impact = CVector3f::Zero();
  const CVector3f target = mgr.GetPlayer(0)->GetTranslation() + skSpitTargetOffset;
  if (GetSpitAngleDiff(target) > (M_PIF / 180.f) * mSpitData.mMaxAimAngle) {
    CVector3f direction = mSpitData.mLocatorTransform.GetTranslation() - mSpineCenterPosition;
    direction.SetZ(0.f);
    if (direction.CanBeNormalized() == true) {
      direction.Normalize();
    }
    const CVector3f position = mSpitData.mLocatorTransform.GetTranslation();
    CVector3f toTarget = target - position;
    toTarget.SetZ(0.f);
    const float distance = toTarget.Magnitude();
    const CVector3f edge = position + distance * direction;
    impact = edge;
    const float step = 0.05f;
    for (float t = step; t < 0.95f; t += step) {
      const float remaining = 1.f - t;
      const CVector3f candidate(target.GetX() * remaining + edge.GetX() * t,
                                target.GetY() * remaining + edge.GetY() * t,
                                target.GetZ() * remaining + edge.GetZ() * t);
      if (GetAngleToTarget(candidate, direction) < (M_PIF / 180.f) * mSpitData.mMaxAimAngle) {
        impact = candidate;
        break;
      }
    }
  } else {
    impact = target;
  }
  mSpitData.mNextSpitTime = 1.5f + mTime;
  CVector3f side = impact - mSpitData.mLocatorTransform.GetTranslation();
  side.SetZ(0.f);
  if (side.CanBeNormalized() == true) {
    side.Normalize();
    LaunchSpitProjectile(mgr, impact + CVector3f(side.GetY(), -side.GetX(), 0.f) * 2.5f);
    LaunchSpitProjectile(mgr, impact + CVector3f(-side.GetY(), side.GetX(), 0.f) * 2.5f);
  }
  LaunchSpitProjectile(mgr, impact);
}

void CSandworm::LaunchSpitProjectile(CStateManager& mgr, const CVector3f& target) {
  CTransform4f xf = mSpitData.mLocatorTransform;
  xf = CTransform4f::LookAt(xf.GetTranslation(), target, xf.GetUp());
  LaunchProjectile(xf, mgr, 6, CWeapon::kPA_None, false,
                   CImpactVisorEffect::ParticleEffect(mSpitData.mVisorEffect,
                                                      CSfxManager::kInternalInvalidSfxId, false),
                   CVector3f(1.f, 1.f, 1.f));
}

void CSandworm::ChargePlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_Charge, msg);
  if (msg == kStateMsg_Activate) {
    mSegmentBlendTarget = 1.f;
    BodyController()->SetLocomotionType(pas::kLT_Internal8);
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mChargeData.mStartPosition = GetTranslation();
    mChargeData.mTargetPosition = mgr.GetPlayer(0)->GetTranslation();
    mChargeData.mHitPlayer = false;
    CVector3f direction = mChargeData.mTargetPosition - mChargeData.mStartPosition;
    direction.SetZ(0.f);
    float distance = direction.Magnitude();
    if (mIsBoss == 1) {
      distance = 100.f;
    }
    if (direction.CanBeNormalized() == true) {
      direction.Normalize();
    }
    direction *= 1.f + distance;
    mChargeData.mTargetPosition = mChargeData.mStartPosition + direction;
    mChargeData.mStartTime = mTime;
    ClearSegmentRange();
    mSurfaced = true;
    mWasSurfaced = mSurfaced;
    mIsCharging = true;
    mChargeData.mSavedTurnSpeed = BodyController()->GetTurnSpeed();
    BodyController()->SetTurnSpeed(1500.f);
  } else if (msg == kStateMsg_Deactivate) {
    ChargeHitPlayer(mgr);
    CheckPathObstruction(mgr);
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    ResetAttackCooldown(mgr);
    mIsCharging = false;
    BodyController()->SetTurnSpeed(mChargeData.mSavedTurnSpeed);
    ClearSegmentRange();
    mChargeData.mStartPosition = CVector3f::Zero();
    mChargeData.mTargetPosition = CVector3f::Zero();
    StopLoopedSounds();
  }
  CVector3f toTarget = mChargeData.mTargetPosition - GetTranslation();
  if (toTarget.CanBeNormalized() == true) {
    toTarget.Normalize();
  }
  const CVector3f blended = GetChargeBlendedDirection(toTarget);
  BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(blended, CVector3f::Zero(), 1.f));
}

void CSandworm::ChargeHitPlayer(CStateManager& mgr) {
  if (mChargeData.mHitPlayer != true) {
    CPlayer* player = mgr.GetPlayer(0);
    if (!((player->GetTranslation() - GetTranslation()).MagSquared() > 25.f)) {
      mgr.ApplyDamage(
          GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
      CVector3f randomOffset = CVector3f::Zero();
      const CVector3f forward(GetTransform().GetForward().GetX(),
                              GetTransform().GetForward().GetY(), 0.f);
      if (mgr.IsRandomAvailable() == true) {
        randomOffset.SetY(mgr.Random()->Float() - 0.5f);
        randomOffset.SetX(mgr.Random()->Float() - 0.5f);
        randomOffset.SetZ(0.f);
      }
      CVector3f direction = forward + randomOffset;
      if (direction.CanBeNormalized() == true) {
        direction.Normalize();
      }
      direction *= mChargeData.mImpulseHorizontal;
      CVector3f impulse = direction + mChargeData.mImpulseVertical * CVector3f::Up();
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        impulse *= 0.4f;
      }
      PushPlayer(mgr, impulse);
      mChargeData.mHitPlayer = true;
    }
  }
}

void CSandworm::SetPathDestination(CStateManager& mgr, const CVector3f& position, float dt) {
  mPathDestination = position;
  mPathFindNavigation.SetDestination(mPathDestination);
  mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
}

bool CSandworm::IsCoverPointUsable(const CScriptCoverPoint* cover) const {
  if (cover == nullptr || !cover->GetActive() || cover->GetCurrentAreaId() != GetCurrentAreaId()) {
    return false;
  }
  const TUniqueId id = cover->GetUniqueId();
  if (id == mLastCoverPointId || id == mCoverPointId) {
    return false;
  }
  return true;
}

TUniqueId CSandworm::PickCoverPoint(CStateManager& mgr) const {
  TUniqueId result = kInvalidUniqueId;
  float bestScore = -1.f;
  int count = 0;
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state == kSS_Connect) {
      ++count;
      TUniqueId id = mgr.GetIdForScript(it->objId);
      CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id));
      if (IsCoverPointUsable(cover) == true) {
        if (!mgr.IsRandomAvailable()) {
          return cover->GetUniqueId();
        }
        const float score = mgr.Random()->Float();
        if (score > bestScore) {
          bestScore = score;
          result = cover->GetUniqueId();
        }
      }
    }
  }
  if (count != 0) {
    return result;
  }
  CObjectList& waypoints = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = waypoints.GetFirstObjectIndex(); i != -1; i = waypoints.GetNextObjectIndex(i)) {
    CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(waypoints[i]);
    if (IsCoverPointUsable(cover) == true) {
      if (!mgr.IsRandomAvailable()) {
        return cover->GetUniqueId();
      }
      const float score = mgr.Random()->Float();
      if (score > bestScore) {
        bestScore = score;
        result = cover->GetUniqueId();
      }
    }
  }
  return result;
}

CVector3f CSandworm::GetPursueTarget(CStateManager& mgr) {
  if (mPlayerOffPathTime < 6.f && mPursueTime < 8.f && mGoToCover == false) {
    CPlayer* player = mgr.GetPlayer(0);
    if (IsIngControlled() == 1) {
      if (1.f + mCachedPlayerPositionTime < mTime) {
        mCachedPlayerPosition = player->GetTranslation();
        mCachedPlayerPositionTime = mTime;
      }
      return mCachedPlayerPosition;
    }
    return player->GetTranslation();
  }
  if (mCoverPointId != kInvalidUniqueId) {
    CScriptCoverPoint* cover = GetCoverPoint(mgr, mCoverPointId);
    if (cover != nullptr) {
      CVector3f offset = GetTranslation() - cover->GetTranslation();
      offset.SetZ(0.f);
      if (offset.Magnitude() < 4.f) {
        mLastCoverPointId = mCoverPointId;
        mCoverPointId = kInvalidUniqueId;
      }
    }
  }
  if (mCoverPointId == kInvalidUniqueId) {
    mCoverPointId = PickCoverPoint(mgr);
  }
  if (mCoverPointId != kInvalidUniqueId) {
    CScriptCoverPoint* cover = GetCoverPoint(mgr, mCoverPointId);
    if (cover != nullptr) {
      return cover->GetTranslation();
    }
  }
  return mgr.GetPlayer(0)->GetTranslation();
}

void CSandworm::Pursue(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_Pursue, msg);
  if (IsIngControlled() == 1) {
    BodyController()->CommandMgr().SetSteeringSpeedRange(0.f, 1.f);
  }
  const CVector3f target = GetPursueTarget(mgr);
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Clamped);
    if (mIsBoss == 1) {
      if (IsIngControlled() == 0) {
        BodyController()->CommandMgr().SetSteeringSpeedRange(0.f, 0.f);
      } else {
        mSpeed = GetPhaseStruct().moveSpeedMultiplier;
      }
    }
    if (mMorphballTossData.mHoldingPlayer == 1) {
      ReleasePlayer(mgr);
    }
    SetPathDestination(mgr, target, dt);
    mSurfaced = true;
    mSegmentBlendTarget = 1.f;
    ClearSegmentRange();
    mFrustrationTime = 0.f;
    StopWalkSound();
    PlayWalkVocalSound();
    mPursueTime = 0.f;
    mPlayerOffPathTime = 0.f;
    mPursueStartPosition = GetTranslation();
    mGoToCover = false;
    if (!mAttackHistory.LastEntriesEqual(kA_Melee, 1)) {
      ResetBombTimer();
    }
    break;
  case kStateMsg_Update: {
    UpdateWalkSound(mgr, dt);
    UpdateBombDrop(mgr, dt);
    UpdateTurnSpeed(dt);
    CheckPathObstruction(mgr);
    if ((target - mPathDestination).MagSquared() > 16.f) {
      SetPathDestination(mgr, target, dt);
    }
    CVector3f move = CVector3f::Zero();
    if (PathShagged(mgr, CTriggerData(0.f)) ||
        mPathFindSearch.GetCurrentWaypoint() >= mPathFindSearch.GetWaypoints().size() - 1) {
      mCoverPointId = kInvalidUniqueId;
      if (ShouldReverseDirection(mgr) == true) {
        ReverseDirection(mgr);
        SetPathDestination(mgr, target, dt);
      } else {
        if (mGoToCover == true) {
          mGoToCover = false;
        } else {
          mGoToCover = true;
        }
        SetPathDestination(mgr, target, dt);
      }
    }
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    move = BodyController()->GetCommandMgr().GetMoveVector();
    const CVector3f blended = GetPursueBlendedDirection(move);
    if (blended.CanBeNormalized() == true) {
      const CVector3f direction = blended.AsNormalized();
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(direction, CVector3f::Zero(), 1.f));
    }
    const float distance = CVector3f(target - GetTranslation()).Magnitude();
    if (distance < mPursuitFrustrationRadius) {
      mFrustrationTime += dt;
      if (IsAnyEyeKilled(mgr) == 0 && !mIsBoss && mFrustrationTime > mPursuitFrustrationTimer) {
        ReverseDirection(mgr);
      }
    }
    mPursueTime += dt;
    if (PlayerIsOnPathMesh(mgr, CTriggerData(0.f)) == true) {
      mPlayerOffPathTime = 0.f;
    } else {
      mPlayerOffPathTime += dt;
    }
    break;
  }
  case kStateMsg_Deactivate:
    BodyController()->SetTurnSpeed(mDefaultTurnSpeed);
    StopWalkSound();
    StopWalkVocalSound();
    mPursueTime = 0.f;
    mPlayerOffPathTime = 0.f;
    ResetBombTimer();
    break;
  }
}

void CSandworm::UpdateTurnSpeed(float dt) {
  mTurnSpeedTimer += dt;
  if (!(mTurnSpeedTimer < 0.2f)) {
    mTurnSpeedTimer = 0.f;
    if (mPathFindSearch.OnPath(GetTranslation() + CVector3f(0.f, 0.f, 1.f)) !=
        CPathFindSearch::kR_Success) {
      BodyController()->SetTurnSpeed(1.5f * mDefaultTurnSpeed);
    } else {
      BodyController()->SetTurnSpeed(mDefaultTurnSpeed);
    }
  }
}

bool CSandworm::MeleeAttackRange(CStateManager& mgr, const CTriggerData& data) const {
  return mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Morphed
             ? false
             : IsPlayerInMeleeRange(mgr);
}

bool CSandworm::PlayerIsMorphball(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer::EPlayerMorphBallState state;
  if (mgr.GetPlayer(0)->GetSpawnedMorphballState() == 0) {
    state = mgr.GetPlayer(0)->GetMorphballTransitionState();
  } else {
    state = CPlayer::kMS_Unmorphed;
  }
  return state == CPlayer::kMS_Morphed;
}

bool CSandworm::IsPlayerInMeleeRange(CStateManager& mgr) const {
  const CVector3f tossPosition = GetMorphballTossPosition(2.f);
  return (mgr.GetPlayer(0)->GetTranslation() - tossPosition).MagSquared() < 16.f;
}

void CSandworm::MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAttackHistory(kA_Melee, msg);
  mSurfaced = true;
  if (msg == kStateMsg_Activate) {
    const CPASDatabase& database = BodyController()->GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_Unknown26, CPASAnimParm::NoParameter(),
                                 CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(),
                                 CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(),
                                 CPASAnimParm::NoParameter());
    rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > 0.f) {
      ModelData()->AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
    }
    mMeleeData.mDamageTimer = 0.5f;
    PlayMeleeSound();
    ResetAttackCooldown(mgr);
  }
}

void CSandworm::UpdateMeleeDamage(CStateManager& mgr, float dt) {
  if (0.f != mMeleeData.mDamageTimer) {
    mMeleeData.mDamageTimer -= dt;
    if (mMeleeData.mDamageTimer < 0.f) {
      mMeleeData.mDamageTimer = 0.f;
      if (IsPlayerInMeleeRange(mgr) == true) {
        CPlayer* player = mgr.GetPlayer(0);
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mMeleeData.mDamage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        const float horizontal = mMeleeData.mImpulseHorizontal;
        const float vertical = mMeleeData.mImpulseVertical;
        const CVector3f impulse =
            horizontal * mSpitData.mLocatorTransform.GetForward() + vertical * CVector3f::Up();
        PushPlayer(mgr, impulse);
      }
    }
  }
}

CVector3f CSandworm::GetPursueBlendedDirection(const CVector3f& direction) const {
  const float threshold = GetSideStepThreshold();
  CVector3f sideways;
  if (mSideStepTimer > 0.5f * threshold) {
    sideways = CVector3f(-direction.GetX(), direction.GetY(), direction.GetZ());
  } else {
    sideways = CVector3f(direction.GetX(), -direction.GetY(), direction.GetZ());
  }
  const float weight = GetBlendWeight();
  const float inverse = 1.f - weight;
  return CVector3f(direction.GetX() * inverse + sideways.GetX() * weight,
                   direction.GetY() * inverse + sideways.GetY() * weight,
                   direction.GetZ() * inverse + sideways.GetZ() * weight);
}

CVector3f CSandworm::GetChargeBlendedDirection(const CVector3f& direction) const {
  const float threshold = GetSideStepThreshold();
  CVector3f sideways;
  if (mSideStepTimer > 0.5f * threshold) {
    sideways = CVector3f(direction.GetY(), -direction.GetX(), direction.GetZ());
  } else {
    sideways = CVector3f(-direction.GetY(), direction.GetX(), direction.GetZ());
  }
  return CVector3f(direction.GetX() * 0.7f + sideways.GetX() * 0.3f,
                   direction.GetY() * 0.7f + sideways.GetY() * 0.3f,
                   direction.GetZ() * 0.7f + sideways.GetZ() * 0.3f);
}

bool CSandworm::InChargeRange(CStateManager& mgr, const CTriggerData& data) const {
  if (IsIngControlled() == 1) {
    return true;
  }
  const float rangeMin = mChargeData.mRangeMin;
  const float rangeMax = mChargeData.mRangeMax;
  const float distSq = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
  return distSq > rangeMin * rangeMin && distSq < rangeMax * rangeMax;
}

float CSandworm::GetSpitAngleDiff(const CVector3f& position) const {
  CVector3f direction = mSpitData.mLocatorTransform.GetTranslation() - mSpineCenterPosition;
  direction.SetZ(0.f);
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
  }
  return GetAngleToTarget(position, direction);
}

float CSandworm::GetAngleToTarget(const CVector3f& position, const CVector3f& direction) const {
  CVector3f toPosition = position - mSpitData.mLocatorTransform.GetTranslation();
  CVector3f flatDirection = direction;
  flatDirection.SetZ(0.f);
  toPosition.SetZ(0.f);
  if (!toPosition.CanBeNormalized() || !flatDirection.CanBeNormalized()) {
    return 100000.f;
  }
  toPosition.Normalize();
  flatDirection.Normalize();
  return CVector3f::GetAngleDiff(flatDirection, toPosition);
}

bool CSandworm::FacingPlayer(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = mgr.GetPlayer(0);
  return GetAngleToTarget(player->GetTranslation(), mSpitData.mLocatorTransform.GetForward()) <
         12.f * (M_PIF / 180.f);
}

bool CSandworm::SpitAngleOK(CStateManager& mgr, const CTriggerData& data) const {
  return GetSpitAngleDiff(mgr.GetPlayer(0)->GetTranslation() + skSpitTargetOffset) <
         35.f * (M_PIF / 180.f);
}

bool CSandworm::FacingPlayerForCharge(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = mgr.GetPlayer(0);
  if (mSpinePositions.size() < 12) {
    return false;
  }
  CVector3f direction = mSpinePositions[6] - mSpinePositions.back();
  direction.SetZ(0.f);
  if (!direction.CanBeNormalized()) {
    return false;
  }
  direction.Normalize();
  return GetAngleToTarget(player->GetTranslation(), direction) < 12.f * (M_PIF / 180.f);
}

bool CSandworm::ChargeOver(CStateManager& mgr, const CTriggerData& data) const {
  if (!mIsBoss) {
    if ((mChargeData.mTargetPosition - GetTranslation()).MagSquared() < 9.f) {
      return true;
    }
    const float travelled = (GetTranslation() - mChargeData.mStartPosition).Magnitude();
    if (travelled > (mChargeData.mTargetPosition - mChargeData.mStartPosition).Magnitude() - 1.f) {
      return true;
    }
  }
  if (mTime > 6.f + mChargeData.mStartTime) {
    return true;
  }
  return !(IsOnPathMesh(GetTranslation(), 0.f) != 0);
}

CProjectileInfo* CSandworm::ProjectileInfo() { return &mSpitData.mProjectileInfo; }

void CSandworm::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  switch (type) {
  case kUE_Unknown44: {
    const CWeaponMode mode = GetHealthInfo()->GetCauseOfDeathWeapon();
    if (!mode.IsComboed() || (mode.GetType() != kWT_Dark && mode.GetType() != kWT_Annihilator)) {
      SetTranslation(mSpineCenterPosition);
      SendScriptMsgs(kSS_XDamage, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  }
  case kUE_Projectile:
    switch (mAttackHistory.mRunningAttack) {
    case kA_SpitAttack:
      FireSpit(mgr);
      break;
    case kA_BombToss:
      LaunchHeldBomb(mgr);
      break;
    case kA_BombSpread:
      TossBombAtPlayer(mgr);
      break;
    case kA_BombFountain:
      TossBombRandom(mgr);
      mFountainData.mBombLaunched = true;
      break;
    }
    return;
  case kUE_ObjectPickUp:
    switch (mAttackHistory.mRunningAttack) {
    case kA_BombToss:
      SpawnHeldBomb(mgr);
      break;
    case kA_BombFountain:
      SpawnHeldBomb(mgr);
      break;
    case kA_GrabAttack:
      if (!mMorphballTossData.mHoldingPlayer) {
        CPlayer* player = mgr.GetPlayer(0);
        if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                 ? player->GetMorphballTransitionState()
                 : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
          GrabPlayerIfClose(mgr);
        }
      }
      break;
    }
    return;
  case kUE_ObjectDrop:
    if (mAttackHistory.mRunningAttack == kA_BombFountain) {
      DeleteHeldBomb(mgr);
    } else if (mMorphballTossData.mHoldingPlayer == true) {
      if (mMorphballTossData.mAttackFlagSet == true) {
        ClearTossAttackFlag(mgr);
      }
      ReleasePlayer(mgr);
      TossPlayer(mgr);
    }
    return;
  case kUE_DamageOn:
    if (mMorphballTossData.mHoldingPlayer == true) {
      mgr.RumbleManager(mgr.MaskUIdNumPlayers(mgr.GetPlayer(0)->GetUniqueId()))
          ->Rumble(mgr, kRFX_TwentyFour, 1.f, kRP_Three);
    }
    return;
  case kUE_BeginAction: {
    mChargeData.mReadyToCharge = true;
    CBodyStateCmd cmd(kBSC_AbortScripted);
    BodyController()->CommandMgr().DeliverCmd(cmd);
    return;
  }
  case kUE_EffectOn:
    mUnderground = false;
    return;
  case kUE_EffectOff:
    mUnderground = true;
    return;
  case kUE_GenerateEnd:
    mTrackPlayerOnRise = false;
    return;
  case kUE_BreakLockOn:
    if (mAttackHistory.mRunningAttack == kA_GrabAttack) {
      SetTossAttackFlag(mgr);
    }
    break;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CSandworm::SetTossAttackFlag(CStateManager& mgr) {
  if (mMorphballTossData.mAttackFlagSet != 1) {
    mMorphballTossData.mAttackFlagSet = true;
    SendScriptMsgs(kSS_InternalState0, mgr, GetUniqueId(), kSM_None);
  }
}

void CSandworm::ClearTossAttackFlag(CStateManager& mgr) {
  if (mMorphballTossData.mAttackFlagSet) {
    mMorphballTossData.mAttackFlagSet = false;
    SendScriptMsgs(kSS_InternalState1, mgr, GetUniqueId(), kSM_None);
  }
}

TUniqueId CSandworm::GetFollowTarget(CStateManager& mgr) const {
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Connect && it->msg == kSM_Follow) {
      return mgr.GetIdForScript(it->objId);
    }
  }
  return kInvalidUniqueId;
}

void CSandworm::GrabPlayerIfClose(CStateManager& mgr) {
  const CVector3f tossPosition = GetMorphballTossPosition(0.f);
  CPlayer* player = mgr.GetPlayer(0);
  if ((player->GetTranslation() - tossPosition).MagSquared() > 25.f) {
    mMorphballTossData.mTossFinished = true;
  } else {
    mMorphballTossData.mHoldingPlayer = true;
    CPlayer* grabbed = mgr.GetPlayer(0);
    grabbed->AttachActorToPlayer(GetUniqueId(), true);
    grabbed->Stop();
    grabbed->RemoveMaterial(kMT_Solid, mgr);
    grabbed->EnableLeaveMorphBall(false);
    grabbed->GetMorphBall()->DisableHalfPipeStatus();
    grabbed->GetMorphBall()->LeaveBoosting();
    grabbed->GetMorphBall()->SetBoostEnabled(false);
  }
}

void CSandworm::ReleasePlayer(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->GetAttachedActorId() == GetUniqueId()) {
    player->DetachActorFromPlayer();
  }
  player->EnableLeaveMorphBall(true);
  player->AddMaterial(kMT_Solid, mgr);
  player->GetMorphBall()->SetBoostEnabled(true);
  mMorphballTossData.mHoldingPlayer = false;
  mMorphballTossData.mTossFinished = true;
}

void CSandworm::TossPlayer(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f forward = GetTransform().GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized() == true) {
    forward.Normalize();
  }
  if (mgr.IsRandomAvailable() == true) {
    mgr.Random()->Float();
    mgr.Random()->Float();
  }
  CVector3f direction = forward;
  if (direction.CanBeNormalized()) {
    direction.Normalize();
    direction *= mMorphballTossData.mImpulseHorizontal;
    CVector3f impulse = direction + mMorphballTossData.mImpulseVertical * CVector3f::Up();
    impulse *= 0.5f;
    PushPlayer(mgr, impulse);
    mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mMorphballTossData.mDamage,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
  }
}

void CSandworm::TeleportPlayerToToss(CStateManager& mgr) {
  const CTransform4f& tossXf = mMorphballTossData.mLocatorTransform;
  const CVector3f position = tossXf.GetTranslation() - 0.5f * tossXf.GetUp();
  CPlayer* player = mgr.GetPlayer(0);
  CTransform4f xf = player->GetTransform();
  xf.SetTranslation(position);
  player->Teleport(xf, mgr, false);
}

void CSandworm::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    SetActive(false);
  }
}

void CSandworm::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  if (msg == kStateMsg_Activate) {
    SetSegmentRange(3, 8);
    mSurfaced = true;
    mDying = true;
    GetHeadEye(mgr)->SetKilled();
    GetTailEye(mgr)->SetKilled();
    SpawnEyeKilledEffects(mgr, false);
  }
  if (BodyController()->GetBodyStateInfo().GetCurrentStateId() == 2) {
    if (!mDeathDeleted) {
      mDeathDeleted = true;
      DeathDelete(mgr);
    }
  }
}

void CSandworm::ResetAttackCooldown(CStateManager& mgr) {
  if (IsIngControlled() == 1) {
    const SLdrSandwormStruct& phase = GetPhaseStruct();
    if (!mgr.IsRandomAvailable()) {
      mAttackCooldown = phase.minTimeBetweenSequences;
    } else {
      mAttackCooldown =
          mgr.Random()->Range(phase.minTimeBetweenSequences, phase.maxTimeBetweenSequences);
    }
  } else if (!mgr.IsRandomAvailable()) {
    mAttackCooldown = GetAverageAttackTime();
  } else {
    const float variation = mAttackTimeVariation;
    const float average = GetAverageAttackTime();
    mAttackCooldown = variation * mgr.Random()->Float() + average;
  }
}

void CSandworm::UpdateAttackCooldown(float dt) {
  mAttackCooldown -= dt;
  if (mAttackCooldown < 0.f) {
    mAttackCooldown = 0.f;
  }
}

bool CSandworm::CanCharge(CStateManager& mgr, const CTriggerData& data) const {
  if (!CanAttack(mgr, CTriggerData(0.f)) || !InChargeRange(mgr, CTriggerData(0.f)) ||
      mForceUnderground == 1) {
    return false;
  }
  if (!IsIngControlled()) {
    if (!PlayerIsOnPathMesh(mgr, CTriggerData(0.f)) ||
        !FacingPlayerForCharge(mgr, CTriggerData(0.f)) ||
        !PlayerReachable(mgr, CTriggerData(0.f))) {
      return false;
    }
  }
  if (!FacingPlayer(mgr, CTriggerData(0.f))) {
    return false;
  }
  return true;
}

bool CSandworm::CanAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsBoss == 1 && !IsIngPossessed()) {
    return false;
  }
  if (IsBeingSnatched() == 1) {
    return false;
  }
  if (0.f == mMinAttackRange && 0.f == mMaxAttackRange) {
    return false;
  }
  if (IsIngControlled() == 1 && 2.5f + mLastRiseTime > mTime) {
    return false;
  }
  if (IsNearHint(mgr) == 1) {
    return false;
  }
  if (mForceUnderground == 1) {
    return false;
  }
  if (mAttackHistory.mCurrentAttack == kA_BombFountain) {
    return true;
  }
  return 0.f == mAttackCooldown;
}

bool CSandworm::PlayerIsOnPathMesh(CStateManager& mgr, const CTriggerData& data) const {
  return IsOnPathMesh(mgr.GetPlayer(0)->GetTranslation(), 0.f);
}

CVector3f CSandworm::GetRadarPointPosition(int index) const {
  if (mSpinePositions.size() > index) {
    return GetSpinePosition(index);
  }
  return GetSpineLocatorPosition(index);
}

int CSandworm::GetRadarPointCount() const {
  if (!GetActive() || !mAlive) {
    return 0;
  }
  return 13;
}

bool CSandworm::DoneLurking(CStateManager& mgr, const CTriggerData& data) const {
  return !GetActive() ? false : PlayerIsOnPathMesh(mgr, data) != 0;
}

float CSandworm::GetAverageModelScale() const {
  const CVector3f& scale = GetModelData()->GetScale();
  return (scale.GetX() + scale.GetY() + scale.GetZ()) / 3.f;
}

void CSandworm::PushPlayer(CStateManager& mgr, const CVector3f& impulse) {
  CPlayer* player = mgr.GetPlayer(0);
  player->Stop();
  player->ApplyImpulseWR(player->GetMass() * impulse, CAxisAngle::Identity());
  player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  mgr.RumbleManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
      ->Rumble(mgr, kRFX_CameraShake, 1.f, kRP_Two);
}

bool CSandworm::CanMeleeAgain(CStateManager& mgr, const CTriggerData& data) const {
  if (mAttackHistory.LastEntriesEqual(kA_Melee, 1) == 1) {
    return false;
  }
  return !(mSoundData.mWalkSoundTimer > mSoundData.mWalkSoundInterval - 2.f);
}

uchar CSandworm::IsHeadCollisionActor(CStateManager& mgr, const CCollisionActor* actor) const {
  const CJointCollisionDescription& desc = mCollisionActorManager->GetCollisionDescFromIndex(0);
  return TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId())) == actor;
}

uchar CSandworm::IsTailCollisionActor(CStateManager& mgr, const CCollisionActor* actor) const {
  const CJointCollisionDescription& desc = mCollisionActorManager->GetCollisionDescFromIndex(14);
  return TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId())) == actor;
}

float CSandworm::GetHealthPercent() const {
  return 100.f * (GetHealthInfo()->GetHP() / GetHealthInfo()->GetInitialHP());
}

bool CSandworm::ShouldSulk(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsBoss == 1) {
    return false;
  }
  if (GetHealthPercent() < 10.f) {
    return false;
  }
  return mSulkHealthDrop + GetHealthPercent() < mSulkHealthPercent;
}

bool CSandworm::SequenceAttack(CStateManager& mgr, const CTriggerData& data) const {
  return !IsIngControlled() ? false : mSequenceStep != 0;
}

void CSandworm::Sulk(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->SetLocomotionType(pas::kLT_Internal7);
  if (msg == kStateMsg_Activate) {
    const float minTime = mLurkUndergroundTimeMin;
    mSulkEndTime = (mLurkUndergroundTimeMax - minTime) * mgr.Random()->Float() + (mTime + minTime);
    mSulkHealthPercent = GetHealthPercent();
  } else if (msg == kStateMsg_Deactivate) {
    SetTransform(GetFarthestSpawnTransform(mgr, mSpawnIndex));
  }
}

bool CSandworm::DoneSulking(CStateManager& mgr, const CTriggerData& data) const {
  if (mTime < mSulkEndTime) {
    return false;
  }
  return PlayerIsOnPathMesh(mgr, data) != 0;
}

int CSandworm::CountGeneratedActors(CStateManager& mgr) const {
  int count = 0;
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state == kSS_GeneratorConnection && it->msg == kSM_Next) {
      TUniqueId id = mgr.GetIdForScript(it->objId);
      if (TCastToPtr< CActor >(mgr.ObjectById(id)) != nullptr) {
        ++count;
      }
    }
  }
  return count;
}

void CSandworm::SpawnEyes(CStateManager& mgr) {
  mHeadEyeId = mgr.AllocateUniqueId();
  CSandwormEye* headEye = rs_new CSandwormEye(
      mHeadEyeId, rstl::string_l("Sandworm Eye Head"),
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
      GetTransform());
  mTailEyeId = mgr.AllocateUniqueId();
  CSandwormEye* tailEye = rs_new CSandwormEye(
      mTailEyeId, rstl::string_l("Sandworm Eye Tail"),
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
      GetTransform());
  mgr.AddObject(headEye);
  mgr.AddObject(tailEye);
}

void CSandworm::UpdateEyeEffects(CStateManager& mgr, float dt) {
  bool headVulnerable = false;
  bool tailVulnerable = false;
  if (GetAlive() == true) {
    if (GetActive() == true) {
      if (5.f + mFountainData.mThresholdHitTime > mTime) {
        tailVulnerable = false;
        headVulnerable = false;
      } else if (mAttackHistory.mRunningAttack == kA_BombFountain) {
        if (!mFountainData.mBombLaunched) {
          headVulnerable = true;
          tailVulnerable = false;
        }
      } else if (IsAnyEyeKilled(mgr) == 0) {
        if (IsIngControlled() == 1) {
          headVulnerable = false;
          tailVulnerable = mAttackHistory.mRunningAttack == kA_Pursue;
        } else {
          headVulnerable = true;
          tailVulnerable = false;
        }
      } else {
        if (IsIngControlled() == 1) {
          headVulnerable = false;
        } else {
          headVulnerable = mEyeKillReactionDone;
        }
        tailVulnerable = false;
      }
    }
  }

  if (headVulnerable == true) {
    RemoveEyeAnimation(0);
  } else {
    AddEyeAnimation(0);
  }
  SetEyeVulnerable(mgr, 0, headVulnerable);
  if (tailVulnerable == true) {
    RemoveEyeAnimation(1);
  } else {
    AddEyeAnimation(1);
  }
  SetEyeVulnerable(mgr, 1, tailVulnerable);

  if (IsBeingSnatched() == true) {
    if (mEyeEffects.mGlow.mGenerator.get() != nullptr) {
      mEyeEffects.mGlow.mGenerator->SetParticleEmission(false);
    }
    if (mEyeEffects.mEffect1.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect1.mGenerator->SetParticleEmission(false);
    }
    if (mEyeEffects.mEffect2.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect2.mGenerator->SetParticleEmission(false);
    }
    if (mEyeEffects.mEffect3.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect3.mGenerator->SetParticleEmission(false);
    }
  } else if (headVulnerable == true || tailVulnerable == true) {
    CElementGen* active;
    CElementGen* inactive;
    if (IsIngControlled() == 1) {
      if (headVulnerable == true) {
        active = mEyeEffects.mEffect2.mGenerator.get();
      } else {
        active = mEyeEffects.mEffect3.mGenerator.get();
      }
      inactive = mEyeEffects.mGlow.mGenerator.get();
    } else {
      active = mEyeEffects.mGlow.mGenerator.get();
      inactive = mEyeEffects.mEffect2.mGenerator.get();
    }
    if (inactive != nullptr) {
      inactive->SetParticleEmission(false);
    }
    if (active != nullptr) {
      active->SetParticleEmission(true);
      active->Update(dt);
      active->SetGlobalScale(mPincerScale * GetModelData()->GetScale());
      if (headVulnerable == true) {
        active->SetGlobalTranslation(mFrontEyePosition);
      } else {
        active->SetGlobalTranslation(mBackEyePosition);
      }
    }
    if (mEyeEffects.mEffect1.mGenerator.get() != nullptr) {
      if (mIsBoss == 1) {
        mEyeEffects.mEffect1.mGenerator->SetParticleEmission(false);
      } else {
        float endTime = 1.f + mLastReverseTime;
        if (endTime > mTime && !mEyeKillReactionDone) {
          mEyeEffects.mEffect1.mGenerator->SetParticleEmission(true);
          mEyeEffects.mEffect1.mGenerator->Update(dt);
          float t = 0.1f + 0.8f * (1.f - (mTime - mLastReverseTime));
          mEyeEffects.mEffect1.mGenerator->SetGlobalScale(mPincerScale *
                                                          (t * GetModelData()->GetScale()));
          if (headVulnerable == true) {
            mEyeEffects.mEffect1.mGenerator->SetGlobalTranslation(mBackEyePosition);
          } else {
            mEyeEffects.mEffect1.mGenerator->SetGlobalTranslation(mFrontEyePosition);
          }
        } else {
          mEyeEffects.mEffect1.mGenerator->SetParticleEmission(false);
        }
      }
    }
  } else {
    if (mEyeEffects.mGlow.mGenerator.get() != nullptr) {
      mEyeEffects.mGlow.mGenerator->SetParticleEmission(false);
    }
    if (mEyeEffects.mEffect1.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect1.mGenerator->SetParticleEmission(false);
    }
    if (mEyeEffects.mEffect2.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect2.mGenerator->SetParticleEmission(false);
    }
    if (mEyeEffects.mEffect3.mGenerator.get() != nullptr) {
      mEyeEffects.mEffect3.mGenerator->SetParticleEmission(false);
    }
  }
}

void CSandworm::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
  if (mEyeEffects.mEffect2.mGenerator.get() != nullptr &&
      mEyeEffects.mEffect2.mGenerator->GetParticleEmission() == true) {
    gpRender->AddParticleGen(*mEyeEffects.mEffect2.mGenerator);
  }
  if (mEyeEffects.mEffect3.mGenerator.get() != nullptr &&
      mEyeEffects.mEffect3.mGenerator->GetParticleEmission() == true) {
    gpRender->AddParticleGen(*mEyeEffects.mEffect3.mGenerator);
  }
}

template < typename T >
void CSandworm::TryCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(cmd);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), state)) {
      BodyController()->CommandMgr().DeliverCmd(cmd);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSandworm::UpdateEyes(CStateManager& mgr, float dt) {
  UpdateEyeEffects(mgr, dt);
  CSandwormEye* headEye = GetHeadEye(mgr);
  CSandwormEye* tailEye = GetTailEye(mgr);
  if (!mAlive || !GetActive() || !mSurfaced) {
    if (headEye != nullptr) {
      headEye->SetOrbitable(mgr, false);
    }
    if (tailEye != nullptr) {
      tailEye->SetOrbitable(mgr, false);
    }
    return;
  }

  CVector3f headPosition = mFrontEyePosition;
  CVector3f tailPosition = mBackEyePosition;
  CVector3f headOrbit = mSpineHeadPosition;
  CVector3f tailOrbit = mSpineTailPosition;
  if (mChargeData.mStartPosition != CVector3f::Zero()) {
    CVector3f direction = mChargeData.mTargetPosition - mChargeData.mStartPosition;
    direction.Normalize();
    CVector3f headOffset = mFrontEyePosition - mChargeData.mStartPosition;
    float headDistance = headOffset.Magnitude();
    headPosition = mChargeData.mStartPosition + headDistance * direction;
    CVector3f tailOffset = mBackEyePosition - mChargeData.mStartPosition;
    float tailDistance = tailOffset.Magnitude();
    tailPosition = mChargeData.mStartPosition + tailDistance * direction;
  }

  bool swapEyes = false;
  if (headEye != nullptr && tailEye != nullptr) {
    if (!headEye->GetFlag0()) {
      headEye->SetFlag0();
      tailEye->SetFlag0();
    } else {
      if ((tailPosition - headEye->GetTranslation()).MagSquared() <
          (headPosition - headEye->GetTranslation()).MagSquared()) {
        swapEyes = true;
      }
    }
  }
  if (swapEyes == true) {
    CSandwormEye* eye = tailEye;
    tailEye = headEye;
    headEye = eye;
  }

  CVector3f headToOrbit = headPosition - headOrbit;
  CVector3f headToTailOrbit = headPosition - tailOrbit;
  float tailToOrbit = headToTailOrbit.Magnitude();
  if (headToOrbit.Magnitude() > tailToOrbit) {
    headOrbit = tailOrbit;
    tailOrbit = headOrbit;
  }

  bool headOrbitable;
  bool tailOrbitable;
  if (IsBeingSnatched() == true) {
    tailOrbitable = false;
    headOrbitable = false;
  } else if (mAttackHistory.mRunningAttack == kA_BombFountain) {
    headOrbitable = true;
    tailOrbitable = false;
  } else if (IsAnyEyeKilled(mgr) == 0) {
    if (IsIngControlled() == 1) {
      headOrbitable = false;
      tailOrbitable = true;
    } else {
      headOrbitable = true;
      tailOrbitable = false;
    }
  } else {
    headOrbitable = true;
    tailOrbitable = false;
  }

  UpdateBounds();
  const CAABox bounds = mCachedBounds;
  if (headEye != nullptr) {
    headEye->SetOrbitable(mgr, headOrbitable);
    headEye->SetTranslation(headPosition);
    headEye->SetTouchBounds(bounds);
    if (mAttackHistory.mRunningAttack != kA_GrabAttack) {
      headEye->SetOrbitPosition(headOrbit);
    }
  }
  if (tailEye != nullptr) {
    tailEye->SetOrbitable(mgr, tailOrbitable);
    tailEye->SetTranslation(tailPosition);
    tailEye->SetTouchBounds(bounds);
    if (mAttackHistory.mRunningAttack != kA_GrabAttack) {
      tailEye->SetOrbitPosition(tailOrbit);
    }
  }

  if (IsAnyEyeKilled(mgr) == 1 && mEyeKilledEffectsSpawned == 0) {
    mEyeKilledEffectsSpawned = 1;
    const CStateManager& constMgr = mgr;
    if (GetHeadEye(constMgr)->IsKilled() == 1) {
      SpawnEyeKilledEffects(mgr, false);
    } else {
      SpawnEyeKilledEffects(mgr, true);
    }
  }
}

void CSandworm::UpdateBounds() {
  if (mTime > mCachedBoundsTime) {
    mCachedBoundsTime = mTime;
    mCachedBounds = CalculateBounds();
  }
}

bool CSandworm::DoneStraightening(CStateManager& mgr, const CTriggerData& data) const {
  return mSpinePose.mStraightenBlend <= 0.002f;
}

void CSandworm::Straighten(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mSurfaced = false;
    mSpinePose.mStraightenBlend = 1.f;
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
  } else if (msg == kStateMsg_Deactivate) {
    mSurfaced = false;
    mWasSurfaced = false;
    mSpinePose.mStraightenBlend = 0.f;
    SetTransform(CQuaternion::FromMatrix(
                     CTransform4f::LookAt(mBackEyePosition, mFrontEyePosition, CVector3f::Up()))
                     .BuildTransform4f(GetTranslation()));
    SetTranslation(mSpineCenterPosition - GetPincerOffset());
  }
  if (mSpinePose.mStraightenBlend > 0.001f) {
    mSpinePose.mStraightenBlend -= 2.f * dt;
    if (mSpinePose.mStraightenBlend < 0.001f) {
      mSpinePose.mStraightenBlend = 0.001f;
    }
  }
}

void CSandworm::StopWalkSound() {
  mSoundData.mWalkSoundTimer = 0.f;
  if (mSoundData.mWalkHandle != CSfxHandle::NullHandle()) {
    CSfxManager::RemoveEmitter(mSoundData.mWalkHandle);
  }
}

void CSandworm::UpdateWalkSound(CStateManager& mgr, float dt) {
  mSoundData.mWalkSoundTimer -= dt;
  if (mSoundData.mWalkSoundTimer <= 0.f) {
    mSoundData.mWalkSoundInterval = 2.f * mgr.Random()->Float() + 4.f;
    mSoundData.mWalkSoundTimer += mSoundData.mWalkSoundInterval;
    if (mSoundData.mWalkHandle != CSfxHandle::NullHandle()) {
      CSfxManager::RemoveEmitter(mSoundData.mWalkHandle);
    }
    mSoundData.mWalkHandle = CSfxManager::AddEmitter(mSoundData.mWalkSound, mSpineCenterPosition,
                                                     127, GetCurrentAreaId().Value(), false, false,
                                                     CSfxManager::kMedPriority + 8);
  }
}

void CSandworm::PlayWalkVocalSound() {
  StopWalkVocalSound();
  mSoundData.mWalkVocalHandle = CSfxManager::AddEmitter(
      mSoundData.mWalkVocalSound, mSpineCenterPosition, 127, GetCurrentAreaId().Value(), false,
      true, CSfxManager::kMedPriority + 8);
}

void CSandworm::StopWalkVocalSound() {
  if (mSoundData.mWalkVocalHandle != CSfxHandle::NullHandle()) {
    CSfxManager::RemoveEmitter(mSoundData.mWalkVocalHandle);
    mSoundData.mWalkVocalHandle = CSfxHandle::NullHandle();
  }
}

void CSandworm::PlayMeleeSound() {
  mSoundData.mWalkSoundTimer = 6.f;
  if (mSoundData.mMeleeHandle != CSfxHandle::NullHandle()) {
    CSfxManager::RemoveEmitter(mSoundData.mMeleeHandle);
  }
  mSoundData.mMeleeHandle = CSfxManager::AddEmitter(
      mSoundData.mMeleeAttackSound, mSpineCenterPosition, 127, GetCurrentAreaId().Value(), false,
      false, CSfxManager::kMedPriority + 8);
}

void CSandworm::UpdateSoundEmitter(CSfxHandle& handle) {
  if (handle != CSfxHandle::NullHandle()) {
    if (!CSfxManager::IsPlaying(handle) && !CSfxManager::IsQueued(handle)) {
      CSfxManager::RemoveEmitter(handle);
      handle = CSfxHandle::NullHandle();
    } else {
      CSfxManager::UpdateEmitter(handle, mSpineCenterPosition, GetTransform().GetForward(), 127);
    }
  }
}

void CSandworm::UpdateSounds(CStateManager& mgr) {
  UpdateSoundEmitter(mSoundData.mMeleeHandle);
  UpdateSoundEmitter(mSoundData.mWalkHandle);
  UpdateSoundEmitter(mSoundData.mWalkVocalHandle);
}

bool CSandworm::PlayerHiding(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsBoss == 1) {
    return false;
  }
  return mPlayerOffPathTime > 8.f;
}

bool CSandworm::ForceUnderground(CStateManager& mgr, const CTriggerData& data) const {
  return mForceUnderground;
}

bool CSandworm::CanDescend(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsBoss == 1) {
    return false;
  }
  if (mPlayerOffPathTime > 6.f) {
    return true;
  }
  return mPursueTime > 3.f;
}

bool CSandworm::CanSpitAgain(CStateManager& mgr, const CTriggerData& data) const {
  return !mAttackHistory.LastEntriesEqual(kA_SpitAttack, 3);
}

bool CSandworm::UpdatePlayerReachable(CStateManager& mgr) const {
  if (mPlayerReachableTime + 1.f < mTime) {
    if (!mReachablePathSearch.Search(GetTranslation(), mgr.GetPlayer(0)->GetTranslation())) {
      mPlayerReachable = true;
    } else {
      mPlayerReachable = false;
    }
    mPlayerReachableTime = mTime;
  }
  return mPlayerReachable;
}

bool CSandworm::PlayerReachable(CStateManager& mgr, const CTriggerData& data) const {
  return UpdatePlayerReachable(mgr);
}

CBouncingBomb* CSandworm::SpawnBomb(CStateManager& mgr, const CVector3f& position) {
  CBouncingBomb* bomb = rs_new CBouncingBomb(
      *mBombData.mBombEffect, *mBombData.mExplosionEffect, mgr.AllocateUniqueId(),
      GetCurrentAreaId(), GetUniqueId(), 1.f, 3.f, kWT_Bomb, 256, CTransform4f::Translate(position),
      mBombData.mDamage, 8.f, GetBombPlacementSound(), GetBombBounceSound(), GetBombExplodeSound(),
      10.f, 0.01f);
  mgr.AddObject(*bomb);
  mLastBombDropTime = mTime;
  return bomb;
}

void CSandworm::ResetBombTimer() { mBombDropTimer = 0.f; }

bool CSandworm::CanDropBomb(CStateManager& mgr) const {
  if (IsIngControlled() == 1 && mBombDropRate > 0.f && !IsAnyEyeKilled(mgr)) {
    return true;
  }
  return false;
}

void CSandworm::UpdateBombDrop(CStateManager& mgr, float dt) {
  if (!CanDropBomb(mgr)) {
    mBombDropTimer = 0.f;
  } else {
    mBombDropTimer += dt;
    if (mBombDropTimer > mBombDropRate) {
      mBombDropTimer -= mBombDropRate;
      SpawnBomb(mgr, mBackEyePosition);
    }
  }
}

bool CSandworm::IsIngControlled() const {
  bool result = false;
  if (mIsBoss == 1 && IsIngPossessed() == 1 && !IsBeingSnatched()) {
    result = true;
  }
  return result;
}

const SLdrSandwormStruct& CSandworm::GetPhaseStruct() const {
  const float health = GetHealthInfo()->GetHP();
  if (health > mSandwormStruct0->unknown_0x98106ee2) {
    return *mSandwormStruct0;
  }
  if (health > mSandwormStruct1->unknown_0x98106ee2) {
    return *mSandwormStruct1;
  }
  if (health > mSandwormStruct2->unknown_0x98106ee2) {
    return *mSandwormStruct2;
  }
  if (health > mSandwormStruct3->unknown_0x98106ee2) {
    return *mSandwormStruct3;
  }
  return *mSandwormStruct4;
}

int CSandworm::GetNextAttack(CStateManager& mgr) const {
  const SLdrSandwormStruct& phase = GetPhaseStruct();
  int attack;
  switch (mSequenceStep) {
  case 0:
    attack = phase.unknown_0x59f14d7c;
    break;
  case 1:
    attack = phase.unknown_0x9606b4b0;
    break;
  default:
    attack = phase.unknown_0xfc2697dd;
    break;
  }
  switch (attack) {
  case 0:
    return kA_SpitAttack;
  case 1:
    return kA_BombToss;
  case 2:
    return kA_BombSpread;
  case 3:
    if (mAttackHistory.LastEntriesEqual(kA_BombSpread, 1) == 1) {
      return kA_BombToss;
    }
    if (mAttackHistory.LastEntriesEqual(kA_BombToss, 1) == 1) {
      return kA_BombSpread;
    }
    if (mgr.IsRandomAvailable() == 1 && mgr.Random()->Range(0.f, 1.f) < 0.5f) {
      return kA_BombSpread;
    }
    return kA_BombToss;
  case 4:
    return kA_BombFountain;
  }
  return -1;
}

void CSandworm::PickMediumAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    if (!IsIngControlled()) {
      mAttackHistory.mCurrentAttack = kA_SpitAttack;
    } else if (mAttackHistory.mCurrentAttack != kA_BombFountain) {
      mAttackHistory.mCurrentAttack = GetNextAttack(mgr);
      ++mSequenceStep;
      if (mSequenceStep > 2 || GetNextAttack(mgr) == -1) {
        mSequenceStep = 0;
      }
    }
  }
}

bool CSandworm::PickedSpitAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackHistory.mCurrentAttack == kA_SpitAttack;
}

bool CSandworm::PickedBombToss(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackHistory.mCurrentAttack == kA_BombToss;
}

bool CSandworm::PickedBombSpread(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackHistory.mCurrentAttack == kA_BombSpread;
}

bool CSandworm::PickedBombFountain(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackHistory.mCurrentAttack == kA_BombFountain;
}

int CSandworm::IsAnyEyeKilled(const CStateManager& mgr) const {
  if (IsIngControlled() == 1) {
    return false;
  }
  if (GetHeadEye(mgr) != nullptr && GetTailEye(mgr) != nullptr && !GetHeadEye(mgr)->IsKilled() &&
      !GetTailEye(mgr)->IsKilled()) {
    return false;
  }
  return true;
}

bool CSandworm::OneEyeKilled(CStateManager& mgr, const CTriggerData& data) const {
  if (mEyeKillReactionDone == 1 || mIsBoss == 1) {
    return false;
  }
  int killed = 0;
  if (GetHeadEye(mgr)->IsKilled() == 1) {
    killed = 1;
  }
  if (GetTailEye(mgr)->IsKilled() == 1) {
    killed += 1;
  }
  return killed == 1;
}

bool CSandworm::HitDuringWindUp(CStateManager& mgr, const CTriggerData& data) const {
  return mFountainData.mHitDuringWindUp;
}

void CSandworm::EyeKilledReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  SetSegmentRange(3, 13);
  mSurfaced = true;
  TryCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Zero));
  if (msg == kStateMsg_Deactivate && IsAnyEyeKilled(mgr) == 1) {
    mEyeKillReactionDone = true;
    if (!mIsBoss) {
      ReverseDirection(mgr);
    } else if (mFountainData.mEyeKilledDuringFountain == 1) {
      ReverseDirection(mgr);
    }
  }
}

void CSandworm::Interrupt(CStateManager& mgr, float dt) {
  BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
  BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
}

CVector3f CSandworm::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (mIsBoss == 1 && !IsIngControlled()) {
    if (!mLocatorsValid) {
      return GetTranslation();
    }
    return mSpineHeadPosition;
  }
  const CSandwormEye* head = GetHeadEye(mgr);
  if (head != nullptr && !head->IsKilled() && head->IsOrbitable() == 1) {
    return head->GetAimPosition(mgr, dt);
  }
  const CSandwormEye* tail = GetTailEye(mgr);
  if (tail != nullptr && !tail->IsKilled() && tail->IsOrbitable() == 1) {
    return tail->GetAimPosition(mgr, dt);
  }
  return mSpineCenterPosition;
}

CVector3f CSandworm::GetIngSnatchingNormal(float t) const {
  CVector3f offset = mSpineHeadPosition - mSpineTailPosition;
  if (!offset.CanBeNormalized()) {
    return GetTransform().GetForward();
  }
  return offset.AsNormalized();
}

CVector3f CSandworm::GetIngSnatchingPoint(float t) const {
  const float t3 = t * t * t;
  return CVector3f((1.f - t3) * mSpineHeadPosition.GetX() + mSpineTailPosition.GetX() * t3,
                   (1.f - t3) * mSpineHeadPosition.GetY() + mSpineTailPosition.GetY() * t3,
                   (1.f - t3) * mSpineHeadPosition.GetZ() + mSpineTailPosition.GetZ() * t3);
}

bool CSandworm::CanBeIngPossessed(CStateManager& mgr) const {
  return !mIsBoss ? false : CPatterned::CanBeIngPossessed(mgr);
}

bool CSandworm::CanBeUnPossessed(CStateManager& mgr) const { return false; }

bool CSandworm::HasUnknown11Hint(CStateManager& mgr) const {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetHintType() == CScriptAIHint::kHT_Unknown11) {
      return true;
    }
  }
  return false;
}

bool CSandworm::IsNearHint(CStateManager& mgr) const {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId()) {
      if (mIsBoss == 1) {
        if (hint->GetHintType() == CScriptAIHint::kHT_Unknown11) {
          CVector3f offset = mSpineCenterPosition - hint->GetTranslation();
          offset.SetZ(0.f);
          if (offset.Magnitude() > hint->GetRadius()) {
            return true;
          }
          offset = mSpineHeadPosition - hint->GetTranslation();
          offset.SetZ(0.f);
          if (offset.Magnitude() > hint->GetRadius()) {
            return true;
          }
          offset = 0.5f * (mSpineCenterPosition + mSpineHeadPosition) - hint->GetTranslation();
          offset.SetZ(0.f);
          if (offset.Magnitude() > hint->GetRadius()) {
            return true;
          }
        }
      } else if (hint->GetHintType() == CScriptAIHint::kHT_Unknown10) {
        CVector3f offset = mSpineCenterPosition - hint->GetTranslation();
        offset.SetZ(0.f);
        if (offset.Magnitude() < hint->GetRadius()) {
          return true;
        }
        offset = mSpineHeadPosition - hint->GetTranslation();
        offset.SetZ(0.f);
        if (offset.Magnitude() < hint->GetRadius()) {
          return true;
        }
        offset = 0.5f * (mSpineCenterPosition + mSpineHeadPosition) - hint->GetTranslation();
        offset.SetZ(0.f);
        if (offset.Magnitude() < hint->GetRadius()) {
          return true;
        }
      }
    }
  }
  return false;
}

CVector3f CSandworm::GetOrbitPosition(const CStateManager& mgr) const {
  if (!mSurfaced || mScanVisorActive == 1) {
    return mFrontEyePosition;
  }
  const CSandwormEye* eye = GetHeadEye(mgr);
  if (eye != nullptr) {
    return eye->GetOrbitPosition(mgr);
  }
  return CPatterned::GetOrbitPosition(mgr);
}

void CSandworm::RenderIngSnatchingTransition(const CStateManager& mgr) const {
  const CVector3f normal = GetIngSnatchingNormal(mIngPossessionBlend);
  const CVector3f point = GetIngSnatchingPoint(mIngPossessionBlend);
  CAnimData* animData = const_cast< CAnimData* >(GetAnimationData());
  const CVector3f& overlap = (0.5f * GetIngSnatchingModelOverlapSize()) * normal;

  const CVector3f& ingPoint = point - overlap;
  const CPlane ingPlane(ingPoint, CUnitVector3f(normal, CUnitVector3f::kN_No));
  GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), ingPlane);
  animData->SetSkinnedModel(*mIngModel);
  CPhysicsActor::Render(mgr);

  const CVector3f& normalPoint = point + overlap;
  const CVector3f negNormal = -1.f * normal;
  const CPlane normalPlane(normalPoint, CUnitVector3f(negNormal, CUnitVector3f::kN_No));
  GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), normalPlane);
  animData->SetSkinnedModel(mNormalModel);
  const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
  CTransform4f xf = GetTransform();
  xf.ScaleBy(0.99f);
  GetModelData()->Render(which, xf, GetActorLights(), GetModelFlags());
}

CSandworm::SAttackHistory::SAttackHistory() : mCurrentAttack(-1), mRunningAttack(-1) {}

void CSandworm::SAttackHistory::AddAttack(EAttack attack) {
  while (mHistory.size() >= 5) {
    mHistory.erase(mHistory.begin());
  }
  mHistory.push_back(attack);
}

void CSandworm::SAttackHistory::Clear() { mHistory.clear(); }

bool CSandworm::SAttackHistory::LastEntriesEqual(int attack, int count) const {
  for (int i = 0; i < count; ++i) {
    const int index = mHistory.size() - (i + 1);
    if (index < 0) {
      return false;
    }
    if (attack != mHistory[index]) {
      return false;
    }
  }
  return true;
}

CSandwormEye::CSandwormEye(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf)
: CPhysicsActor(uid, name, info, 0, xf, CModelData::CModelDataNull(),
                CMaterialList(kMT_Orbit, kMT_Target, kMT_SeekerTarget), CAABox::MakeNullBox(),
                SMoverData(0.f), CActorParameters::None(), CPhysicsActor::skDefaultStepData)
, mTouchBounds(CAABox::MakeNullBox())
, mOrbitPosition(CVector3f::Zero())
, mPositionInitialized(false)
, mKilled(false)
, mRelToken(skRelName, 0) {}

CSandwormEye::~CSandwormEye() {}

CVector3f CSandwormEye::GetOrbitPosition(const CStateManager& mgr) const { return mOrbitPosition; }

CVector3f CSandwormEye::GetAimPosition(const CStateManager& mgr, float dt) const {
  return GetTranslation();
}

rstl::optional_object< CAABox > CSandwormEye::GetTouchBounds() const {
  return rstl::optional_object< CAABox >(mTouchBounds);
}

void CSandwormEye::SetTouchBounds(const CAABox& bounds) { mTouchBounds = bounds; }

void CSandwormEye::SetOrbitable(CStateManager& mgr, bool orbitable) {
  if (orbitable == true) {
    AddMaterial(kMT_Orbit, kMT_SeekerTarget, mgr);
  } else {
    RemoveMaterial(kMT_Orbit, kMT_SeekerTarget, mgr);
  }
  SetActive(orbitable);
}

bool CSandwormEye::IsOrbitable() const { return GetMaterialList().HasMaterial(kMT_Orbit); }

bool CSandwormEye::GetFlag0() const { return mPositionInitialized; }

void CSandwormEye::SetFlag0() { mPositionInitialized = true; }

void CSandwormEye::SetOrbitPosition(const CVector3f& position) { mOrbitPosition = position; }

CVector3f CSandwormEye::GetOrbitPosition() const { return mOrbitPosition; }

bool CSandwormEye::IsKilled() const { return mKilled; }

void CSandwormEye::SetKilled() { mKilled = true; }

CSandworm::SBombData::SBombData(CAssetId bombEffect, CAssetId explosionEffect,
                                const CDamageInfo& damage)
: mBombEffect(TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', bombEffect))))
, mExplosionEffect(
      TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', explosionEffect))))
, mDamage(damage) {}

CSandworm::SBombData::SBombData() : mBombEffect(), mExplosionEffect(), mDamage() {}

CSandworm::SEyeEffect::SEyeEffect(CAssetId effect) : mActive(false), mGenerator(nullptr) {
  CElementGen* gen = nullptr;
  if (effect != kInvalidAssetId) {
    gen = rs_new CElementGen(
        TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', effect))),
        CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  }
  mActive = gen != nullptr;
  mGenerator = gen;
}

CSandworm::SEyeEffect::~SEyeEffect() {}

static inline ushort GetSoundId(int id) {
  return id == -1 ? CSfxManager::kInternalInvalidSfxId : id;
}

CEntity* LoadSandworm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSandworm sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSandworm.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  if (sldrThis.canLinkTransfer == true) {
    return rs_new CSandworm(
        mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
        LdrToEntityInfo(info, sldrThis.editorProperties),
        LdrToTransform4f(sldrThis.editorProperties), *modelData,
        LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
        sldrThis.patterned.stateMachine2, sldrThis.pincerScale, sldrThis.spitAttackMinRange,
        sldrThis.spitAttackMaxRange, sldrThis.unknown_0x61f75902, sldrThis.chargeRangeMin,
        sldrThis.chargeRangeMax, sldrThis.chargeImpulseHorizontal, sldrThis.chargeImpulseVertical,
        sldrThis.unknown_0x06dee4c5 & 1, sldrThis.pincerL, sldrThis.pincerR,
        GetSoundId(sldrThis.walkSound), GetSoundId(sldrThis.walkVocalSound),
        GetSoundId(sldrThis.meleeAttackSound), GetSoundId(sldrThis.eyeKilledSound),
        GetSoundId(sldrThis.bombBounceSound), GetSoundId(sldrThis.bombExplodeSound),
        sldrThis.spitAttackVisorEffect, sldrThis.morphballTossImpulseHorizontal,
        sldrThis.morphballTossImpulseVertical, sldrThis.meleeImpulseHorizontal,
        sldrThis.meleeImpulseVertical, sldrThis.unknown_0xe593f1c6, sldrThis.lurkUndergroundTimeMin,
        sldrThis.lurkUndergroundTimeMax, sldrThis.pursuitFrustrationRadius,
        sldrThis.pursuitFrustrationTimer, sldrThis.projectile,
        LdrToDamageInfo(sldrThis.projectileDamage), LdrToDamageInfo(sldrThis.morphballTossDamage),
        LdrToDamageInfo(sldrThis.pincerSwipeDamage), sldrThis.eyeGlow, sldrThis.pART,
        sldrThis.pART_0x8b2a15ee, sldrThis.ingBossBombFX, sldrThis.ingBossBombExplosionFX,
        LdrToDamageInfo(sldrThis.ingBossBombDamage), sldrThis.ingBossBombDropRate,
        sldrThis.unknown_0x547f9400, sldrThis.unknown_0xefef7b45, sldrThis.sandwormStruct,
        sldrThis.sandwormStruct_0xce246628, sldrThis.sandwormStruct_0x55578cfc,
        sldrThis.sandwormStruct_0x23ee1452, sldrThis.sandwormStruct_0xb89dfe86,
        LdrToActorParameters(sldrThis.actorInformation));
  }

  return rs_new CSandworm(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
      sldrThis.patterned.stateMachine2, sldrThis.pincerScale, sldrThis.spitAttackMinRange,
      sldrThis.spitAttackMaxRange, sldrThis.unknown_0x61f75902, sldrThis.chargeRangeMin,
      sldrThis.chargeRangeMax, sldrThis.chargeImpulseHorizontal, sldrThis.chargeImpulseVertical,
      sldrThis.unknown_0x06dee4c5 & 1, sldrThis.pincerL, sldrThis.pincerR,
      GetSoundId(sldrThis.walkSound), GetSoundId(sldrThis.walkVocalSound),
      GetSoundId(sldrThis.meleeAttackSound), GetSoundId(sldrThis.eyeKilledSound),
      sldrThis.spitAttackVisorEffect, sldrThis.morphballTossImpulseHorizontal,
      sldrThis.morphballTossImpulseVertical, sldrThis.meleeImpulseHorizontal,
      sldrThis.meleeImpulseVertical, sldrThis.unknown_0xe593f1c6, sldrThis.lurkUndergroundTimeMin,
      sldrThis.lurkUndergroundTimeMax, sldrThis.pursuitFrustrationRadius,
      sldrThis.pursuitFrustrationTimer, sldrThis.projectile,
      LdrToDamageInfo(sldrThis.projectileDamage), LdrToDamageInfo(sldrThis.morphballTossDamage),
      LdrToDamageInfo(sldrThis.pincerSwipeDamage), sldrThis.eyeGlow,
      LdrToActorParameters(sldrThis.actorInformation));
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SSandworm_FuncPtrs funcPtrs;
  funcPtrs.mLoadSandworm = &LoadSandworm;
  funcPtrs.mGetRadarPointPosition = &CSandworm::GetRadarPointPosition;
  funcPtrs.mGetRadarPointCount = &CSandworm::GetRadarPointCount;
  SetSSandworm_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSandworm_FuncPtrs(nullptr); }
#endif
