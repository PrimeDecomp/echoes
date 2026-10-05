#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerRagDoll.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Basics/CCast.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CControlHintManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CPlayerHintManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerBodyController.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerStuckTracker.hpp"
#include "MetroidPrime/Player/CPlayerTargeting.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "dolphin/os/OSCache.h"
#include "rstl/math.hpp"
#include <float.h>
#include <math.h>
#include <string.h>

// NonMatching structure pass; incomplete behavior is explicit below.
// Definitions follow reverse target order for the TU's deferred-inlining emission.

const bool kDoubleJumpBreaksOrbit = false;
const bool kDashDoubleJumpBreaksOrbit = false;
const bool gkFreeLookPreventsOrbitMovement = true;
const bool gkAutoAim = false;
const bool gkAutoAimAtOrbitedObject = true;
const int gkMorphBallOrbitMode = 1;

bool gUseSurfaceHack = false;
CPlayer::ESurfaceRestraints gSR_Hack = CPlayer::kSR_Normal;

static CColor skLaggedBurnDeathColor(uchar(255), uchar(255), uchar(192), uchar(255));

static const char* const kGunLocator = "GUN_LCTR";

static const char* const skThirdPersonChargeNames[4] = {
    "PowerChargeThirdPerson", "DarkChargeThirdPerson", "LightChargeThirdPerson",
    "AnnihilatorChargeThirdPerson"};

static const char* const skThirdPersonMuzzleNames[4] = {
    "PowerMuzzleThirdPerson", "DarkMuzzleThirdPerson", "LightMuzzleThirdPerson",
    "AnnihilatorMuzzleThirdPerson"};

typedef rstl::pair< const char*, const char* > TSuitTransitionModelNames;
static TSuitTransitionModelNames skSuitTransitionModelNames[6] = {
    TSuitTransitionModelNames("", ""),
    TSuitTransitionModelNames("BallTransition_Dark_CDML", "BallTransition_Dark_CSKR"),
    TSuitTransitionModelNames("BallTransition_Light_CDML", "BallTransition_Light_CSKR"),
    TSuitTransitionModelNames("", ""),
    TSuitTransitionModelNames("BallTransition_Dark_GravityBoost_CDML",
                              "BallTransition_Dark_GravityBoost_CSKR"),
    TSuitTransitionModelNames("BallTransition_Light_CDML", "BallTransition_Light_CSKR")};

const float CPlayer::skDefaultHudFadeOutSpeed = 0.5f;
const float CPlayer::skDefaultHudFadeInSpeed = 2.5f;

typedef rstl::pair< CPlayerState::EItemType, CControlMapper::ECommands > TVisorToItemMapping;
static TVisorToItemMapping skVisorToItemMapping[4] = {
    TVisorToItemMapping(CPlayerState::kIT_CombatVisor, CControlMapper::kC_NoVisor),
    TVisorToItemMapping(CPlayerState::kIT_EchoVisor, CControlMapper::kC_XrayVisor),
    TVisorToItemMapping(CPlayerState::kIT_ScanVisor, CControlMapper::kC_EnviroVisor),
    TVisorToItemMapping(CPlayerState::kIT_DarkVisor, CControlMapper::kC_ThermoVisor)};

static CDamageVulnerability::TWeaponVulnerability skDarkSuitVulnerabilities[2] = {
    CDamageVulnerability::TWeaponVulnerability(kWT_AreaLight, CWeaponTypeVulnerability::Immune()),
    CDamageVulnerability::TWeaponVulnerability(kWT_AreaDark, CWeaponTypeVulnerability::Immune())};

static CDamageVulnerability::TWeaponVulnerability skLightSuitVulnerabilities[2] = {
    CDamageVulnerability::TWeaponVulnerability(kWT_AreaDark, CWeaponTypeVulnerability::Immune()),
    CDamageVulnerability::TWeaponVulnerability(kWT_PoisonWater2,
                                               CWeaponTypeVulnerability::Immune())};

static const ushort skLeftStepSounds[2][26] = {
    {0xffff, 0x0084, 0x00a0, 0x05d8, 0x1d44, 0xffff, 0x009e, 0x1c19, 0x00a2,
     0x1c5f, 0x1c61, 0x1d46, 0x1c6f, 0xffff, 0x1c72, 0x1c74, 0xffff, 0x1ad1,
     0x1d3a, 0x1d3c, 0xffff, 0xffff, 0x04c6, 0x0622, 0xffff, 0x1d5c},
    {0xffff, 0x26f6, 0x26f0, 0x26ec, 0x2723, 0xffff, 0x26ee, 0x2702, 0x26ea,
     0x2713, 0x2715, 0x2725, 0x2717, 0xffff, 0x2719, 0x271b, 0xffff, 0x26f4,
     0x2732, 0x2734, 0xffff, 0xffff, 0x26fa, 0x26f2, 0xffff, 0x272a}};

static const ushort skRightStepSounds[2][26] = {
    {0xffff, 0x0085, 0x00a1, 0x05d9, 0x1d45, 0xffff, 0x009f, 0x1c1a, 0x0183,
     0x1c60, 0x1c62, 0x1d47, 0x1c70, 0xffff, 0x1c73, 0x1c75, 0xffff, 0x1ad2,
     0x1d3b, 0x1d3d, 0xffff, 0xffff, 0x04c7, 0x0623, 0xffff, 0x1d5d},
    {0xffff, 0x26f7, 0x26f1, 0x26ed, 0x2724, 0xffff, 0x26ef, 0x2703, 0x26eb,
     0x2714, 0x2716, 0x2726, 0x2718, 0xffff, 0x271a, 0x271c, 0xffff, 0x26f5,
     0x2733, 0x2735, 0xffff, 0xffff, 0x26fb, 0x26f3, 0xffff, 0x272b}};

static void StopSound(CSfxHandle& sound) {
  if (sound) {
    CSfxManager::SfxStop(sound);
    sound.Clear();
  }
}

CVector3f CCollisionInfoList::GetCombinedNormalLeft() const {
  CVector3f normal = CVector3f::Zero();
  for (const CCollisionInfo* collision = Begin(); collision != End(); ++collision) {
    normal += collision->GetNormalLeft();
  }
  if (normal.IsNonZero()) {
    return normal.AsNormalized();
  }
  return normal;
}

CPlayer::CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
                 CAssetId stateMachine, float mass, float stepUp, float stepDown, float ballRadius,
                 const CMaterialList& ml, CPlayerState* playerState, CCameraManager* cameraManager,
                 bool multiplayer, int playerIndex, int controlScheme, int charIdx)
: CPhysicsActor(uid, CBasics::Stringize("CPlayer (%d)", playerIndex),
                CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, true), 0, xf,
                CAnimRes(resId, charIdx, CVector3f(1.8f, 1.8f, 1.8f), 0, true), ml, aabb,
                SMoverData(mass), CActorParameters::None().HotInThermal(true),
                StepData(stepUp, stepDown, 1))

, mMovementState(NPlayer::kMS_OnGround)
, mBallTransitionsRes()
, mAttachedActor(kInvalidUniqueId)
, mAttachedActorTime(0.f)
, mEnergyDrain(4)
, mStartingJumpTimeout(0.f)
, mSjTimer(0.f)
, mMinJumpTimeout(0.f)
, mJumpCameraTimer(0.f)
, mJumpPresses(0)
, mFallCameraTimer(0.f)
, mAirborneTimer(0.f)
, mCancelCameraPitch(false)
, mTimeSinceJump(1000.f)
, mTimeSinceDoubleJump(1000.0f)
, mTimeSinceScrewAttackRequest(0.f)
, mLastJumpPosition(CVector3f::Zero())
, mLastSpaceJumpPosition(CVector3f::Zero())
, mSurfaceRestraint(kSR_Normal)
, mAccelerationTable()
, mCurAcceleration(1)
, mAccelerationChangeTimer(0.f)
, mFpBounds(aabb)
, mBallTransHeight(1.f)
, mCameraState(kCS_FirstPerson)
, mMorphBallState(kMS_Unmorphed)
, mSpawnedMorphBallState(kMS_Unmorphed)
, mScrewAttackTransitionPending(false)
, mScrewAttackTransitionState(kMS_Unmorphed)
, mCinematicMorphBallState(kMS_Unmorphed)
, mFallingTime(0.f)
, mOrbitState(kOS_NoOrbit)
, mOrbitType(kOT_Close)
, mOrbitRequest(3)
, mOrbitTargetId(kInvalidUniqueId)
, mOrbitPoint(0.f, 0.f, 0.f)
, mOrbitVector(0.f, 0.f, 0.f)
, mOrbitModeTimer(0.f)
, mOrbitZoneMode(kZI_Targeting)
, mOrbitZoneType(kZT_Ellipse)
, mOrbitScreenBoxType(1)
, mOrbitNextTargetId(kInvalidUniqueId)
, mOrbitPointDistance(0.f)
, mNearbyOrbitObjects()
, mOnScreenOrbitObjects()
, mOffScreenOrbitObjects()
, mOrbitLockEstablished(false)
, mOrbitPreventionTimer(0.f)
, mSidewaysDashing(false)
, mStrafeInputAtDash(0.f)
, mDashTimer(0.f)
, mDashButtonHoldTime(0.f)
, mDoneSidewaysDashing(false)
, mOrbitSource(2)
, mOrbitingEnemy(false)
, mOrbitTargetLineOfSightClear(false)
, mOrbitOcclusionTimer(0.f)
, mOrbitCandidateIndex(0)
, mOrbitCandidateRefreshFrames(0x14)
, mDashSpeedMultiplier(1.5f)
, mNoStrafeDashBlend(false)
, mDashDuration(0.5f)
, mStrafeDashBlendDuration(0.45f)
, mScanState(kSS_NotScanning)
, mScanningTime(0.f)
, mCurScanTime(0.f)
, mScanningObject(kInvalidUniqueId)
, mScanningObjectId(kInvalidAssetId)
, mGrappleState(kGS_None)
, mGrappleSwingTimer(0.f)
, mGrappleSwingAxis(0.f, 1.f, 0.f)
, x5e0_(0.f)
, x5e4_(0.f)
, x5e8_(0.f)
, mGrappleJumpTimeout(0.f)
, mGrappleLocator()
, mInFreeLook(false)
, mLookButtonHeld(false)
, mLookAnalogHeld(false)
, mFreeLookAnglesHeld(false)
, mFreeLookInputLatched(false)
, mCurFreeLookCenteredTime(0.f)
, mFreeLookYawAngle(0.f)
, mHorizFreeLookAngleVel(0.f)
, mFreeLookPitchAngle(0.f)
, mVertFreeLookAngleVel(0.f)
, mAimTarget(kInvalidUniqueId)
, mTargetAimPosition(CVector3f::Zero())
, mAimTargetAverage()
, mAssistedTargetAim(CVector3f::Zero())
, mAimTargetTimer(0.f)
, mAimCandidates()
, mAimCandidateIndex(0)
, mAimCandidateRefreshFrames(20)
, mAimTargetDistance(10000.f)
, mAimTargetScreenDistance(10000.f)
, mGun(rs_new CPlayerGun(uid, multiplayer))
, mGunAlpha(1.f)
, mPlayerDrawFlags(CModelFlags::kT_Opaque, 1.f)
, mTransitionBeamShader(0)
, mTargeting(rs_new CPlayerTargeting(uid))
, mBodyController(nullptr)
, mKnockBackManager()
, mRagDoll(nullptr)
, mPlayerStuckTracker(rs_new CPlayerStuckTracker())
, mMoveSpeedAvg()
, mMoveSpeed(0.f)
, mFlatMoveSpeed(0.f)
, mLookDir(GetTransform().GetForward())
, mMoveDir(GetTransform().GetForward())
, mLeaveMorphDir(GetTransform().GetForward())
, mLastPosForDirCalc(GetTranslation())
, mGunDir(GetTransform().GetForward())
, mTimeMoving(0.f)
, mControlDir(GetTransform().GetForward())
, mControlDirFlat(GetTransform().GetForward())
, mVariaSuitVulnerability(CDamageVulnerability::NormalVulnerabilty())
, mDarkSuitVulnerability(mVariaSuitVulnerability, skDarkSuitVulnerabilities, 2,
                         CDamageVulnerability::kOF_Normal)
, mLightSuitVulnerability(mDarkSuitVulnerability, skLightSuitVulnerabilities, 2,
                          CDamageVulnerability::kOF_Normal)
, mImmuneVulnerability(CDamageVulnerability::ReflectVulnerabilty())
, mScrewAttackVulnerability(CDamageVulnerability::ReflectVulnerabilty())
, mWasDamaged(false)
, mWasDamagedPrev(false)
, mDamageAmount(0.f)
, mPrevDamageAmount(0.f)
, mDamageLocation(CVector3f::Zero())
, mDamageWeaponType(~0u)
, mImmuneTimer(0.f)
, mMorphTime(0.f)
, mMorphDuration(0.f)
, mAlpha(1.f)
, x1144_24_(true)
, x1144_25_(true)
, mStaticTimer(0.f)
, mStaticOutSpeed(0.f)
, mStaticInSpeed(0.f)
, mVisorStaticAlpha(1.f)
, mFrozenTimeout(0.f)
, mIceBreakJumps(0)
, mFrozenDamage(0.f)
, mRezbitState(kRS_None)
, mRezbitGunDrawBlocks()
, mRezbitControlHintId(kInvalidUniqueId)
, mRezbitVirusMemoTimer(0.f)
, mMorphBall(nullptr)
, mCameraBob(rs_new CPlayerCameraBob(CPlayerCameraBob::kCBT_One))
, mDamageLoopSfx()
, mSamusVoiceTimeout(0.f)
, mDashSfx()
, mSamusVoiceSfx()
, mSamusVoicePriority(0)
, mDamageSfxTimer(0.f)
, mTimeSinceDamageImpactSfx(0.f)
, mDamageLoopSfxId(0)
, mDarkAetherDamageLoopSfx()
, mDarkAetherDamageLoopSfxTimer(0.f)
, mDarkAetherDamageLoopSfxId(0xffff)
, mFootstepSfxTimer(0.f)
, mFootstepSfxSel(0)
, mLastVelocity(CVector3f::Zero())
, mVisorSteam(0.f, 0.f, 0.f, kInvalidAssetId)
, x11e4_(1.f)
, x11e8_(1.f)
, mTransitionSuit(CPlayerState::kPS_Varia)
, mAnimRes(resId, charIdx, CVector3f(1.8f, 1.8f, 1.8f), 0, true)
, mTransitionBeam(CPlayerState::kBI_Power)
, mBallTransitionBeamModel(nullptr)
, mGunWorldXf(CTransform4f::Identity())
, mTransitionFilterTimer(0.f)
, mDistanceUnderWater(0.f)
, mRidingPlatform(kInvalidUniqueId)
, mGravityBoostDuration(0.f)
, mGravityBoostSfx()
, mGravityBoostEndSfx()
, mGravityBoostUsed(false)
, mScreenFilterColor(1.f, 1.f, 1.f, 0.f)
, mPlayerHintManager(rs_new CPlayerHintManager(playerIndex, rstl::string_l("Player Hint Manager")))
, mVisorChangeRequested(false)
, mDrawCrosshairs(false)
, x1268_26_(true)
, mCanEnterMorphBall(true)
, mCanLeaveMorphBall(true)
, mSpiderBallControlXY(false)
, mControlDirectionOverridden(false)
, mInSafeZone(false)
, mSlidingOnWall(false)
, mHitWallDuringMove(false)
, mSelectFluidBallSound(false)
, mStepCameraZBiasDirty(true)
, mExtendTargetDistance(false)
, mInterpolatingControlDir(false)
, mOutOfBallLookAtHint(false)
, mIgnoreDarkWorldDamage(false)
, mNoSafeZoneHealing(false)
, mNoMorphBallDamageTimer(false)
, mAimingAtProjectile(false)
, mAligningGrappleSwingTurn(false)
, mOverrideRadarRadius(false)
, mNoDamageLoopSfx(false)
, mOutOfBallLookAtHintActor(false)
, mModelDepthUpdateEnabled(true)
, mHoldScreenFilterAlpha(false)
, x126b_26_(false)
, mBeamParticleDescriptionsInitialized(true)
, mDeathRenderingSuppressed(false)
, mDeathFadeEnabled(false)
, mUseAlternateBeam(false)
, mLandingStrikePending(false)
, mDampBoostEntryVelocity(false)
, mDeathFadeDuration(1.f)
, mDeathFadeDelay(0.f)
, mEyeZBias(0.f)
, mStepCameraZBias(0.f)
, mBombJumpCount(0)
, mBombJumpCheckDelayFrames(0)
, mControlDirOverride(0.f, 1.f, 0.f)
, mOrbitDisableSources()
, mDeathTime(0.f)
, mControlDirInterpTime(0.f)
, mControlDirInterpDuration(0.f)
, mDeathEffectId(kInvalidUniqueId)
, mPreThinkDt(0.f)
, mSteamTextureId(kInvalidAssetId)
, mFreezeSfx()
, mSustainedDamageCount(0)
, mSustainedDamageTime(0.f)
, x12cc_(9999.f)
, mRadarXYRadiusOverride(1.f)
, mRadarZRadiusOverride(1.f)
, mAttachedActorStruggle(0.f)
, mFramesSinceDamageSfx(2)
, mSamusExhaustedVoiceTimer(4.f)
, mDamageColorTimer(0.f)
, x12e8_(~0u)
, x12ec_(0)
, x12f0_(0.f)
, mInvulnerabilityTimer(0.f)
, mTurretState(kTS_None)
, mTurretId(kInvalidUniqueId)
, mTurretGunDrawBlocks()
, mTurretTimer(0.f)
, mPlayerState(playerState)
, mCameraManager(cameraManager)
, mFrozenResources(rs_new SFrozenResources())
, mControlScheme(controlScheme)
, mEchoPulsePhase(0.f)
, mEchoPulseCounter(0)
, mEchoVisorAuxEffectId(0)
, mEchoPulseLeftSfx()
, mEchoPulseRightSfx()
, mCharacterIndex(charIdx)
, mPreviousCameraForwardPoint(CVector3f::Zero())
, mPreviousEyePosition(CVector3f::Zero())
, mScreenPosition(0, 0)
, mDarkWorldDamageExposureTime(0.f)
, mDarkAetherDamage(0.f)
, mDarkAetherDamageFlashTime(0.f)
, mDarkAetherParticleDescriptions(
      rs_new rstl::pair< TToken< CGenDescription >, TToken< CGenDescription > >(
          gpSimplePool->GetObj("PART_DarkWorldDamageEffects"),
          gpSimplePool->GetObj("PART_DarkWorldDamageEffectsThirdPerson")))
