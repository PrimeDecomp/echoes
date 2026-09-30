#include "MetroidPrime/Cameras/CPathCamera.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/math.hpp"

CPathCamera::CPathCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index,
                         int controllerIdx)
: CGameCamera(uid, rstl::string("Path Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, active), xf,
              CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, index, controllerIdx)
, mScriptCameraId(kInvalidUniqueId)
, mPositionDistance(0.f)
, mLookAtDistance(0.f)
, mPlayerDistance(0.f)
, mSpeed(0.f) {}

CPathCamera::~CPathCamera() {}

const CScriptPathCamera* CPathCamera::GetScriptCamera(const CStateManager& mgr) const {
  return TCastToConstPtr< CScriptPathCamera >(mgr.GetObjectById(mScriptCameraId));
}

void CPathCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  // TODO: Select the initial side using the current camera and collision-clamped path;
  // the separate player-spline mode maps normalized progress through both control splines.
  if (const CScriptPathCamera* camera = GetScriptCamera(mgr)) {
    mSpeed = camera->GetSpeed();
  }
}

float CPathCamera::CalculateLookAtDistance(const CStateManager& mgr) const {
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  if (!camera) {
    return 0.f;
  }

  CScriptCameraSpline& spline = camera->GetSpline();
  if (close_enough(spline.GetLength(), 0.f) ||
      spline.GetLookAtSpline().GetControlPointCount() == 0) {
    return 0.f;
  }

  const float progress = CMath::Clamp(0.f, mPlayerDistance / spline.GetLength(), 1.f);
  return spline.LookAtTimeSpline().EvaluateAt(progress) * spline.GetLookAtSpline().GetLength();
}

float CPathCamera::CalculatePositionDistance(float dt, const CStateManager& mgr) const {
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  if (!camera) {
    return 0.f;
  }

  CScriptCameraSpline& spline = camera->GetSpline();
  const float length = spline.GetLength();
  if (close_enough(length, 0.f)) {
    return 0.f;
  }

  float extent = camera->GetDistance();
  if (camera->GetFlags() & 4) {
    const CVector3f pathPosition = spline.GetPositionByLength(mPlayerDistance, GetTransform(), mgr);
    CVector3f toPlayer = GetPlayer(mgr).GetBallPosition() - pathPosition;
    toPlayer.SetZ(0.f);
    const float distance = toPlayer.IsMagnitudeSafe() ? toPlayer.Magnitude() : 0.f;
    const float control = camera->GetPerpendicularDistanceControlSpline().EvaluateAt(distance);
    extent *= 1.f - CMath::Clamp(0.f, control, 1.f);
  }

  float newDistance;
  const bool closedLoop = spline.GetPositionSpline().IsClosedLoop();
  if (closedLoop) {
    const float positive = spline.ValidateLength(mPlayerDistance + extent);
    const float negative = spline.ValidateLength(mPlayerDistance - extent);
    const float distance = CMath::AbsF(mPositionDistance - mPlayerDistance);
    if (mPositionDistance > mPlayerDistance) {
      newDistance = distance <= length - distance ? positive : negative;
    } else {
      newDistance = distance <= length - distance ? negative : positive;
    }
  } else {
    newDistance = spline.ValidateLength(
        mPositionDistance > mPlayerDistance ? mPlayerDistance + extent : mPlayerDistance - extent);
  }

  if (camera->GetFlags() & 1) {
    return newDistance;
  }

  float step;
  if (closedLoop) {
    const float distance = CMath::AbsF(newDistance - mPositionDistance);
    const float nearest = rstl::min_val(distance, length - distance);
    step = CMath::Limit(nearest / camera->GetDampenDistance(), 1.f) * (mSpeed * dt);
    if (mPositionDistance > newDistance) {
      if (distance <= length - distance) {
        step = -step;
      }
    } else if (distance > length - distance) {
      step = -step;
    }
  } else {
    step = CMath::Limit((newDistance - mPositionDistance) / camera->GetDampenDistance(), 1.f) *
           (mSpeed * dt);
  }
  return spline.ValidateLength(mPositionDistance + step);
}

