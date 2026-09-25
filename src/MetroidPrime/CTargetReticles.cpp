#include "MetroidPrime/CTargetReticles.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include <math.h>
#include <stdio.h>

static bool IsDamageOrbit(CPlayer::EPlayerOrbitRequest request) {
  // TODO: recover the Echoes orbit-request enumerators used by damage/lock-break transitions.
  return false;
}

static float offshoot_func(float amplitude, float angularScale, float time) {
  return amplitude * CMath::FastSinR((time - 0.5f) * angularScale) + 0.5f;
}

static float calculate_premultiplied_overshoot_offset(float overshoot) {
  return 2.f * (M_PIF - static_cast< float >(asin(1.f / overshoot)));
}

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(const char* modelName)
: mModel(gpSimplePool->GetObj(modelName))
, mOffshootBaseAngle(0.f)
, mRotationAngle(0.f)
, mBaseAngle(0.f)
, mOffshootAngleDelta(0.f) {}

CCompoundTargetReticle::CCompoundTargetReticle(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex)
, mLeadingOrientation(CQuaternion::NoRotation())
, mLaggingOrientation(CQuaternion::NoRotation())
, mPreviousState(kRS_Unspecified)
, mNextState(kRS_Unspecified)
, mNoDrawTicks(0)
, mOvershootOffsetHalf(0.f)
, mPremultipliedOvershootOffset(0.f)
, mCrosshairs(gpSimplePool->GetObj("CMDL_Crosshairs"))
, mSeeker(gpSimplePool->GetObj("CMDL_Seeker"))
, mTargetFlower(gpSimplePool->GetObj("CMDL_TargetFlower"))
, mMissileBracket(gpSimplePool->GetObj("CMDL_MissileBracket"))
, mInnerBeamIcon(gpSimplePool->GetObj("CMDL_InnerBeamIcon"))
, mLockConfirm(gpSimplePool->GetObj("CMDL_LockConfirm"))
, mLockFire(gpSimplePool->GetObj("CMDL_LockFire"))
, mLockDagger(gpSimplePool->GetObj("CMDL_LockDagger0"))
, mGrapple(gpSimplePool->GetObj("CMDL_Grapple"))
, mChargeTickFirst(gpSimplePool->GetObj("CMDL_ChargeTickFirst"))
, mScanTargetCenter(gpSimplePool->GetObj("CMDL_ScanTargetCenter"))
, mScanTargetLeft(gpSimplePool->GetObj("CMDL_ScanTargetLeft"))
, mScanTargetRight(gpSimplePool->GetObj("CMDL_ScanTargetRight"))
, mChargeGauge("CMDL_ChargeGauge")
, mQuarterCurve(gpSimplePool->GetObj("TXTR_QuaterCurve"))
, mSeekerMissileLockConfirm(gpSimplePool->GetObj("CMDL_SeekerMissileLockConfirm"))
, mSeekerMissileCrosshair(gpSimplePool->GetObj("CMDL_SeekerMissileCrosshair"))
, mRadarPaintFirst(gpSimplePool->GetObj("TXTR_RadarPaint"))
, mRadarPaintSecond(gpSimplePool->GetObj("TXTR_RadarPaint"))
, mTargetId(kInvalidUniqueId)
, mNextTargetId(kInvalidUniqueId)
, mTargetPosition(CVector3f::Zero())
, mLaggingTargetPosition(CVector3f::Zero())
, mCurrentGroupInterpolated(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f, 1.f, true)
, mCurrentGroupA(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f, 1.f, true)
, mCurrentGroupB(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f, 1.f, true)
, mCurrentGroupDuration(0.f)
, mCurrentGroupTimer(0.f)
, mNextGroupInterpolated(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f, 1.f, true)
, mNextGroupA(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f, 1.f, true)
, mNextGroupB(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f, 1.f, true)
, mNextGroupDuration(0.f)
, mNextGroupTimer(0.f)
, mGrapplePointA(kInvalidUniqueId)
, mGrapplePointB(kInvalidUniqueId)
, mGrapplePointFactorA(0.f)
, mGrapplePointFactorB(0.f)
, mVulnerabilityTarget(kInvalidUniqueId)
, mTargetVulnerability(CDamageVulnerability::ImmuneVulnerabilty())
, mCrosshairsScale(0.f)
, mSeekerAngle(0.f)
, mCrosshairsDrawScale(0.f)
, x274(0.f)
, x278(0.f)
, mMissileActive(false)
, mMissileBracketTimer(0.f)
, mMissileBracketScaleTimer(0.f)
, mBeam(CPlayerState::kBI_Power)
, mChargeGaugeOvershootTimer(0.f)
, mLockOnTimer(0.f)
, x294(0.f)
, mLockFireTimer(0.f)
, mFullChargeFadeTimer(0.f)
, mScanBracketFactor(0.f)
, mScanTargetFactor(0.f)
, mBeamShot(false)
, mMissileShot(false)
, mFullyCharged(false) {
  mOuterBeamIconSquares.reserve(9);
  for (int i = 0; i < 9; ++i) {
    char name[64];
    sprintf(name, "CMDL_BeamSquare%d", i);
    mOuterBeamIconSquares.push_back(SOuterItemInfo(name));
  }
  mCrosshairs.Lock();
  mQuarterCurve.Lock();
  mSeeker.Lock();
  mGrapple.Lock();
  mSeekerMissileLockConfirm.Lock();
  mSeekerMissileCrosshair.Lock();
  mRadarPaintFirst.Lock();
  mRadarPaintSecond.Lock();
  // TODO: initial camera orientations, targeting-tweak overshoot and orbit-zone positions.
}

