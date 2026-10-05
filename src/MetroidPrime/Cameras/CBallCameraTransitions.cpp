#include "MetroidPrime/Cameras/CBallCamera.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

namespace {
const CMaterialList skTransitionInclude(kMT_Unknown59);
const CMaterialFilter skTransitionFilter = CMaterialFilter::MakeIncludeExclude(
    skTransitionInclude,
    CMaterialList(kMT_NoPlatformCollision, kMT_Player, kMT_Character, kMT_CameraPassthrough));
}

bool CBallCamera::CheckFailsafeFromMorphBallState(CStateManager& mgr) {
  const float length = mFromBallTransition->mSpline.GetLength();
  CMaterialList hitMaterial;
  const CCameraManager& cameraManager = GetCameraManager(mgr);
  return cameraManager.CheckSplineCollision(mFromBallTransition->mSpline, 0, skTransitionFilter,
                                            mgr, hitMaterial, length / 24.f, 0.f);
}

bool CBallCamera::TransitionFromMorphBallState(CStateManager& mgr) {
  const CTransform4f playerXf = Player(mgr).GetTransform();
  const CTransform4f cameraXf =
      mgr.CameraManager(GetControllerNumber())->CurrentCamera(mgr, false)->GetTransform();
  const CVector3f lookPos = mgr.CameraManager(GetControllerNumber())
                                ->CurrentCamera(mgr, false)
                                ->GetScanObjectIndicatorPosition(mgr);
  mFromBallTransition->mLookPos = lookPos;
  mFromBallTransition->mPlayerXf = playerXf;

  const CVector3f eyePos = Player(mgr).GetEyePosition();
  const float cameraDistance = (lookPos - cameraXf.GetTranslation()).Magnitude();
  const CVector3f forward = playerXf.GetForward();
  const CVector3f desiredPoint = eyePos + (0.6f * -cameraDistance) * forward;
  float hitDistance = cameraDistance;
  CVector3f secondPoint = desiredPoint;
  if (DetectCollision(eyePos, desiredPoint, 0.3f, hitDistance, mgr, GetControllerNumber())) {
    secondPoint = eyePos + -hitDistance * forward;
  }

  rstl::vector< CVector3f > points;
  points.reserve(4);
  points.push_back_unsafe(cameraXf.GetTranslation());
  points.push_back_unsafe(secondPoint);
  points.push_back_unsafe(eyePos);
  points.push_back_unsafe(eyePos);
  mFromBallTransition->mSpline.Initialise(points);
  mFromBallTransition->mSpline.SetDuration(1.f);
  mFromBallTransition->mSpline.CalculateLength();
  return CheckFailsafeFromMorphBallState(mgr);
}

bool CBallCamera::UpdateTransitionFromBallCamera(CStateManager& mgr) {
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
  if (hint != nullptr && (hint->GetInfo().GetFlags() & 0x04000000) != 0) {
    return true;
  }

  const CVector3f oldPosition = GetTranslation();
  CPlayer& player = Player(mgr);
  const float factor = player.GetMorphBallTransitionFactor();
  const CVector3f eyePos = player.GetEyePosition();
  const CVector3f& playerPosition = player.GetTranslation();
  const CTransform4f& previousPlayerXf = mFromBallTransition->mPlayerXf;
  const CVector3f translationDelta(playerPosition.GetX() - previousPlayerXf.Get03(),
                                   playerPosition.GetY() - previousPlayerXf.Get13(),
                                   playerPosition.GetZ() - previousPlayerXf.Get23());
  mFromBallTransition->mSpline.Translate(translationDelta);

  if (player.GetRidingPlatform() != kInvalidUniqueId) {
    const CVector3f currentForward = player.GetTransform().GetForward();
    const CVector3f previousForward = mFromBallTransition->mPlayerXf.GetForward();
    if (fabsf(CVector3f::Dot(currentForward, previousForward)) < 0.9999f) {
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(previousForward), CUnitVector3f(currentForward),
          CRelAngle::FromRadians(6.2831855f));
      mFromBallTransition->mSpline.Rotate(rotation, eyePos);
    }
  }

  CVector3f position =
      mFromBallTransition->mSpline.GetPositionByTime(factor * mFromBallTransition->mSpline.GetDuration());
  const float heightFactor = CMath::Clamp(0.f, 1.f - 1.5f * factor, 1.f);
  position.SetZ(eyePos.GetZ() + heightFactor * (position.GetZ() - eyePos.GetZ()));
  CVector3f horizontalDelta = eyePos - position;
  horizontalDelta.SetZ(0.f);
  const float horizontalDistance = horizontalDelta.Magnitude();
  CCameraManager& cameraManager = const_cast< CCameraManager& >(GetCameraManager(mgr));
  if (horizontalDistance > 0.0011920929f) {
    const float lookFactor = CMath::Clamp(0.f, 1.f - 2.f * factor, 1.f);
    const CVector3f lookPos =
        eyePos + lookFactor * (mFromBallTransition->mLookPos - eyePos);
    SetTransform(CTransform4f::LookAt(position, lookPos, CVector3f::Up()));
  } else {
    SetTransform(cameraManager.FirstPersonCamera()->GetTransform());
    SetTranslation(position);
  }
  cameraManager.FirstPersonCamera()->Reset(GetTransform(), mgr);
  mFromBallTransition->mPlayerXf = player.GetTransform();

  if ((eyePos - GetTranslation()).Magnitude() > 0.5f) {
    CVector3f direction = GetTranslation() - oldPosition;
    if (direction.CanBeNormalized()) {
      direction = direction.AsNormalized();
    } else {
      direction = GetTransform().GetForward();
    }
    const CRayCastResult hit =
        mgr.RayStaticIntersection(GetTranslation(), direction, 0.5f, skTransitionFilter);
    if (hit.IsValid()) {
      const_cast< CCameraManager& >(GetCameraManager(mgr)).StartScreenFlash();
      return true;
    }
  }
  return false;
}