CVector3f CPathCamera::MoveAlongSpline(float dt, const CStateManager& mgr) {
  const CVector3f playerPosition = GetPlayer(mgr).GetBallPosition();
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  if (!camera) {
    return GetTranslation();
  }

  CScriptCameraSpline& spline = camera->GetSpline();
  const CMotionSpline& playerSpline = camera->GetPlayerSpline();
  CMayaSpline& speedControl = camera->GetSpeedControlSpline();
  if (speedControl.GetKnotCount() != 0) {
    float progress = 0.f;
    if (playerSpline.GetControlPointCount() == 0) {
      if (spline.GetPositionSpline().GetControlPointCount() != 0) {
        progress =
            CMath::Clamp(0.f, mPositionDistance / spline.GetPositionSpline().GetLength(), 1.f);
      }
      if (spline.GetLookAtSpline().GetControlPointCount() != 0) {
        progress = CMath::Clamp(0.f, mLookAtDistance / spline.GetLookAtSpline().GetLength(), 1.f);
      }
    } else {
      progress = CMath::Clamp(0.f, mPlayerDistance / playerSpline.GetLength(), 1.f);
    }
    mSpeed = speedControl.EvaluateAt(progress) * camera->GetSpeed();
  }

  if (playerSpline.GetControlPointCount() == 0 || speedControl.GetKnotCount() != 0) {
    mPlayerDistance = spline.FindClosestLengthOnSpline(mPlayerDistance, playerPosition);
    mPositionDistance = CalculatePositionDistance(dt, mgr);
    mLookAtDistance = CalculateLookAtDistance(mgr);
  } else {
    mPlayerDistance = playerSpline.FindClosestLengthOnSpline(mPlayerDistance, playerPosition);
    mPlayerDistance = playerSpline.ValidateLength(mPlayerDistance);
    const float progress = CMath::Clamp(0.f, mPlayerDistance / playerSpline.GetLength(), 1.f);
    if (spline.GetPositionSpline().GetControlPointCount() != 0) {
      mPositionDistance =
          spline.PositionTimeSpline().EvaluateAt(progress) * spline.GetPositionSpline().GetLength();
    }
    if (spline.GetLookAtSpline().GetControlPointCount() != 0) {
      mLookAtDistance =
          spline.LookAtTimeSpline().EvaluateAt(progress) * spline.GetLookAtSpline().GetLength();
    }
  }
  return spline.GetPositionByLength(mPositionDistance, GetTransform(), mgr);
}

CTransform4f CPathCamera::AvoidDoorCollisions(const CTransform4f& xf, const CStateManager& mgr) {
  // TODO: Recover the door's open flag, test ball-camera door proximity, then flip the
  // position distance to the opposite side of the player and resample the position spline.
  return xf;
}

void CPathCamera::Think(float dt, CStateManager& mgr) {
  // TODO: Apply movement, look target, door avoidance and orientation damping; update the
  // linked time keyframe only for the selected camera, then validate the transform and Think.
}

void CPathCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CPathCamera::Render(const CStateManager& mgr) const {}

void CPathCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
}

CVector3f CPathCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  // TODO: Apply the path's look-at/perpendicular modes and camera-hint height overrides.
  return GetCameraManager(mgr).GetBallCamera()->GetScanObjectIndicatorPosition(mgr);
}

void CPathCamera::UpdateOrientation(float dt, const CTransform4f& xf, const CStateManager& mgr) {
  // TODO: Resolve the active camera hint, clamp angular speed near vertical directions,
  // and rotate towards xf with the shared quaternion look-at helper.
}

void CPathCamera::UpdateFov(const CStateManager& mgr) {
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  const CMotionSpline& playerSpline = camera->GetPlayerSpline();
  if (playerSpline.GetControlPointCount() == 0) {
    SetTargetFov(camera->GetSpline().GetFovByLength(mPlayerDistance));
  } else {
    const float time = playerSpline.GetDuration() * (mPlayerDistance / playerSpline.GetLength());
    SetTargetFov(camera->GetSpline().GetFovByTime(time));
  }
}