, mDarkAetherParticles(nullptr)
, mDarkAetherThirdPersonParticles(nullptr)
, mUnderwaterParticleDescriptions(
      rs_new rstl::pair< TToken< CGenDescription >, TToken< CGenDescription > >(
          gpSimplePool->GetObj("PART_UnderWaterEffects"),
          gpSimplePool->GetObj("PART_UnderWaterEffectsThirdPerson")))
, mUnderwaterParticles(nullptr)
, mUnderwaterThirdPersonParticles(nullptr)
, mGunParticleLocator()
, mBeamEffectTokens()
, mBeamParticleDescriptions()
, mBeamParticles(nullptr)
, mParticleBeam(static_cast< CPlayerState::EBeamId >(-1))
, mBeamAuxParticles(nullptr)
, mPlayerIndex(playerIndex)
, mReflectionTextureData(CMemory::Alloc(0x800, IAllocator::kHI_RoundUpLen))
, mIndirectTextureData(CMemory::Alloc(0x1000, IAllocator::kHI_RoundUpLen))
, mMaskTextureData(CMemory::Alloc(0x800, IAllocator::kHI_RoundUpLen))
, mRezbitRecoveryDirection(0)
, mRezbitRecoveryInputCount(0)
, mControlMapper(0)
, mControlHintManager(
      rs_new CControlHintManager(playerIndex, rstl::string_l("Control Hint Manager")))
, mPlayerHintControlHintId(kInvalidUniqueId)
, x14c0_(0.f)
, mEnemyLockOnActorId(kInvalidUniqueId)
, mEnemyLockOnCount(0) {
  SetRenderParticleDatabaseInside(false);
  SetUpdateDuringCinematicSkip(false);

  memset(mReflectionTextureData.get(), 0, 0x800);
  memset(mIndirectTextureData.get(), 0, 0x1000);
  memset(mMaskTextureData.get(), 0, 0x800);
  DCFlushRange(mReflectionTextureData.get(), 0x800);
  DCFlushRange(mIndirectTextureData.get(), 0x1000);
  DCFlushRange(mMaskTextureData.get(), 0x800);

  CAssetId beam = gpTweakPlayerRes->GetBallTransitionBeamResId(mTransitionBeam);
  if (multiplayer) {
    beam = gpTweakPlayerRes->GetBallTransitionBeamResIdMultiplayer(mTransitionBeam);
  }
  CModelData beamModel(CStaticRes(beam, CVector3f(1.8f, 1.8f, 1.8f)));
  mBallTransitionBeamModel = beamModel.IsNull() ? nullptr : rs_new CModelData(beamModel);
  if (!multiplayer && mBallTransitionBeamModel.get() && !mBallTransitionBeamModel->IsNull()) {
    mBallTransitionBeamModel->Touch();
  }
  mMorphBall = rs_new CMorphBall(*this, ballRadius, multiplayer);
  mBodyController = rs_new CPlayerBodyController(*this, stateMachine);

  SetInertiaTensorScalar(GetMass());
  SetLastNonCollidingState(GetMotionState());
  mGun->SetTransform(GetTransform());
  mGun->GrappleArm()->SetTransform(GetTransform());
  const CAABox bounds = GetModelData()->GetBounds(CTransform4f::Identity());
  mBallTransHeight = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
  SetCalculateLighting(true);
  ActorLights()->SetCastShadows(true);
  if (multiplayer) {
    ActorLights()->SetMaxAreaLights(2);
  }

  mMoveDir.SetZ(0.f);
  if (mMoveDir.CanBeNormalized()) {
    mMoveDir.Normalize();
  }
  mAccelerationTable.push_back(0.074f);
  mAccelerationTable.push_back(0.296f);
  mAccelerationTable.push_back(0.296f);
  mAccelerationTable.push_back(1.f);
  SetMaxVelocityAfterCollision(25.f);

  CAnimData* animation = AnimationData();
  mGrappleLocator = animation->GetLocatorSegId(rstl::string_l("L_wrist"));
  mGunParticleLocator = animation->GetLocatorSegId(rstl::string_l("GUN_Particle_LCTR"));
  animation->SetAnimationTreeLimit(16);
  ModelData()->SetScale(CVector3f(1.8f, 1.8f, 1.8f));
  mBallTransitionBeamModel->SetScale(CVector3f(1.8f, 1.8f, 1.8f));
  CollectBallTransitionAnimationTokens();

  const CWeaponTypeVulnerability darkWorldDamage(
      1.f - GetTweakPlayer()->GetDarkWorldDamageReduction(), CWeaponTypeVulnerability::kE_Normal,
      false);
  mDarkSuitVulnerability.SetVulnerability(kWT_AreaDark, darkWorldDamage);
  mImmuneVulnerability.SetVulnerability(
      kWT_CannonBall, mVariaSuitVulnerability.GetVulnerability(CWeaponMode(kWT_CannonBall)));
  mImmuneVulnerability.SetVulnerability(
      kWT_UnknownSource, mVariaSuitVulnerability.GetVulnerability(CWeaponMode(kWT_UnknownSource)));
  mScrewAttackVulnerability = mImmuneVulnerability;
  mScrewAttackVulnerability.SetVulnerability(kWT_AreaDark, darkWorldDamage);
  mControlMapper.Reset();
  mControlMapper.SetControlScheme(mControlScheme);
}

CPlayer::~CPlayer() {
  NWeaponTypes::unlock_tokens(mBeamEffectTokens);
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mReflectionTextureData.release());
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mIndirectTextureData.release());
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mMaskTextureData.release());
}

void CPlayer::ResetPlayerState(CStateManager& mgr, int state) {
  const int playerIndex = GetPlayerIndex();
  mRezbitGunDrawBlocks.Clear(mgr);
  mTurretGunDrawBlocks.Clear(mgr);
  mGun->Reset(mgr);
  if (mgr.IsMultiplayer()) {
    CSamusHud::RefreshBeamMenu(mgr, GetPlayerIndex());
  }
  mKnockBackManager.ResetEffects(mgr, *this);
  AddMaterial(kMT_Orbit, kMT_Target, kMT_Unknown59, mgr);
  RemoveMaterial(kMT_Unknown54, mgr);
  if (mgr.IsMultiplayer()) {
    RemoveMaterial(kMT_NoPlatformCollision, mgr);
  }
  CSamusHud::DisplayHudMemo(rstl::wstring_l(L""),
                            CHUDMemoParms(FLT_EPSILON, true, false, false, 1 << playerIndex, true));
  if (!mRagDoll.null()) {
    mRagDoll->RestoreActorCollision(mgr);
    mRagDoll = nullptr;
  }

  mDeathRenderingSuppressed = false;
  mDeathFadeEnabled = false;
  mDeathFadeDuration = 1.f;
  mDeathFadeDelay = 0.f;
  mVisorSteam.Reset();
  SetTurretState(kTS_None, mgr);
  BreakFrozenState(mgr, kBFS_Break, false);
  mRezbitState = kRS_None;
  if (state != 0) {
    TUniqueId cameraId = mCameraManager->GetFirstPersonCamera()->GetUniqueId();
    if (mCinematicMorphBallState == kMS_Morphed) {
      cameraId = mCameraManager->GetBallCamera()->GetUniqueId();
    }
    mCameraManager->Reset(cameraId, mgr);
    GetPlayerHintManager()->Reset(mgr);
    GetControlHintManager()->Reset(mgr);
    ResetPlayerHintState(mgr);
    mMorphBall->SetBallState(CMorphBall::kBS_Normal);
    if (mgr.IsMultiplayer()) {
      mInvulnerabilityTimer = 2.f;
    }
    mControlMapper.ResetCommandOverrides();
    ClearFluidList(mgr);
  }

  mBodyController->ResetStates(mgr);
  mCameraManager->UpdateCameraTriggers(mCameraManager->GetCurrentCameraId(true), mgr);
  UpdateWaterInhabitants(0.01f, mgr);
  if (mAttachedActor != kInvalidUniqueId) {
    DetachActorFromPlayer();
    mEnergyDrain.Clear();
  }
}

bool CPlayer::fn_80019e20(const CStateManager& mgr) const { return IsPlayerDeadEnough(mgr); }

bool CPlayer::IsMorphBallTransitioning() const {
  return mMorphBallState == kMS_Morphing || mMorphBallState == kMS_Unmorphing;
}

void CPlayer::SetAimTarget(TUniqueId target) {
  if (target == kInvalidUniqueId || target != mAimTarget) {
    mAimTargetAverage.clear();
  }
  mAimTarget = target;
}

// Target-derived correspondence with Prime's aim prediction.
void CPlayer::UpdateAimPrediction(const CTransform4f& transform, CStateManager& mgr) {
  if (mAimTarget == kInvalidUniqueId) {
    return;
  }
  const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mAimTarget));
  if (!target) {
    return;
  }

  mAimingAtProjectile = TCastToConstPtr< CGameProjectile >(target) != nullptr;
  const CVector3f instantTarget = target->GetAimPosition(mgr, 0.f);
  const CVector3f gunToTarget = instantTarget - transform.GetTranslation();
  const float timeToTarget = gunToTarget.Magnitude() / mGun->GetBeamVelocity();
  const CVector3f predictedTarget = target->GetAimPosition(mgr, timeToTarget);
  const CVector3f predictionOffset = predictedTarget - instantTarget;
  mTargetAimPosition = instantTarget;

  if (predictionOffset.Magnitude() < 0.1f) {
    mAimTargetAverage.AddValue(CVector3f::Zero());
  } else {
    mAimTargetAverage.AddValue(predictedTarget - instantTarget);
  }
  if (mAimTargetAverage.GetAverage() && !mAimingAtProjectile) {
    mAssistedTargetAim = instantTarget + *mAimTargetAverage.GetAverage();
  } else {
    mAssistedTargetAim = predictedTarget;
  }
}

void CPlayer::UpdateAssistedAiming(const CTransform4f& transform, CStateManager& mgr) {
  CTransform4f assistTransform = transform;
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mAimTarget))) {
    CVector3f gunToTarget = mAssistedTargetAim - transform.GetTranslation();
    CVector3f gunToTargetFlat(gunToTarget.GetX(), gunToTarget.GetY(), 0.f);
    const float targetFlatMagnitude = gunToTargetFlat.Magnitude();
    CVector3f gunDirectionFlat = transform.GetForward();
    gunDirectionFlat.SetZ(0.f);
    const float gunFlatMagnitude = gunDirectionFlat.Magnitude();
    if (gunToTargetFlat.CanBeNormalized() && gunDirectionFlat.CanBeNormalized()) {
      gunToTargetFlat /= targetFlatMagnitude;
      gunDirectionFlat /= gunFlatMagnitude;
      const float targetElevation = float(atan2(gunToTarget.GetZ(), targetFlatMagnitude));
      const float gunElevation = float(atan2(transform.GetForward().GetZ(), gunFlatMagnitude));
      float verticalAngle = targetElevation - gunElevation;
      bool hasVerticalAngle = true;
      if (!mAimingAtProjectile &&
          fabsf(verticalAngle) > GetTweakPlayer()->GetAimAssistVerticalAngle()) {
        if (GetTweakPlayerControls()->GetAssistedAimingIgnoreVertical()) {
          verticalAngle = 0.f;
          hasVerticalAngle = false;
        } else if (verticalAngle > 0.f) {
          verticalAngle = GetTweakPlayer()->GetAimAssistVerticalAngle();
        } else {
          verticalAngle = -GetTweakPlayer()->GetAimAssistVerticalAngle();
        }
      }

      const bool targetToLeft = CVector3f::Cross(gunDirectionFlat, gunToTargetFlat).GetZ() > 0.f;
      float horizontalAngle =
          float(acos(CMath::Limit(CVector3f::Dot(gunToTargetFlat, gunDirectionFlat), 1.f)));
      bool hasHorizontalAngle = true;
      if (!mAimingAtProjectile &&
          fabsf(horizontalAngle) > GetTweakPlayer()->GetAimAssistHorizontalAngle()) {
        horizontalAngle = GetTweakPlayer()->GetAimAssistHorizontalAngle();
        if (GetTweakPlayerControls()->GetAssistedAimingIgnoreHorizontal()) {
          horizontalAngle = 0.f;
          hasHorizontalAngle = false;
        }
      }
      if (targetToLeft) {
        horizontalAngle = -horizontalAngle;
      }
      if (!hasVerticalAngle || !hasHorizontalAngle) {
        verticalAngle = 0.f;
        horizontalAngle = 0.f;
      }

      gunToTarget = CVector3f(float(sin(horizontalAngle)) * float(cos(verticalAngle)),
                              float(cos(horizontalAngle)) * float(cos(verticalAngle)),
                              float(sin(verticalAngle)));
      gunToTarget = transform.Rotate(gunToTarget);
      assistTransform = CTransform4f::LookAt(CVector3f::Zero(), gunToTarget);
    }
  }
  mGun->SetAssistAimTransform(assistTransform);
}

void CPlayer::UpdateGunTransform(const CVector3f& position, CStateManager& mgr) {
  CTransform4f transform = GetTransform();
  const float eyeHeight = GetEyeHeight();
  CTransform4f cameraTransform = mCameraManager->GetFirstPersonCamera()->GetTransform();
  CVector3f gunPosition;
  CTransform4f gunTransform = cameraTransform;
  if (mMorphBallState == kMS_Morphing) {
    gunPosition = cameraTransform * CVector3f(position - CVector3f(0.f, 0.f, eyeHeight));
  } else {
    gunPosition =
        GetEyePosition() + cameraTransform.Rotate(position - CVector3f(0.f, 0.f, eyeHeight));
  }
  cameraTransform.SetTranslation(gunPosition);
  mGun->UpdateTransform(mgr, gunPosition, cameraTransform, gunTransform);
  CTransform4f aimTransform = gunTransform;
  UpdateAimPrediction(aimTransform, mgr);
  UpdateAssistedAiming(aimTransform, mgr);
}

const CTransform4f& CPlayer::GetFirstPersonCameraTransform() const {
  return mCameraManager->GetFirstPersonCamera()->GetGunFollowTransform();
}

