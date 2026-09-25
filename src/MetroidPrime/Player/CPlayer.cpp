#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "rstl/math.hpp"
#include <float.h>
#include <math.h>

// NonMatching structure pass; incomplete behavior is explicit below.
// Definitions follow reverse target order for the TU's deferred-inlining emission.

CPlayer::CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
                 const CVector3f& playerScale, float mass, float stepUp, float stepDown,
                 float ballRadius, const CMaterialList& ml, CPlayerState* playerState,
                 CCameraManager* cameraManager, bool multiplayer, int playerIndex,
                 int controlScheme, int charIdx)
: CPhysicsActor(uid, CBasics::Stringize("CPlayer (%d)", playerIndex),
                CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, true), 0, xf,
                CAnimRes(resId, charIdx, CVector3f(1.8f, 1.8f, 1.8f), 0, true), ml, aabb,
                SMoverData(mass), CActorParameters::None(), StepData(stepUp, stepDown, 1))

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
, mCancelCameraPitch(false)
, mTimeSinceJump(1000.f)
, x324_(1000.0f)
, x328_(0.f)
, x32c_(CVector3f::Zero())
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
, x39c_(kMS_Unmorphed)
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
, x591_(false)
, x594_(0.f)
, mOrbitCandidateIndex(0)
, mOrbitCandidateRefreshFrames(0x14)
, mDashSpeedMultiplier(1.5f)
, mNoStrafeDashBlend(false)
, mDashDuration(0.5f)
, mStrafeDashBlendDuration(0.449f)
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
, x5f4_(false)
, x5f5_(false)
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
, xed0_(0)
, mTargeting(nullptr)
, mBodyController(nullptr)
, mKnockBackManager()
, xf6c_(0)
, mPlayerStuckTracker(nullptr)
, mMoveSpeedAvg()
, mMoveSpeed(0.f)
, mFlatMoveSpeed(0.f)
, mLookDir(xf.GetForward())
, mMoveDir(xf.GetForward())
, xfe8_(xf.GetForward())
, mLastPosForDirCalc(GetTranslation())
, mGunDir(xf.GetForward())
, mTimeMoving(0.f)
, mControlDir(xf.GetForward())
, mControlDirFlat(xf.GetForward())
, mVariaSuitVulnerability(CDamageVulnerability::NormalVulnerabilty())
, mDarkSuitVulnerability(mVariaSuitVulnerability)
, mLightSuitVulnerability(mDarkSuitVulnerability)
, mImmuneVulnerability(CDamageVulnerability::ImmuneVulnerabilty())
, mScrewAttackVulnerability(mImmuneVulnerability)
, mWasDamaged(false)
, mWasDamagedPrev(false)
, mDamageAmount(0.f)
, mPrevDamageAmount(0.f)
, mDamageLocation(CVector3f::Zero())
, x1130_(~0u)
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
, mIceBreakJumpTimeout(0.f)
, mRezbitState(kRS_None)
, mRezbitEffectToken(0)
, mRezbitEffectId(kInvalidUniqueId)
, mRezbitRecoveryTimer(0.f)
, mMorphBall(nullptr)
, mCameraBob(nullptr)
, mLandingSfx()
, mLandingSfxTimer(0.f)
, x1184_()
, x1188_()
, x118c_(0)
, x1190_(0.f)
, x1194_(0.f)
, mSamusVoicePriority(0)
, mSamusVoiceSfx()
, mDamageSfxTimer(0.f)
, mDamageLoopSfxId(0xffff)
, mFootstepSfxTimer(0.f)
, mFootstepSfx(0)
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
, x1258_()
, mGravityBoostActive(false)
, mGravityBoostColor(1.f, 1.f, 1.f, 0.f)
, mPlayerHintManager(nullptr)
, x1268_24_(false)
, mDrawCrosshairs(false)
, x1268_26_(true)
, x1268_27_(true)
, x1268_28_(true)
, x1268_29_(false)
, x1268_30_(false)
, x1268_31_(false)
, x1269_24_(false)
, mHitWallDuringMove(false)
, x1269_26_(false)
, x1269_27_(true)
, x1269_28_(false)
, mInterpolatingControlDir(false)
, x1269_30_(false)
, x1269_31_(false)
, x126a_(0)
, x126b_24_(true)
, x126b_25_(false)
, x126b_26_(false)
, x126b_27_(true)
, x126b_28_(false)
, mDeathFadeEnabled(false)
, mUseAlternateBeam(false)
, x126b_31_(false)
, x126c_24_(false)
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
, mDeathPowerBomb(kInvalidUniqueId)
, mPreThinkDt(0.f)
, mSteamTextureId(kInvalidAssetId)
, mFreezeSfx()
, mSustainedDamageCount(0)
, mSustainedDamageTime(0.f)
, x12cc_(9999.f)
, x12d0_(1.f)
, x12d4_(1.f)
, mAttachedActorStruggle(0.f)
, x12dc_(2)
, x12e0_(4.f)
, mDamageColorTimer(0.f)
, x12e8_(~0u)
, x12ec_(0)
, x12f0_(0.f)
, mInvulnerabilityTimer(0.f)
, mTurretState(kTS_None)
, mTurretId(kInvalidUniqueId)
, x1300_(0)
, mTurretTimer(0.f)
, mPlayerState(playerState)
, mCameraManager(cameraManager)
, mFrozenResources(nullptr)
, mControlScheme(controlScheme)
, x1324_(0.f)
, x1328_(0)
, x132c_(0)
, mDarkAetherDamageSfx()
, mSafeZoneHealSfx()
, mCharacterIndex(charIdx)
, x133c_(CVector3f::Zero())
, x1348_(CVector3f::Zero())
, mScreenPosition(0, 0)
, mSafeZoneHealSfxTimer(0.f)
, mDarkAetherDamage(0.f)
, mDarkAetherDamageFlashTime(0.f)
, mDarkAetherParticleDescriptions(nullptr)
, mDarkAetherParticles(nullptr)
, mDarkAetherThirdPersonParticles(nullptr)
, mUnderwaterParticleDescriptions(nullptr)
, mUnderwaterParticles(nullptr)
, mUnderwaterThirdPersonParticles(nullptr)
, mGunParticleLocator()
, mBeamEffectTokens()
, mBeamParticleDescriptions()
, mBeamParticles(nullptr)
, mParticleBeam(static_cast< CPlayerState::EBeamId >(-1))
, mBeamAuxParticles(nullptr)
, mPlayerIndex(playerIndex)
, mReflectionTextureData(nullptr)
, mIndirectTextureData(nullptr)
, mMaskTextureData(nullptr)
, mRezbitRecoveryDirection(0)
, mRezbitRecoveryInputCount(0)
, mControlMapper(controlScheme)
, mControlHintManager(nullptr)
, x14bc_(kInvalidUniqueId)
, x14c0_(0.f)
, x14c4_(kInvalidUniqueId)
, x14c6_(false) {
  // TODO: Construct targeting, body controller, failsafe, camera bob and hint managers.
  // TODO: Acquire beam/particle resources and reflection buffers, initialize control mapping,
  //       apply suit-specific vulnerabilities, and configure animation/lighting.
  mMorphBall = rs_new CMorphBall(*this, ballRadius, multiplayer);
}