bool CBallCamera::CheckFailsafeToMorphBallState(CStateManager& mgr) {
  const float length = mToBallTransition->mSpline.GetLength();
  CMaterialList hitMaterial;
  const CCameraManager& cameraManager = GetCameraManager(mgr);
  return cameraManager.CheckSplineCollision(mToBallTransition->mSpline, 0, skTransitionFilter, mgr,
                                            hitMaterial, length / 24.f, 0.f);
}

bool CBallCamera::TransitionToMorphBallState(CStateManager& mgr) {
  const CTransform4f playerXf = Player(mgr).GetTransform();
  const CTransform4f firstPersonXf = CameraManager(mgr).FirstPersonCamera()->GetTransform();
  TeleportCamera(firstPersonXf, mgr);
  TeleportLookAtStuff(mgr);

  mToBallTransition->mLookPos =
      CameraManager(mgr).FirstPersonCamera()->GetScanObjectIndicatorPosition(mgr);
  mToBallTransition->mPlayerXf = playerXf;
  CVector3f eyePos = Player(mgr).GetEyePosition();
  float distance = mTargetMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  distance = mTargetMinDistance;
  const CVector3f ballPos =
      FindDesiredPosition(distance, elevation, Player(mgr).GetTranslation(), mgr, false);

  const CVector3f desiredPoint = eyePos + (0.6f * -distance) * playerXf.GetForward();
  float hitDistance;
  CVector3f secondPoint;
  if (DetectCollision(eyePos, desiredPoint, 0.3f, hitDistance, mgr, GetControllerNumber())) {
    secondPoint = eyePos + -hitDistance * playerXf.GetForward();
  } else {
    secondPoint = desiredPoint;
  }

  rstl::vector< CVector3f > points;
  points.reserve(4);
  points.push_back_unsafe(eyePos);
  points.push_back_unsafe(secondPoint);
  points.push_back_unsafe(ballPos);
  points.push_back_unsafe(ballPos);
  mToBallTransition->mSpline.Initialise(points);
  mToBallTransition->mSpline.SetDuration(1.f);
  mToBallTransition->mSpline.CalculateLength();
  return CheckFailsafeToMorphBallState(mgr);
}