void CPlayer::UpdateArmAndGunTransforms(float dt, CStateManager& mgr) {
  CVector3f grappleOffset = CVector3f::Zero();
  CVector3f gunOffset;
  const bool screwAttack = mMorphBallState == kMS_Morphed && mMorphBall->InScrewAttackMode();
  if (mMorphBallState == kMS_Morphed && !screwAttack) {
    gunOffset = CVector3f(0.f, 0.f, 0.6f);
  } else {
    gunOffset = gpTweakPlayerGun->GetGunPosition();
    CGrappleArm* arm = mGun->GrappleArm();
    grappleOffset = arm && !arm->IsGrappling() ? CVector3f::Zero()
                                               : gpTweakPlayerGun->GetGrapplingArmPosition();
    gunOffset[kDZ] += GetEyeHeight();
    grappleOffset[kDZ] += GetEyeHeight();
  }
  UpdateGunTransform(gunOffset + mCameraBob->GetGunBobTransformation().GetTranslation(), mgr);
  UpdateGrappleArmTransform(grappleOffset, mgr, dt);
}

void CPlayer::ForceGunOrientation(const CTransform4f& transform, CStateManager& mgr) {
  mGun->Holster(mgr);
  mGunDir = transform.GetForward();
  mGun->SetTransform(transform);
  UpdateArmAndGunTransforms(0.01f, mgr);
}

void CPlayer::UpdateDebugCamera(CStateManager& mgr) {}

void CPlayer::Update(float dt, CStateManager& mgr) {
  if (!mBeamParticleDescriptionsInitialized && NWeaponTypes::are_tokens_ready(mBeamEffectTokens)) {
    mBeamParticleDescriptionsInitialized = true;
    mBeamParticleDescriptions.reserve(4);
    for (int i = 0; i < 4; ++i) {
      CToken muzzle(gpSimplePool->GetObj(skThirdPersonMuzzleNames[i]));
      CToken charge(gpSimplePool->GetObj(skThirdPersonChargeNames[i]));
      mBeamParticleDescriptions.push_back_unsafe(rstl::pair< CToken, CToken >(muzzle, charge));
      mBeamParticleDescriptions[i].first.Lock();
      mBeamParticleDescriptions[i].second.Lock();
    }
  }

  SetCoefficientOfRestitutionModifier(0.f);
  if (mTurretState != kTS_Active) {
    mKnockBackManager.Update(dt, mgr, *this);
    UpdatePlayerBodyController(dt, mgr);
    if (!UpdatePlayerRagDoll(dt, mgr) && (!GetFrozenState() || mBodyController->IsUnfreezing())) {
      UpdateMorphBallTransition(dt, mgr);
    }
  }

  CPlayerState::EBeamId newBeam = mPlayerState->GetCurrentBeam();
  if (mTransitionBeam != newBeam) {
    mTransitionBeam = newBeam;
    if (mgr.IsMultiplayer()) {
      mTransitionBeamShader = mTransitionBeam;
    } else {
      CModelData modelData(CStaticRes(gpTweakPlayerRes->GetBallTransitionBeamResId(mTransitionBeam),
                                      mAnimRes.GetScale()));
      mBallTransitionBeamModel = modelData.IsNull() ? nullptr : rs_new CModelData(modelData);
      if (mBallTransitionBeamModel.get() && !mBallTransitionBeamModel->IsNull()) {
        mBallTransitionBeamModel->LockTextures();
      }
    }
  }

  if (mPlayerState->IsPlayerAlive()) {
    if (mDeathTime > 0.f) {
      mDeathTime = 0.f;
    }
  } else {
    if (mDeathTime == 0.f) {
      mWasDamaged = false;
      mWasDamagedPrev = false;
      mDamageWeaponType = kWT_None;
      AddMaterial(kMT_Unknown54, mgr);
      if (mgr.IsMultiplayer()) {
        AddMaterial(kMT_NoPlatformCollision, mgr);
      }
      ApplySubmergedPitchBend(CSfxManager::SfxStart(
          mgr.ReturnFirstIfSingleElseSecond(0xb7, 0x2579), 127, GetSoundPan(kMSP_Player),
          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority));
      if (mMorphBallState != kMS_Unmorphed && !mgr.IsMultiplayer()) {
        ApplySubmergedPitchBend(CSfxManager::SfxStart(0x245c, 127, GetSoundPan(kMSP_Player),
                                                      CSfxManager::kAllAreas, true, false,
                                                      CSfxManager::kMedPriority));
      }
      BreakFrozenState(mgr, kBFS_Break, false);
    }
    if (!mRagDoll.get() && !mKnockBackManager.IsRagDollPending()) {
      CPhysicsActor::Stop();
    }
    mDeathTime += dt;
  }

  if (mMorphBallState == kMS_Unmorphing || mMorphBallState == kMS_Morphing ||
      mMorphBall->InScrewAttackMode() ||
      (mgr.IsMultiplayer() && mMorphBallState == kMS_Unmorphed)) {
    CTransform4f gunXf = GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    mGunWorldXf = GetTransform() * gunXf;
  }

  if (mMorphBallState == kMS_Unmorphed) {
    UpdateAimTargetTimer(dt);
    UpdateAimTarget(mgr);
    UpdateOrbitModeTimer(dt);
  }
  UpdateOrbitPreventionTimer(dt);
  if (mMorphBallState == kMS_Morphed) {
    mMorphBall->Update(dt, mgr);
  } else {
    mMorphBall->StopSounds();
  }
  if (mMorphBallState == kMS_Morphing || mMorphBallState == kMS_Unmorphing) {
    mMorphBall->UpdateEffects(dt, mgr);
  }
  mMorphBall->UpdateBallLight(dt, mgr);
  UpdateGunAlpha(mgr);
  UpdateDebugCamera(mgr);
  UpdateVisorTransition(dt, mgr);
  mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
  UpdatePlayerSounds(dt);
  mTargeting->Update(dt, mgr);
  if (mAttachedActor != kInvalidUniqueId) {
    mAttachedActorTime += dt;
  }

  mStaticTimer = rstl::max_val(0.f, mStaticTimer - dt);
  const float outSpeed = mStaticOutSpeed;
  const float inSpeed = mStaticInSpeed;
  if (mStaticTimer > 0.f) {
    mVisorStaticAlpha = rstl::max_val(0.f, mVisorStaticAlpha - dt * outSpeed);
  } else {
    mVisorStaticAlpha = rstl::min_val(1.f, mVisorStaticAlpha + dt * inSpeed);
  }

  mEnergyDrain.ProcessEnergyDrain(mgr, dt);
  mMoveSpeedAvg.AddValue(mMoveSpeed);
  mPlayerState->UpdateStaticInterference(mgr, dt);
  if (!ShouldSampleFailsafe(mgr)) {
    CPhysicsActor::Stop();
    mMorphBall->StopSounds();
  }

  if (!mgr.IsMultiplayer()) {
    mSamusExhaustedVoiceTimer = IsEnergyLow() ? mSamusExhaustedVoiceTimer - dt : 4.f;
    if (!mCameraManager->IsInCinematicCamera() && mSamusExhaustedVoiceTimer <= 0.f) {
      StartSamusVoiceSfx(0x10a1, 127, 7);
      mSamusExhaustedVoiceTimer = 4.f;
    }

    int suit = mPlayerState->GetCurrentSuitRaw();
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
      suit += 3;
    }
    if (suit != mTransitionSuit) {
      mTransitionSuit = static_cast< CPlayerState::EPlayerSuit >(suit);
      CAssetId modelId;
      CAssetId skinRulesId;
      if (strlen(skSuitTransitionModelNames[mTransitionSuit].first) == 0) {
        const CCharacterInfo& character = GetModelData()->GetAnimationData()->GetCharacterInfo();
        modelId = character.GetModelId();
        skinRulesId = character.GetSkinRulesId();
      } else {
        modelId =
            NWeaponTypes::get_asset_id_from_name(skSuitTransitionModelNames[mTransitionSuit].first);
        skinRulesId = NWeaponTypes::get_asset_id_from_name(
            skSuitTransitionModelNames[mTransitionSuit].second);
      }
      TLockedToken< CSkinnedModel > model(rs_new CSkinnedModel(
          TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', modelId))),
          TLockedToken< CSkinRules >(gpSimplePool->GetObj(SObjectTag('CSKR', skinRulesId))),
          GetModelData()->GetAnimationData()->GetModelData()->GetLayoutInfo()));
      AnimationData()->SetSkinnedModel(model);
    }
  }
}

void CPlayer::UpdatePlayerDrawFlags(CStateManager& mgr) {
  CPlayerState& playerState = *mPlayerState;
  mModelDepthUpdateEnabled = true;
  float alpha = mAlpha;
  if (mDeathFadeEnabled) {
    alpha *= GetDeathAlpha();
  }

  mUseAlternateBeam = false;
  if (playerState.GetItemAmount(CPlayerState::kIT_Invincibility, true) != 0) {
    const float timeLeft = playerState.GetTimeLeft(CPlayerState::kIT_Invincibility);
    if (!(timeLeft < 2.f) || (static_cast< int >(20.f * timeLeft) & 1) == 0) {
      mUseAlternateBeam = true;
    }
  }
  if (mInvulnerabilityTimer > 0.f && (static_cast< int >(20.f * mInvulnerabilityTimer) & 1) == 0) {
    mUseAlternateBeam = true;
  }

  if (mKnockBackManager.IsLaggedBurnDeath()) {
    mPlayerDrawFlags =
        CModelFlags(static_cast< CModelFlags::ETrans >(3),
                    CColor(skLaggedBurnDeathColor.GetRedu8(), skLaggedBurnDeathColor.GetGreenu8(),
                           skLaggedBurnDeathColor.GetBlueu8(), uchar(255))
                        .WithAlphaModulatedBy(alpha));
    SetModelFlags(mPlayerDrawFlags);
  } else if (mKnockBackManager.IsBurnDeath()) {
    mPlayerDrawFlags = CModelFlags::AlphaBlended(
        CColor(uchar(0), uchar(0), uchar(0), uchar(255)).WithAlphaModulatedBy(alpha));
    SetModelFlags(mPlayerDrawFlags);
  } else if (mDeathFadeEnabled) {
    mPlayerDrawFlags = CModelFlags::AlphaBlended(CColor::White().WithAlphaModulatedBy(alpha));
    SetModelFlags(mPlayerDrawFlags);
  } else if (playerState.GetItemAmount(CPlayerState::kIT_Invisibility, true) != 0) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Dark) {
      const float timeLeft = playerState.GetTimeLeft(CPlayerState::kIT_Invisibility);
      float invisibilityAlpha = 0.5f;
      if (timeLeft < 2.f && (static_cast< int >(20.f * timeLeft) & 1) != 0) {
        invisibilityAlpha = 0.75f;
      }
      invisibilityAlpha *= alpha;
      mModelDepthUpdateEnabled = false;
      mPlayerDrawFlags = CModelFlags::AdditiveRGB(CColor::Blue().WithAlphaOf(invisibilityAlpha));
    } else {
      mPlayerDrawFlags = CModelFlags::AlphaBlended(alpha);
    }
  } else if (playerState.GetItemAmount(CPlayerState::kIT_DoubleDamage, true) != 0) {
    const float pulse = (1.f + CMath::FastSinR(M_PIF * CGraphics::GetSecondsMod900())) / 2.f;
    const float timeLeft = playerState.GetTimeLeft(CPlayerState::kIT_DoubleDamage);
    if (!(timeLeft < 2.f) || (static_cast< int >(20.f * timeLeft) & 1) == 0) {
      mPlayerDrawFlags = CModelFlags(CModelFlags::kT_Two,
                                     CColor::Lerp(CColor::Black(), CColor(1.f, 0.5f, 0.5f), pulse));
    } else {
      mPlayerDrawFlags = CModelFlags::Normal();
    }
  } else if (mDamageColorTimer > 0.f) {
    if (mgr.IsMultiplayer()) {
      const float blend = rstl::max_val(0.f, rstl::min_val(1.f, mDamageColorTimer / 0.333f));
      mPlayerDrawFlags = CModelFlags(CModelFlags::kT_Two,
                                     CColor::Lerp(CColor::Black(), CColor(0.5f, 0.f, 0.f), blend));
    } else {
      const float color = 1.f - mDamageColorTimer;
      mPlayerDrawFlags = CModelFlags::ColorModulate(CColor(1.f, color, color, alpha));
    }
  } else if (alpha != 1.f && GetPlayerIndex() == mgr.GetCurrentRenderPlayer()->GetPlayerIndex()) {
    const CModelFlags flags = CModelFlags::AlphaBlended(alpha);
    mPlayerDrawFlags = CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_DrawNormal);
  } else {
    mPlayerDrawFlags = CModelFlags::Normal();
  }

  SetModelFlags(mPlayerDrawFlags);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    mModelDepthUpdateEnabled = false;
  }
  SetModelFlags(GetModelFlags().UseShaderSet(mgr.IsMultiplayer() ? GetCurrentBeam() : 0));
}

bool CPlayer::ShouldSampleFailsafe(const CStateManager& mgr) const {
  const bool dead =
      !mPlayerState->IsPlayerAlive() && !mRagDoll.get() && !mKnockBackManager.IsRagDollPending();
  const CCinematicCamera* cinematic =
      TCastToConstPtr< CCinematicCamera >(mCameraManager->GetCurrentCamera(mgr, true));
  if (dead || (mCameraManager->IsInCinematicCamera() && mCameraState == kCS_Spawned && cinematic &&
               (cinematic->GetFlags() & 0x80))) {
    return false;
  }
  if (const CScriptPlayerHint* hint =
          TCastToConstPtr< CScriptPlayerHint >(mPlayerHintManager->GetCurrentHint(mgr))) {
    if (hint->GetOverrideFlags() & 0x2000) {
      return false;
    }
  }
  return true;
}

CScannableObjectInfo* CPlayer::GetScannableObjectInfo() const {
  // TODO: Construct and select the per-player scan descriptors.
  return nullptr;
}

void CPlayer::UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr) {
  mVisorSteam.Update(dt);
  const EPlayerMorphBallState morphState = mMorphBallState;
  CPlayerState* const playerState = mPlayerState;
  const CScriptGrapplePoint* grapplePoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mOrbitTargetId));
  if (mOrbitState == kOS_Grapple || grapplePoint) {
    return;
  }
  if (morphState == kMS_Unmorphed && !playerState->GetIsVisorTransitioning() &&
      mScanState == kSS_NotScanning) {
    const CPlayerState::EPlayerVisor currentVisor = playerState->GetTransitioningVisor();
    int selectedVisor = currentVisor;
    if (mPlayerState->GetItemAmount(CPlayerState::kIT_ScanVirus)) {
      selectedVisor = CPlayerState::kPV_Scan;
    } else {
      if (currentVisor == CPlayerState::kPV_Scan &&
          (FireBeamPressed(input) ||
           mControlMapper.GetPressInput(CControlMapper::kC_MissileOrPowerBomb, input)) &&
          playerState->HasPowerUp(CPlayerState::kIT_CombatVisor)) {
        playerState->StartTransitionToVisor(CPlayerState::kPV_Combat);
        mGun->DrawGun(mgr);
      }

      for (int i = 0; i < 4; ++i) {
        const TVisorToItemMapping& mapping = skVisorToItemMapping[i];
        if (playerState->HasPowerUp(mapping.first) &&
            mControlMapper.GetPressInput(mapping.second, input)) {
          mVisorChangeRequested = true;
          selectedVisor = i;
          if (currentVisor != i) {
            break;
          }
        }
        if (!playerState->HasPowerUp(mapping.first) && currentVisor == i) {
          selectedVisor = CPlayerState::kPV_Combat;
          break;
        }
      }
      if (mControlMapper.GetPressInput(CControlMapper::kC_VisorUp, input)) {
        do {
          selectedVisor = (selectedVisor + 1) % 4;
        } while (selectedVisor != currentVisor &&
                 !playerState->HasPowerUp(skVisorToItemMapping[selectedVisor].first));
      } else if (mControlMapper.GetPressInput(CControlMapper::kC_VisorDown, input)) {
        do {
          selectedVisor = (selectedVisor + 3) % 4;
        } while (selectedVisor != currentVisor &&
                 !playerState->HasPowerUp(skVisorToItemMapping[selectedVisor].first));
      }
      if (mControlMapper.GetPressInput(CControlMapper::kC_DarkVisorToggle, input)) {
        if (mgr.IsMultiplayer()) {
          CSfxManager::SfxStart(0x25a4, 127, GetSoundPan(kMSP_4), CSfxManager::kAllAreas, true,
                                false, CSfxManager::kMedPriority);
        } else if (currentVisor == CPlayerState::kPV_Dark) {
          selectedVisor = CPlayerState::kPV_Combat;
        } else {
          selectedVisor = CPlayerState::kPV_Dark;
        }
      }
    }

    if (selectedVisor != currentVisor &&
        playerState->HasPowerUp(skVisorToItemMapping[selectedVisor].first)) {
      const CPlayerState::EPlayerVisor visor =
          static_cast< CPlayerState::EPlayerVisor >(selectedVisor);
      playerState->StartTransitionToVisor(visor);
      if (visor == CPlayerState::kPV_Scan) {
        mGun->HolsterGun(mgr);
      } else {
        mGun->DrawGun(mgr);
      }
    }
  }
}

