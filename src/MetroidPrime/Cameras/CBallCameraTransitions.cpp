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
CMaterialList skTransitionInclude = CMaterialList(kMT_Solid);
CMaterialList skTransitionExclude =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
CMaterialFilter skTransitionFilter =
    CMaterialFilter::MakeIncludeExclude(skTransitionInclude, skTransitionExclude);
} // namespace

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
  const CVector3f cameraPos = cameraXf.GetTranslation();

  const float lookDistance = (lookPos - cameraPos).Magnitude();
  const CVector3f desiredPoint = (0.6f * -lookDistance) * playerXf.GetForward() + eyePos;
  CVector3f behindPos = desiredPoint;
  float hitDistance;
  if (DetectCollision(eyePos, desiredPoint, 0.3f, hitDistance, mgr, GetControllerNumber())) {
    behindPos = -hitDistance * playerXf.GetForward() + eyePos;
  } else {
    hitDistance = lookDistance;
  }

  rstl::vector< CVector3f > points;
  points.reserve(4);
  points.push_back_unsafe(cameraPos);
  points.push_back_unsafe(behindPos);
  points.push_back_unsafe(eyePos);
  points.push_back_unsafe(eyePos);
  mFromBallTransition->mSpline.Initialise(points);
  mFromBallTransition->mSpline.SetDuration(1.f);
  mFromBallTransition->mSpline.CalculateLength();
  return CheckFailsafeFromMorphBallState(mgr);
}

bool CBallCamera::UpdateTransitionFromBallCamera(CStateManager& mgr) {
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
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
    if (fabsf(CVector3f::Dot(player.GetTransform().GetForward(),
                             mFromBallTransition->mPlayerXf.GetForward())) < 0.9999f) {
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(mFromBallTransition->mPlayerXf.GetForward()),
          CUnitVector3f(player.GetTransform().GetForward()), CRelAngle::FromRadians(6.2831855f));
      mFromBallTransition->mSpline.Rotate(rotation, eyePos);
    }
  }

  CVector3f position = mFromBallTransition->mSpline.GetPositionByTime(
      factor * mFromBallTransition->mSpline.GetDuration());
  const float heightFactor = CMath::Clamp(0.f, 1.f - 1.5f * factor, 1.f);
  position.SetZ(eyePos.GetZ() + heightFactor * (position.GetZ() - eyePos.GetZ()));
  CVector3f horizontalDelta = eyePos - position;
  horizontalDelta.SetZ(0.f);
  const float horizontalDistance = horizontalDelta.Magnitude();
  CCameraManager& cameraManager = const_cast< CCameraManager& >(GetCameraManager(mgr));
  if (horizontalDistance > 0.0011920929f) {
    const float lookFactor = CMath::Clamp(0.f, 1.f - 2.f * factor, 1.f);
    const CVector3f lookPos = eyePos + lookFactor * (mFromBallTransition->mLookPos - eyePos);
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

  const CVector3f lookPos =
      CameraManager(mgr).FirstPersonCamera()->GetScanObjectIndicatorPosition(mgr);
  mToBallTransition->mLookPos = lookPos;
  mToBallTransition->mPlayerXf = playerXf;
  CVector3f eyePos = Player(mgr).GetEyePosition();
  float distance = mTargetMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  distance = mTargetMinDistance;
  const CVector3f forward = Player(mgr).GetTransform().GetForward();

  const CVector3f ballPos = FindDesiredPosition(distance, elevation, forward, mgr, false);
  const float backDistance = -distance;
  CVector3f desiredPoint = (0.6f * backDistance) * playerXf.GetForward() + eyePos;
  float hitDistance;
  if (DetectCollision(eyePos, desiredPoint, 0.3f, hitDistance, mgr, GetControllerNumber())) {
    desiredPoint = -hitDistance * playerXf.GetForward() + eyePos;
  }

  rstl::vector< CVector3f > points;
  points.reserve(4);
  points.push_back_unsafe(eyePos);
  points.push_back_unsafe(desiredPoint);
  points.push_back_unsafe(ballPos);
  points.push_back_unsafe(ballPos);
  mToBallTransition->mSpline.Initialise(points);
  mToBallTransition->mSpline.SetDuration(1.f);
  mToBallTransition->mSpline.CalculateLength();
  return CheckFailsafeToMorphBallState(mgr);
}