bool CCompoundTargetReticle::CheckLoadComplete() { return true; }

EReticleState CCompoundTargetReticle::GetDesiredReticleState(const CStateManager& mgr) const {
  switch (mgr.GetPlayerState(mPlayerIndex)->GetCurrentVisor()) {
  case CPlayerState::kPV_Scan:
    return kRS_Scan;
  case CPlayerState::kPV_Dark:
    return kRS_Dark;
  case CPlayerState::kPV_Echo:
    return kRS_Echo;
  default:
    return kRS_Combat;
  }
}

void CCompoundTargetReticle::Update(float dt, const CStateManager& mgr) {
  // TODO: orientation lag, visor state transitions, token locks and group updates.
}

void CCompoundTargetReticle::UpdateCurrLockOnGroup(float dt, const CStateManager& mgr) {
  // TODO: lock-on transitions, vulnerability, beam/missile and charge state.
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr) {
  // TODO: next-target transition and interpolation.
}

void CCompoundTargetReticle::UpdateOrbitZoneGroup(float dt, const CStateManager& mgr) {
  // TODO: crosshair/seeker factors and angles.
}

void CCompoundTargetReticle::Draw(const CStateManager& mgr, bool hideLockOn) const {
  // TODO: choose and render visor-specific reticle groups.
}

void CCompoundTargetReticle::DrawGrappleGroup(const CMatrix3f& rotation, const CStateManager& mgr,
                                              bool hideLockOn) const {
  // TODO: interpolate and draw grapple candidates.
}

void CCompoundTargetReticle::DrawGrapplePoint(const CScriptGrapplePoint& point, float factor,
                                              const CStateManager& mgr, const CMatrix3f& rotation,
                                              bool zEqual) const {
  // TODO: grapple model position, scale, color and depth mode.
}

void CCompoundTargetReticle::DrawCurrLockOnGroup(const CMatrix3f& rotation,
                                                 const CStateManager& mgr) const {
  // TODO: current-target, charge/missile, seeker-missile and radar-paint rendering.
}

void CCompoundTargetReticle::DrawSeeker(const CMatrix3f& rotation, const CStateManager& mgr) const {
  // TODO: scan/next-target seeker rendering.
}

void CCompoundTargetReticle::DrawCrosshairs(const CMatrix3f& rotation) const {
  // TODO: crosshair scale, alpha and model rendering.
}

void CCompoundTargetReticle::DrawScanTargetGroup(const CMatrix3f& rotation,
                                                 const CStateManager& mgr) const {
  // TODO: scan target center and left/right completion brackets.
}

void CCompoundTargetReticle::DrawNextLockOnGroup(const CMatrix3f& rotation,
                                                 const CStateManager& mgr) const {
  // TODO: next-target lock-on rendering.
}

void CCompoundTargetReticle::DrawOrbitZoneGroup(const CMatrix3f& rotation,
                                                const CStateManager& mgr) const {
  // TODO: orbit-zone group rendering.
}

void CCompoundTargetReticle::UpdateTargetParameters(CTargetReticleRenderState& state,
                                                    const CStateManager& mgr) {
  // TODO: resolve target and update its world position, radius and viewport clamp.
}