void CPlayer::UpdateVisorTransition(float dt, CStateManager& mgr) {
  CPlayerState* playerState = mPlayerState;
  if (playerState->GetIsVisorTransitioning() && playerState->UpdateVisorTransition(dt)) {
    mOrbitCandidateRefreshFrames = 0;
    mOrbitCandidateIndex = 0;
    mOrbitNextTargetId = kInvalidUniqueId;
    mOrbitTargetId = kInvalidUniqueId;
    mOnScreenOrbitObjects.clear();
    mNearbyOrbitObjects.clear();
    mOffScreenOrbitObjects.clear();
  }
}

void CPlayer::UpdateCrosshairsState(const CFinalInput& input) {
  mDrawCrosshairs = mControlMapper.GetDigitalInput(CControlMapper::kC_Crosshairs, input);
}

void CPlayer::UpdatePlayerSounds(float dt) {
  mTimeSinceDamageImpactSfx += dt;
  if (mDamageSfxTimer > 0.f) {
    mDamageSfxTimer -= dt;
    if (mDamageSfxTimer <= 0.f) {
      CSfxManager::SfxStop(mDamageLoopSfx);
      mDamageLoopSfx.Clear();
    }
  }

  if (mDarkAetherDamageLoopSfxTimer > 0.f) {
    mDarkAetherDamageLoopSfxTimer -= dt;
    if (mDarkAetherDamageLoopSfxTimer <= 0.f) {
      CSfxManager::SfxStop(mDarkAetherDamageLoopSfx);
      mDarkAetherDamageLoopSfx.Clear();
    }
  }
}

void CPlayer::UpdateFootstepSounds(float dt, const CFinalInput& input, CStateManager& mgr) {
  if (mMorphBallState == kMS_Unmorphed && mMovementState == NPlayer::kMS_OnGround && !mInFreeLook &&
      !mLookButtonHeld) {
    char sfxVol = 127;
    mFootstepSfxTimer += dt;
    float turn = TurnInput(input);
    const float forward = fabsf(ForwardInput(input, turn));
    turn = fabsf(turn);
    float sfxDelay = 0.f;
    if (forward > 0.05f || mOrbitState != kOS_NoOrbit) {
      CVector3f velocity = GetVelocityWR();
      float mag = velocity.Magnitude();
      float vel = rstl::min_val(mag / GetActualFirstPersonMaxVelocity(dt), 1.f);
      if (vel > 0.05f) {
        sfxDelay = (0.375f - 0.85f) * vel + 0.85f;
        if (mFootstepSfxSel == kFS_None) {
          mFootstepSfxSel = kFS_Left;
        }
      } else {
        mFootstepSfxTimer = 0.f;
        mFootstepSfxSel = kFS_None;
      }

      sfxVol = CCast::ToInt8((vel * 38.f + 89.f) * 1.f);
    } else if (turn > 0.05f) {
      if (mFootstepSfxSel == kFS_Left) {
        sfxDelay = (0.187f - 1.f) * turn + 1.f;
      } else {
        sfxDelay = -2.438f * turn + 3.f;
      }
      if (mFootstepSfxSel == kFS_None) {
        mFootstepSfxSel = kFS_Left;
        sfxDelay = mFootstepSfxTimer;
      }
      sfxVol = 96;
    } else {
      mFootstepSfxTimer = 0.f;
      mFootstepSfxSel = kFS_None;
    }

    if (mFootstepSfxSel != kFS_None && mFootstepSfxTimer > sfxDelay) {
      static const float earHeight = GetEyeHeight() - 0.1f;
      const short pan =
          GetSoundPan(static_cast< EMultiPlayerSoundPan >(mFootstepSfxSel == kFS_Left ? 0 : 1));
      if (GetFluidCount() != 0 && mDistanceUnderWater > 0.f && mDistanceUnderWater < earHeight) {
        const ushort sfx = mFootstepSfxSel == kFS_Left
                               ? mgr.ReturnFirstIfSingleElseSecond(0x97, 0x26f8)
                               : mgr.ReturnFirstIfSingleElseSecond(0x98, 0x26f9);
        const CSfxHandle sound = CSfxManager::SfxStart(sfx, sfxVol, pan, GetCurrentAreaId().Value(),
                                                       true, false, CSfxManager::kMedPriority);
        CSfxManager::SetIgnoreAreaLowPass(sound, true);
        ApplySubmergedPitchBend(sound);
      } else {
        ushort sfx;
        if (mFootstepSfxSel == kFS_Left) {
          sfx = GetMaterialSoundUnderPlayer(mgr, skLeftStepSounds[mgr.IsMultiplayer()], 26, 0xffff);
        } else {
          sfx =
              GetMaterialSoundUnderPlayer(mgr, skRightStepSounds[mgr.IsMultiplayer()], 26, 0xffff);
        }
        const CSfxHandle sound = CSfxManager::SfxStart(sfx, sfxVol, pan, GetCurrentAreaId().Value(),
                                                       true, false, CSfxManager::kMedPriority);
        CSfxManager::SetIgnoreAreaLowPass(sound, true);
        ApplySubmergedPitchBend(sound);
      }

      mFootstepSfxTimer = 0.f;
      if (mFootstepSfxSel == kFS_Left) {
        mFootstepSfxSel = kFS_Right;
      } else {
        mFootstepSfxSel = kFS_Left;
      }
    }
  }
}

int CPlayer::SfxIdFromMaterial(const CMaterialList& mat, const ushort* idList, int tableLen,
                               ushort defId) {
  int id = defId;
  for (short i = 0; i < tableLen; ++i) {
    if (mat.HasMaterial(static_cast< EMaterialTypes >(i)) && idList[i] != 0xFFFF) {
      id = idList[i];
    }
  }
  return static_cast< ushort >(id);
}

ushort CPlayer::GetMaterialSoundUnderPlayer(CStateManager& mgr, const ushort* table, int length,
                                            ushort defId) {
  int ret = defId;
  static const CVector3f skDown(0.f, 0.f, -1.f);
  static const CMaterialFilter matFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));

  TUniqueId collideId = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  CAABox aabb = GetBoundingBox();
  aabb.AccumulateBounds(GetTranslation() + skDown);
  mgr.BuildNearList(nearList, aabb, matFilter, nullptr);
  const CRayCastResult result =
      mgr.RayWorldIntersection(collideId, GetTranslation(), skDown, 1.5f, matFilter, nearList);
  if (result.IsValid()) {
    ret = SfxIdFromMaterial(result.GetMaterial(), table, length, defId);
  }
  return ret;
}

CPlayer::CVisorSteam::CVisorSteam(float targetAlpha, float alphaInDuration, float alphaOutDuration,
                                  CAssetId texture)
: mTargetAlpha(targetAlpha)
, mAlphaInDuration(alphaInDuration)
, mAlphaOutDuration(alphaOutDuration)
, mTexture(texture)
, mNextTargetAlpha(0.f)
, mNextAlphaInDuration(0.f)
, mNextAlphaOutDuration(0.f)
, mNextTexture(kInvalidAssetId)
, mAlpha(0.f)
, mDelayTimer(0.f) {}

void CPlayer::CVisorSteam::Update(float dt) {
  if (mNextTexture == kInvalidAssetId) {
    mTargetAlpha = 0.f;
  } else {
    mTargetAlpha = mNextTargetAlpha;
    mAlphaInDuration = mNextAlphaInDuration;
    mAlphaOutDuration = mNextAlphaOutDuration;
    mTexture = mNextTexture;
  }
  mNextTexture = kInvalidAssetId;
  if (fabs(mAlpha - mTargetAlpha) < 0.00001f && fabs(mAlpha) < 0.00001f) {
    return;
  }
  if (mAlpha <= mTargetAlpha) {
    CToken texture = gpSimplePool->GetObj(SObjectTag('TXTR', mTexture));
    if (texture.IsLoaded()) {
      mAlpha = rstl::min_val(mTargetAlpha, mAlpha + dt / mAlphaInDuration);
      mDelayTimer = 0.1f;
    }
  } else if (mDelayTimer > 0.f) {
    mDelayTimer = rstl::max_val(0.f, mDelayTimer - dt);
  } else {
    mAlpha = rstl::max_val(mTargetAlpha, mAlpha - dt / mAlphaOutDuration);
  }
}

void CPlayer::CVisorSteam::Reset() {
  mNextTexture = kInvalidAssetId;
  mTexture = kInvalidAssetId;
  Update(0.f);
}

void CPlayer::CVisorSteam::SetSteam(float targetAlpha, float alphaInDuration,
                                    float alphaOutDuration, CAssetId texture) {
  if (mNextTexture != kInvalidAssetId && targetAlpha <= mNextTargetAlpha) {
    return;
  }
  mNextTargetAlpha = targetAlpha;
  mNextAlphaInDuration = alphaInDuration;
  mNextAlphaOutDuration = alphaOutDuration;
  mNextTexture = texture;
}

void CPlayer::SetVisorSteam(float targetAlpha, float alphaInDuration, float alphaOutDuration,
                            CAssetId texture) {
  mVisorSteam.SetSteam(targetAlpha, alphaInDuration, alphaOutDuration, texture);
}

void CPlayer::fn_80016a74(float value) { x11e4_ = value; }

void CPlayer::fn_80016a6c(float value) { x11e8_ = value; }

void CPlayer::SetMorphBallState(EPlayerMorphBallState state, EPlayerMorphBallState spawnedState) {
  mMorphBallState = state;
  mSpawnedMorphBallState = spawnedState;
  SetStandardCollider(state == kMS_Morphed);
  if (state == kMS_Morphed || state == kMS_Morphing) {
    mMorphBall->LoadMorphBallModel();
  }
}

void CPlayer::SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr) {
  mCinematicMorphBallState = state;
  SetCameraState(kCS_Spawned, mgr);
  if (mCinematicMorphBallState != mMorphBallState) {
    Stop();
    SetOrbitRequest(kOR_Respawn, mgr);
    switch (mCinematicMorphBallState) {
    case kMS_Unmorphed: {
      CVector3f position = CVector3f::Zero();
      if (CanLeaveMorphBallState(mgr, position)) {
        SetTranslation(GetTranslation() + position);
        LeaveMorphBallState(mgr);
        ForceGunOrientation(GetTransform(), mgr);
        mGun->DrawGun(mgr);
      }
      break;
    }
    case kMS_Morphed: {
      EnterMorphBallState(mgr, kMS_Unmorphed);
      ActivateMorphBallCamera(mgr);
      mCameraManager->HintManager()->Reset(mgr);
      mCameraManager->BallCamera()->Reset(CreateTransformFromMovementDirection(), mgr);
      mGun->Holster(mgr);
      break;
    }
    default:
      break;
    }
  }
}

void CPlayer::UpdateCinematicState(CStateManager& mgr) {
  if (mCameraManager->IsInCinematicCamera()) {
    if (mCameraState != kCS_Spawned) {
      mCinematicMorphBallState = mMorphBallState;
      if (mCinematicMorphBallState == kMS_Unmorphing) {
        mCinematicMorphBallState = kMS_Unmorphed;
      }
      if (mCinematicMorphBallState == kMS_Morphing) {
        mCinematicMorphBallState = kMS_Morphed;
      }
      SetCameraState(kCS_Spawned, mgr);
    }
  } else if (mCameraState == kCS_Spawned) {
    if (mCinematicMorphBallState == mMorphBallState) {
      switch (mCinematicMorphBallState) {
      case kMS_Morphed:
        SetCameraState(kCS_MorphBall, mgr);
        break;
      case kMS_Unmorphed:
        SetCameraState(kCS_FirstPerson, mgr);
        if (mPlayerState->GetCurrentVisor() != CPlayerState::kPV_Scan) {
          ForceGunOrientation(GetTransform(), mgr);
          mGun->DrawGun(mgr);
        }
        break;
      default:
        break;
      }
    } else {
      Stop();
      SetOrbitRequest(kOR_Respawn, mgr);
      switch (mCinematicMorphBallState) {
      case kMS_Unmorphed: {
        CVector3f position = CVector3f::Zero();
        if (CanLeaveMorphBallState(mgr, position)) {
          SetTranslation(GetTranslation() + position);
          LeaveMorphBallState(mgr);
          SetCameraState(kCS_FirstPerson, mgr);
          ForceGunOrientation(GetTransform(), mgr);
          mGun->DrawGun(mgr);
        }
        break;
      }
      case kMS_Morphed:
        EnterMorphBallState(mgr, kMS_Unmorphed);
        ActivateMorphBallCamera(mgr);
        mCameraManager->HintManager()->Reset(mgr);
        mCameraManager->BallCamera()->Reset(CreateTransformFromMovementDirection(), mgr);
        break;
      default:
        break;
      }
    }
  }
}

void CPlayer::UpdateCameraState(CStateManager& mgr) { UpdateCinematicState(mgr); }

void CPlayer::SetCameraState(EPlayerCameraState state, CStateManager& mgr) {
  if (mCameraState == state) {
    return;
  }
  mCameraState = state;
  CCameraManager* cameraManager = mCameraManager;
  switch (state) {
  case kCS_FirstPerson: {
    cameraManager->SetCurrentCameraId(cameraManager->GetFirstPersonCamera()->GetUniqueId());
    const bool ballLight =
        mgr.GetIsDarkWorld() && mPlayerState->GetItemAmount(CPlayerState::kIT_LightSuit);
    mMorphBall->SetBallLightActive(mgr, ballLight);
    break;
  }
  case kCS_MorphBall:
    if (cameraManager->GetCurrentCameraId(false) ==
        cameraManager->GetFirstPersonCamera()->GetUniqueId()) {
      cameraManager->SetCurrentCameraId(cameraManager->BallCamera()->GetUniqueId());
    }
    mMorphBall->SetBallLightActive(mgr, true);
    break;
  case kCS_MorphBallTransition:
    cameraManager->SetCurrentCameraId(cameraManager->BallCamera()->GetUniqueId());
    mMorphBall->SetBallLightActive(mgr, true);
    break;
  case kCS_Spawned: {
    bool ballLight = false;
    if (const CCinematicCamera* cinematic =
            TCastToConstPtr< CCinematicCamera >(cameraManager->GetCurrentCamera(mgr, true))) {
      ballLight = mMorphBallState == kMS_Morphed && (cinematic->GetFlags() & 0x40);
    }
    mMorphBall->SetBallLightActive(mgr, ballLight);
    break;
  }
  default:
    break;
  }
}

