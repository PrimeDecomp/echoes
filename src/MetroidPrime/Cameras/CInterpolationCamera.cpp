#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

namespace {
// Guessed names.
const CMaterialList skCollisionIncludeList =
    CMaterialList(kMT_Solid, kMT_Wall, kMT_Floor, kMT_Ceiling);
const CMaterialList skCollisionExcludeList =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
const CMaterialFilter skCollisionFilter =
    CMaterialFilter::MakeIncludeExclude(skCollisionIncludeList, skCollisionExcludeList);
} // namespace

CInterpolationCamera::CInterpolationCamera(TUniqueId uid, const CTransform4f& xf, int index,
                                           int controllerIdx)
: CGameCamera(uid, rstl::string_l("Interpolation Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, false), xf,
              CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, index, controllerIdx)
, mTargetId(kInvalidUniqueId)
, mTime(0.f)
, mDuration(0.f)
, mStartTransform(CTransform4f::Identity())
, mLookPosition(CVector3f::Zero())
, mInitialDistance(0.f)
, mInitialAngle(0.f)
, mAngularSpeed(M_PIF)
, mPositionMode(kPM_Direct)
, mRotationMode(kRM_LinearSlerp)
, mSpline(false, 1.f, CMotionSpline::kST_Bezier)
, x2a0_(0.f)
, mInterpolateRotation(false)
, x2a4_1_(false)
, mRotationFinished(false) {}

CInterpolationCamera::~CInterpolationCamera() {}

void CInterpolationCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {}

CTransform4f CInterpolationCamera::CalculateOrientation(float dt, const CVector3f& position,
                                                        bool& done, const CStateManager& mgr) {
  done = false;
  CTransform4f xf = GetTransform();
  if (mInterpolateRotation) {
    const CGameCamera* target = TCastToConstPtr< CGameCamera >(mgr.GetObjectById(mTargetId));
    if (!target) {
      return xf;
    }

    float remaining;
    switch (mRotationMode) {
    case kRM_Linear:
    case kRM_LinearSlerp:
      remaining = CMath::Clamp(0.f, 1.f - mTime / mDuration, 1.f);
      break;
    case kRM_Sine:
      remaining = CMath::Clamp(0.f, 1.f - sinf((M_PIF * 0.5f * mTime) / mDuration), 1.f);
      break;
    case kRM_SinusoidalEase:
      remaining = 1.f - CMath::EaseInOut(CMath::Limit(mTime / mDuration, 1.f),
                                         CMath::kET_Sinusoidal, 0.25f, 0.75f, 0.f, 1.f, 2.f);
      break;
    case kRM_SinusoidalEaseFull:
      remaining = 1.f - CMath::EaseInOut(CMath::Limit(mTime / mDuration, 1.f),
                                         CMath::kET_Sinusoidal, 0.f, 1.f, 0.f, 1.f, 2.f);
      break;
    case kRM_QuadraticEase:
      remaining = 1.f - CMath::EaseInOut(CMath::Limit(mTime / mDuration, 1.f), CMath::kET_Quadratic,
                                         0.25f, 0.75f, 0.f, 1.f, 2.f);
      break;
    case kRM_QuadraticEaseFull:
      remaining = 1.f - CMath::EaseInOut(CMath::Limit(mTime / mDuration, 1.f), CMath::kET_Quadratic,
                                         0.f, 1.f, 0.f, 1.f, 2.f);
      break;
    default:
      remaining = 1.f - CMath::EaseInOut(CMath::Limit(mTime / mDuration, 1.f),
                                         CMath::kET_Sinusoidal, 0.4f, 0.6f, 0.f, 1.f, 2.f);
      break;
    }

    CVector3f direction = target->GetTransform().GetForward();
    if (direction.IsMagnitudeSafe()) {
      direction.Normalize();
    } else {
      direction = GetTransform().GetForward();
    }
    if (direction.DropZ().IsMagnitudeSafe()) {
      const float projection =
          CMath::Limit(CVector3f::Dot(GetTransform().GetForward(), direction), 1.f);
      xf = CTransform4f::LookAt(position, position + direction);
      if (projection >= 0.999999f || mRotationFinished) {
        mRotationFinished = true;
      } else {
        const CRelAngle angle = CRelAngle::FromRadians(mInitialAngle * remaining);
        CVector3f rotated;
        if (mRotationMode == kRM_LinearSlerp) {
          rotated = CVector3f::Slerp(direction, mStartTransform.GetForward(), angle);
        } else {
          const CQuaternion rotation =
              CQuaternion::LookAt(direction, mStartTransform.GetForward(), angle);
          rotated = rotation.Transform(direction);
        }
        xf = CTransform4f::LookAt(position, position + rotated);
      }
    } else {
      xf.SetTranslation(position);
    }
    if (mTime >= mDuration) {
      done = true;
    }
  } else {
    CVector3f direction = mLookPosition - position;
    if (direction.IsMagnitudeSafe()) {
      direction.Normalize();
    } else {
      direction = GetTransform().GetForward();
    }
    if (direction.DropZ().IsMagnitudeSafe()) {
      const float projection =
          CMath::Limit(CVector3f::Dot(GetTransform().GetForward(), direction), 1.f);
      const float angle = acosf(projection);
      float speedScale = 1.f;
      const float slowdownAngle = CRelAngle::FromDegrees(15.f).AsRadians();
      if (angle < slowdownAngle) {
        const float progress = CMath::Clamp(0.f, angle / slowdownAngle, 1.f);
        speedScale = CMath::Limit(0.001f + sinf(M_PIF * 0.5f * progress), 1.f);
      }
      const float timeScale = CMath::Limit(mTime / (0.2f * mDuration), 1.f);
      if (projection >= 0.999999f || mRotationFinished) {
        mRotationFinished = true;
        done = true;
        xf = CTransform4f::LookAt(position, position + direction);
      } else {
        const CRelAngle step =
            CRelAngle::FromRadians(dt * (mAngularSpeed * speedScale) * timeScale);
        const CQuaternion rotation =
            CQuaternion::LookAt(GetTransform().GetForward(), direction, step);
        xf = CTransform4f::LookAt(position,
                                  position + rotation.Transform(GetTransform().GetForward()));
      }
    } else {
      xf.SetTranslation(position);
    }
  }
  return xf;
}

bool CInterpolationCamera::InterpolatePosition(float dt, CTransform4f& xf, const CVector3f& target,
                                               const CStateManager& mgr) {
  float time = mTime;
  if (time > mDuration) {
    time = mDuration;
  }
  const float progress = CMath::Limit(time / mDuration, 1.f);
  const float remaining =
      1.f - CMath::EaseInOut(progress, CMath::kET_Sinusoidal, 0.1f, 0.9f, 0.f, 1.f, 2.f);
  CVector3f delta = GetTranslation() - target;
  const float distance = delta.Magnitude();
  const float limit = mInitialDistance * remaining;
  if (distance > limit && delta.CanBeNormalized()) {
    delta = delta.AsNormalized() * limit;
  }
  const CVector3f position = target + delta;
  bool done = false;
  xf = CalculateOrientation(dt, position, done, mgr);
  return time >= mDuration && done;
}

bool CInterpolationCamera::InterpolateSpline(float dt, CTransform4f& xf, const CVector3f& target,
                                             const CStateManager& mgr) {
  if (mSpline.GetControlPointCount() == 0) {
    return InterpolatePosition(dt, xf, target, mgr);
  }
  mSpline.SetKnotAndControlPoint(mSpline.GetKnotCount() - 1, target, true);
  const CVector3f position = mSpline.GetPositionByTime(mTime);
  bool done = false;
  xf = CalculateOrientation(dt, position, done, mgr);
  return mTime >= mDuration;
}

void CInterpolationCamera::SetInterpolation(const CTransform4f& xf, TUniqueId from, TUniqueId to,
                                            bool interpolateRotation, EPositionMode positionMode,
                                            ERotationMode rotationMode, CStateManager& mgr,
                                            bool flag, float duration, float fov) {
  SetActive(true);
  SetTransform(xf);
  mStartTransform = xf;
  mTargetId = to;
  mInterpolateRotation = interpolateRotation;
  x2a4_1_ = flag;
  mDuration = duration;
  mPositionMode = positionMode;
  mRotationMode = rotationMode;
  x2a0_ = 0.f;
  mRotationFinished = false;
  mTime = 0.f;

  CGameCamera* source =
      const_cast< CGameCamera* >(TCastToConstPtr< CGameCamera >(mgr.GetObjectById(from)));
  CGameCamera* target =
      const_cast< CGameCamera* >(TCastToConstPtr< CGameCamera >(mgr.GetObjectById(to)));
  SetTransform(mStartTransform);
  if (target) {
    mAngularSpeed = M_PIF;
    mLookPosition = target->GetScanObjectIndicatorPosition(mgr);
    mInitialDistance = CVector3f(target->GetTranslation() - xf.GetTranslation()).Magnitude();
    if (source) {
      const_cast< CCameraManager& >(GetCameraManager(mgr)).TransferCameraState(*source, *this, mgr);
      SetTransform(xf);
      SetFov(source->GetFov());
      InterpolateFOV(source->GetFov(), duration, 0.f, to, mgr);
      mInitialAngle = acosf(
          CMath::Limit(CVector3f::Dot(xf.GetForward(), target->GetTransform().GetForward()), 1.f));
    } else {
      SetFovAndTarget(target->GetFov());
    }
  } else {
    if (source) {
      SetFov(source->GetFov());
    }
    InterpolateFOV(fov, duration, 0.f);
  }
}

void CInterpolationCamera::EndInterpolation(EEndReason reason, CStateManager& mgr) {
  SetActive(false);
  CCameraManager& cameraManager = const_cast< CCameraManager& >(GetCameraManager(mgr));
  CGameCamera* target = TCastToPtr< CGameCamera >(mgr.ObjectById(mTargetId));
  if (!target) {
    return;
  }
  if (target->GetActive()) {
    if (reason == kER_Completed) {
      cameraManager.TransferCameraState(*this, *target, mgr);
    }
    cameraManager.SetCurrentCameraId(mTargetId, mgr);
  } else {
    switch (Player(mgr).GetMorphballTransitionState()) {
    case CPlayer::kMS_Unmorphed:
    case CPlayer::kMS_Unmorphing:
      if (reason == kER_Completed) {
        cameraManager.TransferCameraState(*this, *cameraManager.FirstPersonCamera(), mgr);
      }
      cameraManager.SetCurrentCameraId(cameraManager.FirstPersonCamera()->GetUniqueId(), mgr);
      break;
    default: {
      const CBallCamera* camera = cameraManager.GetBallCamera();
      cameraManager.SetupInterpolation(GetTransform(), GetUniqueId(), camera->GetUniqueId(), false,
                                       kPM_Direct, kRM_LinearSlerp, mgr, true, 1.f,
                                       camera->GetFov());
      break;
    }
    }
  }
}

void CInterpolationCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CTransform4f oldXf = GetTransform();
  mTime += dt;
  if (mTime > mDuration) {
    mTime = mDuration;
  }
  CTransform4f xf = GetTransform();
  const CGameCamera* target = TCastToConstPtr< CGameCamera >(mgr.GetObjectById(mTargetId));
  if (!target || !target->GetActive()) {
    EndInterpolation(kER_TargetUnavailable, mgr);
    return;
  }

  const CVector3f position = target->GetTransform().GetTranslation();
  mLookPosition = target->GetScanObjectIndicatorPosition(mgr);
  bool done;
  switch (mPositionMode) {
  case kPM_Direct:
    done = InterpolatePosition(dt, xf, position, mgr);
    break;
  case kPM_Spline:
    done = InterpolateSpline(dt, xf, position, mgr);
    break;
  default:
    done = true;
    break;
  }
  xf = ValidateCameraTransform(xf, oldXf, dt);
  SetTransform(xf);
  if (done) {
    EndInterpolation(kER_Completed, mgr);
  } else if (mPositionMode == kPM_Direct ||
             target->GetUniqueId() == GetCameraManager(mgr).GetBallCamera()->GetUniqueId()) {
    if (CVector3f(target->GetTranslation() - xf.GetTranslation()).Magnitude() > 3.f) {
      CVector3f direction = xf.GetTranslation() - oldXf.GetTranslation();
      if (direction.CanBeNormalized()) {
        direction = direction.AsNormalized();
      } else {
        direction = xf.GetForward();
      }
      const CRayCastResult result =
          mgr.RayStaticIntersection(GetTranslation(), direction, 3.f, skCollisionFilter);
      if (result.IsValid()) {
        EndInterpolation(kER_Obstruction, mgr);
        const_cast< CCameraManager& >(GetCameraManager(mgr)).StartScreenFlash();
      }
    }
  }
  CActor::Think(dt, mgr);
}

void CInterpolationCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CInterpolationCamera::Render(const CStateManager& mgr) const {}

void CInterpolationCamera::SetSpline(const CMotionSpline& spline) {
  mSpline = spline;
  mSpline.CalculateLength();
}

CVector3f CInterpolationCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  return mLookPosition;
}
