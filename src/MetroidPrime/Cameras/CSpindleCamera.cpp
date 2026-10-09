#include "MetroidPrime/Cameras/CSpindleCamera.hpp"

#include "Kyoto/Math/CLine.hpp"
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
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

CSpindleCameraInterpolant::CSpindleCameraInterpolant(ESpindleInput input, const CMayaSpline& spline)
: mInput(input), mSpline(spline) {}

float CSpindleCameraInterpolant::InterpolateValue(float input) const {
  return mSpline.EvaluateAt(input);
}

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
: CGameCamera(uid, rstl::string_l("Spindle Camera"),
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
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      GetCameraManager(mgr).HintManager()->GetCurrentHint(mgr));
  if (!GetActive() || hint == nullptr) {
    return;
  }

  mInResetThink = true;
  GetCameraManager(mgr).BallCamera()->UpdateLookAtPosition(0.01f, mgr, false);
  Think(0.01f, mgr);
  mInResetThink = false;
  mFixedPositionInitialized = false;
}

float CSpindleCamera::CalculateTargetSplineDistance(CStateManager& mgr) const {
  const CScriptSpindleCamera* script =
      TCastToConstPtr< CScriptSpindleCamera >(mgr.GetObjectById(mSpindleCameraId));
  if (script == nullptr) {
    return 0.f;
  }

  const CMotionSpline& targetSpline = script->GetTargetSpline();
  if (targetSpline.GetControlPointCount() == 0) {
    return 0.f;
  }

  const CMotionSpline& playerSpline = script->GetPlayerSpline();
  if (playerSpline.GetControlPointCount() == 0) {
    return targetSpline.FindClosestLengthOnSpline(mTargetSplineDistance,
                                                  GetPlayer(mgr).GetBallPosition());
  }

  const float playerLength = playerSpline.GetLength();
  if (close_enough(playerLength, 0.f)) {
    return 0.f;
  }
  const float progress = CMath::Clamp(0.f, mPlayerSplineDistance / playerLength, 1.f);
  return script->GetTargetControlSpline().EvaluateAt(progress) * targetSpline.GetLength();
}

float CSpindleCamera::GetInVar(const CSpindleCameraInterpolant& interpolant) const {
  return mInVars[interpolant.GetInput()];
}

float CSpindleCamera::GetInterpolant(const CSpindleCameraInterpolant& interpolant) const {
  return interpolant.InterpolateValue(GetInVar(interpolant));
}

void CSpindleCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const CScriptSpindleCamera* script =
      TCastToConstPtr< CScriptSpindleCamera >(mgr.GetObjectById(mSpindleCameraId));
  if (script == nullptr) {
    return;
  }

  const CSpindleCameraParameters& params = script->GetParameters();
  if ((params.GetFlags() & 0x10000) != 0 && mFixedPositionInitialized) {
    return;
  }

  const CTransform4f oldXf = GetTransform();
  if (script->GetPlayerSpline().GetControlPointCount() != 0) {
    mPlayerSplineDistance = script->GetPlayerSpline().FindClosestLengthOnSpline(
        mPlayerSplineDistance, GetPlayer(mgr).GetBallPosition());
  }
  mTargetSplineDistance = CalculateTargetSplineDistance(mgr);

  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      GetCameraManager(mgr).HintManager()->GetCurrentHint(mgr));
  if (hint == nullptr) {
    return;
  }

  const CTransform4f scriptXf = script->GetTransform();
  const CVector3f hintPos = scriptXf.GetTranslation();
  const CLine hintLine(hintPos, CUnitVector3f(scriptXf.GetUp()));

  CVector3f hintToCamDir = GetTranslation() - hintLine.GetClosestPoint(GetTranslation());
  const CVector3f ballPos = GetPlayer(mgr).GetBallPosition();
  CVector3f hintToBallDir = ballPos - hintLine.GetClosestPoint(ballPos);

  float hintBallToCamAzimuth;
  float hintToBallDist = 0.f;
  CVector3f hintDir = script->GetTransform().GetForward();
  const CVector3f up = script->GetTransform().GetUp();
  if (hintDir.IsMagnitudeSafe()) {
    hintDir.Normalize();
  } else {
    hintDir = CVector3f(0.f, 1.f, 0.f);
  }

  if (hintToBallDir.IsMagnitudeSafe()) {
    hintToBallDist = hintToBallDir.Magnitude();
    hintToBallDir.Normalize();
  } else {
    hintToBallDir = hintDir;
  }

  mInVars.clear();
  mInVars.push_back(hintToBallDist);

  mInVars.push_back(CVector3f(hintLine.GetClosestPoint(ballPos) - hintPos).Magnitude());

  const float hintBallAngle =
      CMath::AbsF(acos(CMath::Limit(CVector3f::Dot(hintToBallDir, hintDir), 1.f)));
  const float hintBallAngleDegrees = hintBallAngle * (180.f / M_PIF);
  mInVars.push_back(hintBallAngleDegrees);

  const float hintBallCross = CVector3f::Dot(CVector3f::Cross(hintToBallDir, hintDir), up);
  if (hintBallCross >= 0.f) {
    mInVars.push_back(hintBallAngleDegrees);
    mInVars.push_back((2.f * M_PIF - hintBallAngle) * (180.f / M_PIF));
  } else {
    mInVars.push_back((2.f * M_PIF - hintBallAngle) * (180.f / M_PIF));
    mInVars.push_back(hintBallAngleDegrees);
  }

  const CLine origLine(script->GetOrigXf().GetTranslation(),
                       CUnitVector3f(script->GetOrigXf().GetUp()));
  const CVector3f hintDeltaVOff =
      origLine.GetClosestPoint(script->GetTranslation()) - script->GetOrigXf().GetTranslation();
  const float hintDeltaVOffDist = hintDeltaVOff.Magnitude();
  CVector3f hintDelta =
      script->GetTranslation() - origLine.GetClosestPoint(script->GetTranslation());
  float hintDeltaDist = 0.f;
  if (hintDelta.IsMagnitudeSafe()) {
    hintDeltaDist = hintDelta.Magnitude();
  }
  mInVars.push_back(hintDeltaDist);
  mInVars.push_back(hintDeltaVOffDist);

  float splineProgress = 0.f;
  if (script->GetTargetSpline().GetControlPointCount() != 0) {
    splineProgress =
        CMath::Clamp(0.f, mTargetSplineDistance / script->GetTargetSpline().GetLength(), 1.f);
  } else if (script->GetPlayerSpline().GetControlPointCount() != 0) {
    splineProgress =
        CMath::Clamp(0.f, mPlayerSplineDistance / script->GetPlayerSpline().GetLength(), 1.f);
  }
  mInVars.push_back(splineProgress);

  if ((params.GetFlags() & 0x2000) != 0 &&
      hintToBallDist > GetInterpolant(params.GetDeactivateRadius())) {
    if (hint->GetDelegatedCameraId() == GetUniqueId()) {
      CameraManager(mgr).HintManager()->ForceRemoveHint(hint->GetUniqueId(), mgr, kInvalidUniqueId);
    }
    return;
  }

  if ((params.GetFlags() & 0x800) == 0) {
    hintToBallDir = hintDir;
  }

  float newHintToCamDist = GetInterpolant(params.GetMotionRadius());
  if ((params.GetFlags() & 0x40) != 0) {
    newHintToCamDist = hintToBallDist + GetInterpolant(params.GetRadialOffset());
  }

  CVector3f newCamPos = GetTranslation();
  float hintToCamDist = hintToCamDir.Magnitude();
  if (hintToCamDir.IsMagnitudeSafe()) {
    hintToCamDir.Normalize();
  } else {
    hintToCamDir = hintDir;
    hintToCamDist = GetInterpolant(params.GetMotionRadius());
  }

  const float hintCamCross = CVector3f::Dot(CVector3f::Cross(hintToCamDir, hintDir), up);
  if ((params.GetFlags() & 0x20) != 0) {
    if (!mOutsideClampedAzimuth) {
      if (hintBallAngle > 0.017453292f * GetInterpolant(params.GetAngularConstraint())) {
        mLookDir = hintToBallDir;
        mOutsideClampedAzimuth = true;
      }
    } else {
      if ((hintBallAngle < 0.017453292f * GetInterpolant(params.GetConstraintFlipAngle()) &&
           hintBallCross * hintCamCross < 0.f) ||
          hintBallAngle <= 0.017453292f * GetInterpolant(params.GetAngularConstraint())) {
        mOutsideClampedAzimuth = false;
      } else {
        hintToBallDir = mLookDir;
      }
    }
  }

  float hintBallToCamTargetAzimuth =
      0.017453292f * GetInterpolant(params.GetDesiredAngularOffset());
  if ((params.GetFlags() & 0x4000) == 0 &&
      CVector3f::Dot(CVector3f::Cross(hintToCamDir, hintToBallDir), up) >= 0.f) {
    hintBallToCamTargetAzimuth = -hintBallToCamTargetAzimuth;
  }

  CQuaternion azimuthQuat = CQuaternion::AxisAngle(
      CUnitVector3f(hintLine.GetNormal()), CRelAngle::FromRadians(hintBallToCamTargetAzimuth));
  const CVector3f targetHintToCam = azimuthQuat.Transform(hintToBallDir);
  CVector3f newHintToCamDir = hintToCamDir;

  const float hintToCamDeltaAngleRange =
      CMath::AbsF(acos(CMath::Limit(CVector3f::Dot(hintToCamDir, targetHintToCam), 1.f)));
  const float hintToCamDeltaAngleSpeedFactor = CMath::Limit(
      hintToCamDeltaAngleRange / (0.017453292f * GetInterpolant(params.GetAngularDampening())),
      1.f);

  float targetHintToCamDeltaAngleVel = 0.017453292f * GetInterpolant(params.GetAngularSpeed());
  if ((params.GetFlags() & 0x100) == 0) {
    targetHintToCamDeltaAngleVel =
        CMath::Limit(0.017453292f * GetInterpolant(params.GetLinearSpeed()) / hintToCamDist,
                     targetHintToCamDeltaAngleVel);
  }

  if ((CVector3f::Dot(CVector3f::Cross(hintToBallDir, hintToCamDir), up) >= 0.f &&
       CVector3f::Dot(CVector3f::Cross(targetHintToCam, hintToCamDir), up) < 0.f) ||
      (CVector3f::Dot(CVector3f::Cross(hintToBallDir, hintToCamDir), up) < 0.f &&
       CVector3f::Dot(CVector3f::Cross(targetHintToCam, hintToCamDir), up) >= 0.f)) {
    targetHintToCamDeltaAngleVel =
        CMath::Limit(targetHintToCamDeltaAngleVel,
                     0.017453292f * GetInterpolant(params.GetDesiredAngularSpeed()));
  }

  float targetHintToCamDeltaAngle =
      targetHintToCamDeltaAngleVel * (dt * hintToCamDeltaAngleSpeedFactor);
  CVector3f camToBall = ballPos - GetTranslation();
  camToBall[kDZ] = 0.f;

  float camToBallDist = 0.f;
  if (camToBall.IsMagnitudeSafe()) {
    camToBallDist = camToBall.Magnitude();
  }

  targetHintToCamDeltaAngle *=
      (1.f - CMath::Clamp(0.f, (camToBallDist - 2.f) / 2.f, 1.f)) * 10.f + 1.f;
  targetHintToCamDeltaAngle = CMath::Limit(targetHintToCamDeltaAngle, hintToCamDeltaAngleRange);

  if (CMath::AbsF(CMath::Limit(CVector3f::Dot(hintToCamDir, targetHintToCam), 1.f)) < 0.9999999f) {
    azimuthQuat = CQuaternion::ShortestRotationArcClamped(
        hintToCamDir, targetHintToCam, CRelAngle::FromRadians(targetHintToCamDeltaAngle));
    newHintToCamDir = azimuthQuat.Transform(hintToCamDir);
  } else {
    newHintToCamDir = targetHintToCam;
  }

  if ((params.GetFlags() & 0x8) != 0 || mInResetThink) {
    newHintToCamDir = targetHintToCam;
  }

  hintBallToCamAzimuth = acos(CMath::Limit(CVector3f::Dot(hintToBallDir, newHintToCamDir), 1.f));
  const float minHintBallToCamAzimuth = 0.017453292f * GetInterpolant(params.GetMinAngularOffset());
  if (CMath::AbsF(hintBallToCamAzimuth) < minHintBallToCamAzimuth) {
    azimuthQuat = CQuaternion::AxisAngle(CUnitVector3f(hintLine.GetNormal()),
                                         CRelAngle::FromRadians(minHintBallToCamAzimuth));
    if (CVector3f::Dot(CVector3f::Cross(hintToBallDir, newHintToCamDir), hintLine.GetNormal()) <
        0.f) {
      azimuthQuat = CQuaternion::AxisAngle(CUnitVector3f(hintLine.GetNormal()),
                                           CRelAngle::FromRadians(-minHintBallToCamAzimuth));
    }
    newHintToCamDir = azimuthQuat.Transform(hintToBallDir);
  }

  const float maxHintBallToCamAzimuth = 0.017453292f * GetInterpolant(params.GetMaxAngularOffset());
  if (CMath::AbsF(hintBallToCamAzimuth) > maxHintBallToCamAzimuth) {
    mMaxAzimuthInterpTimer += dt;
    if (mMaxAzimuthInterpTimer < 3.f) {
      const float azimuthInterp = CMath::Limit(mMaxAzimuthInterpTimer / 3.f, 1.f);
      float azimuthDelta = CMath::AbsF(maxHintBallToCamAzimuth - hintBallToCamAzimuth);
      if (CVector3f::Dot(CVector3f::Cross(hintToBallDir, newHintToCamDir), hintLine.GetNormal()) >
          0.f) {
        azimuthDelta = -azimuthDelta;
      }
      azimuthQuat = CQuaternion::AxisAngle(CUnitVector3f(hintLine.GetNormal()),
                                           CRelAngle::FromRadians(azimuthDelta * azimuthInterp));
      newHintToCamDir = azimuthQuat.Transform(newHintToCamDir);
    } else {
      if (hintBallToCamTargetAzimuth > 0.f) {
        azimuthQuat = CQuaternion::AxisAngle(CUnitVector3f(hintLine.GetNormal()),
                                             CRelAngle::FromRadians(maxHintBallToCamAzimuth));
      } else {
        azimuthQuat = CQuaternion::AxisAngle(CUnitVector3f(hintLine.GetNormal()),
                                             CRelAngle::FromRadians(-maxHintBallToCamAzimuth));
      }
      newHintToCamDir = azimuthQuat.Transform(hintToBallDir);
    }
  } else {
    mMaxAzimuthInterpTimer = 0.f;
  }

  if ((params.GetFlags() & 0x20) != 0) {
    CVector3f flatHintDir = script->GetTransform().GetForward();
    flatHintDir[kDZ] = 0.f;
    if (flatHintDir.IsMagnitudeSafe()) {
      flatHintDir.Normalize();
      const float hintCamAzimuth =
          CMath::AbsF(acos(CMath::Limit(CVector3f::Dot(flatHintDir, newHintToCamDir), 1.f)));
      float clampedAzimuth = CMath::Limit(
          hintCamAzimuth, 0.017453292f * GetInterpolant(params.GetAngularConstraint()));
      if (CVector3f::Dot(CVector3f::Cross(flatHintDir, newHintToCamDir), hintLine.GetNormal()) <
          0.f) {
        clampedAzimuth = -clampedAzimuth;
      }
      azimuthQuat = CQuaternion::AxisAngle(CUnitVector3f(hintLine.GetNormal()),
                                           CRelAngle::FromRadians(clampedAzimuth));
      newHintToCamDir = azimuthQuat.Transform(flatHintDir);
    }
  }

  newHintToCamDir *= newHintToCamDist;
  newCamPos = hintPos + newHintToCamDir;

  const CVector3f zOffset =
      scriptXf.Rotate(CVector3f(0.f, 0.f, GetInterpolant(params.GetZOffset())));
  if ((params.GetFlags() & 0x80) != 0) {
    newCamPos += (hintLine.GetClosestPoint(ballPos) - hintPos) + zOffset;
  } else {
    newCamPos += zOffset;
  }

  SetTranslation(newCamPos);
  mLookPosition = GetScanObjectIndicatorPosition(mgr);

  CVector3f lookDelta = mLookPosition - newCamPos;
  if (lookDelta.IsMagnitudeSafe()) {
    SetTransform(CTransform4f::LookAt(newCamPos, newCamPos + lookDelta));
  }

  SetTargetFov(GetInterpolant(params.GetFov()));
  if ((params.GetFlags() & 0x10000) != 0) {
    mFixedPositionInitialized = true;
  }

  const CTransform4f validXf = ValidateCameraTransform(GetTransform(), oldXf, dt);
  SetTransform(validXf);
  CActor::Think(dt, mgr);
}

void CSpindleCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CSpindleCamera::Render(const CStateManager& mgr) const {}

CVector3f CSpindleCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CScriptSpindleCamera* script =
      TCastToConstPtr< CScriptSpindleCamera >(mgr.GetObjectById(mSpindleCameraId));
  CVector3f lookPos = GetCameraManager(mgr).GetBallCamera()->GetScanObjectIndicatorPosition(mgr);
  if (script != nullptr) {
    const CSpindleCameraParameters& params = script->GetParameters();
    if ((params.GetFlags() & 0x10000) != 0 && mFixedPositionInitialized) {
      return mLookPosition;
    }

    const CTransform4f scriptXf = script->GetTransform();
    const CVector3f hintPos = scriptXf.GetTranslation();
    const CLine hintLine(hintPos, CUnitVector3f(scriptXf.GetUp()));
    const CVector3f ballPos = GetPlayer(mgr).GetBallPosition();
    hintLine.GetClosestPoint(ballPos);

    if ((params.GetFlags() & 0x8000) != 0) {
      lookPos = ballPos;
    } else if (script->GetTargetSpline().GetControlPointCount() != 0) {
      lookPos = script->GetTargetSpline().GetPositionByLength(mTargetSplineDistance);
    } else {
      CVector3f zOffset = CVector3f::Zero();
      if (mInVars.size() != 0) {
        zOffset = scriptXf.Rotate(CVector3f(0.f, 0.f, GetInterpolant(params.GetLookAtZOffset())));
      }

      if ((params.GetFlags() & 0x200) != 0) {
        lookPos += zOffset;
      } else {
        lookPos =
            script->GetTranslation() + (lookPos - hintLine.GetClosestPoint(lookPos)) + zOffset;
      }
    }

    hintLine.GetClosestPoint(GetTranslation());
    CVector3f hintToBallDir = ballPos - hintLine.GetClosestPoint(ballPos);
    const CVector3f camPos = GetTranslation();
    CVector3f lookDelta = lookPos - camPos;

    if ((params.GetFlags() & 0x1) != 0) {
      lookPos = hintLine.GetClosestPoint(lookPos);
      lookDelta = lookPos - camPos;
    }

    if ((params.GetFlags() & 0x2) != 0) {
      lookDelta = lookPos - hintLine.GetClosestPoint(camPos);
      lookPos = camPos + lookDelta;
    }

    CVector3f flatLookDelta = lookDelta;
    flatLookDelta.SetZ(0.f);
    if (flatLookDelta.IsMagnitudeSafe()) {
      const float lookDist = flatLookDelta.Magnitude();
      flatLookDelta.Normalize();

      float camLookRelAzimuth = 0.017453292f * -GetInterpolant(params.GetLookAtAngularOffset());
      CVector3f hintToCamDir = camPos - hintLine.GetClosestPoint(camPos);
      if (hintToCamDir.IsMagnitudeSafe()) {
        hintToCamDir.Normalize();
      } else {
        hintToCamDir = hintToBallDir;
      }

      // The target takes the absolute value of the comparison result, not of the dot product.
      if (CMath::AbsF(CVector3f::Dot(hintToCamDir.AsNormalized(), hintToBallDir.AsNormalized()) <
                      0.99999f)) {
        if (CVector3f::Dot(CVector3f::Cross(hintToCamDir, hintToBallDir), hintLine.GetNormal()) >=
            0.f) {
          camLookRelAzimuth = -camLookRelAzimuth;
        }

        if ((params.GetFlags() & 0x1000) != 0) {
          camLookRelAzimuth *= CMath::Limit(
              acosf(CMath::AbsF(CMath::Limit(CVector3f::Dot(hintToBallDir, hintToCamDir), 1.f))) /
                  0.17453292f,
              1.f);
        }

        const CQuaternion azimuthQuat = CQuaternion::AxisAngle(
            CUnitVector3f(hintLine.GetNormal()), CRelAngle::FromRadians(camLookRelAzimuth));
        const CVector3f rotated = azimuthQuat.Transform(flatLookDelta);
        const float lookZ = lookPos.GetZ();
        lookPos = camPos + rotated * cos(camLookRelAzimuth) * lookDist;
        lookPos[kDZ] = lookZ;
      }
    }
  }

  return lookPos;
}