void CPlayer::UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr) {
  if (mOrbitState == kOS_ForcedOrbitObject || IsMorphBallTransitioning() ||
      mMorphBallState != kMS_Unmorphed || mGrappleState != kGS_None) {
    mInFreeLook = false;
    mLookButtonHeld = false;
    mLookAnalogHeld = false;
    mHorizFreeLookAngleVel = 0.f;
    mVertFreeLookAngleVel = 0.f;
    mDrawCrosshairs = false;
    return;
  }

  bool ignoreButtons = false;
  const uint ignoreFreeLookButtons = 0x80000;
  if (const CScriptPlayerHint* hint =
          TCastToConstPtr< CScriptPlayerHint >(mPlayerHintManager->GetCurrentHint(mgr))) {
    ignoreButtons = (hint->GetOverrideFlags() & ignoreFreeLookButtons) != 0;
  }

  if (GetTweakPlayerControls()->GetHoldButtonsForFreeLook()) {
    if ((GetTweakPlayerControls()->GetTwoButtonsForFreeLook() &&
         mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold1, input) &&
         mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold2, input)) ||
        (!GetTweakPlayerControls()->GetTwoButtonsForFreeLook() &&
         (mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold1, input) ||
          mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold2, input))) ||
        ignoreButtons) {
      if (!mLookButtonHeld) {
        const CVector3f lookDir =
            mCameraManager->GetFirstPersonCamera()->GetTransform().GetForward();
        CVector3f lookDirFlat = lookDir;
        lookDirFlat.SetZ(0.f);
        mFreeLookYawAngle = 0.f;
        if (lookDirFlat.CanBeNormalized()) {
          lookDirFlat.Normalize();
          mFreeLookPitchAngle =
              float(acos(CMath::Limit(CVector3f::Dot(lookDir, lookDirFlat), 1.f)));
          if (lookDir.GetZ() < 0.f) {
            mFreeLookPitchAngle = -mFreeLookPitchAngle;
          }
        }
        const CFirstPersonCamera* camera = mCameraManager->GetFirstPersonCamera();
        if (camera->GetScriptPitchId() != kInvalidUniqueId) {
          mFreeLookPitchAngle -= camera->GetPitch();
        }
      }
      mInFreeLook = true;
      mLookButtonHeld = true;
      if (mControlMapper.GetAnalogInput(CControlMapper::kC_LookLeft, input) >= 0.1f ||
          mControlMapper.GetAnalogInput(CControlMapper::kC_LookRight, input) >= 0.1f ||
          mControlMapper.GetAnalogInput(CControlMapper::kC_LookDown, input) >= 0.1f ||
          mControlMapper.GetAnalogInput(CControlMapper::kC_LookUp, input) >= 0.1f) {
        mLookAnalogHeld = true;
      } else {
        mLookAnalogHeld = false;
      }
    } else {
      mInFreeLook = false;
      mLookButtonHeld = false;
      mLookAnalogHeld = false;
      mHorizFreeLookAngleVel = 0.f;
      mVertFreeLookAngleVel = 0.f;
    }
  } else {
    mLookButtonHeld = false;
    if (mControlMapper.GetAnalogInput(CControlMapper::kC_LookLeft, input) >= 0.1f ||
        mControlMapper.GetAnalogInput(CControlMapper::kC_LookRight, input) >= 0.1f ||
        mControlMapper.GetAnalogInput(CControlMapper::kC_LookDown, input) >= 0.1f ||
        mControlMapper.GetAnalogInput(CControlMapper::kC_LookUp, input) >= 0.1f) {
      mFreeLookAnglesHeld = false;
      mFreeLookInputLatched = true;
      mLookAnalogHeld = true;
      mLookButtonHeld = true;
      mCurFreeLookCenteredTime = 0.f;
      mInFreeLook = true;
    } else {
      const float velocityThreshold = GetTweakPlayer()->GetVelocityFreeLookThreshold();
      if (GetTweakPlayer()->GetVelocityFreeLookEnabled()) {
        const CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
        if (fabsf(localVelocity.GetY()) > velocityThreshold || !mFreeLookInputLatched) {
          mFreeLookAnglesHeld = false;
          mFreeLookInputLatched = false;
          mLookAnalogHeld = false;
        } else {
          mFreeLookAnglesHeld = true;
          mLookAnalogHeld = false;
          mLookButtonHeld = false;
          mCurFreeLookCenteredTime = 0.f;
          mInFreeLook = true;
        }
      } else {
        mLookAnalogHeld = false;
      }
    }

    if (fabsf(mFreeLookYawAngle) < GetTweakPlayer()->GetFreeLookCenteredThresholdAngle() &&
        fabsf(mFreeLookPitchAngle) < GetTweakPlayer()->GetFreeLookCenteredThresholdAngle()) {
      if (mCurFreeLookCenteredTime > GetTweakPlayer()->GetFreeLookCenteredTime()) {
        if (!mLookAnalogHeld) {
          mInFreeLook = false;
        }
        mHorizFreeLookAngleVel = 0.f;
        mVertFreeLookAngleVel = 0.f;
      } else {
        mCurFreeLookCenteredTime += dt;
      }
    } else {
      mInFreeLook = true;
      mCurFreeLookCenteredTime = 0.f;
    }
  }

  UpdateCrosshairsState(input);
}

void CPlayer::UpdateCameraTimers(float dt, const CFinalInput& input) {
  if (((mInFreeLook || mLookButtonHeld) && mMovementState == NPlayer::kMS_OnGround) ||
      mCameraManager->IsInCinematicCamera()) {
    mJumpCameraTimer = 0.f;
    mFallCameraTimer = 0.f;
    return;
  }

  if (GetTweakPlayerControls()->GetFiringCancelsCameraPitch()) {
    if (FireBeamHeld(input) ||
        mControlMapper.GetDigitalInput(CControlMapper::kC_MissileOrPowerBomb, input)) {
      if (mStartingJumpTimeout > 0.f) {
        mCancelCameraPitch = true;
        return;
      }
    }
  }

  if (JumpPressed(input)) {
    ++mJumpPresses;
  }
  if (JumpHeld(input) && mJumpCameraTimer > 0.f && !mCancelCameraPitch && mJumpPresses <= 2) {
    mJumpCameraTimer += dt;
  }
  if (mFallCameraTimer > 0.f && !mCancelCameraPitch) {
    mFallCameraTimer += dt;
  }
}

void CPlayer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                              float dt) {
  if (!mMorphBall->DoUserAnimEvent(mgr, node, type)) {
    CActor::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CPlayer::PreThink(float dt, CStateManager& mgr) {
  if (mWasDamaged && !mgr.IsMultiplayer()) {
    mImmuneTimer = 0.5f;
  }
  mWasDamaged = false;
  mWasDamagedPrev = false;
  mDamageAmount = 0.f;
  mPrevDamageAmount = 0.f;
  mDamageLocation = CVector3f::Zero();
  mDamageWeaponType = uint(kWT_None);
  x12cc_ = 9999.f; // Role unresolved; only native initialization/reset stores are known.
  mPreThinkDt = dt;
}

void CPlayer::UpdateWaterSurfaceCameraBias(CStateManager& mgr) {
  const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
  if (!water) {
    return;
  }

  const CVector3f bounds = water->GetTriggerBoundsWR().GetMaxPoint();
  CVector3f eyePos = GetEyePosition();
  eyePos.SetZ(eyePos.GetZ() - mEyeZBias);
  const float waterToEyeDelta = eyePos.GetZ() - bounds.GetZ();
  if (eyePos.GetZ() >= bounds.GetZ() && waterToEyeDelta <= 0.25f) {
    SetEyeZBias(mEyeZBias + bounds.GetZ() + 0.25f - eyePos.GetZ());
  } else if (eyePos.GetZ() < bounds.GetZ() && waterToEyeDelta >= -0.23f) {
    SetEyeZBias(mEyeZBias + bounds.GetZ() - 0.23f - eyePos.GetZ());
  }
}

void CPlayer::Think(float dt, CStateManager& mgr) {
  if (!mHoldScreenFilterAlpha) {
    mScreenFilterColor.SetAlpha(rstl::max_val(0.f, mScreenFilterColor.GetAlpha() - 2.f * dt));
  }
  if (mTurretState == kTS_Active) {
    return;
  }
  if (mInvulnerabilityTimer > 0.f) {
    mInvulnerabilityTimer -= dt;
  }

  UpdateEchoVisorEffects(dt, mgr);
  UpdateDarkAetherDamage(dt, mgr);
  UpdateUnderwaterParticles(dt, mgr);
  UpdateStepCameraZBias(dt, mgr);
  UpdateWaterSurfaceCameraBias(mgr);
  UpdateDamageTimers(dt);
  UpdateFreeLook(dt);
  UpdateBombJumpStuff();

  if (0.f < mStartingJumpTimeout) {
    mStartingJumpTimeout -= dt;
    if (0.f >= mStartingJumpTimeout) {
      SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }
  if (!mCameraManager->IsInCinematicCamera()) {
    if (mAirborneTimer > 0.f) {
      mAirborneTimer += dt;
    }
  } else {
    mAirborneTimer = 0.f;
  }
  if (mSamusVoiceTimeout > 0.f) {
    mSamusVoiceTimeout -= dt;
  }
  if (mDamageColorTimer > 0.f) {
    mDamageColorTimer -= dt;
  }
  if (0.f < mSjTimer) {
    mSjTimer -= dt;
  }
  mFallingTime += dt;
  if (mMovementState == NPlayer::kMS_FallingMorphed && mFallingTime > 0.4f) {
    SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }

  if (mPlayerState->GetItemAmount(CPlayerState::kIT_Invisibility, true)) {
    RemoveMaterial(kMT_Target, mgr);
  } else if (mMorphBallState == kMS_Unmorphed) {
    AddMaterial(kMT_Target, mgr);
  }
  if (mImmuneTimer > 0.f) {
    mImmuneTimer -= dt;
  }
  Update(dt, mgr);
  UpdateTransitionFilter(dt, mgr);

  if (mgr.IsMultiplayer()) {
    const CTransform4f particleXf = GetTransform() * GetScaledLocatorTransform(mGunParticleLocator);
    if (mBeamParticles.get()) {
      mBeamParticles->Update(dt);
      mBeamParticles->SetGlobalOrientAndTrans(particleXf);
    }
    if (mBeamAuxParticles.get()) {
      mBeamAuxParticles->Update(dt);
      mBeamAuxParticles->SetGlobalOrientAndTrans(particleXf);
    }
  }
  if (mMorphBallState != kMS_Morphing && mMorphBallState != kMS_Unmorphing &&
      !mMorphBall->InScrewAttackMode()) {
    CalculatePlayerMovementDirection(dt, GetTranslation() - mLastPosForDirCalc);
  }
  UpdatePlayerControlDirection(dt, mgr);
  if (gUseSurfaceHack) {
    SetSurfaceRestraint(gSR_Hack);
  }

  if (mMorphBallState != kMS_Morphed) {
    if (fabsf(GetTransform().GetRight().GetZ()) > FLT_EPSILON ||
        fabsf(GetTransform().GetForward().GetZ()) > FLT_EPSILON) {
      const CVector3f translation = GetTranslation();
      CVector3f lookDirFlat = GetTransform().GetForward();
      lookDirFlat.SetZ(0.f);
      if (lookDirFlat.CanBeNormalized()) {
        SetTransform(CTransform4f::LookAt(CUnitVector3f(CVector3f::Zero()),
                                          CUnitVector3f(lookDirFlat.AsNormalized())));
      } else {
        SetTransform(CTransform4f::Identity());
      }
      SetTranslation(translation);
    }
  }
  mLastVelocity = GetVelocityWR();
  mDarkSuitVulnerability.SetVulnerability(
      kWT_AreaDark, CWeaponTypeVulnerability(GetTweakPlayer()->GetDarkWorldDamageReduction(),
                                             CWeaponTypeVulnerability::kE_Normal, false));
  mScrewAttackVulnerability.SetVulnerability(
      kWT_AreaDark, CWeaponTypeVulnerability(GetTweakPlayer()->GetDarkWorldDamageReduction(),
                                             CWeaponTypeVulnerability::kE_Normal, false));
  CActor::Think(dt, mgr);
}

void CPlayer::Freeze(float timeout, CStateManager& mgr, CAssetId steamTexture, uint sfx,
                     CAssetId iceTexture) {
  if (mCameraManager->IsInCinematicCamera() || mMorphBall->InScrewAttackMode() ||
      IsMorphBallTransitioning() || GetFrozenState()) {
    return;
  }

  ResetRezbitRecoveryInput();
  CEnvironmentVariable* variable = gpGameState->PersistentOptions().FindEnvironmentVariable(
      mMorphBallState == kMS_Unmorphed ? "FreezeInstructionsFirstPerson"
                                       : "FreezeInstructionsMorphBall");
  if (variable->GetValue() != variable->GetMaximum()) {
    const int playerIndex = GetPlayerIndex();
    const bool ball = mMorphBallState == kMS_Morphed || mMorphBallState == kMS_Morphing;
    CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(
                                  ball ? "FrozenPlayerMorphBall" : "FrozenPlayerFirstPerson")),
                              CHUDMemoParms(5.f, true, false, false, 1 << playerIndex, true));
  }

  mFrozenTimeout = timeout;
  mIceBreakJumps = 0;
  mFrozenDamage = 0.f;
  Stop();
  ClearForcesAndTorques();
  if (mGrappleState != kGS_None) {
    BreakGrapple(kOR_Freeze, mgr);
  } else {
    SetOrbitRequest(kOR_Freeze, mgr);
  }
  AddMaterial(kMT_Immovable, mgr);
  IsMorphBallTransitioning();
  mSteamTextureId =
      steamTexture != kInvalidAssetId ? steamTexture : mFrozenResources->mSteamTexture;
  mIceTextureId = iceTexture != kInvalidAssetId ? iceTexture : mFrozenResources->mIceTexture;

  ushort freezeSfx = mgr.IsMultiplayer() ? mFrozenResources->mMultiplayerFreezeSfx
                                         : mFrozenResources->mSinglePlayerFreezeSfx;
  if (ushort(sfx) != CSfxManager::kInternalInvalidSfxId) {
    freezeSfx = sfx;
  }
  mFreezeSfx =
      CSfxManager::SfxStart(freezeSfx, 127, GetSoundPan(kMSP_Player), CSfxManager::kAllAreas, false,
                            false, CSfxManager::kMedPriority);
  ApplySubmergedPitchBend(mFreezeSfx);
}

bool CPlayer::GetFrozenState() const { return mFrozenTimeout > 0.f; }

void CPlayer::BreakFrozenState(CStateManager& mgr, EBreakFrozenState state, bool recordEscape) {
  if (!GetFrozenState()) {
    return;
  }

  mFrozenTimeout = 0.f;
  mIceBreakJumps = 0;
  mFrozenDamage = 0.f;
  StopSound(mFreezeSfx);
  Stop();
  ClearForcesAndTorques();
  RemoveMaterial(kMT_Immovable, mgr);
  if (!mCameraManager->IsInCinematicCamera() && state == kBFS_BreakWithEffects &&
      mIceTextureId != kInvalidAssetId) {
    const int playerIndex = GetPlayerIndex();
    mgr.AddObject(rs_new CHUDBillboardEffect(
        rstl::optional_object< TToken< CGenDescription > >(
            gpSimplePool->GetObj(SObjectTag('PART', mIceTextureId))),
        rstl::optional_object_null(), mgr.AllocateUniqueId(), true,
        rstl::string_l("FrostExplosion"),
        CHUDBillboardEffect::GetNearClipDistance(mgr, playerIndex),
        CHUDBillboardEffect::GetScaleForPOV(mgr), playerIndex, CColor(1.f, 1.f, 1.f, 1.f),
        CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f), false));
    ApplySubmergedPitchBend(CSfxManager::SfxStart(
        mgr.ReturnFirstIfSingleElseSecond(0x1aef, 0x281e), 127, GetSoundPan(kMSP_Player),
        CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority));
  }

  mMorphBall->ResetMorphBallIceBreak();
  SetVisorSteam(0.f, 0.3f / 0.7f, 1.f / 14.f, mSteamTextureId);
  if (recordEscape) {
    CEnvironmentVariable* variable =
        gpGameState->PersistentOptions().FindEnvironmentVariable("FreezeInstructionsFirstPerson");
    variable->Set(variable->GetValue() + 1);
    const int playerIndex = GetPlayerIndex();
    CSamusHud::DisplayHudMemo(rstl::wstring_l(L""),
                              CHUDMemoParms(0.f, true, true, true, 1 << playerIndex, true));
  }
}

void CPlayer::UpdateFrozenState(const CFinalInput& input, CStateManager& mgr) {
  if (mFrozenTimeout - input.DeltaTime() > 0.f) {
    SetVisorSteam(0.7f, 0.3f / 0.7f, 1.f / 14.f, mSteamTextureId);
    if (mFrozenTimeout < mBodyController->GetCurrentAnimationDuration() &&
        !mBodyController->IsUnfreezing()) {
      mBodyController->CommandMgr().DeliverCmd(
          CPBCAdditiveReactionCmd(CPBCAdditiveReactionCmd::kART_UnFreeze, false));
    }
  } else {
    BreakFrozenState(mgr, kBFS_BreakWithEffects, false);
    return;
  }
  mFrozenTimeout -= input.DeltaTime();

  if (mMovementState == NPlayer::kMS_OnGround || mMovementState == NPlayer::kMS_FallingMorphed) {
    Stop();
    ClearForcesAndTorques();
  }
  mVisorSteam.Update(input.DeltaTime());

  switch (mMorphBallState) {
  case kMS_Morphed:
    mGun->ProcessInput(input, mgr);
    break;
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing:
    if (JumpPressed(input)) {
      if (mIceBreakJumps != 0) {
        ApplySubmergedPitchBend(CSfxManager::SfxStart(
            mgr.ReturnFirstIfSingleElseSecond(0x1aed, 0x281c), 127, GetSoundPan(kMSP_Player),
            CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority));
      } else {
        ApplySubmergedPitchBend(CSfxManager::SfxStart(
            mgr.ReturnFirstIfSingleElseSecond(0x1aee, 0x281d), 127, GetSoundPan(kMSP_Player),
            CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority));
      }
      if (++mIceBreakJumps > GetTweakPlayer()->GetIceBreakJumpCount()) {
        BreakFrozenState(mgr, kBFS_BreakWithEffects, true);
      }
    }
    break;
  }

  if (mFrozenDamage > GetTweakPlayer()->GetFrozenDamageThreshold()) {
    BreakFrozenState(mgr, kBFS_BreakWithEffects, false);
  }
  const int recoveryInputCount = int(mRezbitRecoveryInputCount);
  if (recoveryInputCount > 2) {
    BreakFrozenState(mgr, kBFS_BreakWithEffects, true);
  } else if (recoveryInputCount + mIceBreakJumps > 2) {
    BreakFrozenState(mgr, kBFS_BreakWithEffects, true);
  }
  Stop();
}