bool CBallCamera::UpdateTransitionToBallCamera(float dt, CStateManager& mgr) {
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
  if (hint != nullptr && (hint->GetInfo().GetFlags() & 0x08000000) != 0) {
    return true;
  }

  const CVector3f oldPosition = GetTranslation();
  CPlayer& player = Player(mgr);
  const float factor = player.GetMorphBallTransitionFactor();
  const CVector3f eyePos = player.GetEyePosition();
  const CVector3f& playerPosition = player.GetTranslation();
  const CTransform4f& previousPlayerXf = mToBallTransition->mPlayerXf;
  const CVector3f translationDelta(playerPosition.GetX() - previousPlayerXf.Get03(),
                                   playerPosition.GetY() - previousPlayerXf.Get13(),
                                   playerPosition.GetZ() - previousPlayerXf.Get23());
  mToBallTransition->mSpline.Translate(translationDelta);
  if (player.GetRidingPlatform() != kInvalidUniqueId) {
    const CVector3f currentForward = player.GetTransform().GetForward();
    const CVector3f previousForward = mToBallTransition->mPlayerXf.GetForward();
    if (fabsf(CVector3f::Dot(currentForward, previousForward)) < 0.9999f) {
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(previousForward), CUnitVector3f(currentForward),
          CRelAngle::FromRadians(6.2831855f));
      mToBallTransition->mSpline.Rotate(rotation, eyePos);
    }
  }

  const CVector3f splinePosition =
      mToBallTransition->mSpline.GetPositionByTime(factor * mToBallTransition->mSpline.GetDuration());
  const CTransform4f oldTransform = GetTransform();
  CPlayer& transitionPlayer = Player(mgr);
  const float splineDistance = (splinePosition - eyePos).Magnitude();
  const float currentDistance = (oldPosition - eyePos).Magnitude();
  mBallCameraSpring.ApplyDistanceSpring(splineDistance, currentDistance, dt);

  CVector3f position = splinePosition;
  if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId))) {
    actor->SetTranslation(GetTranslation());
    position = MoveCollisionActor(ClampElevationToWater(position, mgr), dt, mgr);
    CVector3f lookDirection =
        mLookPos - mToBallTransition->mSpline.GetPositionByTime(mToBallTransition->mSpline.GetDuration());
    if (lookDirection.IsMagnitudeSafe()) {
      lookDirection.Normalize();
      CVector3f currentForward = GetTransform().GetForward();
      currentForward.SetZ(0.f);
      currentForward.Normalize();
      const float dot = CMath::Limit(CVector3f::Dot(currentForward, lookDirection), 1.f);
      if (fabsf(dot) < 0.999999f) {
        const float rotationFactor = CMath::Limit(1.15f * factor, 1.f);
        const CQuaternion rotation = CQuaternion::LookAt(
            CUnitVector3f(currentForward), CUnitVector3f(lookDirection),
            CRelAngle::FromRadians(rotationFactor * acosf(dot)));
        SetTransform(rotation.BuildTransform4f() *
                     CTransform4f::LookAt(position, position + currentForward, CVector3f::Up()));
      } else {
        SetTransform(CTransform4f::LookAt(CVector3f::Zero(), lookDirection, CVector3f::Up()));
      }
    }
  }

  SetTransform(ValidateCameraTransform(GetTransform(), oldTransform));
  SetTranslation(position);
  TeleportCamera(position, mgr);
  mToBallTransition->mPlayerXf = transitionPlayer.GetTransform();
  if ((mToBallTransition->mSpline.GetPositionByTime(mToBallTransition->mSpline.GetDuration()) -
       GetTranslation())
          .Magnitude() > 0.5f) {
    CVector3f direction = GetTranslation() - oldPosition;
    if (direction.CanBeNormalized()) {
      direction = direction.AsNormalized();
    } else {
      direction = GetTransform().GetForward();
    }
    const CRayCastResult hit =
        mgr.RayStaticIntersection(GetTranslation(), direction, 0.5f, skTransitionFilter);
    return hit.IsValid();
  }
  return false;
}

bool CBallCamera::UpdateTransitionToBallCamera(CStateManager& mgr) {
  mLookAtBall = false;
  CPlayer& player = Player(mgr);
  const CVector3f position = GetTranslation();
  CVector3f lookDirection = mLookPos - position;
  if (lookDirection.IsMagnitudeSafe()) {
    lookDirection.Normalize();
    CVector3f currentForward = GetTransform().GetForward();
    currentForward.SetZ(0.f);
    currentForward.Normalize();
    float dot = CMath::Limit(CVector3f::Dot(currentForward, lookDirection), 1.f);
    if (fabsf(dot) < 0.99999f) {
      float morphFactor = player.GetMorphBallTransitionFactor();
      const float rotationFactor = CMath::Limit(1.5f * morphFactor, 1.f);
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(currentForward), CUnitVector3f(lookDirection),
          CRelAngle::FromRadians(rotationFactor * acosf(dot)));
      const CTransform4f lookXf =
          CTransform4f::LookAt(position, position + currentForward, CVector3f::Up());
      SetTransform(rotation.BuildTransform4f() * lookXf);
    } else {
      SetTransform(CTransform4f::LookAt(position, position + lookDirection, CVector3f::Up()));
    }
  }
  SetTranslation(position);
  TeleportCamera(position, mgr);
  return false;
}