CPlayer::~CPlayer() {
  for (rstl::vector< CToken >::iterator it = mBeamEffectTokens.begin();
       it != mBeamEffectTokens.end(); ++it) {
    it->Unlock();
  }
  // TODO: Release targeting, body controller, camera bob, failsafe and hint managers
  //       once their owning interfaces are recovered.
  CMemory::Free(mReflectionTextureData);
  CMemory::Free(mIndirectTextureData);
  CMemory::Free(mMaskTextureData);
  delete mFrozenResources;
  delete mMorphBall;
}

void CPlayer::fn_80019E40(CStateManager& mgr, int state) {
  // TODO: Recover the remaining target behavior.
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

// Guessed name
void CPlayer::UpdateAimPrediction(const CTransform4f& transform, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateAssistedAiming(const CTransform4f& transform, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateGunTransform(const CVector3f& position, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

const CTransform4f& CPlayer::fn_80019360() const {
  // TODO: Return the first-person camera transform through its shared interface.
  return GetTransform();
}

void CPlayer::UpdateArmAndGunTransforms(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ForceGunOrientation(const CTransform4f& transform, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::Update(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdatePlayerDrawFlags(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::ShouldSampleFailsafe(const CStateManager& mgr) const {
  // TODO: Inspect hint, collision and movement exclusions.
  return false;
}

CScannableObjectInfo* CPlayer::GetScannableObjectInfo() const {
  // TODO: Construct and select the per-player scan descriptors.
  return nullptr;
}

void CPlayer::UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateVisorTransition(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateCrosshairsState(const CFinalInput& input) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80017358(float dt) {
  // TODO: Recover the remaining target behavior.
}

float CPlayer::fn_80016ce4(float dt, const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover controller-jostle accumulation.
  return 0.f;
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
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_8001660c(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_800165ec(CStateManager& mgr) { fn_8001660c(mgr); }

void CPlayer::SetCameraState(EPlayerCameraState state, CStateManager& mgr) {
  // TODO: Notify camera manager and configure player rendering.
  mCameraState = state;
}

void CPlayer::UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80015d64(float dt, const CFinalInput& input) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                              float dt) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::PreThink(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateWaterSurfaceCameraBias(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::Think(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::Freeze(float timeout, CStateManager& mgr, CAssetId steamTexture, uint sfx,
                     CAssetId iceTexture) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::GetFrozenState() const { return mFrozenTimeout > 0.f; }

void CPlayer::BreakFrozenState(CStateManager& mgr, EBreakFrozenState state, bool playSound) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateFrozenState(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
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

float CPlayer::fn_80012e14() const {
  // TODO: Select jump duration from surface restraint and equipment.
  return 0.f;
}

bool CPlayer::AttachActorToPlayer(TUniqueId actor, bool disableGun) {
  if (mAttachedActor != kInvalidUniqueId) {
    return false;
  }
  mAttachedActor = actor;
  mAttachedActorTime = 0.f;
  mAttachedActorStruggle = 0.f;
  // TODO: Set the gun-disable flag and reset the ball attachment state.
  return true;
}

void CPlayer::DetachActorFromPlayer() {
  mAttachedActor = kInvalidUniqueId;
  mAttachedActorTime = 0.f;
  mAttachedActorStruggle = 0.f;
  // TODO: Clear the gun's attachment-disable flag.
}

void CPlayer::UpdateFreeLook(float dt) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ComputeFreeLook(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateGunAlpha(const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
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
  // TODO: Query gun holster state through its shared interface.
  return false;
}

bool CPlayer::GetExplorationMode() const {
  // TODO: Query gun holster state through its shared interface.
  return false;
}

void CPlayer::SetScanningState(EPlayerScanState state, CStateManager& mgr) {
  // TODO: Scan completion, camera and sound side effects.
  mScanState = state;
}

bool CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr) const {
  // TODO: Check input, visor, orbit target and scan progress.
  return false;
}

void CPlayer::UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::Touch(CActor& actor, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

rstl::optional_object< CAABox > CPlayer::GetTouchBounds() const {
  // TODO: Select morph-ball/standing bounds and damage-state exclusions.
  return rstl::optional_object< CAABox >();
}

void CPlayer::SetHudDisable(float staticTimer, float fadeOutSpeed, float fadeInSpeed) {
  mStaticTimer = staticTimer;
  mStaticOutSpeed = fadeOutSpeed;
  mStaticInSpeed = fadeInSpeed;
}

bool CPlayer::CanEnterMorphBallState() const {
  // TODO: Check grapple, Rezbit, turret and death states.
  return false;
}

bool CPlayer::CanLeaveMorphBallState(CStateManager& mgr, CVector3f& position) const {
  // TODO: Test the standing collision volume at candidate positions.
  return false;
}

CHealthInfo* CPlayer::HealthInfo() { return mPlayerState->HealthInfo(); }

void CPlayer::TakeDamage(bool significant, const CVector3f& location, float damage,
                         TUniqueId source, TUniqueId owner, const CWeaponMode& weapon,
                         CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::WasDamaged() const { return mWasDamaged || mWasDamagedPrev; }

float CPlayer::GetDamageAmount() const { return mDamageAmount; }

float CPlayer::GetPrevDamageAmount() const { return mPrevDamageAmount; }

CVector3f CPlayer::GetDamageLocationWR() const { return mDamageLocation; }

uint CPlayer::fn_8000f6e8() const { return x1130_; }

void CPlayer::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::ObjectInScanningRange(TUniqueId id, const CStateManager& mgr) {
  // TODO: Validate target, visor and scan distance.
  return false;
}

CVector3f CPlayer::GetAimPosition(const CStateManager& mgr, float dt) const {
  // TODO: Predict the eye/ball aim position from the motion state.
  return GetTranslation();
}

CVector3f CPlayer::GetHomingPosition(const CStateManager& mgr, float dt) const {
  // TODO: Account for morph/turret state and movement prediction.
  return GetTranslation();
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

void CPlayer::fn_8000eba0() {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::AsyncLoadSuit(CStateManager& mgr) { mGun->AsyncLoadSuit(mgr); }

bool CPlayer::IsPlayerDeadEnough(const CStateManager& mgr) const {
  // TODO: Death animation, morph state and multiplayer fade conditions.
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
  CEntity* bomb = mgr.ObjectById(mDeathPowerBomb);
  if (bomb) {
    bomb->Think(dt, mgr);
  }
}

void CPlayer::DoPreThink(float dt, CStateManager& mgr) {
  PreThink(dt, mgr);
  CEntity* bomb = mgr.ObjectById(mDeathPowerBomb);
  if (bomb) {
    bomb->PreThink(dt, mgr);
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

void CPlayer::fn_8000e85c(float dt) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ApplySubmergedPitchBend(CSfxHandle& handle) {
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
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::StartSamusVoiceSfx(ushort sfx, short volume, int priority) {
  // TODO: Handle morph state and sound priority before replacing the voice.
  return false;
}

// Guessed name
void CPlayer::GetDamageSfx(float damage, TUniqueId source, TUniqueId owner, EWeaponType weaponType,
                           const CStateManager& mgr, ushort& impactSfx, ushort& loopSfx,
                           ushort& voiceSfx) {
  // TODO: Recover the remaining target behavior.
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
  return mMorphBallState == kMS_Morphed ? GetBallPosition() : GetEyePosition();
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

const CHintManager* CPlayer::GetPlayerHintManager() const { return mPlayerHintManager; }

CHintManager* CPlayer::GetPlayerHintManager() { return mPlayerHintManager; }

int CPlayer::fn_8000d0ac(const CStateManager& mgr, int channel) const {
  // TODO: Select the per-viewport sound-pan table.
  return 0;
}

short CPlayer::GetSoundPan(EMultiPlayerSoundPan channel) const { return mPlayerSoundPan[channel]; }

const CPlayerGun* CPlayer::GetPlayerGun() const { return mGun.get(); }

CPlayerGun* CPlayer::GetPlayerGun() { return mGun.get(); }

int CPlayer::GetPlayerIndex() const { return mPlayerIndex; }

void CPlayer::fn_8000ce94(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateDarkAetherDamage(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_8000c124(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

CColor CPlayer::GetDarkAetherDamageColor(const CStateManager& mgr, int view) const {
  // TODO: Reconstruct the safe-zone/suit damage-color calculation.
  return CColor::Black();
}

CTweakPlayer* CPlayer::GetTweakPlayer() const {
  return mControlScheme == 1 ? gpTweakPlayerB : gpTweakPlayerA;
}

CTweakPlayerControls* CPlayer::GetTweakPlayerControls() const {
  // TODO: Expose the two control-tweak globals through their shared declaration.
  return nullptr;
}

float CPlayer::GetDarkAetherDamage() const { return mDarkAetherDamage; }

float CPlayer::fn_8000bf1c() const {
  // TODO: Normalize the safe-zone healing timer by the tweak period.
  return 0.f;
}

float CPlayer::GetDeathAlpha() const {
  if (mDeathFadeEnabled) {
    const float elapsed = rstl::max_val(0.f, mDeathTime - mDeathFadeDelay);
    const float duration = rstl::max_val(FLT_EPSILON, mDeathFadeDuration);
    return 1.f - rstl::min_val(1.f, elapsed / duration);
  }
  return mPlayerState->IsPlayerAlive() ? 1.f : 0.f;
}

void CPlayer::fn_8000bd5c(CStateManager& mgr, bool createNew) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_8000BC44(CStateManager& mgr) {
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
  // TODO: Recover the remaining target behavior.
}

const CHintManager* CPlayer::GetControlHintManager() const { return mControlHintManager; }

CHintManager* CPlayer::GetControlHintManager() { return mControlHintManager; }

bool CPlayer::IsOnGround() const { return mMovementState == NPlayer::kMS_OnGround; }

void CPlayer::SkipMorphTransition() { mMorphTime = 10000.f; }

void CPlayer::fn_8000ba60(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UnkVtable20(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::SetSurfaceRestraint(ESurfaceRestraints restraint) { mSurfaceRestraint = restraint; }

CPlayer::ESurfaceRestraints CPlayer::GetSurfaceRestraint() const { return mSurfaceRestraint; }

bool CPlayer::UnkVtable98() const { return mAlpha < 1.f; }