void CPlayer::ProcessInput(float dt, const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateMorphBallState(const CFinalInput& input, float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80012eb8(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

float CPlayer::GetMaximumPlayerPositiveVerticalVelocity(const CStateManager& mgr) const {
  const bool spaceJump = mPlayerState->GetItemAmount(CPlayerState::kIT_SpaceJumpBoots, true) != 0;
  if (GetSurfaceRestraint() == kSR_Phazon &&
      mPlayerState->GetItemAmount(CPlayerState::kIT_GravityBoost, true) == 0) {
    return spaceJump ? 5.25f : 4.75f;
  }
  return spaceJump ? 14.f : 11.66666f;
}

bool CPlayer::AttachActorToPlayer(TUniqueId actor, bool disableGun) {
  if (mAttachedActor == kInvalidUniqueId) {
    if (disableGun) {
      mGun->SetBombsDisabled(true);
    }
    mAttachedActor = actor;
    mAttachedActorTime = 0.f;
    mAttachedActorStruggle = 0.f;
    mMorphBall->StopParticleWakes();
    return true;
  }
  return false;
}

void CPlayer::DetachActorFromPlayer() {
  mAttachedActor = kInvalidUniqueId;
  mAttachedActorTime = 0.f;
  mAttachedActorStruggle = 0.f;
  mGun->SetBombsDisabled(false);
}

CVector3f CPlayer::CalculateLeftStickEdgePosition(float strafeInput, float forwardInput) const {
  float edgeX = -1.f;
  float cornerX = -0.555f;
  float cornerY = 0.555f;
  if (strafeInput >= 0.f) {
    edgeX = -edgeX;
    cornerX = -cornerX;
  }
  if (forwardInput < 0.f) {
    cornerY = -cornerY;
  }

  const float angle = static_cast< float >(atan(fabsf(forwardInput) / fabsf(strafeInput)));
  const float blend = CMath::Limit(angle / (M_PIF / 4.f), 1.f);
  return CVector3f(edgeX, 0.f, 0.f) +
         CVector3f::ByElementMultiply(CVector3f(blend, blend, blend),
                                      CVector3f(cornerX, cornerY, 0.f) -
                                          CVector3f(edgeX, 0.f, 0.f));
}

void CPlayer::UpdateFreeLook(float dt) {
  if (GetFrozenState() || mCameraManager->IsInCinematicCamera()) {
    return;
  }

  float lookDeltaAngle = dt * GetTweakPlayer()->GetFreeLookSpeed();
  if (!mLookAnalogHeld) {
    lookDeltaAngle = dt * GetTweakPlayer()->GetFreeLookSnapSpeed();
  }

  float angleVel = mVertFreeLookAngleVel - mFreeLookPitchAngle;
  float minDamp = 0.f;
  if (close_enough(mVertFreeLookAngleVel, 0.f) ||
      close_enough(mVertFreeLookAngleVel, GetTweakPlayer()->GetVerticalFreeLookAngleVel()) ||
      close_enough(mVertFreeLookAngleVel, -GetTweakPlayer()->GetVerticalFreeLookAngleVel())) {
    minDamp = 0.1f;
  }
  const float vertLookDamp = CMath::Clamp(minDamp, fabsf(angleVel / 1.0471976f), 1.f);
  if (fabsf(mVertFreeLookAngleVel - mFreeLookPitchAngle) < 0.0017453293f) {
    mFreeLookPitchAngle = mVertFreeLookAngleVel;
  } else {
    const float step = lookDeltaAngle * CMath::EaseInOut(vertLookDamp, CMath::kET_Quadratic, 0.25f,
                                                         0.75f, 0.f, 1.f, 2.f);
    if (angleVel > 0.f) {
      mFreeLookPitchAngle = CMath::Clamp(-M_PIF, step + mFreeLookPitchAngle, mVertFreeLookAngleVel);
    } else {
      mFreeLookPitchAngle = CMath::Clamp(mVertFreeLookAngleVel, mFreeLookPitchAngle - step, M_PIF);
    }
  }

  angleVel = mHorizFreeLookAngleVel - mFreeLookYawAngle;
  const float step =
      lookDeltaAngle *
      CMath::Clamp(0.f, fabsf(angleVel / GetTweakPlayer()->GetHorizontalFreeLookAngleVel()), 1.f);
  if (0.f <= angleVel) {
    mFreeLookYawAngle += step;
  } else {
    mFreeLookYawAngle -= step;
  }
  if (GetTweakPlayerControls()->GetFreeLookTurnsPlayer()) {
    mFreeLookYawAngle = 0.f;
  }
}

void CPlayer::ComputeFreeLook(const CFinalInput& input, CStateManager& mgr) {
  float lookLeft = mControlMapper.GetAnalogInput(CControlMapper::kC_LookLeft, input);
  float lookRight = mControlMapper.GetAnalogInput(CControlMapper::kC_LookRight, input);
  float lookUp = mControlMapper.GetAnalogInput(CControlMapper::kC_LookUp, input);
  float lookDown = mControlMapper.GetAnalogInput(CControlMapper::kC_LookDown, input);
  CGameOptions& options = gpGameState->GameOptions();
  CPlayerOptions& playerOptions = options.PlayerOptions(mPlayerIndex);
  const bool invertY =
      mgr.IsMultiplayer() ? playerOptions.GetInvertYAxis() : options.GetInvertYAxis();
  if (invertY) {
    lookUp = mControlMapper.GetAnalogInput(CControlMapper::kC_LookDown, input);
    lookDown = mControlMapper.GetAnalogInput(CControlMapper::kC_LookUp, input);
  }

  if (!GetTweakPlayerControls()->GetStayInFreeLookWhileFiring() &&
      (FireBeamHeld(input) || mOrbitState != kOS_NoOrbit)) {
    mHorizFreeLookAngleVel = 0.f;
    mVertFreeLookAngleVel = 0.f;
  } else {
    if (mInFreeLook) {
      if (!mFreeLookAnglesHeld) {
        mHorizFreeLookAngleVel =
            (lookLeft - lookRight) * GetTweakPlayer()->GetHorizontalFreeLookAngleVel();
        mVertFreeLookAngleVel =
            (lookUp - lookDown) * GetTweakPlayer()->GetVerticalFreeLookAngleVel();
      } else {
        mHorizFreeLookAngleVel = mFreeLookYawAngle;
        mVertFreeLookAngleVel = mFreeLookPitchAngle;
      }
    }
    if ((!mLookAnalogHeld && !mFreeLookAnglesHeld) ||
        (GetTweakPlayerControls()->GetHoldButtonsForFreeLook() && !mLookButtonHeld)) {
      mHorizFreeLookAngleVel = 0.f;
      mVertFreeLookAngleVel = 0.f;
    }
  }

  bool ignoreButtons = false;
  const uint ignoreFreeLookButtons = 0x80000;
  if (const CScriptPlayerHint* hint =
          TCastToConstPtr< CScriptPlayerHint >(mPlayerHintManager->GetCurrentHint(mgr))) {
    ignoreButtons = (hint->GetOverrideFlags() & ignoreFreeLookButtons) != 0;
  }
  if (GetTweakPlayerControls()->GetHoldButtonsForFreeLook() && !ignoreButtons) {
    if ((GetTweakPlayerControls()->GetTwoButtonsForFreeLook() &&
         (!mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold1, input) ||
          !mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold2, input))) ||
        (!mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold1, input) &&
         !mControlMapper.GetDigitalInput(CControlMapper::kC_LookHold2, input))) {
      mHorizFreeLookAngleVel = 0.f;
      mVertFreeLookAngleVel = 0.f;
    }
  }
  if (IsMorphBallTransitioning()) {
    mHorizFreeLookAngleVel = 0.f;
    mVertFreeLookAngleVel = 0.f;
  }
}

void CPlayer::UpdateGunAlpha(const CStateManager& mgr) {
  switch (mGun->GetGunHolsterState()) {
  case CPlayerGunBase::kGHS_Holstered:
    mGunAlpha = 0.f;
    break;
  case CPlayerGunBase::kGHS_Holstering:
    mGunAlpha = CMath::Clamp(
        0.f, mGun->GetGunHolsterRemTime() / gpTweakPlayerGun->GetGunHolsterTime(), 1.f);
    break;
  case CPlayerGunBase::kGHS_Drawing:
    mGunAlpha = 1.f - CMath::Clamp(0.f, mGun->GetGunHolsterRemTime() / 0.45f, 1.f);
    break;
  case CPlayerGunBase::kGHS_Drawn:
    mGunAlpha = 1.f;
    break;
  }
}

void CPlayer::AddToRenderer(const CStateManager& mgr) const {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::PreRenderAllViewports(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::PreRender(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::RenderReflectedPlayer(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80012040(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
void CPlayer::fn_80011fc0() const {
  // TODO: Recover the remaining target behavior.
}

rstl::pair< bool, CColor > CPlayer::GetHackedEffectColor() const {
  // TODO: Recover the per-frame hacked-effect color selection.
  return rstl::pair< bool, CColor >(false, CColor::Black());
}

void CPlayer::Render(const CStateManager& mgr) const {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::RenderGun(const CStateManager& mgr, const CVector3f& position) const {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80010f4c(const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80010bf4(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::GetCombatMode() const {
  switch (mGun->GetGunHolsterState()) {
  case CPlayerGunBase::kGHS_Drawing:
  case CPlayerGunBase::kGHS_Drawn:
    return true;
  case CPlayerGunBase::kGHS_Holstered:
  case CPlayerGunBase::kGHS_Holstering:
    return false;
  }
  return false;
}

bool CPlayer::GetExplorationMode() const {
  switch (mGun->GetGunHolsterState()) {
  case CPlayerGunBase::kGHS_Drawing:
  case CPlayerGunBase::kGHS_Drawn:
    return false;
  case CPlayerGunBase::kGHS_Holstered:
  case CPlayerGunBase::kGHS_Holstering:
    return true;
  default:
    return false;
  }
}

void CPlayer::SetScanningState(EPlayerScanState state, CStateManager& mgr) {
  // TODO: Scan completion, camera and sound side effects.
  mScanState = state;
}

bool CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr) const {
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_ScanItem, input)) {
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()));
    if (mOrbitState == kOS_OrbitObject && actor &&
        actor->GetMaterialList().HasMaterial(kMT_Scannable) && actor->GetScannableObjectInfo() &&
        actor->GetCurrentAreaId() == GetCurrentAreaId()) {
      if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
        if (mPlayerState->GetItemAmount(CPlayerState::kIT_ScanVirus, true) == 0 ||
            player->GetTurretState() == kTS_Active) {
          return false;
        }
        if (player->GetPlayerState()->GetItemAmount(CPlayerState::kIT_HackedEffect, true) &
            (1 << GetPlayerIndex())) {
          return false;
        }
      } else if (mgr.IsMultiplayer()) {
        return false;
      }

      CVector3f targetToPlayer = GetTranslation() - actor->GetTranslation();
      if (targetToPlayer.CanBeNormalized() &&
          targetToPlayer.Magnitude() < GetTweakPlayer()->GetScanningRange()) {
        return true;
      }
    }
  }
  return false;
}

void CPlayer::UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::Touch(CActor& actor, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

rstl::optional_object< CAABox > CPlayer::GetTouchBounds() const {
  if (mMorphBallState == kMS_Morphed) {
    const float radius = mMorphBall->GetBallTouchRadius();
    const float negativeRadius = -radius;
    const CVector3f center = GetTranslation() + CVector3f(0.f, 0.f, mMorphBall->GetBallRadius());
    return CAABox(center + CVector3f(negativeRadius, negativeRadius, negativeRadius),
                  center + CVector3f(radius, radius, radius));
  }
  return GetBoundingBox();
}

void CPlayer::SetHudDisable(float staticTimer, float fadeOutSpeed, float fadeInSpeed) {
  mStaticTimer = staticTimer;
  mStaticOutSpeed = fadeOutSpeed;
  mStaticInSpeed = fadeInSpeed;
  if (mStaticOutSpeed == 0.f) {
    mVisorStaticAlpha = mStaticTimer == 0.f ? 1.f : 0.f;
  }
}

bool CPlayer::CanEnterMorphBallState() const {
  if (mGrappleState != kGS_None || !mCanEnterMorphBall) {
    return false;
  }
  if (mPlayerState->GetItemAmount(CPlayerState::kIT_ScanVirus, true) != 0 &&
      mPlayerState->GetItemAmount(CPlayerState::kIT_DeathBall, true) == 0) {
    return false;
  }
  return true;
}

bool CPlayer::CanLeaveMorphBallState(CStateManager& mgr, CVector3f& position) const {
  if (mMorphBall->IsProjectile() || !mCanLeaveMorphBall) {
    return false;
  }

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f& nearPosition = GetTranslation();
  mgr.BuildColliderList(nearList, *this,
                        CAABox(mFpBounds.GetMinPoint() - CVector3f(1.f, 1.f, 1.f) + nearPosition,
                               mFpBounds.GetMaxPoint() + CVector3f(1.f, 1.f, 1.f) + nearPosition));
  const CAABox& baseBounds = GetBaseBoundingBox();
  const CVector3f& playerPosition = GetTranslation();
  position = CVector3f::Zero();

  for (int i = 0; i < 8; ++i) {
    CCollidableAABox bounds(CAABox(baseBounds.GetMinPoint() + position + playerPosition,
                                   baseBounds.GetMaxPoint() + position + playerPosition),
                            CMaterialList());
    if (!CGameCollision::DetectCollisionBoolean(mgr, bounds, CTransform4f::Identity(), filter,
                                                nearList)) {
      return true;
    }
    position[kDZ] += 0.1f;
  }
  return false;
}

CHealthInfo* CPlayer::HealthInfo() { return mPlayerState->HealthInfo(); }

void CPlayer::TakeDamage(bool significant, const CVector3f& location, float damage,
                         TUniqueId source, TUniqueId owner, const CDamageInfo& damageInfo,
                         CStateManager& mgr) {
  const EWeaponType type = EWeaponType(damageInfo.GetWeaponMode().GetRawType());
  if (significant) {
    if (damage >= 0.f) {
      mDamageAmount += damage;
      mPrevDamageAmount += (type == kWT_AI && damage == 0.00002f) ? 10.f : damage;
      if (location.IsNonZero()) {
        mDamageLocation = location;
      }
      if (damageInfo.NoImmunity()) {
        mWasDamagedPrev = true;
      } else {
        mWasDamaged = true;
      }
      if (type != kWT_AreaDark || mDamageWeaponType == uint(kWT_None)) {
        mDamageWeaponType = type;
      }
      mDamageColorTimer = 0.33333334f;

      ushort suitDamageSfx = damageInfo.GetDamageSfxId();
      ushort damageLoopSfx = damageInfo.GetDamageLoopSfxId();
      ushort damageSamusVoiceSfx = damageInfo.GetSamusVoiceSfxId();
      if (suitDamageSfx == CSfxManager::kInternalInvalidSfxId &&
          damageLoopSfx == CSfxManager::kInternalInvalidSfxId &&
          damageSamusVoiceSfx == CSfxManager::kInternalInvalidSfxId) {
        GetDamageSfx(damage, source, owner, type, mgr, suitDamageSfx, damageLoopSfx,
                     damageSamusVoiceSfx);
      }

      bool voiceStarted = false;
      bool doRumble = false;
      if (damageSamusVoiceSfx != CSfxManager::kInternalInvalidSfxId && mSamusVoiceTimeout <= 0.f) {
        voiceStarted = StartSamusVoiceSfx(damageSamusVoiceSfx, 127, 8);
        mSamusVoiceTimeout = mgr.Random()->Range(3.f, 4.f);
        doRumble = true;
      }

      const bool playingLoopSfx = mDamageLoopSfx;
      if (damageLoopSfx != CSfxManager::kInternalInvalidSfxId && !mNoDamageLoopSfx &&
          mFramesSinceDamageSfx >= 2) {
        if (!playingLoopSfx || mDamageLoopSfxId != damageLoopSfx) {
          if (playingLoopSfx && mDamageLoopSfxId != damageLoopSfx) {
            CSfxManager::SfxStop(mDamageLoopSfx);
          }
          mDamageLoopSfx =
              CSfxManager::SfxStart(damageLoopSfx, 127, GetSoundPan(kMSP_Player),
                                    CSfxManager::kAllAreas, false, true, CSfxManager::kMedPriority);
          mDamageLoopSfxId = damageLoopSfx;
        }
        mDamageSfxTimer = 0.5f;
      }

      if (suitDamageSfx != CSfxManager::kInternalInvalidSfxId &&
          mTimeSinceDamageImpactSfx > mgr.Random()->Range(0.075f, 0.125f)) {
        if (playingLoopSfx) {
          CSfxManager::SfxStop(mDamageLoopSfx);
          mDamageLoopSfx.Clear();
        }
        CSfxManager::SfxStart(suitDamageSfx, 127, GetSoundPan(kMSP_Player), CSfxManager::kAllAreas,
                              false, false, CSfxManager::kMedPriority);
        mDamageLoopSfxId = suitDamageSfx;
        mFramesSinceDamageSfx = 0;
        mTimeSinceDamageImpactSfx = 0.f;
        doRumble = true;
      }

      if (doRumble && mMorphBallState == kMS_Unmorphed) {
        mGun->DamageRumble(location, damage, mgr);
      }
      if (voiceStarted) {
        mgr.RumbleManager(GetPlayerIndex())
            ->Rumble(mgr, kRFX_PlayerBump, CMath::Limit(mDamageAmount / 25.f, 1.f), kRP_One);
      }

      if (mMorphBallState != kMS_Unmorphed) {
        if (type != kWT_AreaDark || !mPlayerState->HasPowerUp(CPlayerState::kIT_DarkSuit)) {
          mMorphBall->TakeDamage(mDamageAmount);
        }
        if (!mNoMorphBallDamageTimer && type != kWT_AreaDark) {
          mMorphBall->SetDamageTimer(0.4f);
        }
      }
    }

    if (mGrappleState != kGS_None && type != kWT_AreaDark && type != kWT_PoisonWater2) {
      BreakGrapple(kOR_KnockBack, mgr);
    }
  }

  if (GetFrozenState()) {
    mFrozenDamage += damage;
  }
}

bool CPlayer::WasDamaged() const { return mWasDamaged || mWasDamagedPrev; }

float CPlayer::GetDamageAmount() const { return mDamageAmount; }

float CPlayer::GetPrevDamageAmount() const { return mPrevDamageAmount; }

CVector3f CPlayer::GetDamageLocationWR() const { return mDamageLocation; }

uint CPlayer::GetDamageWeaponType() const { return mDamageWeaponType; }

void CPlayer::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::ObjectInScanningRange(TUniqueId id, const CStateManager& mgr) {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id))) {
    CVector3f delta = actor->GetTranslation() - GetTranslation();
    if (delta.CanBeNormalized() && delta.Magnitude() < GetTweakPlayer()->GetScanningRange()) {
      return true;
    }
  }
  return false;
}

CVector3f CPlayer::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f position = GetTranslation();
  if (dt > 0.f) {
    if (mOrbitState == kOS_NoOrbit) {
      const CMotionState motion = PredictMotion(dt);
      CVector3f delta = motion.GetTranslation();
      if (mMorphBallState == kMS_Morphed && (mMovementState == NPlayer::kMS_OnGround ||
                                             mMovementState == NPlayer::kMS_FallingMorphed)) {
        const CVector3f normal = mMorphBall->GetCollisionInfos().GetCombinedNormalLeft();
        const float intoSurface = CVector3f::Dot(normal, delta);
        if (intoSurface < 0.f) {
          delta -= intoSurface * normal;
        }
      }
      position += delta;
    } else {
      position = CSteeringBehaviors::ProjectOrbitalPosition(GetTranslation(), GetVelocityWR(),
                                                            GetOrbitPoint(), dt, mPreThinkDt);
    }
  }
  position[kDZ] +=
      mMorphBallState == kMS_Morphed ? GetTweakPlayer()->GetBallRadius() : GetEyeHeight();
  return position;
}