bool CBallCamera::UpdateTransitionToBallCamera(float dt, CStateManager& mgr) {
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
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
    if (fabsf(CVector3f::Dot(player.GetTransform().GetForward(),
                             mToBallTransition->mPlayerXf.GetForward())) < 0.9999f) {
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(mToBallTransition->mPlayerXf.GetForward()),
          CUnitVector3f(player.GetTransform().GetForward()), CRelAngle::FromRadians(6.2831855f));
      mToBallTransition->mSpline.Rotate(rotation, eyePos);
    }
  }

  const CVector3f splinePosition = mToBallTransition->mSpline.GetPositionByTime(
      factor * mToBallTransition->mSpline.GetDuration());
  const CTransform4f oldTransform = GetTransform();
  CPlayer& transitionPlayer = Player(mgr);
  const float splineDistance = (splinePosition - eyePos).Magnitude();
  const float currentDistance = (GetTranslation() - eyePos).Magnitude();
  mBallCameraSpring.ApplyDistanceSpring(splineDistance, currentDistance, dt);

  CVector3f position = splinePosition;
  if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId))) {
    actor->SetTranslation(GetTranslation());
    position = MoveCollisionActor(ClampElevationToWater(position, mgr), dt, mgr);
    CVector3f lookDirection = mLookPos - mToBallTransition->mSpline.GetPositionByTime(
                                             mToBallTransition->mSpline.GetDuration());
    if (lookDirection.IsMagnitudeSafe()) {
      lookDirection.Normalize();
      CVector3f currentForward = GetTransform().GetForward();
      currentForward.SetZ(0.f);
      currentForward.Normalize();
      const float dot = CMath::Limit(CVector3f::Dot(currentForward, lookDirection), 1.f);
      if (fabsf(dot) < 0.999999f) {
        const float rotationFactor =
            CMath::Limit(1.15f * transitionPlayer.GetMorphBallTransitionFactor(), 1.f);
        const CQuaternion rotation =
            CQuaternion::LookAt(CUnitVector3f(currentForward), CUnitVector3f(lookDirection),
                                CRelAngle::FromRadians(rotationFactor * acosf(fabsf(dot))));
        SetTransform(rotation.BuildTransform4f() *
                     CTransform4f::LookAt(position, position + currentForward, CVector3f::Up()));
      } else {
        SetTransform(CTransform4f::LookAt(CVector3f::Zero(), lookDirection, CVector3f::Up()));
      }
    }
  }

  SetTransform(ValidateCameraTransform(GetTransform(), oldTransform, dt));
  SetTranslation(position);
  TeleportCamera(position, mgr);
  mToBallTransition->mPlayerXf = player.GetTransform();
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
  CVector3f lookDirection = player.GetTransform().GetForward();
  lookDirection = mLookPos - GetTranslation();
  const CVector3f position = GetTranslation();
  if (lookDirection.IsMagnitudeSafe()) {
    lookDirection.Normalize();
    CVector3f currentForward = GetTransform().GetForward();
    currentForward.SetZ(0.f);
    currentForward.Normalize();
    const float absDot =
        CMath::AbsF(CMath::Limit(CVector3f::Dot(currentForward, lookDirection), 1.f));
    if (absDot < 0.99999f) {
      float rotationFactor = 1.5f * player.GetMorphBallTransitionFactor();
      rotationFactor = CMath::Limit(rotationFactor, 1.f);
      const CQuaternion rotation =
          CQuaternion::LookAt(CUnitVector3f(currentForward), CUnitVector3f(lookDirection),
                              CRelAngle::FromRadians(rotationFactor * acosf(absDot)));
      SetTransform(rotation.BuildTransform4f() *
                   CTransform4f::LookAt(position, position + currentForward, CVector3f::Up()));
    } else {
      SetTransform(CTransform4f::LookAt(position, position + lookDirection, CVector3f::Up()));
    }
  }
  SetTranslation(position);
  TeleportCamera(position, mgr);
  return false;
}
