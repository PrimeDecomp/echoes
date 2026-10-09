#include "MetroidPrime/Cameras/CPathCamera.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimeKeyframe.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "rstl/math.hpp"

#include <math.h>

// Guessed names.
static const CMaterialList kPathLineOfSightIncludeList = CMaterialList(kMT_Solid);
static const CMaterialList kPathLineOfSightExcludeList = CMaterialList(kMT_ProjectilePassthrough);
static const CMaterialFilter kPathLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kPathLineOfSightIncludeList, kPathLineOfSightExcludeList);

CPathCamera::CPathCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index,
                         int controllerIdx)
: CGameCamera(uid, rstl::string_l("Path Camera"),
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
  CPlayer& player = Player(mgr);
  CVector3f playerPosition = player.GetTranslation();
  playerPosition[kDZ] += player.GetTweakPlayer()->GetBallRadius();
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  if (!camera) {
    return;
  }

  CScriptCameraSpline& spline = camera->GetSpline();
  mSpeed = camera->GetSpeed();
  if (camera->GetPlayerSpline().GetControlPointCount() != 0) {
    const CMotionSpline& playerSpline = camera->GetPlayerSpline();
    mPlayerDistance = playerSpline.FindClosestLengthOnSpline(mPlayerDistance, playerPosition);
    const float progress = CMath::Clamp(0.f, mPlayerDistance / playerSpline.GetLength(), 1.f);
    if (spline.GetPositionSpline().GetControlPointCount() != 0) {
      mPositionDistance =
          spline.PositionTimeSpline().EvaluateAt(progress) * spline.GetPositionSpline().GetLength();
    } else {
      mPositionDistance = 0.f;
    }
    if (spline.GetLookAtSpline().GetControlPointCount() != 0) {
      mLookAtDistance =
          spline.LookAtTimeSpline().EvaluateAt(progress) * spline.GetLookAtSpline().GetLength();
    } else {
      mLookAtDistance = 0.f;
    }
    const CVector3f position = spline.GetPositionByLength(mPositionDistance, GetTransform(), mgr);
    SetTranslation(position);
    const CTransform4f cameraXf =
        CTransform4f::LookAt(position, GetScanObjectIndicatorPosition(mgr));
    SetTransform(cameraXf);
    Think(0.02f, mgr);
    UpdateFov(mgr);
  } else {
    mPlayerDistance = spline.FindClosestLengthOnSpline(mPlayerDistance, playerPosition);
    const float playerDistance = mPlayerDistance;
    const float negativeDistance = rstl::max_val(0.f, playerDistance - camera->GetDistance());
    const CVector3f negative = spline.GetPositionByLength(negativeDistance, GetTransform(), mgr);
    const float positiveDistance =
        rstl::min_val(playerDistance + camera->GetDistance(), camera->GetSpline().GetLength());
    const CVector3f positive = spline.GetPositionByLength(positiveDistance, GetTransform(), mgr);

    const CTransform4f currentXf = CameraManager(mgr).GetCurrentCamera(mgr, false)->GetTransform();
    const CVector3f currentPosition = currentXf.GetTranslation();
    bool useNegative = false;
    if (camera->GetInitialPosition() == kISP_BallCamBasis) {
      const CVector3f toPlayer = playerPosition - negative;
      if (toPlayer.IsMagnitudeSafe() &&
          CVector3f::Dot(currentXf.GetForward(), toPlayer.AsNormalized()) > 0.f) {
        useNegative = true;
      }
    } else {
      useNegative = camera->GetInitialPosition() == kISP_Negative;
    }

    const CVector3f toNegative = negative - currentPosition;
    const CRayCastResult negativeHit = mgr.RayStaticIntersection(
        currentPosition, toNegative.AsNormalized(), toNegative.Magnitude(), kPathLineOfSightFilter);
    const CVector3f toPositive = positive - currentPosition;
    const CRayCastResult positiveHit = mgr.RayStaticIntersection(
        currentPosition, toPositive.AsNormalized(), toPositive.Magnitude(), kPathLineOfSightFilter);

    CVector3f position = CVector3f::Zero();
    if (useNegative) {
      mPositionDistance = negativeDistance;
      position = negative;
    } else {
      mPositionDistance = positiveDistance;
      position = positive;
    }
    if (camera->GetInitialPosition() == kISP_ClampBasis) {
      const float clamped = ScriptCameraSpline::ClampLength(
          spline.GetPositionSpline(), playerPosition, false, kPathLineOfSightFilter, mgr);
      if (clamped <= negativeDistance) {
        mPositionDistance = negativeDistance;
        position = negative;
      } else {
        mPositionDistance = positiveDistance;
        position = positive;
      }
    }

    const CVector3f look = GetScanObjectIndicatorPosition(mgr);
    if (!close_enough(position, look, 0.0001f)) {
      const CTransform4f cameraXf = CTransform4f::LookAt(position, look);
      SetTransform(cameraXf);
    } else {
      SetTranslation(position);
    }
    UpdateFov(mgr);
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
  if (close_enough(spline.GetLength(), 0.f)) {
    return 0.f;
  }

  float extent = camera->GetDistance();
  if (camera->GetFlags() & 4) {
    float distance = 0.f;
    const CVector3f pathPosition = spline.GetPositionByLength(mPlayerDistance, GetTransform(), mgr);
    CVector3f toPlayer = Player(const_cast< CStateManager& >(mgr)).GetBallPosition() - pathPosition;
    toPlayer.SetZ(0.f);
    if (toPlayer.IsMagnitudeSafe()) {
      distance = toPlayer.Magnitude();
    }
    const float control = camera->GetPerpendicularDistanceControlSpline().EvaluateAt(distance);
    extent *= 1.f - CMath::Clamp(0.f, control, 1.f);
  }

  float newDistance;
  if (spline.GetPositionSpline().IsClosedLoop()) {
    const float positive = spline.ValidateLength(mPlayerDistance + extent);
    const float negative = spline.ValidateLength(mPlayerDistance - extent);
    const float distance = CMath::AbsF(mPositionDistance - mPlayerDistance);
    const float remaining = spline.GetLength() - distance;
    if (mPositionDistance > mPlayerDistance) {
      newDistance = distance <= remaining ? positive : negative;
    } else {
      newDistance = distance <= remaining ? negative : positive;
    }
  } else if (mPositionDistance > mPlayerDistance) {
    newDistance = spline.ValidateLength(mPlayerDistance + extent);
  } else {
    newDistance = spline.ValidateLength(mPlayerDistance - extent);
  }

  if (camera->GetFlags() & 1) {
  } else if (spline.GetPositionSpline().IsClosedLoop()) {
    const float distance = CMath::AbsF(newDistance - mPositionDistance);
    float nearest = distance;
    if (distance > spline.GetLength() - distance) {
      nearest = spline.GetLength() - distance;
    }
    const float scale = CMath::Limit(nearest / camera->GetDampenDistance(), 1.f);
    float step = (mSpeed * dt) * scale;
    const float offset = CMath::AbsF(mPositionDistance - newDistance);
    const float remaining = spline.GetLength() - offset;
    if (mPositionDistance > newDistance) {
      if (offset <= remaining) {
        step *= -1.f;
      }
    } else if (offset > remaining) {
      step *= -1.f;
    }
    newDistance = spline.ValidateLength(mPositionDistance + step);
  } else {
    const float scale =
        CMath::Limit((newDistance - mPositionDistance) / camera->GetDampenDistance(), 1.f);
    const float step = (mSpeed * dt) * scale;
    newDistance = spline.ValidateLength(mPositionDistance + step);
  }
  return newDistance;
}

CVector3f CPathCamera::MoveAlongSpline(float dt, CStateManager& mgr) {
  const CVector3f translation = GetTranslation();
  const CVector3f playerPosition = Player(mgr).GetBallPosition();
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  if (!camera) {
    return translation;
  }

  CScriptCameraSpline& spline = camera->GetSpline();
  const CMotionSpline& playerSpline = camera->GetPlayerSpline();
  if (camera->GetSpeedControlSpline().GetKnotCount() != 0) {
    float progress = 0.f;
    if (playerSpline.GetControlPointCount() != 0) {
      progress = CMath::Clamp(0.f, mPlayerDistance / playerSpline.GetLength(), 1.f);
    } else {
      if (spline.GetPositionSpline().GetControlPointCount() != 0) {
        progress =
            CMath::Clamp(0.f, mPositionDistance / spline.GetPositionSpline().GetLength(), 1.f);
      }
      if (spline.GetLookAtSpline().GetControlPointCount() != 0) {
        progress = CMath::Clamp(
            0.f, mLookAtDistance / camera->GetSpline().GetLookAtSpline().GetLength(), 1.f);
      }
    }
    mSpeed = camera->GetSpeedControlSpline().EvaluateAt(progress) * camera->GetSpeed();
  }

  if (playerSpline.GetControlPointCount() != 0 &&
      camera->GetSpeedControlSpline().GetKnotCount() == 0) {
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
  } else {
    mPlayerDistance = spline.FindClosestLengthOnSpline(mPlayerDistance, playerPosition);
    mPositionDistance = CalculatePositionDistance(dt, mgr);
    mLookAtDistance = CalculateLookAtDistance(mgr);
  }
  const CVector3f position = spline.GetPositionByLength(mPositionDistance, GetTransform(), mgr);
  return position;
}

CTransform4f CPathCamera::AvoidDoorCollisions(const CTransform4f& xf, CStateManager& mgr) {
  CTransform4f result(xf);
  const CBallCamera* ballCamera = CameraManager(mgr).GetBallCamera();
  const CScriptDoor* door =
      TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(ballCamera->GetTooCloseActorId()));
  if (door && !door->IsOpen() &&
      GetCameraManager(mgr).GetBallCamera()->CheckDoorProximity(xf.GetTranslation(), mgr)) {
    const CScriptPathCamera* camera = GetScriptCamera(mgr);
    if (!camera) {
      return xf;
    }
    float newDistance = mPlayerDistance + camera->GetDistance();
    if (mPositionDistance > mPlayerDistance) {
      newDistance = mPlayerDistance - camera->GetDistance();
    }
    mPositionDistance = newDistance;
    result.SetTranslation(camera->GetSpline().GetPositionByLength(newDistance, result, mgr));
  }
  return result;
}

void CPathCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const CTransform4f oldXf = GetTransform();
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  if (!camera) {
    return;
  }

  const CVector3f position = MoveAlongSpline(dt, mgr);
  CTransform4f xf = CTransform4f::LookAt(position, GetScanObjectIndicatorPosition(mgr));
  if ((camera->GetFlags() & 0x10) && camera->GetPlayerSpline().GetControlPointCount() == 0) {
    xf = AvoidDoorCollisions(xf, mgr);
  }
  UpdateOrientation(dt, xf, mgr);
  UpdateFov(mgr);

  if (CScriptTimeKeyframe* keyframe =
          TCastToPtr< CScriptTimeKeyframe >(mgr.ObjectById(camera->GetTimeKeyframeId()))) {
    const float length = camera->GetSpline().GetPositionSpline().GetLength();
    float time = CMath::Clamp(0.f, mPositionDistance / length, 1.f);
    if (CameraManager(mgr).GetCurrentCameraId(false) != GetUniqueId()) {
      time = 0.f;
    }
    keyframe->SetTime(time, mgr);
  }

  xf = ValidateCameraTransform(GetTransform(), oldXf, dt);
  SetTransform(xf);
  CActor::Think(dt, mgr);
}

void CPathCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CPathCamera::Render(const CStateManager& mgr) const {}

void CPathCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
}

CVector3f CPathCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  CVector3f result = CameraManager(const_cast< CStateManager& >(mgr))
                         .GetBallCamera()
                         ->GetScanObjectIndicatorPosition(mgr);
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      CameraManager(const_cast< CStateManager& >(mgr)).GetHintManager()->GetCurrentHint(mgr));
  if (!camera) {
    return result;
  }

  if (camera->GetFlags() & 0x20) {
    if (hint && (hint->GetInfo().GetFlags() & 0x40)) {
      result = Player(const_cast< CStateManager& >(mgr)).GetBallPosition();
    }
  } else {
    if (camera->GetFlags() & 0x80) {
      float planarDistance = 0.f;
      const CVector3f pathPosition =
          camera->GetSpline().GetPositionByLength(mPlayerDistance, GetTransform(), mgr);
      const CVector3f playerPosition = Player(const_cast< CStateManager& >(mgr)).GetBallPosition();
      CVector3f toPlayer = playerPosition - pathPosition;
      toPlayer.SetZ(0.f);
      if (toPlayer.IsMagnitudeSafe()) {
        planarDistance = toPlayer.Magnitude();
      }
      const float interp = CMath::Clamp(
          0.f, camera->GetPerpendicularInterpControlSpline().EvaluateAt(planarDistance), 1.f);
      if ((camera->GetSpline().GetFlags() & CGameSpline::kF_UsePositionForLookAt) &&
          camera->GetSpline().GetPositionSpline().GetControlPointCount() != 0) {
        const float distance =
            camera->GetSpline().FindClosestLengthOnSpline(mPositionDistance, playerPosition);
        result = camera->GetSpline().CGameSpline::GetPositionByLength(distance);
      } else {
        result = camera->GetSpline().GetLookAtByLength(mLookAtDistance);
      }
      const CVector3f ballIndicator = CameraManager(const_cast< CStateManager& >(mgr))
                                          .GetBallCamera()
                                          ->GetScanObjectIndicatorPosition(mgr);
      result = result + interp * (ballIndicator - result);
    } else {
      if (camera->GetSpline().GetLookAtKnotCount() == 0 &&
          (camera->GetSpline().GetFlags() & CGameSpline::kF_UsePositionForLookAt) &&
          camera->GetSpline().GetPositionSpline().GetControlPointCount() != 0) {
        const CVector3f playerPosition =
            Player(const_cast< CStateManager& >(mgr)).GetBallPosition();
        const float distance =
            camera->GetSpline().FindClosestLengthOnSpline(mPositionDistance, playerPosition);
        result = camera->GetSpline().CGameSpline::GetPositionByLength(distance);
      } else {
        const CQuaternion orientation = camera->GetSpline().GetOrientationByLength(
            mPositionDistance, mLookAtDistance, GetTransform(), mgr);
        const CVector3f forward = orientation.BuildTransform4f().GetForward();
        const CVector3f playerPosition =
            Player(const_cast< CStateManager& >(mgr)).GetBallPosition();
        const float distance = CMath::FastMax(
            CMath::AbsF(CVector3f::Dot(playerPosition - GetTranslation(), forward)), 1.f);
        result = GetTranslation() + distance * forward;
      }
    }
  }

  if ((camera->GetFlags() & 8) && hint) {
    result.SetZ(hint->GetTranslation().GetZ());
  }
  if (camera->GetFlags() & 0x40) {
    if (camera->GetPlayerSpline().GetControlPointCount() != 0) {
      result.SetZ(camera->GetPlayerSpline().GetPositionByLength(mPlayerDistance).GetZ());
    } else if (camera->GetSpline().GetLookAtSpline().GetControlPointCount() != 0) {
      const CVector3f playerPosition = Player(const_cast< CStateManager& >(mgr)).GetBallPosition();
      const float distance =
          camera->GetSpline().GetLookAtSpline().FindClosestLengthOnSpline(0.f, playerPosition);
      result.SetZ(camera->GetSpline().GetLookAtSpline().GetPositionByLength(distance).GetZ());
    } else if (camera->GetSpline().GetPositionSpline().GetControlPointCount() != 0) {
      const CVector3f playerPosition = Player(const_cast< CStateManager& >(mgr)).GetBallPosition();
      const float distance =
          camera->GetSpline().GetPositionSpline().FindClosestLengthOnSpline(0.f, playerPosition);
      result.SetZ(camera->GetSpline().GetPositionSpline().GetPositionByLength(distance).GetZ());
    }
    if (hint) {
      result.SetZ(result.GetZ() + hint->GetInfo().GetLookAtOffset().GetZ());
    }
  }
  return result;
}