CVector3f CPlayer::GetHomingPosition(const CStateManager& mgr, float dt) const {
  if (const CScriptPlayerHint* hint =
          TCastToConstPtr< CScriptPlayerHint >(GetPlayerHintManager()->GetCurrentHint(mgr))) {
    if ((hint->GetOverrideFlags() & 0x40000) != 0 && mPlayerState->GetChargeBeamFactor() == 1.f) {
      return mGun->GetGunWorldTransform() * mGun->GetBeamLocalTransform().GetTranslation();
    }
  }
  CVector3f position = GetAimPosition(mgr, dt);
  if (mMorphBallState != kMS_Morphed) {
    position[kDZ] -= 0.25f * GetEyeHeight();
  }
  return position;
}

const CDamageVulnerability* CPlayer::GetDamageVulnerability() const {
  return GetDamageVulnerability(CVector3f::Zero(), CVector3f(0.f, 0.f, 1.f), CDamageInfo());
}

const CDamageVulnerability* CPlayer::GetDamageVulnerability(const CVector3f& position,
                                                            const CVector3f& direction,
                                                            const CDamageInfo& damage) const {
  if (mInvulnerabilityTimer > 0.f ||
      mPlayerState->GetItemAmount(CPlayerState::kIT_Invincibility, true) != 0) {
    return &mImmuneVulnerability;
  }
  if (mMorphBall->InScrewAttackMode()) {
    return mPlayerState->HasPowerUp(CPlayerState::kIT_LightSuit) ? &mImmuneVulnerability
                                                                 : &mScrewAttackVulnerability;
  }
  if (mMorphBallState == kMS_Morphed) {
    if (mImmuneTimer > 0.f && !damage.NoImmunity()) {
      return &CDamageVulnerability::ImmuneVulnerabilty();
    }
    if (mPlayerState->GetItemAmount(CPlayerState::kIT_CannonBall, true) != 0) {
      return &mImmuneVulnerability;
    }
  }
  if (mPlayerState->HasPowerUp(CPlayerState::kIT_LightSuit)) {
    return &mLightSuitVulnerability;
  }
  return mPlayerState->HasPowerUp(CPlayerState::kIT_DarkSuit) ? &mDarkSuitVulnerability
                                                              : &mVariaSuitVulnerability;
}

bool CPlayer::CanRenderUnsorted(const CStateManager& mgr) const { return false; }

bool CPlayer::HasTransitionBeamModel() const {
  return mBallTransitionBeamModel.get() &&
         (mBallTransitionBeamModel->HasAnimation() || mBallTransitionBeamModel->HasNormalModel());
}

void CPlayer::CollectBallTransitionAnimationTokens() {
  GetModelData()->GetAnimationData()->CollectAnimationTokens(mBallTransitionsRes, true);
}

void CPlayer::AsyncLoadSuit(CStateManager& mgr) { mGun->AsyncLoadSuit(mgr); }

bool CPlayer::IsPlayerDeadEnough(const CStateManager& mgr) const {
  switch (mMorphBallState) {
  case kMS_Unmorphed:
  case kMS_Unmorphing:
    if (mDeathTime > 10.f) {
      return true;
    }
    if (mRagDoll.get()) {
      return mRagDoll->IsOver() || mDeathTime > 5.f;
    }
    if (mDeathFadeEnabled && !close_enough(GetDeathAlpha(), 0.f)) {
      return false;
    }
    if (mgr.IsMultiplayer() && mKnockBackManager.IsDeathAnimationStarted() &&
        !mBodyController->IsDeathReactionOver()) {
      return false;
    }
    return !(mDeathTime < 3.5f);
  case kMS_Morphed:
  case kMS_Morphing:
    return mDeathTime > 2.5f;
  }
  return false;
}

void CPlayer::SetControlDirectionInterpolation(float duration) {
  mInterpolatingControlDir = true;
  mControlDirInterpTime = 0.f;
  mControlDirInterpDuration = duration;
}

void CPlayer::ResetControlDirectionInterpolation() {
  mInterpolatingControlDir = false;
  mControlDirInterpTime = 0.f;
}

void CPlayer::DoThink(float dt, CStateManager& mgr) {
  Think(dt, mgr);
  CEntity* effect = mgr.ObjectById(mDeathEffectId);
  if (effect) {
    effect->Think(dt, mgr);
  }
}

void CPlayer::DoPreThink(float dt, CStateManager& mgr) {
  PreThink(dt, mgr);
  CEntity* effect = mgr.ObjectById(mDeathEffectId);
  if (effect) {
    effect->PreThink(dt, mgr);
  }
}

void CPlayer::PushSustainedDamage() {
  if (mSustainedDamageCount == 0) {
    mSustainedDamageTime = 0.f;
  }
  ++mSustainedDamageCount;
}

void CPlayer::PopSustainedDamage() {
  if (mSustainedDamageCount != 0) {
    --mSustainedDamageCount;
  }
}

void CPlayer::UpdateDamageTimers(float dt) {
  mFramesSinceDamageSfx = rstl::min_val(mFramesSinceDamageSfx + 1, 2);
  if (mSustainedDamageCount != 0) {
    mSustainedDamageTime += dt;
    if (mSustainedDamageTime > 2.f) {
      mSustainedDamageTime = 0.f;
    }
  }
}

void CPlayer::ApplySubmergedPitchBend(CSfxHandle handle) {
  if (CheckSubmerged()) {
    CSfxManager::PitchBend(handle, 0);
  }
}

void CPlayer::SetPlayerHitWallDuringMove() {
  mHitWallDuringMove = true;
  mCurAcceleration = 1;
}

void CPlayer::SetMinimalAccelerationTimer(float duration) {
  mAccelerationChangeTimer = duration;
  mCurAcceleration = 1;
}

void CPlayer::PostUpdate(float dt, CStateManager& mgr) {
  mPlayerHintManager->Update(dt, mgr);
  mControlHintManager->Update(dt, mgr);
  UpdateCameraState(mgr);
  UpdateArmAndGunTransforms(dt, mgr);
  if (mGun->GrappleArm() && mGrappleState == kGS_Swinging) {
    mGun->GrappleArm()->SetSwingT(mGrappleSwingTimer / GetTweakPlayer()->GetGrappleSwingPeriod());
  }
  if (mCameraManager->IsInCinematicCamera()) {
    *mCameraBob = CPlayerCameraBob(CPlayerCameraBob::kCBT_One);
  } else {
    UpdateCameraBob(dt, mgr);
  }
  if (mTurretState != kTS_Active) {
    mGun->Update(dt, mgr);
    mBodyController->PlayGunReaction(mgr);
  }
  UpdateOrbitTarget(mgr);
  UpdateOrbitOrientation(mgr);
}

bool CPlayer::StartSamusVoiceSfx(ushort sfx, short volume, int priority) {
  bool started = true;
  if (mMorphBallState == kMS_Morphed) {
    return false;
  }

  if (mSamusVoiceSfx && CSfxManager::IsPlaying(mSamusVoiceSfx)) {
    started = false;
    if (priority > mSamusVoicePriority) {
      CSfxManager::SfxStop(mSamusVoiceSfx);
      started = true;
    }
  }

  if (started) {
    mSamusVoiceSfx =
        CSfxManager::SfxStart(sfx, volume, GetSoundPan(kMSP_Player), CSfxManager::kAllAreas, false,
                              false, CSfxManager::kMedPriority);
    mSamusVoicePriority = priority;
  }
  return started;
}

// Guessed name
void CPlayer::GetDamageSfx(float damage, TUniqueId source, TUniqueId owner, EWeaponType weaponType,
                           const CStateManager& mgr, ushort& impactSfx, ushort& loopSfx,
                           ushort& voiceSfx) {
  impactSfx = loopSfx = voiceSfx = CSfxManager::kInternalInvalidSfxId;
  if (owner != kInvalidUniqueId && mAttachedActor == owner) {
    return;
  }

  if (mgr.IsMultiplayer()) {
    switch (weaponType) {
    case kWT_Phazon:
      loopSfx = 0x2864;
      break;
    case kWT_PoisonWater1:
      loopSfx = 0x27f7;
      break;
    case kWT_AreaDark:
      loopSfx = 0x2621;
      break;
    case kWT_PoisonWater2:
      loopSfx = 0x27f7;
      break;
    case kWT_Lava:
    case kWT_Heat:
      break;
    default:
      if (mKnockBackManager.GetBurnRemainingTime() > 0.f) {
        loopSfx = 0x2688;
      }
      if (source == GetUniqueId()) {
        break;
      }
      if (mMorphBallState == kMS_Unmorphed) {
        if (damage > 20.f) {
          impactSfx = 0x2812;
        } else if (damage > 10.f) {
          impactSfx = 0x2811;
        } else {
          impactSfx = 0x2810;
        }
      } else {
        if (damage > 20.f) {
          impactSfx = 0x280d;
        } else if (damage > 10.f) {
          impactSfx = 0x280f;
        } else {
          impactSfx = 0x280e;
        }
      }
      break;
    }
  } else {
    switch (weaponType) {
    case kWT_Phazon:
      loopSfx = 0x2862;
      voiceSfx = 0x10c2;
      break;
    case kWT_PoisonWater1:
      loopSfx = 0x99;
      voiceSfx = 0x10be;
      break;
    case kWT_PoisonWater2:
      loopSfx = 0x5c8;
      break;
    case kWT_Lava:
    case kWT_Heat:
    case kWT_AreaDark:
      break;
    default:
      if (mMorphBallState == kMS_Unmorphed) {
        if (damage > 30.f) {
          voiceSfx = 0xb2;
        } else if (damage > 15.f) {
          voiceSfx = 0xb1;
        } else {
          voiceSfx = 0x9c;
        }
        impactSfx = 0x9b1;
      } else {
        if (damage > 30.f) {
          impactSfx = 0x1d31;
        } else if (damage > 15.f) {
          impactSfx = 0x1d30;
        } else {
          impactSfx = 0x1d2f;
        }
      }
      break;
    }
  }
}

float CPlayer::GetAttachedActorStruggle() const { return mAttachedActorStruggle; }

