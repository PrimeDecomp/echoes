#include "MetroidPrime/Cameras/CSpindleCamera.hpp"

#include "MetroidPrime/CCameraManager.hpp"

CSpindleCameraInterpolant::CSpindleCameraInterpolant(ESpindleInput input, const CMayaSpline& spline)
: mInput(input), mSpline(spline) {}

float CSpindleCameraInterpolant::InterpolateValue(float input) { return mSpline.EvaluateAt(input); }

CSpindleCameraParameters::CSpindleCameraParameters(
    uint flags, const CSpindleCameraInterpolant& angularSpeed,
    const CSpindleCameraInterpolant& linearSpeed, const CSpindleCameraInterpolant& motionRadius,
    const CSpindleCameraInterpolant& radialOffset,
    const CSpindleCameraInterpolant& desiredAngularOffset,
    const CSpindleCameraInterpolant& minAngularOffset,
    const CSpindleCameraInterpolant& maxAngularOffset,
    const CSpindleCameraInterpolant& lookAtAngularOffset,
    const CSpindleCameraInterpolant& lookAtZOffset, const CSpindleCameraInterpolant& zOffset,
    const CSpindleCameraInterpolant& angularConstraint,
    const CSpindleCameraInterpolant& angularDampening,
    const CSpindleCameraInterpolant& desiredAngularSpeed,
    const CSpindleCameraInterpolant& deactivateRadius,
    const CSpindleCameraInterpolant& constraintFlipAngle, const CSpindleCameraInterpolant& fov)
: mFlags(flags)
, mAngularSpeed(angularSpeed)
, mLinearSpeed(linearSpeed)
, mMotionRadius(motionRadius)
, mRadialOffset(radialOffset)
, mDesiredAngularOffset(desiredAngularOffset)
, mMinAngularOffset(minAngularOffset)
, mMaxAngularOffset(maxAngularOffset)
, mLookAtAngularOffset(lookAtAngularOffset)
, mLookAtZOffset(lookAtZOffset)
, mZOffset(zOffset)
, mAngularConstraint(angularConstraint)
, mAngularDampening(angularDampening)
, mDesiredAngularSpeed(desiredAngularSpeed)
, mDeactivateRadius(deactivateRadius)
, mConstraintFlipAngle(constraintFlipAngle)
, mFov(fov) {}

CSpindleCameraParameters::~CSpindleCameraParameters() {}

CSpindleCamera::CSpindleCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index,
                               int controllerIdx)
: CGameCamera(uid, rstl::string("Spindle Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, active), xf,
              CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, index, controllerIdx)
, mSpindleCameraId(kInvalidUniqueId)
, mInVars()
, mMaxAzimuthInterpTimer(0.f)
, mLookDir(xf.GetForward())
, mTargetSplineDistance(0.f)
, mPlayerSplineDistance(0.f)
, mLookPosition(CVector3f::Zero())
, mOutsideClampedAzimuth(false)
, mInResetThink(false)
, mFixedPositionInitialized(false) {}

CSpindleCamera::~CSpindleCamera() {}

void CSpindleCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  // TODO: Resolve the active camera hint, refresh the ball camera's look position, and run reset
  // Think.
}

float CSpindleCamera::CalculateTargetSplineDistance(CStateManager& mgr) const {
  // TODO: Project the player onto the target spline, or map progress through the control spline.
  return 0.f;
}

float CSpindleCamera::GetInVar(const CSpindleCameraInterpolant& interpolant) const {
  return mInVars[interpolant.GetInput()];
}

float CSpindleCamera::GetInterpolant(CSpindleCameraInterpolant& interpolant) const {
  return interpolant.InterpolateValue(GetInVar(interpolant));
}

void CSpindleCamera::Think(float dt, CStateManager& mgr) {
  // TODO: Populate the eight spline inputs, apply radial/angular constraints and update the camera.
}

void CSpindleCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CSpindleCamera::Render(const CStateManager& mgr) const {}

CVector3f CSpindleCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  // TODO: Recover the ball-camera look target, spline projection and angular/vertical offsets.
  return mLookPosition;
}