void CPathCamera::UpdateOrientation(float dt, const CTransform4f& xf, const CStateManager& mgr) {
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      CameraManager(const_cast< CStateManager& >(mgr)).GetHintManager()->GetCurrentHint(mgr));
  if (!camera) {
    return;
  }
  if (!hint) {
    return;
  }

  const CTransform4f currentXf = GetTransform();
  const CVector3f targetForward = xf.GetForward();
  const CVector3f position = xf.GetTranslation();
  CVector3f flatForward = targetForward;
  flatForward.SetZ(0.f);
  if (!flatForward.IsMagnitudeSafe()) {
    SetTranslation(position);
    return;
  }

  CVector3f currentForward = GetTransform().GetForward();
  if (currentForward.IsMagnitudeSafe()) {
    currentForward.Normalize();
  } else {
    SetTransform(CTransform4f::LookAt(position, position + targetForward));
    return;
  }

  const float alignment = CMath::Limit(CVector3f::Dot(currentForward, targetForward), 1.f);
  if (CMath::AbsF(alignment) >= 0.999999f) {
    SetTranslation(position);
  } else {
    if (hint->GetInfo().GetFlags() & 0x40) {
      SetTransform(xf);
      return;
    }

    const float ratio = CMath::Clamp(0.f, acosf(alignment) / (1.0471976f * dt), 1.f);
    CRelAngle step = CRelAngle::FromRadians(dt * (ratio * camera->GetAngularSpeed()));
    const float vertical =
        CMath::AbsF(CMath::Limit(CVector3f::Dot(targetForward, CVector3f::Up()), 1.f));
    const float verticalStep = 12.566371f * dt * (1.f - vertical);
    if (step.AsRadians() > verticalStep &&
        !Player(const_cast< CStateManager& >(mgr)).IsMorphBallTransitioning() &&
        vertical > 0.999f) {
      step = CRelAngle::FromRadians(verticalStep);
    }

    const CQuaternion rotation =
        CQuaternion::LookAt(CUnitVector3f(currentForward), CUnitVector3f(targetForward), step);
    SetTransform(rotation.BuildTransform4f() * GetTransform().GetRotation());
  }
  SetTranslation(position);
}

void CPathCamera::UpdateFov(const CStateManager& mgr) {
  const CScriptPathCamera* camera = GetScriptCamera(mgr);
  CScriptCameraSpline& spline = camera->GetSpline();
  const CMotionSpline& playerSpline = camera->GetPlayerSpline();
  if (playerSpline.GetControlPointCount() != 0) {
    const float time = playerSpline.GetDuration() * (mPlayerDistance / playerSpline.GetLength());
    SetTargetFov(spline.GetFovByTime(time));
  } else {
    SetTargetFov(spline.GetFovByLength(mPlayerDistance));
  }
}