void CPlayer::FinishNewScan(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::IsEnergyLow() const {
  return GetHealthInfo()->GetHP() <
         (mPlayerState->GetItemCapacity(CPlayerState::kIT_EnergyTanks) > 3 ? 100.f : 30.f);
}

CVector3f CPlayer::GetOrbitPosition(const CStateManager& mgr) const {
  if (mMorphBallState == kMS_Morphed) {
    return GetBallPosition();
  }
  return GetEyePosition();
}

void CPlayer::SetTurretState(ETurretState state, CStateManager& mgr) {
  // TODO: Apply entry/exit camera, collision, controls and animation changes.
  mTurretState = state;
}

void CPlayer::StartTurret(TUniqueId turret, CStateManager& mgr) {
  // TODO: Camera, controls, orbit, visor and multiplayer displacement.
  mTurretId = turret;
  SetTurretState(kTS_Entering, mgr);
}

void CPlayer::ExitTurret(CStateManager& mgr) { SetTurretState(kTS_Exiting, mgr); }

void CPlayer::EjectFromTurret(TUniqueId turret, CStateManager& mgr) {
  if (mTurretId == turret) {
    SetTurretState(kTS_Ejected, mgr);
  }
}

void CPlayer::fn_8000d5dc(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_8000d540(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::fn_8000d40c(const CVector3f& direction, CStateManager& mgr) {
  // TODO: Check the turret's allowed horizontal aiming cone.
  return false;
}

void CPlayer::fn_8000d3ac(const CVector3f& direction, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

CTransform4f CPlayer::GetTurretTransform(const CStateManager& mgr) const {
  // TODO: Query the turret's camera transform.
  return GetTransform();
}

TUniqueId CPlayer::GetTurretId() const { return mTurretId; }

bool CPlayer::IsInTurret() const {
  return mTurretState == kTS_Active || mTurretState == kTS_Entering;
}

const CHintManager* CPlayer::GetPlayerHintManager() const { return mPlayerHintManager.get(); }

CHintManager* CPlayer::GetPlayerHintManager() { return mPlayerHintManager.get(); }

int CPlayer::fn_8000d0ac(const CStateManager& mgr, int channel) const {
  // TODO: Select the per-viewport sound-pan table.
  return 0;
}

short CPlayer::GetSoundPan(EMultiPlayerSoundPan channel) const { return mPlayerSoundPan[channel]; }

const CPlayerGun* CPlayer::GetPlayerGun() const { return mGun.get(); }

CPlayerGun* CPlayer::GetPlayerGun() { return mGun.get(); }

int CPlayer::GetPlayerIndex() const { return mPlayerIndex; }

void CPlayer::UpdateEchoVisorEffects(float dt, CStateManager& mgr) {
  mEchoPulsePhase += dt / gpTweakGui->GetEchoBigRingScanTime();
  const bool echoVisor = mPlayerState->GetActiveVisor(mgr) == CPlayerState::kPV_Echo;
  if (mEchoPulsePhase > 1.f) {
    mEchoPulsePhase -= 1.f;
    ++mEchoPulseCounter;
    if (echoVisor) {
      mEchoPulseLeftSfx = CSfxManager::SfxStart(0x14, 127, 0, CSfxManager::kAllAreas, false, false,
                                                CSfxManager::kMedPriority);
      mEchoPulseRightSfx = CSfxManager::SfxStart(0x15, 127, 127, CSfxManager::kAllAreas, false,
                                                 false, CSfxManager::kMedPriority);
    }
  }

  if (!echoVisor &&
      (CSfxManager::IsPlaying(mEchoPulseLeftSfx) || CSfxManager::IsPlaying(mEchoPulseRightSfx))) {
    CSfxManager::SfxStop(mEchoPulseLeftSfx);
    CSfxManager::SfxStop(mEchoPulseRightSfx);
    mEchoPulseLeftSfx.Clear();
    mEchoPulseRightSfx.Clear();
  }

  if (mEchoVisorAuxEffectId == 0 && echoVisor) {
    SFilteredDelayAuxParameters params;
    params.mDelayMs[0] = 160;
    params.mDelayMs[1] = 160;
    params.mDelayMs[2] = 160;
    params.mFeedbackPercent[0] = 80;
    params.mFeedbackPercent[1] = 80;
    params.mFeedbackPercent[2] = 0;
    params.mOutputPercent[0] = 70;
    params.mOutputPercent[1] = 70;
    params.mOutputPercent[2] = 0;
    params.mLowPassFrequency = 6000;
    params.mHighPassFrequency = 3000;
    mEchoVisorAuxEffectId = CSfxManager::AddAuxEffect(CSfxManager::kAllAreas, params, 127, 5);
  } else if (mEchoVisorAuxEffectId != 0 && !echoVisor) {
    CSfxManager::RemoveAuxEffect(mEchoVisorAuxEffectId);
    mEchoVisorAuxEffectId = 0;
  }
}

void CPlayer::UpdateDarkAetherDamage(float dt, CStateManager& mgr) {
  bool noDamage = true;
  if (!mgr.GetIsDarkWorld()) {
    mInSafeZone = true;
  } else {
    mInSafeZone = mgr.GetSafeZoneManager()->IsObjectInSafeZone(*this, mgr);
    if (mInSafeZone) {
      mDarkWorldDamageExposureTime =
          rstl::max_val(0.f, mDarkWorldDamageExposureTime -
                                 dt * GetTweakPlayer()->GetDarkWorldDamageRecoveryRate());
      if (!mNoSafeZoneHealing) {
        CPlayerState* state = mPlayerState;
        const float maxHealth = state->CalculateHealth();
        const CHealthInfo* health = state->HealthInfo();
        if (health && health->GetHP() < maxHealth) {
          const float previousHealth = health->GetHP();
          mPlayerState->IncrementHealth(dt);
          if (!mCameraManager->IsInCinematicCamera()) {
            const float currentHealth = health->GetHP();
            if (int(previousHealth) < int(currentHealth) && currentHealth < maxHealth) {
              CSfxManager::SfxStart(0x246f, 50, GetSoundPan(kMSP_4), CSfxManager::kAllAreas, false,
                                    false, CSfxManager::kMedPriority);
            }
          }
        }
      }
    }
  }

  if (!mgr.GetIsDarkWorld() || mPlayerState->HasPowerUp(CPlayerState::kIT_LightSuit)) {
    mDarkWorldDamageExposureTime = 0.f;
    mDarkAetherDamageFlashTime = 0.f;
    if (mDarkAetherParticles.get()) {
      mDarkAetherParticles = nullptr;
      mDarkAetherThirdPersonParticles = nullptr;
      mDarkAetherParticleDescriptions->first.Unlock();
      mDarkAetherParticleDescriptions->second.Unlock();
    }
  } else {
    if (!mInSafeZone) {
      mDarkWorldDamageExposureTime += dt;
      const float gracePeriod = GetTweakPlayer()->GetDarkWorldDamageGracePeriod();
      if (mDarkWorldDamageExposureTime >= gracePeriod) {
        if (!mIgnoreDarkWorldDamage) {
          mgr.ApplyDamage(
              kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId,
              CDamageInfo(GetTweakPlayer()->GetDarkWorldDamageInfo(), dt),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
              CVector3f::Zero());
        }
        mDarkWorldDamageExposureTime = gracePeriod;
        mDarkAetherDamageFlashTime += dt;
        if (mDarkAetherDamageFlashTime > 0.75f) {
          mDarkAetherDamageFlashTime -= 0.75f;
        }
        noDamage = false;
      }
    }

    const float targetDamage =
        mIgnoreDarkWorldDamage
            ? 0.f
            : mDarkWorldDamageExposureTime / GetTweakPlayer()->GetDarkWorldDamageGracePeriod();
    const float damageStep = dt / GetTweakPlayer()->GetDarkWorldDamageGracePeriod();
    if (targetDamage < mDarkAetherDamage) {
      mDarkAetherDamage = rstl::max_val(0.f, mDarkAetherDamage - damageStep);
    } else {
      mDarkAetherDamage = rstl::min_val(1.f, mDarkAetherDamage + damageStep);
    }
    float particleRate = mDarkAetherDamage;
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_DarkSuit)) {
      particleRate *= GetTweakPlayer()->GetDarkSuitEffectGenerationScale();
    }

    if (!mDarkAetherParticles.get()) {
      mDarkAetherParticleDescriptions->first.Lock();
      mDarkAetherParticleDescriptions->second.Lock();
      if (mDarkAetherParticleDescriptions->first.IsLoaded() &&
          mDarkAetherParticleDescriptions->second.IsLoaded()) {
        mDarkAetherParticles = rs_new CElementGen(mDarkAetherParticleDescriptions->first);
        mDarkAetherThirdPersonParticles =
            rs_new CElementGen(mDarkAetherParticleDescriptions->second);
      }
    }
    if (mDarkAetherParticles.get()) {
      mDarkAetherParticles->SetGeneratorRate(mInSafeZone ? 0.f : particleRate);
      mDarkAetherThirdPersonParticles->SetGeneratorRate(mInSafeZone ? 0.f : particleRate);
      if (mMorphBallState == kMS_Unmorphed) {
        if (particleRate > 0.f) {
          mDarkAetherParticles->SetTranslation(mGun->GetRainSplashPosition());
        }
        mDarkAetherParticles->Update(dt);
        mDarkAetherThirdPersonParticles->DestroyParticles();
      } else {
        if (particleRate > 0.f) {
          const float radius = mMorphBall->GetBallRadius();
          CRandom16* random = mgr.Random();
          CVector3f direction = CVector3f::Zero();
          do {
            const float x = random->Range(-1.f, 1.f);
            const float y = random->Range(-1.f, 1.f);
            const float z = random->Range(0.f, 1.f);
            direction = CVector3f(x, y, z);
          } while (!direction.CanBeNormalized());
          direction.Normalize();
          const CVector3f position =
              GetTranslation() + CVector3f(0.f, 0.f, radius) + radius * direction;
          mDarkAetherThirdPersonParticles->SetGlobalOrientAndTrans(CTransform4f::Identity());
          mDarkAetherThirdPersonParticles->SetTranslation(position);
          mDarkAetherThirdPersonParticles->SetOrientation(CTransform4f::Identity());
        }
        mDarkAetherThirdPersonParticles->Update(dt);
        mDarkAetherParticles->DestroyParticles();
      }
    }

    if (noDamage) {
      mDarkAetherDamageFlashTime = 0.f;
    }
    if (!mCameraManager->IsInCinematicCamera()) {
      const bool playSound = mDarkAetherDamage > 0.1f && mDarkAetherThirdPersonParticles.get();
      if (playSound && !mNoDamageLoopSfx && mFramesSinceDamageSfx > 1) {
        ushort sound = 0x2193;
        if (mPlayerState->HasPowerUp(CPlayerState::kIT_DarkSuit)) {
          sound = 0x0437;
        }
        if (mDarkAetherDamageLoopSfxId != sound || !mDarkAetherDamageLoopSfx) {
          if (mDarkAetherDamageLoopSfx) {
            CSfxManager::SfxStop(mDarkAetherDamageLoopSfx);
          }
          mDarkAetherDamageLoopSfxId = sound;
          mDarkAetherDamageLoopSfx =
              CSfxManager::SfxStart(sound, 127, GetSoundPan(kMSP_4), CSfxManager::kAllAreas, false,
                                    true, CSfxManager::kMedPriority);
        }
        CSfxManager::SfxVolume(mDarkAetherDamageLoopSfx, uchar(127.f * GetDarkAetherDamage()));
        mDarkAetherDamageLoopSfxTimer = 0.25f;
      }
    }
  }
}

void CPlayer::UpdateUnderwaterParticles(float dt, CStateManager& mgr) {
  if (mgr.IsMultiplayer()) {
    return;
  }
  if (!mgr.GetIsDarkWorld() && mCameraManager->GetCurrentCamera(mgr, true)->GetFluidCount() != 0) {
    if (!mUnderwaterParticles.get()) {
      mUnderwaterParticleDescriptions->first.Lock();
      mUnderwaterParticleDescriptions->second.Lock();
      if (mUnderwaterParticleDescriptions->first.IsLoaded() &&
          mUnderwaterParticleDescriptions->second.IsLoaded()) {
        mUnderwaterParticles = rs_new CElementGen(mUnderwaterParticleDescriptions->first);
        mUnderwaterThirdPersonParticles =
            rs_new CElementGen(mUnderwaterParticleDescriptions->second);
      }
    }
    if (!mUnderwaterParticles.get()) {
      return;
    }

    if (mMorphBallState == kMS_Unmorphed) {
      const float rate = (mGun->GetGunHolsterState() == CPlayerGunBase::kGHS_Holstered ||
                          mGun->GetFidgetState() == CFidget::kS_HolsterBeam)
                             ? 0.f
                             : 1.f;
      mUnderwaterParticles->SetGeneratorRate(rate);
      if (rate > 0.f) {
        mUnderwaterParticles->SetTranslation(mGun->GetTransform() * mGun->GetRainSplashPosition());
      }
      mUnderwaterParticles->Update(dt);
      mUnderwaterThirdPersonParticles->DestroyParticles();
    } else {
      const float rate = CMath::Clamp(
          0.f, 4.f * ((GetVelocityWR().Magnitude() - 2.f) / mMorphBall->ComputeMaxSpeed()), 4.f);
      mUnderwaterThirdPersonParticles->SetGeneratorRate(rate);
      if (rate > 0.f) {
        const float radius = mMorphBall->GetBallRadius();
        CVector3f direction = CVector3f::Zero();
        do {
          direction.SetX(mgr.Random()->Range(-1.f, 1.f));
          direction.SetY(mgr.Random()->Range(-1.f, 1.f));
          direction.SetZ(mgr.Random()->Range(-1.f, 1.f));
        } while (!direction.CanBeNormalized());
        direction.Normalize();
        mUnderwaterThirdPersonParticles->SetTranslation(
            GetTranslation() + CVector3f(0.f, 0.f, radius) + radius * direction);
      }
      mUnderwaterThirdPersonParticles->Update(dt);
      mUnderwaterParticles->DestroyParticles();
    }
  } else if (mUnderwaterParticles.get()) {
    mUnderwaterParticleDescriptions->first.Unlock();
    mUnderwaterParticleDescriptions->second.Unlock();
    mUnderwaterParticles = nullptr;
    mUnderwaterThirdPersonParticles = nullptr;
  }
}

CColor CPlayer::GetDarkAetherDamageColor(const CStateManager& mgr, int view) const {
  // TODO: Reconstruct the safe-zone/suit damage-color calculation.
  return CColor::Black();
}

CTweakPlayer* CPlayer::GetTweakPlayer() const {
  CTweakPlayer* tweak = gpTweakPlayerA.get();
  if (mControlScheme == 1) {
    tweak = gpTweakPlayerB.get();
  }
  return tweak;
}

CTweakPlayerControls* CPlayer::GetTweakPlayerControls() const {
  CTweakPlayerControls* tweak = gpTweakPlayerControlsA.get();
  if (mControlScheme == 1) {
    tweak = gpTweakPlayerControlsB.get();
  }
  return tweak;
}

float CPlayer::GetDarkAetherDamage() const { return mDarkAetherDamage; }

float CPlayer::GetDarkWorldDamageExposureFraction() const {
  if (mPlayerState->HasPowerUp(CPlayerState::kIT_LightSuit)) {
    return 0.f;
  }
  return mDarkWorldDamageExposureTime / GetTweakPlayer()->GetDarkWorldDamageGracePeriod();
}

float CPlayer::GetDeathAlpha() const {
  if (mDeathFadeEnabled) {
    const float elapsed = rstl::max_val(0.f, mDeathTime - mDeathFadeDelay);
    const float duration = rstl::max_val(FLT_EPSILON, mDeathFadeDuration);
    return 1.f - rstl::min_val(1.f, elapsed / duration);
  }
  return mPlayerState->IsPlayerAlive() ? 1.f : 0.f;
}

void CPlayer::SetMultiplayerBeamAuxParticlesEnabled(CStateManager& mgr, bool createNew) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::EmitMultiplayerBeamParticles(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_8000bbb4(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

CPlayerState::EBeamId CPlayer::GetCurrentBeam() const {
  return mUseAlternateBeam ? static_cast< CPlayerState::EBeamId >(4)
                           : mPlayerState->GetCurrentBeam();
}

void CPlayer::StopSounds() {
  StopLoopedSounds();
  StopSound(mDamageLoopSfx);
  StopSound(mDarkAetherDamageLoopSfx);
  mDamageSfxTimer = 0.f;
  mDarkAetherDamageLoopSfxTimer = 0.f;
  StopSound(mDashSfx);
  StopSound(mSamusVoiceSfx);
  StopSound(mGravityBoostSfx);
  StopSound(mFreezeSfx);
  mMorphBall->StopSounds();
}

const CHintManager* CPlayer::GetControlHintManager() const { return mControlHintManager.get(); }

CHintManager* CPlayer::GetControlHintManager() { return mControlHintManager.get(); }

bool CPlayer::IsOnGround() const { return mMovementState == NPlayer::kMS_OnGround; }

void CPlayer::SkipMorphTransition() { mMorphTime = 10000.f; }

void CPlayer::UpdateWaterInhabitants(float dt, CStateManager& mgr) {
  CObjectList& triggers = mgr.ObjectListById(kOL_Trigger);
  for (int index = triggers.GetFirstObjectIndex(); index != -1;
       index = triggers.GetNextObjectIndex(index)) {
    if (CScriptWater* water = TCastToPtr< CScriptWater >(triggers[index])) {
      water->UpdateInhabitants(dt, mgr);
    }
  }
}

void CPlayer::ClearFluidList(CStateManager& mgr) {
  const rstl::reserved_vector< TUniqueId, 4 > fluids = GetFluidList();
  for (int i = 0; i < fluids.size(); ++i) {
    if (CScriptWater* water = TCastToPtr< CScriptWater >(mgr.ObjectById(fluids[i]))) {
      water->RemoveInhabitant(GetUniqueId(), mgr);
    }
  }
  CActor::ClearFluidList(mgr);
}

void CPlayer::SetSurfaceRestraint(ESurfaceRestraints restraint) { mSurfaceRestraint = restraint; }

CPlayer::ESurfaceRestraints CPlayer::GetSurfaceRestraint() const { return mSurfaceRestraint; }

bool CPlayer::UnkVtable98() const { return mAlpha < 1.f; }