float CCompoundTargetReticle::CalculateRadiusWorld(const CActor& actor,
                                                   const CStateManager& mgr) const {
  // TODO: derive reticle radius from the actor's bounds.
  return 1.f;
}

CVector3f CCompoundTargetReticle::CalculatePositionWorld(const CActor& actor,
                                                         const CStateManager& mgr) const {
  // TODO: select aim/orbit position according to actor and reticle state.
  return actor.GetOrbitPosition(mgr);
}

CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr,
                                                                    bool lag) const {
  // TODO: selected player's orbit-zone projection.
  return CVector3f::Zero();
}

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId id, const CStateManager& mgr) {
  // TODO: recover the grapple-point cast interface.
  return false;
}

float CCompoundTargetReticle::CalculateClampedScale(CVector3f position, float scale, float clampMin,
                                                    float clampMax, const CStateManager& mgr,
                                                    int playerIndex) {
  // TODO: player viewport scaling and perspective-size clamp.
  return scale;
}

CTargetReticleRenderState::CTargetReticleRenderState(TUniqueId target, float radius,
                                                     CVector3f position, float factor,
                                                     float minimumViewportScale,
                                                     bool orbitZoneIdlePosition)
: mTarget(target)
, mRadius(radius)
, mPosition(position)
, mFactor(factor)
, mMinimumViewportScale(minimumViewportScale)
, mOrbitZoneIdlePosition(orbitZoneIdlePosition) {}

void CTargetReticleRenderState::InterpolateWithClamp(const CTargetReticleRenderState& a,
                                                     CTargetReticleRenderState& out,
                                                     const CTargetReticleRenderState& b, float t) {
  t = CMath::Clamp(0.f, t, 1.f);
  out.mRadius = (1.f - t) * a.mRadius + t * b.mRadius;
  out.mFactor = (1.f - t) * a.mFactor + t * b.mFactor;
  out.mMinimumViewportScale = (1.f - t) * a.mMinimumViewportScale + t * b.mMinimumViewportScale;
  out.mPosition = (1.f - t) * a.mPosition + t * b.mPosition;
  out.mTarget = t == 1.f ? b.mTarget : t == 0.f ? a.mTarget : kInvalidUniqueId;
}

CTargetingManager::CTargetingManager(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex), mTargetReticle(mgr, playerIndex), mOrbitPointMarker(playerIndex) {}

bool CTargetingManager::CheckLoadComplete() {
  return mTargetReticle.CheckLoadComplete() && mOrbitPointMarker.CheckLoadComplete();
}

void CTargetingManager::Update(float dt, const CStateManager& mgr) {
  mTargetReticle.Update(dt, mgr);
  mOrbitPointMarker.Update(dt, mgr);
}

void CTargetingManager::Draw(const CStateManager& mgr, bool hideLockOn) const {
  // TODO: establish ambient lighting, view and perspective before drawing.
  mOrbitPointMarker.Draw(mgr);
  mTargetReticle.Draw(mgr, hideLockOn);
}

void CCompoundTargetReticle::Touch() const {
  // TODO: touch the loaded model set and the nine outer beam squares.
}

void CTargetingManager::Touch() const { mTargetReticle.Touch(); }

COrbitPointMarker::COrbitPointMarker(int playerIndex)
: mPlayerIndex(playerIndex)
, mZOffset(0.f)
, mCameraRelativeZ(true)
, mLagAzimuth(0.f)
, mAzimuth(0.f)
, mLagTargetPosition(CVector3f::Zero())
, mLastFreeOrbit(false)
, mInterpolationTimer(0.f)
, mCurrentTime(0.f)
, mOrbitPointModel(gpSimplePool->GetObj("CMDL_OrbitPoint")) {
  mOrbitPointModel.Lock();
  // TODO: obtain mZOffset from the Echoes targeting tweak.
}

bool COrbitPointMarker::CheckLoadComplete() { return mOrbitPointModel.IsLoaded(); }

void COrbitPointMarker::Update(float dt, const CStateManager& mgr) {
  // TODO: free-orbit state, target lag, azimuth and interpolation.
}

void COrbitPointMarker::Draw(const CStateManager& mgr) const {
  // TODO: orbit-marker camera-relative projection and rendering.
}

void COrbitPointMarker::ResetInterpolationTimer(float time) { mInterpolationTimer = time; }
