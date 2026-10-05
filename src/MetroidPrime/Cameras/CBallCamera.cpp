#include "MetroidPrime/Cameras/CBallCamera.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CLine.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CHintState.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

namespace {
const CMaterialFilter skLineOfSightFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_Unknown59),
    CMaterialList(kMT_NoPlatformCollision, kMT_Player, kMT_Character, kMT_CameraPassthrough));
}

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const CTransform4f& xf, float fovY,
                         float nearZ, float farZ, float aspect, int index, int controllerIdx)
: CGameCamera(uid, rstl::string("Ball Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, true), xf, fovY, nearZ, farZ, aspect,
              watchedId, index, controllerIdx)
, mBehaviour(kBCB_Default)
, x204_24_(true)
, mChaseAllowed(true)
, mBoostAllowed(true)
, mObscureAvoidance(true)
, mVolumeCollider(true)
, mClampAttitude(false)
, mClampAzimuth(false)
, mClearLOS(true)
, mPrevClearLOS(true)
, mAvoidGeometryFull(false)
, mLookAtBall(false)
, mForceProcessing(false)
, mObtuseDirection(false)
, mNoElevationInterp(false)
, mDirectElevation(false)
, mOverrideLookDir(false)
, mNoElevationVelClamp(false)
, mNoSpline(false)
, x206_26_(false)
, mNearbyDoorClosed(false)
, mNearbyDoorClosing(false)
, x206_29_(false)
, mCurMinDistance(gpTweakBall->GetBallCameraMinSpeedDistance())
, mTargetMinDistance(gpTweakBall->GetBallCameraMinSpeedDistance())
, mMaxDistance(gpTweakBall->GetBallCameraMaxSpeedDistance())
, mBackwardsDistance(gpTweakBall->GetBallCameraBackwardsDistance())
, mElevation(2.736f)
, mCurAnglePerSecond(gpTweakBall->GetBallCameraAnglePerSecond())
, mTargetAnglePerSecond(gpTweakBall->GetBallCameraAnglePerSecond())
, mAttitudeRange(1.553343f)
, mAzimuthRange(1.553343f)
, mLookAtOffset(gpTweakBall->GetBallCameraOffset())
, mLookPosAhead(0.f, 0.f, 0.f)
, mFixedLookPos(CVector3f::Zero())
, mLookPos(0.f, 0.f, 0.f)
, mNextLookXf(CTransform4f::Identity())
, mBallCameraSpring(gpTweakBall->GetBallCameraSpringConstant(),
                    gpTweakBall->GetBallCameraSpringMax(), gpTweakBall->GetBallCameraSpringTardis())
, mBallCameraCentroidSpring(gpTweakBall->GetBallCameraCentroidSpringConstant(),
                            gpTweakBall->GetBallCameraCentroidSpringMax(),
                            gpTweakBall->GetBallCameraCentroidSpringTardis())
, mBallCameraLookAtSpring(gpTweakBall->GetBallCameraLookAtSpringConstant(),
                          gpTweakBall->GetBallCameraLookAtSpringMax(),
                          gpTweakBall->GetBallCameraLookAtSpringTardis() * 1.1f)
, mBallCameraCentroidDistanceSpring(gpTweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                                    gpTweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                    gpTweakBall->GetBallCameraCentroidDistanceSpringTardis())
, mSmallColliders()
, mMediumColliders()
, mLargeColliders()
, mAvoidGeomCycle(0)
, mColliderMag(1.f)
, mDampedPos(0.f, 0.f, 0.f)
, x3b0_(0)
, x3b4_(0)
, x3b8_(0)
, mPrevBallPos(0.f, 0.f, 0.f)
, mBallVelFlat(0.f)
, mMaxBallVel(0.f)
, mBallDelta(CVector3f::Zero())
, mBallDeltaFlat(CVector3f::Zero())
, mSpeedFactor(0.f)
, mSpeedingTime(0.f)
, mCollidersAABB(CAABox::Identity())
, mObscuredTime(0.f)
, mObscuringMaterial(kMT_NoStepLogic)
, mUnobscureMag(0.f)
, mSplineIntermediatePos(CVector3f::Zero())
, mObscuringObjectId(kInvalidUniqueId)
, mSplineState(kBSS_Invalid)
, mReevalSplineEnd(false)
, mSplineCtrl(0.f)
, mCamSpline(false, 1.f, CMotionSpline::kST_Bezier)
, mCollisionExcludeList(kMT_NoStepLogic)
, mCamBehindFloorOrWall(false)
, mSplineEndPosition(CVector3f::Zero())
, x498_(0.f)
, mElevInterpTimer(0.f)
, mElevInterpStart(0.f)
, mTooCloseActorId(kInvalidUniqueId)
, mTooCloseActorDist(10000.f)
, mPendingFailsafe(false)
, x4b0_(0.f)
, mFreeLookYawDelta(0.f)
, mFreeLookPitchDelta(0.f)
, mFreeLookDistance(2.f)
, mFreeLookZoomOutInput(0.f)
, mFreeLookZoomInInput(0.f)
, mState(kBCS_Default)
, mChaseDistance(gpTweakBall->GetBallCameraChaseDistance())
, mChaseYawSpeed(gpTweakBall->GetBallCameraChaseYawSpeed())
, mChaseAnglePerSecond(gpTweakBall->GetBallCameraChaseAnglePerSecond())
, mBallCameraChaseSpring(gpTweakBall->GetBallCameraChaseSpringConstant(),
                         gpTweakBall->GetBallCameraChaseSpringMax(),
                         gpTweakBall->GetBallCameraChaseSpringTardis())
, mBoostDistance(gpTweakBall->GetBallCameraBoostDistance())
, mBoostYawSpeed(gpTweakBall->GetBallCameraBoostYawSpeed())
, mBoostAnglePerSecond(gpTweakBall->GetBallCameraBoostAnglePerSecond())
, mBoostLookAtOffset(gpTweakBall->GetBallCameraBoostLookAtOffset())
, mBallCameraBoostSpring(gpTweakBall->GetBallCameraBoostSpringConstant(),
                         gpTweakBall->GetBallCameraBoostSpringMax(),
                         gpTweakBall->GetBallCameraBoostSpringTardis())
, mOverrideBallToCam(CVector3f::Zero())
, mHintLocalOffset(CVector3f::Zero())
, mConservativeDoorCamDistance(gpTweakBall->GetBallCameraConfinedDistance())
, mCollisionActorId(kInvalidUniqueId)
, mClampVelTimer(0.f)
, mClampVelRange(10.f)
, mShortMoveCount(0)
, mFromBallTransition(rs_new SFromBallTransition)
, mToBallTransition(rs_new SToBallTransition)
, mInitialForward(xf.GetForward())
, mFixedTransform(xf)
, mDesiredPosition(xf.GetTranslation()) {
  mSmallColliders.SetupColliders(7.f * 0.33f, 7.f * 0.33f, 0.1f, 3, -M_PIF / 2.f);
  mMediumColliders.SetupColliders(7.f * 0.66f, 7.f * 0.66f, 0.1f, 6, -M_PIF / 2.f);
  mLargeColliders.SetupColliders(7.f, 7.f, 0.1f, 12, -M_PIF / 2.f);
}

CBallCamera::~CBallCamera() {}

void CBallCamera::TeleportCamera(const CVector3f& position, CStateManager& mgr) {
  mDampedPos = position;
  mSmallColliders.TeleportColliders(position);
  mMediumColliders.TeleportColliders(position);
  mLargeColliders.TeleportColliders(position);
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionActorId))) {
    actor->SetTranslation(position);
  }
}

void CBallCamera::TeleportLookAtStuff(CStateManager& mgr) {
  UpdateLookAtPosition(0.01f, mgr, true);
}

void CBallCamera::TeleportCamera(const CTransform4f& xf, CStateManager& mgr) {
  SetTransform(xf);
  TeleportCamera(xf.GetTranslation(), mgr);
  CameraManager(mgr).UpdateCameraTriggers(GetUniqueId(), mgr);
}

void CBallCamera::ResetToTweaks(CStateManager& mgr) {
  mBehaviour = kBCB_Default;
  mChaseAllowed = true;
  mBoostAllowed = true;
  mObscureAvoidance = true;
  mVolumeCollider = true;
  mClampAttitude = false;
  mClampAzimuth = false;
  mTargetMinDistance = gpTweakBall->GetBallCameraMinSpeedDistance();
  mMaxDistance = gpTweakBall->GetBallCameraMaxSpeedDistance();
  mBackwardsDistance = gpTweakBall->GetBallCameraBackwardsDistance();
  mBallCameraSpring = CCameraSpring(gpTweakBall->GetBallCameraSpringConstant(),
                                    gpTweakBall->GetBallCameraSpringMax(),
                                    gpTweakBall->GetBallCameraSpringTardis());
  mBallCameraCentroidDistanceSpring =
      CCameraSpring(gpTweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                    gpTweakBall->GetBallCameraCentroidDistanceSpringMax(),
                    gpTweakBall->GetBallCameraCentroidDistanceSpringTardis());
  mLookAtOffset = gpTweakBall->GetBallCameraOffset();
  mElevation = 2.736f;
  mAttitudeRange = M_PIF / 2.f;
  mAzimuthRange = M_PIF / 2.f;
  InterpolateFOV(CCameraManager::GetDefaultThirdPersonVerticalFOV(), 1.f, 0.f);
  mTargetAnglePerSecond = gpTweakBall->GetBallCameraAnglePerSecond();
  mNoElevationInterp = false;
  mDirectElevation = false;
  mOverrideLookDir = false;
  mNoElevationVelClamp = false;
  mNoSpline = false;
  x206_26_ = false;
  SetWatchedObject(Player(mgr).GetUniqueId());
}

void CBallCamera::Render(const CStateManager& mgr) const {}

void CBallCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  ResetToTweaks(mgr);
  mBallCameraSpring.Reset();
  mBallCameraCentroidSpring.Reset();
  mBallCameraLookAtSpring.Reset();
  mBallCameraCentroidDistanceSpring.Reset();
  mBallCameraChaseSpring.Reset();
  mBallCameraBoostSpring.Reset();

  CVector3f position =
      FindDesiredPosition(mCurMinDistance, mElevation, xf.GetForward(), mgr, false);
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()))) {
    TeleportLookAtStuff(mgr);
    CVector3f direction = mLookPos - position;
    direction.SetZ(0.f);
    if (direction.IsMagnitudeSafe()) {
      TeleportCamera(CTransform4f::LookAt(position, mLookPos), mgr);
    } else {
      CTransform4f cameraXf = player->CreateTransformFromMovementDirection();
      cameraXf.SetTranslation(position);
      TeleportCamera(cameraXf, mgr);
      CameraManager(mgr).SetPlayerCamera(mgr, GetUniqueId());
      ResetFovInterpolation(GetTargetFov());
    }

    mBallVelFlat = 0.f;
    mMaxBallVel = 0.f;
    mCurMinDistance = mTargetMinDistance;
    mBallDeltaFlat = CVector3f::Zero();
    mBallDelta = CVector3f::Zero();
    mObtuseDirection = false;
    mSpeedFactor = 0.f;
    mPrevBallPos = player->GetBallPosition();
    mDampedPos = GetTranslation();
    x3b0_ = 0;
    x3b4_ = 0;
    x3b8_ = 0;
    mColliderMag = 1.f;
    InvalidateSpline();
    mAvoidGeometryFull = true;
    mForceProcessing = true;
    Think(0.1f, mgr);
    mAvoidGeometryFull = false;
    mForceProcessing = false;
  }
}

CVector3f CBallCamera::ApplyColliders() {
  float centroidX = 0.f;
  float centroidZ = 0.f;
  if (mSmallColliders.GetCentroid().GetY() == 0.f) {
    centroidX = mSmallColliders.GetCentroid().GetX();
    centroidZ = mSmallColliders.GetCentroid().GetZ();
  }
  if (mMediumColliders.GetCentroid().GetY() == 0.f) {
    centroidX += mMediumColliders.GetCentroid().GetX();
    centroidZ += mMediumColliders.GetCentroid().GetZ();
  }
  if (mLargeColliders.GetCentroid().GetY() == 0.f) {
    centroidX += mLargeColliders.GetCentroid().GetX();
    centroidZ += mLargeColliders.GetCentroid().GetZ();
  }

  if (mClearLOS) {
    centroidX /= 3.f;
  }
  centroidZ /= 3.f;

  if (!mClearLOS && mObscuringObjectId == kInvalidUniqueId) {
    float xScale = 1.5f;
    float zScale = 1.f;
    if (mObscuringMaterial.HasMaterial(kMT_Floor)) {
      zScale += 2.f * mUnobscureMag;
    }
    if (mObscuringMaterial.HasMaterial(kMT_Wall)) {
      xScale += 3.f * CMath::Clamp(0.f, mUnobscureMag - 0.25f, 1.f);
    }
    centroidX *= xScale;
    centroidZ *= zScale;
  }

  if (!mVolumeCollider) {
    return CVector3f::Zero();
  }
  if (CMath::AbsF(centroidX) < 0.05f) {
    centroidX = 0.f;
  }
  if (CMath::AbsF(centroidZ) < 0.05f) {
    centroidZ = 0.f;
  }
  if (mClearLOS) {
    centroidZ *= 4.f;
    if (centroidZ > 0.f) {
      centroidZ = 0.f;
    }
  }
  return CVector3f(CMath::Limit(centroidX, 3.5f), 0.f, CMath::Limit(centroidZ, 4.f));
}

CVector3f CBallCamera::AvoidGeometryFull(const CTransform4f& xf,
                                         const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                         CStateManager& mgr) {
  mSmallColliders.UpdateColliders(xf, GetPlayer(mgr).GetBallPosition(), 1, 4.f, nearList, mgr);
  mMediumColliders.UpdateColliders(xf, GetPlayer(mgr).GetBallPosition(), 3, 4.f, nearList, mgr);
  mLargeColliders.UpdateColliders(xf, GetPlayer(mgr).GetBallPosition(), 4, 4.f, nearList, mgr);
  return ApplyColliders();
}

CVector3f CBallCamera::AvoidGeometry(const CTransform4f& xf,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                     CStateManager& mgr) {
  switch (mAvoidGeomCycle) {
  case 0:
    mSmallColliders.UpdateColliders(xf, GetPlayer(mgr).GetBallPosition(), 1, 4.f, nearList, mgr);
    break;
  case 1:
    mMediumColliders.UpdateColliders(xf, GetPlayer(mgr).GetBallPosition(), 3, 4.f, nearList, mgr);
    break;
  case 2:
  case 3:
    mLargeColliders.UpdateColliders(xf, GetPlayer(mgr).GetBallPosition(), 4, 4.f, nearList, mgr);
    break;
  }

  if (++mAvoidGeomCycle > 3) {
    mAvoidGeomCycle = 0;
  }
  return ApplyColliders();
}

bool CBallCamera::DetectCollision(const CVector3f& from, const CVector3f& to, float radius,
                                  float& distance, const CStateManager& mgr, int controllerIdx) {
  const CVector3f delta = to - from;
  const float length = delta.Magnitude();
  const CVector3f direction = (1.f / length) * delta;
  if (length > 1.1920929e-6f) {
    const float margin = 2.f * radius;
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    bounds.AccumulateBounds(from);
    bounds.AccumulateBounds(to);
    bounds = CAABox(bounds.GetMinPoint() - CVector3f(margin, margin, margin),
                    bounds.GetMaxPoint() + CVector3f(margin, margin, margin));

    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildColliderList(nearList, *mgr.GetPlayer(controllerIdx), bounds);
    CAreaCollisionCache cache(bounds);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    const CCollidableSphere sphere(CSphere(CVector3f::Zero(), radius),
                                   CMaterialList(kMT_Unknown59));
    const CTransform4f startTransform = CTransform4f::Translate(from);
    if (CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, sphere, startTransform,
                                                      skLineOfSightFilter, nearList)) {
      distance = -1.f;
      return true;
    }

    if (cache.HasCacheOverflowed()) {
      return true;
    }
    {
      const uint stepCount = static_cast< uint >(length / 0.5f);
      const CVector3f step = (1.f / float(stepCount)) * delta;
      CTransform4f testTransform = startTransform;
      for (uint i = 0; i < stepCount; ++i) {
        TUniqueId hitId = kInvalidUniqueId;
        CCollisionInfo hitInfo;
        double hitDistance = step.Magnitude();
        if (CGameCollision::DetectCollision_Cached_Moving(mgr, cache, sphere, testTransform,
                                                          skLineOfSightFilter, nearList, direction,
                                                          hitId, hitInfo, hitDistance)) {
          distance = float(hitDistance + i * step.Magnitude());
          return true;
        }
        testTransform.SetTranslation(testTransform.GetTranslation() + step);
      }
    }
  }
  return false;
}

bool CBallCamera::fn_801a6b20(const CVector3f& from, const CVector3f& direction, CVector3f& result,
                              CStateManager& mgr) {
  const CRelAngle step = CRelAngle::FromDegrees(30.f);
  const CTransform4f negativeRotation =
      CQuaternion::ZRotation(CRelAngle::FromRadians(-step.AsRadians())).BuildTransform4f();
  const CTransform4f positiveRotation = CQuaternion::ZRotation(step).BuildTransform4f();
  CVector3f negativeDirection = negativeRotation * direction;
  CVector3f positiveDirection = positiveRotation * direction;
  const float desiredDistance = direction.Magnitude();
  for (int i = 0; i < 6; ++i) {
    float collisionDistance = negativeDirection.Magnitude();
    if (!DetectCollision(from, from + negativeDirection, 0.3f, collisionDistance, mgr,
                         GetControllerNumber()) ||
        desiredDistance <= collisionDistance) {
      result = collisionDistance * negativeDirection.AsNormalized();
      return true;
    }
    collisionDistance = positiveDirection.Magnitude();
    if (!DetectCollision(from, from + positiveDirection, 0.3f, collisionDistance, mgr,
                         GetControllerNumber()) ||
        desiredDistance < collisionDistance) {
      result = collisionDistance * positiveDirection.AsNormalized();
      return true;
    }
    negativeDirection = negativeRotation * negativeDirection;
    positiveDirection = positiveRotation * positiveDirection;
  }
  return false;
}

bool CBallCamera::fn_801a67a4(float radius, const CVector3f& from, const CVector3f& direction,
                              const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                              CVector3f& result, CStateManager& mgr) {
  const CRelAngle step = CRelAngle::FromDegrees(30.f);
  const CTransform4f negativeRotation =
      CQuaternion::ZRotation(CRelAngle::FromRadians(-step.AsRadians())).BuildTransform4f();
  const CTransform4f positiveRotation = CQuaternion::ZRotation(step).BuildTransform4f();
  float distance = direction.Magnitude();
  while (distance >= radius) {
    const CVector3f sought = distance * direction.AsNormalized();
    CVector3f negativeDirection = negativeRotation * sought;
    CVector3f positiveDirection = positiveRotation * sought;
    for (int i = 0; i < 6; ++i) {
      if (mgr.RayCollideWorld(from, from + negativeDirection, nearList, skLineOfSightFilter,
                              nullptr)) {
        result = negativeDirection;
        return true;
      }
      if (mgr.RayCollideWorld(from, from + positiveDirection, nearList, skLineOfSightFilter,
                              nullptr)) {
        result = positiveDirection;
        return true;
      }
      negativeDirection = negativeRotation * negativeDirection;
      positiveDirection = positiveRotation * positiveDirection;
    }
    distance -= 0.3f;
  }
  return false;
}

CVector3f CBallCamera::FindDesiredPosition(float distance, float elevation, CVector3f direction,
                                           CStateManager& mgr, bool fullTest) {
  const CActor* watched = TCastToConstPtr< CActor >(mgr.GetObjectById(GetWatchedObject()));
  if (watched == nullptr) {
    return GetTranslation();
  }

  CVector3f ballPos = watched->GetOrbitPosition(mgr);
  const CPlayer* watchedPlayer = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()));
  if (watchedPlayer != nullptr) {
    ballPos = watchedPlayer->GetBallPosition();
  }
  if (!direction.IsMagnitudeSafe()) {
    direction = CVector3f(0.f, 1.f, 0.f);
  }
  const CTransform4f lookRotation = CTransform4f::LookAt(CVector3f::Zero(), direction);

  float constrainedDistance = distance;
  float constrainedElevation = elevation;
  ConstrainElevationAndDistance(constrainedElevation, constrainedDistance, 0.f, mgr);
  CVector3f eyePos = Player(mgr).GetEyePosition();
  if (watchedPlayer != nullptr &&
      watchedPlayer->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    eyePos = mLookPosAhead;
  }
  if (!mgr.RayCollideWorld(ballPos, eyePos, skLineOfSightFilter, nullptr)) {
    eyePos = ballPos;
  }

  CVector3f desiredOffset(0.f, -constrainedDistance,
                          constrainedElevation - (eyePos.GetZ() - ballPos.GetZ()));
  desiredOffset = lookRotation.GetRotation() * desiredOffset;
  CVector3f resultOffset(0.f, distance, constrainedElevation - (eyePos.GetZ() - ballPos.GetZ()));
  float collisionDistance = desiredOffset.Magnitude();
  const bool clear = !DetectCollision(eyePos, eyePos + desiredOffset, 0.3f, collisionDistance, mgr,
                                      GetControllerNumber());
  bool found = false;
  float minSeekDistance = constrainedDistance;

  if (clear || collisionDistance > 0.f) {
    bool movingForward = false;
    if (mBallVelFlat > 1.25f && mBallDeltaFlat.IsMagnitudeSafe() && watchedPlayer != nullptr &&
        (watchedPlayer->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed ||
         watchedPlayer->GetMorphballTransitionState() == CPlayer::kMS_Morphing)) {
      movingForward =
          CVector3f::Dot(mBallDeltaFlat.AsNormalized(), watched->GetTransform().GetForward()) > 0.f;
    }
    if (clear || (!fullTest && (collisionDistance > 0.95f * distance || movingForward))) {
      if (movingForward) {
        resultOffset = desiredOffset;
      } else {
        resultOffset = collisionDistance * desiredOffset.AsNormalized();
      }
      found = true;
    } else {
      found = fn_801a6b20(eyePos, desiredOffset, resultOffset, mgr);
      if (!found) {
        CVector3f flatOffset = desiredOffset;
        flatOffset.SetZ(0.f);
        found = fn_801a6b20(eyePos, flatOffset, resultOffset, mgr);
      }
      if (!found) {
        CVector3f reflectedOffset = desiredOffset;
        reflectedOffset.SetZ(-reflectedOffset.GetZ());
        found = fn_801a6b20(eyePos, reflectedOffset, resultOffset, mgr);
      }
      minSeekDistance = collisionDistance;
    }
  }

  if (!found) {
    const CAABox bounds(ballPos.GetX() - distance, ballPos.GetY() - distance,
                        ballPos.GetZ() - constrainedElevation, ballPos.GetX() + distance,
                        ballPos.GetY() + distance, ballPos.GetZ() + constrainedElevation);
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    const CActor* ignored = TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId));
    mgr.BuildNearList(nearList, bounds, skLineOfSightFilter, ignored);
    found = fn_801a67a4(minSeekDistance, eyePos, desiredOffset, nearList, resultOffset, mgr);
    if (!found) {
      CVector3f flatOffset = desiredOffset;
      flatOffset.SetZ(0.f);
      found = fn_801a67a4(minSeekDistance, eyePos, flatOffset, nearList, resultOffset, mgr);
    }
    if (!found) {
      CVector3f reflectedOffset = desiredOffset;
      reflectedOffset.SetZ(-reflectedOffset.GetZ());
      found = fn_801a67a4(minSeekDistance, eyePos, reflectedOffset, nearList, resultOffset, mgr);
    }
  }

  mDesiredPosition =
      found ? eyePos + resultOffset : CameraManager(mgr).GetLastCameraTransform().GetTranslation();
  return mDesiredPosition;
}

CTransform4f CBallCamera::FindDesiredTransform(CVector3f direction, CStateManager& mgr) {
  if (!direction.IsMagnitudeSafe()) {
    direction = CVector3f(0.f, 1.f, 0.f);
  }
  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  CVector3f position = FindDesiredPosition(distance, elevation, direction, mgr, false);
  UpdateLookAtPosition(0.f, mgr, false);
  return CTransform4f::LookAt(position, mLookPos);
}

void CBallCamera::UpdateObjectTooCloseId(CStateManager& mgr) {
  mTooCloseActorDist = 1000000.f;
  mTooCloseActorId = kInvalidUniqueId;

  const CPlayer& player = GetPlayer(mgr);
  const CVector3f ballPosition = player.GetBallPosition();
  const rstl::list< CEntity* >& doors = mgr.GetDoorList();
  for (rstl::list< CEntity* >::const_iterator it = doors.begin(); it != doors.end(); ++it) {
    const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(*it);
    if (door == nullptr || door->GetCurrentAreaId() != player.GetCurrentAreaId() ||
        door->IsHorizontal()) {
      continue;
    }

    const CVector3f& doorPosition = door->GetTranslation();
    const float cameraDist = (doorPosition - GetTranslation()).MagSquared();
    const float playerDist = (doorPosition - ballPosition).MagSquared();
    const float distance = CMath::Min(cameraDist, playerDist);
    if (distance < 900.f && distance < mTooCloseActorDist) {
      mTooCloseActorId = door->GetUniqueId();
      mTooCloseActorDist = distance;
    }
  }
  if (mTooCloseActorId != kInvalidUniqueId) {
    mTooCloseActorDist = CMath::SqrtF(mTooCloseActorDist);
  }
}

bool CBallCamera::ConstrainElevationAndDistance(float& elevation, float& distance, float dt,
                                                CStateManager& mgr) {
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
  if (hint != nullptr && (hint->GetInfo().GetFlags() & 0x800000) != 0) {
    return false;
  }

  const CPlayer& player = GetPlayer(mgr);
  if (GetWatchedObject() != player.GetUniqueId()) {
    return false;
  }

  const CVector3f ballToCamera = GetTranslation() - player.GetBallPosition();
  float currentDistance = 0.f;
  if (ballToCamera.IsMagnitudeSafe()) {
    currentDistance = CVector2f(ballToCamera.GetX(), ballToCamera.GetY()).Magnitude();
  }

  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  bool nearDoor = false;
  float stretch = 1.f;
  float targetDistance = distance;
  float baseElevation = elevation;
  float springScale = 1.f;
  if (door != nullptr && !door->IsBallDoor()) {
    stretch = CMath::Limit(CMath::AbsF(mTooCloseActorDist / (3.f * distance)), 1.f);
    nearDoor = mTooCloseActorDist < 3.f * distance;
    if (door->IsOpen()) {
      targetDistance =
          stretch * (distance - mConservativeDoorCamDistance) + mConservativeDoorCamDistance;
    } else {
      targetDistance = stretch * (distance - 5.f) + 5.f;
    }
    if (mObtuseDirection) {
      targetDistance *= 1.f + mSpeedFactor;
    }
    baseElevation = door->IsOpen() ? 0.75f : 1.5f;
    springScale = 4.f;
  }

  distance =
      mBallCameraSpring.ApplyDistanceSpring(targetDistance, currentDistance, dt * springScale);
  elevation = (elevation - baseElevation) * stretch + baseElevation;
  return nearDoor;
}

CVector3f CBallCamera::ConstrainYawAngle(const CPlayer& player, float yawSpeed, float dampenAngle,
                                         float dt, CStateManager& mgr) {
  CVector3f playerToCamera = GetTranslation() - player.GetTranslation();
  playerToCamera.SetZ(0.f);

  CVector3f lookDir = player.GetTransform().GetForward();
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    lookDir = player.GetMovementDirection();
    const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
    if ((door == nullptr || !door->IsOpen()) &&
        (mState == kBCS_Boost || mState == kBCS_Chase || mBehaviour == kBCB_FreezeLookPosition)) {
      lookDir = player.GetLeaveMorphDirection();
    }
  } else if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
    lookDir = player.GetLeaveMorphDirection();
  }

  lookDir.SetZ(0.f);
  if (lookDir.IsMagnitudeSafe()) {
    lookDir.Normalize();
  } else {
    lookDir = -playerToCamera;
  }
  if (!playerToCamera.IsMagnitudeSafe()) {
    return -lookDir;
  }
  playerToCamera.Normalize();

  float dot = CMath::Limit(CVector3f::Dot(playerToCamera, -lookDir), 1.f);
  const float angle = acosf(dot);
  if (dot >= 0.99999f) {
    return -lookDir;
  }

  float rotation = yawSpeed * dt;
  if (x204_24_) {
    rotation *= CMath::Clamp(0.f, 1.f - mSpeedFactor, 1.f);
  }
  rotation *= CMath::Clamp(0.f, angle / dampenAngle, 1.f);
  const CQuaternion quat = CQuaternion::LookAt(
      CUnitVector3f(playerToCamera), CUnitVector3f(-lookDir), CRelAngle::FromRadians(rotation));
  return quat.Transform(playerToCamera);
}

void CBallCamera::UpdateTransform(const CVector3f& lookDirection, const CVector3f& position,
                                  float dt, CStateManager& mgr) {
  CVector3f desiredLook = lookDirection;
  if (mOverrideLookDir) {
    const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
        CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
    if (hint != nullptr) {
      desiredLook = hint->GetTransform().GetForward();
    }
  }

  CVector3f flatLook = desiredLook;
  flatLook.SetZ(0.f);
  if (!flatLook.IsMagnitudeSafe()) {
    SetTranslation(position);
    return;
  }

  CVector3f currentLook = GetTransform().GetForward();
  if (!currentLook.IsMagnitudeSafe()) {
    SetTransform(CTransform4f::LookAt(position, position + desiredLook, CVector3f::Up()));
    return;
  }
  currentLook.Normalize();

  const float dot = CMath::Limit(CVector3f::Dot(currentLook, desiredLook), 1.f);
  if (CMath::AbsF(dot) >= 0.99999988f) {
    SetTransform(CTransform4f::LookAt(position, position + desiredLook, CVector3f::Up()));
  } else {
    const float speedFactor = CMath::Clamp(0.f, acosf(dot) / (1.0471976f * dt), 1.f);
    float angle = dt * (mCurAnglePerSecond * speedFactor);
    const float upDot = CMath::Limit(CVector3f::Dot(desiredLook, CVector3f::Up()), 1.f);
    const float absUpDot = CMath::AbsF(upDot);
    float maxAngle = (12.566371f * dt) * (1.f - absUpDot);
    if (mSplineState == kBSS_One) {
      maxAngle = 4.1887903f * dt;
      if (angle > maxAngle) {
        angle = maxAngle;
      }
    }
    if (angle > maxAngle && !Player(mgr).IsMorphBallTransitioning() && absUpDot > 0.999f) {
      angle = maxAngle;
    }
    if (mState == kBCS_Chase && mChaseAllowed) {
      angle = dt * (mChaseAnglePerSecond * speedFactor);
    } else if (mState == kBCS_Boost) {
      angle = dt * (mBoostAnglePerSecond * speedFactor);
    }

    if (mLookAtBall || CameraManager(mgr).IsInterpolationCameraActive()) {
      mLookAtBall = false;
      angle = 6.2831855f;
    }
    const CQuaternion rotation = CQuaternion::LookAt(
        CUnitVector3f(currentLook), CUnitVector3f(desiredLook), CRelAngle::FromRadians(angle));
    SetTransform(rotation.BuildTransform4f() * GetTransform().GetRotation());
  }
  SetTranslation(position);
}

void CBallCamera::UpdatePlayerMovement(float dt, CStateManager& mgr) {
  const CPlayer& player = Player(mgr);
  mMaxBallVel = CMath::AbsF(player.GetActualBallMaxVelocity(dt));
  CVector3f ballPos = player.GetBallPosition();
  mBallDelta = ballPos - mPrevBallPos;
  mBallDeltaFlat = mBallDelta;
  mBallDeltaFlat.SetZ(0.f);
  const CVector3f& velocity = player.GetVelocityWR();
  mBallVelFlat = CVector2f(velocity.GetX(), velocity.GetY()).Magnitude();
  mMaxBallVel = gpTweakBall->GetBallTranslationMaxSpeed(CPlayer::kSR_Normal);
  if (!mBallDeltaFlat.IsMagnitudeSafe() || mBallDeltaFlat.Magnitude() < dt) {
    mBallVelFlat = 0.f;
  }
  mPrevBallPos = ballPos;

  mObtuseDirection = false;
  CVector3f camToBallFlat = ballPos - GetTranslation();
  camToBallFlat.SetZ(0.f);
  if (camToBallFlat.IsMagnitudeSafe()) {
    camToBallFlat.Normalize();
    float dot = CMath::Limit(CVector3f::Dot(camToBallFlat, player.GetMovementDirection()), 1.f);
    mObtuseDirection = CMath::AbsF(CMath::FastArcCosR(dot)) > 1.7453293f;
  }

  mSpeedFactor = CMath::Clamp(0.f, mBallVelFlat / mMaxBallVel, 1.f);
  mCurMinDistance = mTargetMinDistance + mSpeedFactor * (mMaxDistance - mTargetMinDistance);
  if (mSpeedFactor > 0.25f && (player.GetPlayerMovementState() == NPlayer::kMS_OnGround ||
                               player.GetPlayerMovementState() == NPlayer::kMS_FallingMorphed)) {
    mSpeedingTime += dt * mSpeedFactor;
  } else {
    mSpeedingTime = 0.f;
  }
  mSpeedingTime = CMath::Clamp(0.f, mSpeedingTime, 3.f);
}

CVector3f CBallCamera::InterpolateCameraElevation(CVector3f position, float dt) {
  if (mElevation >= 2.f) {
    if (!mClearLOS && mObscuringMaterial.HasMaterial(kMT_Floor)) {
      mElevInterpTimer = 1.f;
      mElevInterpStart = GetTranslation().GetZ();
      position.SetZ(mElevInterpStart);
    } else if (mElevInterpTimer > 0.f) {
      mElevInterpTimer -= dt;
      float t = 1.f - CMath::Clamp(0.f, mElevInterpTimer, 1.f);
      position.SetZ((position.GetZ() - mElevInterpStart) * t + mElevInterpStart);
    }
  }
  return position;
}

bool CBallCamera::ShouldResetSpline(CStateManager& mgr) const {
  if (mState == kBCS_ToBall ||
      Player(mgr).GetMorphBall()->GetBallState() == CMorphBall::kBS_Spider ||
      mSplineState != kBSS_Invalid) {
    return false;
  }
  switch (mBehaviour) {
  case kBCB_Unknown4:
  case kBCB_Unknown5:
  case kBCB_Unknown6:
  case kBCB_Unknown7:
  case kBCB_Unknown8:
  case kBCB_Unknown9:
  case kBCB_FixedTransform:
    return false;
  default:
    return true;
  }
}

void CBallCamera::BuildSpline(CStateManager& mgr) {
  const CPlayer& player = Player(mgr);
  const CVector3f ballPos = player.GetBallPosition();
  TUniqueId intersectId = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CVector3f down(0.f, 0.f, -1.f);
  const CRayCastResult downHit =
      mgr.RayWorldIntersection(intersectId, ballPos, down, 20.f, skLineOfSightFilter, nearList);
  const float downFactor =
      downHit.IsValid() ? CMath::Clamp(0.f, downHit.GetTime() / 20.f, 1.f) : 1.f;

  mSplineState = kBSS_One;
  mReevalSplineEnd = true;
  mCamBehindFloorOrWall = false;
  mCamSpline.Reset(4);
  mCamSpline.AddKnotAndControlPoint(GetTranslation());

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  CVector3f knot1 = mSplineIntermediatePos;
  knot1.SetZ(GetTranslation().GetZ());
  mCamSpline.AddKnotAndControlPoint(knot1);

  const CVector3f delta = (0.5f + downFactor) * (mSplineIntermediatePos - GetTranslation());
  CVector3f knot2 = knot1 + delta;
  const float firstLength = delta.Magnitude();
  if (delta.CanBeNormalized()) {
    mgr.BuildNearList(nearList, knot1, delta.AsNormalized(), firstLength, skLineOfSightFilter,
                      nullptr);
    const CRayCastResult hit = mgr.RayWorldIntersection(intersectId, knot1, delta.AsNormalized(),
                                                        firstLength, skLineOfSightFilter, nearList);
    if (hit.IsValid()) {
      knot2 = hit.GetPoint();
      const CActor* hitActor = TCastToConstPtr< CActor >(mgr.GetObjectByIdFromListAll(intersectId));
      if ((intersectId == kInvalidUniqueId && hit.GetMaterial().HasMaterial(kMT_Floor)) ||
          (hitActor != nullptr && hitActor->GetMaterialList().HasMaterial(kMT_Floor))) {
        knot2.SetZ(knot2.GetZ() + elevation);
      }
    }
  }
  mCamSpline.AddKnotAndControlPoint(knot2);

  CVector3f knot3(knot2.GetX() - downFactor * delta.GetX(),
                  knot2.GetY() - downFactor * delta.GetY(),
                  knot2.GetZ() + (0.25f + downFactor) * delta.GetZ());
  const CVector3f secondDelta = knot3 - knot2;
  const float secondLength = secondDelta.Magnitude();
  if (secondDelta.CanBeNormalized()) {
    nearList.clear();
    mgr.BuildNearList(nearList, knot2, secondDelta.AsNormalized(), secondLength,
                      skLineOfSightFilter, nullptr);
    const CRayCastResult hit =
        mgr.RayWorldIntersection(intersectId, knot2, secondDelta.AsNormalized(), secondLength,
                                 skLineOfSightFilter, nearList);
    if (hit.IsValid()) {
      knot3 = hit.GetPoint();
      const CActor* hitActor = TCastToConstPtr< CActor >(mgr.GetObjectByIdFromListAll(intersectId));
      if ((intersectId == kInvalidUniqueId && hit.GetMaterial().HasMaterial(kMT_Floor)) ||
          (hitActor != nullptr && hitActor->GetMaterialList().HasMaterial(kMT_Floor))) {
        knot3.SetZ(knot3.GetZ() + elevation);
      }
    }
  }
  mCamSpline.AddKnotAndControlPoint(knot3);

  CVector3f toBall = ballPos - knot3;
  toBall.SetZ(0.f);
  if (toBall.IsMagnitudeSafe()) {
    toBall.Normalize();
  } else {
    toBall = player.GetMovementDirection();
  }
  FindDesiredPosition(distance, elevation, toBall, mgr, false);
  mCamSpline.CalculateLength();

  CMaterialList hitMaterial;
  mCamBehindFloorOrWall = false;
  mCollisionExcludeList = CMaterialList(kMT_Floor, kMT_Ceiling);
  if (!SplineIntersectTest(hitMaterial, mgr) &&
      (hitMaterial.HasMaterial(kMT_Floor) || hitMaterial.HasMaterial(kMT_Wall))) {
    CVector3f adjustedKnot2 = knot1;
    adjustedKnot2.SetZ(knot2.GetZ());
    mCamSpline.SetKnotAndControlPoint(2, adjustedKnot2, true);
    if (!SplineIntersectTest(hitMaterial, mgr) && hitMaterial.HasMaterial(kMT_Floor)) {
      mCamBehindFloorOrWall = true;
      mCollisionExcludeList = CMaterialList();
    }
  }

  mSplineCtrl = 0.5f * downFactor + 0.5f;
  mSplineCtrlRange = mSplineCtrl;
  mCamSpline.SetDuration(mSplineCtrlRange);
  mSplineEndPosition = mCamSpline.GetKnot(mCamSpline.GetKnotCount() - 1);
  x498_ = 0.f;
}

void CBallCamera::UpdateUsingSpline(float dt, CStateManager& mgr) {
  if (mState == kBCS_ToBall || mState == kBCS_FromBall) {
    mSplineState = kBSS_Invalid;
    return;
  }
  if (mSplineState == kBSS_One && ((mBehaviour >= kBCB_Unknown4 && mBehaviour <= kBCB_Unknown9) ||
                                   mBehaviour == kBCB_FixedTransform)) {
    mSplineState = kBSS_Invalid;
    return;
  }

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  const CVector3f ballPos = Player(mgr).GetBallPosition();
  CVector3f direction = ballPos - GetTranslation();
  direction.SetZ(0.f);
  if (direction.IsMagnitudeSafe()) {
    direction.Normalize();
  } else {
    direction = Player(mgr).GetMovementDirection();
  }

  const CVector3f endPosition = mCamSpline.GetKnot(mCamSpline.GetKnotCount() - 1);
  CVector3f desiredPosition = FindDesiredPosition(distance, elevation, direction, mgr, false);
  mSplineCtrl -= dt;
  const float remaining = CMath::Clamp(0.f, mSplineCtrl / mSplineCtrlRange, 1.f);
  const float progress = 1.f - remaining;
  if (mCamBehindFloorOrWall && !close_enough(desiredPosition, GetTranslation(), 0.1f)) {
    desiredPosition += remaining * (GetTranslation() - desiredPosition);
  }

  if (mSplineCtrl <= 0.f || (progress > 0.95f && mClearLOS)) {
    mSplineState = kBSS_Invalid;
    const CTransform4f previous = GetTransform();
    const CTransform4f desired = FindDesiredTransform(direction, mgr);
    TeleportCamera(desired, mgr);
    CameraManager(mgr).SetupInterpolation(
        previous, GetUniqueId(), GetUniqueId(), false, CInterpolationCamera::kPM_Direct,
        CInterpolationCamera::kRM_LinearSlerp, mgr, true, 0.5f, GetFov());
    return;
  }

  const float splineLength = progress * mCamSpline.GetLength();
  CVector3f cameraPos = mCamSpline.GetPositionByLength(splineLength);
  const CCollisionActor* collisionActor =
      TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mCollisionActorId));
  if (collisionActor != nullptr) {
    const CMaterialFilter previousFilter = collisionActor->GetMaterialFilter();
    CMaterialList include = previousFilter.GetIncludeList();
    include.Add(kMT_Wall);
    CMaterialList exclude = previousFilter.GetExcludeList();
    exclude.Add(mCollisionExcludeList);
    CCollisionActor* mutableCollisionActor =
        static_cast< CCollisionActor* >(mgr.ObjectById(mCollisionActorId));
    mutableCollisionActor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    cameraPos = MoveCollisionActor(cameraPos, dt, mgr);
    mutableCollisionActor->SetMaterialFilter(previousFilter);
  }

  const CVector3f lookAt = mLookAtBall ? ballPos : mLookPos;
  CVector3f lookDir = lookAt - cameraPos;
  if (lookDir.IsMagnitudeSafe()) {
    lookDir.Normalize();
    UpdateTransform(lookDir, cameraPos, dt, mgr);
  }
  TeleportCamera(cameraPos, mgr);
  if (mCamBehindFloorOrWall && mSplineCtrl / mSplineCtrlRange < 0.5f) {
    mSplineState = kBSS_Invalid;
  }
}

bool CBallCamera::fn_801a39d0(float distance, float dt, CVector3f& position, CStateManager& mgr) {
  const CVector3f cameraPos = GetTranslation();
  const CVector3f extent(8.f, 8.f, 8.f);
  const CAABox bounds(cameraPos - extent, cameraPos + extent);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeInclude(CMaterialList(kMT_Pillar)),
                    this);

  const CVector3f ballPos = Player(mgr).GetBallPosition();
  CVector3f ballDirection = ballPos - cameraPos;
  ballDirection.SetZ(0.f);
  position = CVector3f::Zero();
  bool found = false;
  for (uint i = 0; i < nearList.size(); ++i) {
    const CEntity* entity = mgr.GetObjectById(nearList[i]);
    const CScriptRepulsor* repulsor =
        entity == nullptr
            ? nullptr
            : static_cast< const CScriptRepulsor* >(entity->TypesMatch(kET_ScriptRepulsor));
    if (repulsor == nullptr || !repulsor->GetActive() ||
        (repulsor->GetFlags() & CScriptRepulsor::kF_RepelPlayer) == 0) {
      continue;
    }

    CVector3f repulsorDirection = repulsor->GetTranslation() - cameraPos;
    repulsorDirection.SetZ(0.f);
    if (!ballDirection.IsMagnitudeSafe() ||
        repulsorDirection.Magnitude() >= repulsor->GetRadius() ||
        CVector3f::Dot(ballDirection, repulsorDirection) <= 0.f) {
      continue;
    }
    found = true;
    if (repulsorDirection.Magnitude() >= distance) {
      continue;
    }

    const CLine line(cameraPos, CUnitVector3f(ballDirection.AsNormalized()));
    const CVector3f closestPoint = line.GetClosestPoint(repulsor->GetTranslation());
    const float falloff =
        1.f - CMath::Clamp(0.f, repulsorDirection.Magnitude() / repulsor->GetRadius(), 1.f);
    CVector3f pushDirection = closestPoint - repulsor->GetTranslation();
    pushDirection.SetZ(0.f);
    if (CMath::AbsF(CVector3f::Dot(pushDirection, ballDirection)) > 0.999f) {
      pushDirection = CVector3f(pushDirection.GetY(), -pushDirection.GetX(), 0.f);
    }
    if (repulsor->GetFlags() & CScriptRepulsor::kF_UseRightVector) {
      pushDirection = repulsor->GetTransform().GetRight();
    }
    position += falloff * (repulsor->GetStrength() * (dt * pushDirection.AsNormalized()));
  }
  return found;
}

bool CBallCamera::fn_801a36f0(float distance, float dt, CVector3f& position, CStateManager& mgr) {
  position = CVector3f::Zero();
  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  if (door == nullptr || !door->IsOpen() || door->IsHorizontal()) {
    return false;
  }
  const CScriptDock* dock =
      TCastToConstPtr< CScriptDock >(mgr.GetObjectById(door->GetConnectedDockID()));
  if (dock == nullptr) {
    return false;
  }

  const CPlane plane = dock->GetPlane(mgr);
  if (plane.IsFacing(Player(mgr).GetBallPosition()) == plane.IsFacing(GetTranslation())) {
    return false;
  }
  const CLine planeLine(dock->GetTranslation(), plane.GetNormal());
  CVector3f fromCamera = planeLine.GetClosestPoint(GetTranslation()) - GetTranslation();
  fromCamera.SetZ(0.f);
  if (!fromCamera.IsMagnitudeSafe()) {
    return false;
  }
  const float strength = CMath::Clamp(0.f, fromCamera.Magnitude() / 5.f, 1.f);
  position = strength * (40.f * (dt * fromCamera.AsNormalized()));
  return true;
}

void CBallCamera::UpdateUsingColliders(float dt, CStateManager& mgr) {
  if (Player(mgr).GetBombJumpCounter() == 1) {
    const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
    if (door != nullptr && !door->IsOpen()) {
      return;
    }
  }

  CVector3f ballPos = Player(mgr).GetBallPosition();
  if (GetWatchedObject() != Player(mgr).GetUniqueId()) {
    if (const CActor* watched = TCastToConstPtr< CActor >(mgr.GetObjectById(GetWatchedObject()))) {
      ballPos = watched->GetOrbitPosition(mgr);
    }
  }
  const CPlayer* player = &Player(mgr);
  if (player->GetBombJumpCounter() == 2) {
    CVector3f lookDir(mLookPos.GetX() - GetTransform().Get03(),
                      mLookPos.GetY() - GetTransform().Get13(),
                      mLookPos.GetZ() - GetTransform().Get23());
    if (mLookAtBall) {
      lookDir = CVector3f(ballPos.GetX() - GetTransform().Get03(),
                          ballPos.GetY() - GetTransform().Get13(),
                          ballPos.GetZ() - GetTransform().Get23());
    }

    if (lookDir.CanBeNormalized()) {
      lookDir.Normalize();
      UpdateTransform(lookDir, GetTranslation(), dt, mgr);
    }
    return;
  }

  if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed && !mAvoidGeometryFull) {
    return;
  }

  const CTransform4f oldXf = GetTransform();
  const CVector3f oldPos = GetTranslation();
  x3b0_ = mSmallColliders.CountObscuredColliders();
  x3b4_ = mMediumColliders.CountObscuredColliders();
  x3b8_ = mLargeColliders.CountObscuredColliders();

  CVector3f ballToCamFlat(GetTransform().Get03() - ballPos.GetX(),
                          GetTransform().Get13() - ballPos.GetY(),
                          GetTransform().Get23() - ballPos.GetZ());
  CVector3f posAtBallLevel(0.f, 0.f, ballToCamFlat.GetZ());
  ballToCamFlat[kDZ] = 0.f;
  float ballToCamFlatMag = 0.f;
  if (ballToCamFlat.CanBeNormalized()) {
    ballToCamFlatMag = ballToCamFlat.Magnitude();
  } else {
    ballToCamFlat = -player->GetMovementDirection();
  }

  posAtBallLevel = CVector3f(GetTransform().Get03() - posAtBallLevel.GetX(),
                             GetTransform().Get13() - posAtBallLevel.GetY(),
                             GetTransform().Get23() - posAtBallLevel.GetZ());
  CTransform4f ballToUnderCamLook = CTransform4f::Identity();
  if (CVector3f(posAtBallLevel - ballPos).CanBeNormalized()) {
    ballToUnderCamLook = CTransform4f::LookAt(ballPos, posAtBallLevel, CVector3f::Up());
  }

  float distance = mBallCameraSpring.ApplyDistanceSpring(mCurMinDistance, ballToCamFlatMag,
                                                         dt * (3.f + mSpeedFactor));
  CVector3f camToBall(ballPos.GetX() - GetTransform().Get03(),
                      ballPos.GetY() - GetTransform().Get13(),
                      ballPos.GetZ() - GetTransform().Get23());
  camToBall[kDZ] = 0.f;
  if (camToBall.CanBeNormalized()) {
    camToBall.Normalize();
    float dot = CVector3f::Dot(camToBall, player->GetMovementDirection());
    dot = CMath::Limit(dot, 1.f);
    if (CMath::AbsF(acosf(dot)) > (150.f * (M_PIF / 180.f))) {
      CVector3f velocity = player->GetVelocityWR();
      if (velocity.CanBeNormalized()) {
        distance = mBallCameraSpring.ApplyDistanceSpring(
            mCurMinDistance + mSpeedFactor * (mBackwardsDistance - mCurMinDistance),
            ballToCamFlatMag, 3.f * dt);
      }
    }
  }

  mCollidersAABB = mLargeColliders.CalculateCollidersBoundingBox();
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, mCollidersAABB, skLineOfSightFilter,
                    TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId)));

  if (!mClearLOS && mObscuringObjectId == kInvalidUniqueId) {
    if (mObscuredTime > 0.f || mObscuringMaterial.HasMaterial(kMT_Floor) ||
        mObscuringMaterial.HasMaterial(kMT_Wall)) {
      mColliderMag += 2.f * dt;
      if (mColliderMag < 2.f) {
        mColliderMag = 2.f;
      }
      if (mColliderMag > 2.f) {
        mColliderMag = 2.f;
      }
      mSmallColliders.UpdateCollidersDistances(7.f * 0.33f * mColliderMag,
                                               7.f * 0.33f * mColliderMag / 2.f, -M_PIF / 2.f);
      mMediumColliders.UpdateCollidersDistances(7.f * 0.66f * mColliderMag,
                                                7.f * 0.66f * mColliderMag / 2.f, -M_PIF / 2.f);
      mLargeColliders.UpdateCollidersDistances(7.f * mColliderMag, 7.f * mColliderMag / 2.f,
                                               -M_PIF / 2.f);
    }
  } else {
    float targetColliderMag = 1.f;
    if (mPrevClearLOS && player->GetMoveSpeed() < 1.f) {
      targetColliderMag = 0.25f;
    }
    mColliderMag += 2.f * ((targetColliderMag - mColliderMag) * dt);
    mSmallColliders.UpdateCollidersDistances(mColliderMag * (7.f * 0.33f),
                                             mColliderMag * (7.f * 0.33f), -M_PIF / 2.f);
    mMediumColliders.UpdateCollidersDistances(mColliderMag * (7.f * 0.66f),
                                              mColliderMag * (7.f * 0.66f), -M_PIF / 2.f);
    mLargeColliders.UpdateCollidersDistances(mColliderMag * 7.f, mColliderMag * 7.f, -M_PIF / 2.f);
  }

  float elevation = mElevation;
  bool interpolateElevation = true;
  if (ConstrainElevationAndDistance(elevation, distance, dt, mgr)) {
    interpolateElevation = false;
  }
  CVector3f desiredBallToCam(0.f, distance, elevation);
  desiredBallToCam = ballToUnderCamLook.Rotate(desiredBallToCam);

  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  if ((door == nullptr || !door->IsOpen()) &&
      ((mChaseAllowed && mState == kBCS_Chase) || mBehaviour == kBCB_FreezeLookPosition ||
       mState == kBCS_Boost)) {
    CVector3f ballToCam(GetTranslation() - ballPos);
    if (ballToCam.CanBeNormalized()) {
      ballToCam.Normalize();
    } else {
      ballToCam = -player->GetMovementDirection();
    }
    if (CMath::AbsF(ballToCamFlatMag - mChaseDistance) < 3.f) {
      float yawSpeed = gpTweakBall->GetBallCameraChaseYawSpeed();
      float dampenAngle = gpTweakBall->GetBallCameraChaseDampenAngle();
      if (mState == kBCS_Boost) {
        yawSpeed = gpTweakBall->GetBallCameraBoostYawSpeed();
        dampenAngle = gpTweakBall->GetBallCameraBoostDampenAngle();
      }
      ballToCam = ConstrainYawAngle(*player, yawSpeed, dampenAngle, dt, mgr);
    }
    ballToCam[kDZ] = 0.f;
    if (ballToCam.CanBeNormalized()) {
      ballToCam.Normalize();
    } else {
      ballToCam = -player->GetMovementDirection();
    }
    ballToCam *= distance;
    ballToCam[kDZ] = elevation;
    desiredBallToCam = ballToCam;
    interpolateElevation = false;
  }

  switch (mBehaviour) {
  default:
    break;
  case kBCB_HintLocalOffset: {
    const CGameHint* hint = CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr);
    mOverrideBallToCam = hint->GetTransform().Rotate(mHintLocalOffset);
  }
  case kBCB_HintBallToCam: {
    desiredBallToCam = mOverrideBallToCam;
    if (mObscureAvoidance) {
      CVector3f ballToCamDir = desiredBallToCam;
      if (ballToCamDir.CanBeNormalized()) {
        ballToCamDir.Normalize();
      } else {
        ballToCamDir = -player->GetMovementDirection();
      }
      TUniqueId intersectId = kInvalidUniqueId;
      const CRayCastResult& result = mgr.RayWorldIntersection(
          intersectId, ballPos, ballToCamDir, distance, skLineOfSightFilter, nearList);
      const float hitTime = result.GetTime();
      if (result.IsValid()) {
        float hitX = hitTime * ballToCamDir.GetX();
        float hitY = hitTime * ballToCamDir.GetY();
        float hitZ = hitTime * ballToCamDir.GetZ();
        desiredBallToCam = CVector3f(0.9f * hitX, 0.9f * hitY, 0.9f * hitZ);
      }
    }
    interpolateElevation = false;
    break;
  }
  }

  distance = desiredBallToCam.Magnitude();
  CVector3f desiredCamPos(ballPos.GetX() + desiredBallToCam.GetX(),
                          ballPos.GetY() + desiredBallToCam.GetY(),
                          ballPos.GetZ() + desiredBallToCam.GetZ());
  float collDist = 0.f;
  bool noCollision = !DetectCollision(ballPos,
                                      CVector3f(ballPos.GetX() + desiredBallToCam.GetX(),
                                                ballPos.GetY() + desiredBallToCam.GetY(),
                                                ballPos.GetZ() + desiredBallToCam.GetZ()),
                                      0.3f, collDist, mgr, GetControllerNumber());
  if (!noCollision) {
    const float collisionDistance = collDist;
    if (collisionDistance >= 1.f) {
      const CVector3f& normalizedDir = desiredBallToCam.AsNormalized();
      desiredBallToCam = CVector3f(collisionDistance * normalizedDir.GetX(),
                                   collisionDistance * normalizedDir.GetY(),
                                   collisionDistance * normalizedDir.GetZ());
      desiredCamPos = CVector3f(ballPos.GetX() + desiredBallToCam.GetX(),
                                ballPos.GetY() + desiredBallToCam.GetY(),
                                ballPos.GetZ() + desiredBallToCam.GetZ());
    } else {
      desiredCamPos = GetTranslation();
      desiredBallToCam =
          CVector3f(desiredCamPos.GetX() - ballPos.GetX(), desiredCamPos.GetY() - ballPos.GetY(),
                    desiredCamPos.GetZ() - ballPos.GetZ());
    }
  }

  CTransform4f lookXf = CTransform4f::LookAt(desiredCamPos, mLookPos, CVector3f::Up());
  CTransform4f oldLookXf = CTransform4f::LookAt(GetTranslation(), mLookPos, CVector3f::Up());
  mNextLookXf = lookXf;
  lookXf = oldLookXf;

  CVector3f colliderPointLocal = CVector3f::Zero();
  if (mAvoidGeometryFull || !mClearLOS) {
    colliderPointLocal = AvoidGeometryFull(lookXf, nearList, mgr);
  } else {
    colliderPointLocal = AvoidGeometry(lookXf, nearList, mgr);
  }

  CVector3f oldBallToCamFlat(GetTranslation().GetX() - ballPos.GetX(),
                             GetTranslation().GetY() - ballPos.GetY(), 0.f);
  if (oldBallToCamFlat.Magnitude() < 2.f) {
    if (mClearLOS && mShortMoveCount > 2) {
      colliderPointLocal *= 1.f / float(mShortMoveCount);
    }
    if (collDist < 3.f) {
      colliderPointLocal *= 0.25f;
      if (mClearLOS && mShortMoveCount > 0) {
        colliderPointLocal *= mSpeedFactor;
      }
    }
    if (collDist < 1.f) {
      colliderPointLocal = CVector3f::Zero();
    }
  }

  CVector3f rotatedColliderPoint = lookXf.Rotate(colliderPointLocal);
  CVector3f camDelta(rotatedColliderPoint[kDX] + desiredCamPos[kDX] - ballPos[kDX],
                     rotatedColliderPoint[kDY] + desiredCamPos[kDY] - ballPos[kDY],
                     rotatedColliderPoint[kDZ] + desiredCamPos[kDZ] - ballPos[kDZ]);
  if (camDelta.CanBeNormalized()) {
    camDelta.Normalize();
  }
  float desiredX = distance * camDelta[kDX];
  float desiredY = distance * camDelta[kDY];
  float desiredZ = distance * camDelta[kDZ];
  CVector3f desiredPos(ballPos.GetX() + desiredX, ballPos.GetY() + desiredY,
                       ballPos.GetZ() + desiredZ);

  if (mBehaviour == kBCB_Unknown6) {
    desiredPos = CameraManager(mgr).GetPathCamera()->GetTranslation();
  }

  camDelta = mDampedPos - desiredPos;
  float dampDeltaMag = camDelta.Magnitude();
  if (camDelta.CanBeNormalized()) {
    camDelta.Normalize();
  }
  float springDist = mBallCameraCentroidSpring.ApplyDistanceSpring(0.f, dampDeltaMag, dt);
  float springX = springDist * camDelta[kDX];
  float springY = springDist * camDelta[kDY];
  float springZ = springDist * camDelta[kDZ];
  mDampedPos = CVector3f(desiredPos.GetX() + springX, desiredPos.GetY() + springY,
                         desiredPos.GetZ() + springZ);

  CVector3f posDelta(oldPos.GetX() - mDampedPos.GetX(), oldPos.GetY() - mDampedPos.GetY(),
                     oldPos.GetZ() - mDampedPos.GetZ());
  float posDeltaMag = posDelta.Magnitude();
  if (posDelta.CanBeNormalized()) {
    posDelta.Normalize();
  }

  float springSpeed = 1.f;
  if (mObtuseDirection) {
    springSpeed = 3.f;
  }
  float springMag =
      mBallCameraCentroidDistanceSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt * springSpeed);
  if (GetState() == kBCS_Boost) {
    springMag = mBallCameraBoostSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt);
  } else if (mChaseAllowed) {
    if (mState == kBCS_Chase || mBehaviour == kBCB_FreezeLookPosition) {
      springMag = mBallCameraChaseSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt);
    }
  }

  float finalX = springMag * posDelta.GetX();
  float finalY = springMag * posDelta.GetY();
  float finalZ = springMag * posDelta.GetZ();
  CVector3f finalPos(mDampedPos.GetX() + finalX, mDampedPos.GetY() + finalY,
                     mDampedPos.GetZ() + finalZ);
  if (player->GetMorphBall()->GetBallState() != CMorphBall::kBS_ScrewAttack) {
    if (player->GetMorphBall()->GetBallState() != CMorphBall::kBS_Spider && !mNoElevationVelClamp &&
        player->GetVelocityWR().GetZ() > 8.f) {
      CVector3f delta(finalPos.GetX() - oldPos.GetX(), finalPos.GetY() - oldPos.GetY(),
                      finalPos.GetZ() - oldPos.GetZ());
      delta[kDZ] = CMath::Limit(delta.GetZ(), 0.1f * dt);
      finalPos = CVector3f(oldPos.GetX() + delta.GetX(), oldPos.GetY() + delta.GetY(),
                           oldPos.GetZ() + delta.GetZ());
    }

    if (interpolateElevation && mState != kBCS_ToBall) {
      finalPos = InterpolateCameraElevation(finalPos, dt);
    }
    if (mNoElevationInterp) {
      finalPos[kDZ] = elevation + ballPos.GetZ();
    }

    if (oldBallToCamFlat.Magnitude() < 2.f) {
      if (finalPos.GetZ() < 2.f + ballPos.GetZ()) {
        finalPos[kDZ] = 2.f + ballPos.GetZ();
      }
      mBallCameraSpring.Reset();
    }

    finalPos = ClampElevationToWater(finalPos, mgr);
    if (oldBallToCamFlat.Magnitude() < 2.f) {
      if (mTooCloseActorId != kInvalidUniqueId && mTooCloseActorDist < 5.f) {
        door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(TUniqueId(mTooCloseActorId)));
        if (door != nullptr && !door->IsOpen()) {
          finalPos[kDX] = GetTranslation()[kDX];
          finalPos[kDY] = GetTranslation()[kDY];
          finalPos[kDZ] = GetTranslation()[kDZ];
        }
      }
    }

  } else {
    finalPos.SetZ(oldPos.GetZ());
    float ceilingZ = player->GetLastSpaceJumpPosition().GetZ() + 5.f;
    CVector3f forward = GetTransform().GetColumn(kDY);
    forward.SetZ(0.f);
    if (forward.CanBeNormalized()) {
      forward.Normalize();
      CVector3f castPos(finalPos.GetX(), finalPos.GetY(), ceilingZ + 0.6f);
      for (;;) {
        rstl::reserved_vector< TUniqueId, 1024 > blockers;
        mgr.BuildNearList(blockers, castPos, forward, 10.f, skLineOfSightFilter, nullptr);
        TUniqueId hitId = kInvalidUniqueId;
        const CRayCastResult hit =
            mgr.RayWorldIntersection(hitId, castPos, forward, 10.f, skLineOfSightFilter, blockers);
        if (!hit.IsValid()) {
          break;
        }
        ceilingZ -= 0.6f;
        castPos.SetZ(castPos.GetZ() - 0.6f);
      }
    }
    float targetZ = ceilingZ;
    const float upwardDistance = ceilingZ - finalPos.GetZ();
    if (upwardDistance > 0.f) {
      rstl::reserved_vector< TUniqueId, 1024 > blockers;
      mgr.BuildNearList(blockers, finalPos, CVector3f::Up(), upwardDistance, skLineOfSightFilter,
                        nullptr);
      TUniqueId hitId = kInvalidUniqueId;
      const CRayCastResult hit = mgr.RayWorldIntersection(
          hitId, finalPos, CVector3f::Up(), upwardDistance, skLineOfSightFilter, blockers);
      if (hit.IsValid() && hit.GetPoint().GetZ() - 0.6f < targetZ) {
        targetZ = hit.GetPoint().GetZ() - 0.6f;
      }
    }
    const float deltaZ = targetZ - finalPos.GetZ();
    const float step = 8.f * dt * CMath::Clamp(0.f, CMath::AbsF(deltaZ / 0.25f), 1.f);
    if (CMath::AbsF(deltaZ - 0.05f) >= 1e-5f) {
      finalPos.SetZ(finalPos.GetZ() + (deltaZ < 0.f ? -step : step));
    } else {
      finalPos.SetZ(oldPos.GetZ());
    }
  }

  const float backupZ = finalPos.GetZ();
  finalPos = MoveCollisionActor(finalPos, dt, mgr);
  if (mClearLOS && mShortMoveCount > 0) {
    finalPos[kDZ] = backupZ;
    finalPos = MoveCollisionActor(finalPos, dt, mgr);
  }

  CVector3f lookDir(mLookPos.GetX() - finalPos.GetX(), mLookPos.GetY() - finalPos.GetY(),
                    mLookPos.GetZ() - finalPos.GetZ());
  if (mLookAtBall) {
    lookDir = ballPos - finalPos;
  }
  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
    UpdateTransform(lookDir, finalPos, dt, mgr);
  }

  if (mClampVelTimer > 0.f) {
    mClampVelTimer -= dt;
  }
}

void CBallCamera::UpdateUsingFreeLook(float dt, CStateManager& mgr) {
  CVector3f ballPos = Player(mgr).GetBallPosition();
  mLookPos = ballPos;
  mLookPos.SetZ(mLookPos.GetZ() + mLookAtOffset.GetZ());
  CVector3f ballToCam = GetTranslation() - mLookPos;
  float distance = ballToCam.Magnitude();
  if (ballToCam.IsMagnitudeSafe()) {
    ballToCam.Normalize();
  }

  float zoom = CMath::Limit((mFreeLookDistance - distance) /
                                (gpTweakBall->GetBallCameraFreeLookMaxDistance() -
                                 gpTweakBall->GetBallCameraFreeLookMinDistance()),
                            1.f);
  ballToCam *= distance + zoom * (dt * gpTweakBall->GetBallCameraFreeLookZoomSpeed());
  ballToCam =
      CQuaternion::ZRotation(CRelAngle::FromRadians(mFreeLookYawDelta)).Transform(ballToCam);
  CVector3f flatDirection(ballToCam.GetX(), ballToCam.GetY(), 0.f);
  if (flatDirection.IsMagnitudeSafe()) {
    flatDirection.Normalize();
  }
  CUnitVector3f right(flatDirection.GetY(), -flatDirection.GetX(), 0.f, CUnitVector3f::kN_Yes);
  ballToCam = CQuaternion::AxisAngle(right, CRelAngle::FromRadians(-mFreeLookPitchDelta))
                  .Transform(ballToCam);

  float upDot = CMath::Limit(CVector3f::Dot(ballToCam.AsNormalized(), CVector3f::Up()), 1.f);
  float angle = CMath::ArcCosineR(CMath::AbsF(upDot));
  if (angle > M_PIF / 2.f - gpTweakBall->GetBallCameraFreeLookMaxVertAngle()) {
    CVector3f desiredPos = mLookPos + ballToCam;
    CVector3f position = MoveCollisionActor(desiredPos, dt, mgr);
    if ((position - desiredPos).IsMagnitudeSafe()) {
      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId))) {
        actor->SetTranslation(GetTranslation());
        actor->Stop();
      }
    } else if (mgr.RayCollideWorld(position, ballPos, skLineOfSightFilter, nullptr)) {
      mDampedPos = position;
      SetTransform(CTransform4f::LookAt(position, mLookPos));
    }
  }
}

void CBallCamera::UpdateUsingTransitions(float dt, CStateManager& mgr) {
  mLookAtBall = false;
  CPlayer& player = Player(mgr);

  if (mState == kBCS_FromBall) {
    if (UpdateTransitionFromBallCamera(mgr)) {
      player.SkipMorphTransition();
    }
  } else if (mState == kBCS_ToBall) {
    bool finished;
    if (player.GetSpawnedMorphballState() == CPlayer::kMS_Morphed) {
      finished = UpdateTransitionToBallCamera(mgr);
    } else {
      finished = UpdateTransitionToBallCamera(dt, mgr);
    }
    CameraManager(mgr).FirstPersonCamera()->SetTransform(GetTransform());
    if (finished) {
      player.SkipMorphTransition();
    }
  }
}

CVector3f CBallCamera::TweenVelocity(const CVector3f& currentVelocity, const CVector3f& newVelocity,
                                     float rate, float dt) {
  CVector3f delta = newVelocity - currentVelocity;
  if (!delta.IsMagnitudeSafe()) {
    return newVelocity;
  }
  float t = CMath::Limit(delta.Magnitude() / (rate * dt), 1.f);
  return currentVelocity + t * (dt * (rate * delta.AsNormalized()));
}

CVector3f CBallCamera::ComputeVelocity(CVector3f currentVelocity, CVector3f positionDelta,
                                       float dt) {
  float magnitude = positionDelta.Magnitude();
  if (mClampVelTimer > 0.f && positionDelta.IsMagnitudeSafe() && !mObtuseDirection) {
    positionDelta = positionDelta.AsNormalized() * CMath::Limit(magnitude, mClampVelRange);
  }
  return positionDelta;
}

void CBallCamera::UpdateAnglePerSecond(float dt) {
  float delta = mTargetAnglePerSecond - mCurAnglePerSecond;
  if (CMath::AbsF(delta) >= 0.0017453292f) {
    mCurAnglePerSecond += CMath::Limit(delta / M_PIF, 1.f) * (10.471975f * dt);
  } else {
    mCurAnglePerSecond = mTargetAnglePerSecond;
  }
}

CVector3f CBallCamera::ClampElevationToWater(CVector3f position, CStateManager& mgr) const {
  const CPlayer& player = GetPlayer(mgr);
  const CScriptWater* water =
      TCastToConstPtr< CScriptWater >(mgr.GetObjectById(player.InFluidId()));
  if (water == nullptr) {
    water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
  }
  if (water != nullptr) {
    const float waterZ = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
    const float deltaZ = position.GetZ() - waterZ;
    if (position.GetZ() >= waterZ && deltaZ <= 0.25f) {
      position.SetZ(waterZ + 0.25f);
    } else if (position.GetZ() < waterZ && deltaZ >= -0.12f) {
      position.SetZ(waterZ - 0.12f);
    }
  }
  return position;
}

CVector3f CBallCamera::MoveCollisionActor(const CVector3f& position, float dt, CStateManager& mgr) {
  CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId));
  if (actor == nullptr) {
    return position;
  }
  CVector3f delta = position - actor->GetTranslation();
  if (!delta.IsMagnitudeSafe() || delta.Magnitude() < 0.01f) {
    actor->Stop();
    return actor->GetTranslation();
  }

  CVector3f oldVelocity = actor->GetVelocityWR();
  CVector3f oldPosition = actor->GetTranslation();
  CVector3f velocity = ComputeVelocity(oldVelocity, delta / dt, dt);
  actor->SetVelocityWR(velocity);
  actor->SetMovable(true);
  actor->AddMaterial(kMT_Unknown59, mgr);
  CGameCollision::Move(mgr, *actor, dt, nullptr);

  CVector3f remaining = actor->GetTranslation() - position;
  if (remaining.IsMagnitudeSafe() && remaining.Magnitude() > 0.1f) {
    actor->SetTranslation(oldPosition);
    actor->SetVelocityWR(TweenVelocity(oldVelocity, velocity, 50.f, dt));
    CGameCollision::Move(mgr, *actor, dt, nullptr);
    remaining = actor->GetTranslation() - position;
    if (remaining.Magnitude() > 0.1f) {
      ++mShortMoveCount;
    } else {
      mShortMoveCount = 0;
    }
  } else {
    actor->Stop();
    mShortMoveCount = 0;
  }

  actor->SetMovable(false);
  actor->RemoveMaterial(kMT_Unknown59, mgr);
  return actor->GetTranslation();
}

void CBallCamera::UpdateLookAtPosition(float dt, CStateManager& mgr, bool teleport) {
  const CEntity* watched = mgr.GetObjectById(GetWatchedObject());
  const CPlayer* player = TCastToConstPtr< CPlayer >(watched);
  if (player == nullptr) {
    if (const CActor* actor = TCastToConstPtr< CActor >(watched)) {
      mLookPos = actor->GetOrbitPosition(mgr);
    }
    return;
  }

  if (player->GetBombJumpCounter() == 1) {
    const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
    if (door != nullptr && !door->IsOpen()) {
      return;
    }
  }

  const CVector3f ballPosition = player->GetBallPosition();
  CVector3f offset(mSpeedFactor * mLookAtOffset.GetX(), mSpeedFactor * mLookAtOffset.GetY(),
                   mLookAtOffset.GetZ());
  const CTransform4f moveRotation = player->CreateTransformFromMovementDirection().GetRotation();
  if (mBallDeltaFlat.IsMagnitudeSafe()) {
    offset = moveRotation * offset;
  }

  bool cameraTransitioning = CameraManager(mgr).IsBallCameraTransitioning(mgr);
  CHintManager* hintManager = CameraManager(mgr).HintManager();
  if (cameraTransitioning && hintManager->HasHint(mgr)) {
    if (CHintState* best = hintManager->GetBestHintState()) {
      const CScriptTrigger* sender =
          TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(best->GetFirstSender()));
      if (sender != nullptr && (sender->GetTriggerFlags() & 0x10000006) == 0x10000000) {
        cameraTransitioning = false;
      }
    }
  }

  CVector3f lookAhead = ballPosition + offset;
  if (cameraTransitioning) {
    offset.SetZ(GetTranslation().GetZ() - 2.f);
    lookAhead.SetZ(offset.GetZ());
  }
  mLookPosAhead = lookAhead;
  mFixedLookPos = ballPosition + CVector3f(0.f, 0.f, offset.GetZ());

  if (!teleport) {
    CVector3f lookDelta = mLookPos - lookAhead;
    const float lookDeltaMagnitude = lookDelta.Magnitude();
    if (lookDelta.IsMagnitudeSafe()) {
      lookDelta.Normalize();
    }
    const float springScale = 1.f + 2.f * CMath::Clamp(0.f, mSpeedingTime / 3.f, 1.f);
    const float springDistance =
        mBallCameraLookAtSpring.ApplyDistanceSpring(0.f, lookDeltaMagnitude, dt * springScale);
    if (springDistance > 0.0001f) {
      lookAhead += springDistance * lookDelta;
    }
  }
  mLookPos = lookAhead;

  if (mDirectElevation) {
    mLookPos.SetZ(ballPosition.GetZ() + mLookAtOffset.GetZ());
    mLookPosAhead.SetZ(mLookPos.GetZ());
    mFixedLookPos.SetZ(mLookPos.GetZ());
  }
  if (player->IsMorphBallTransitioning()) {
    mLookPos = mLookPosAhead;
    mFixedLookPos = mLookPosAhead;
  }

  if (mOverrideLookDir && mBehaviour != kBCB_Unknown4 && mBehaviour != kBCB_Unknown5 &&
      hintManager->HasHint(mgr) && !cameraTransitioning) {
    const CScriptCameraHint* hint =
        TCastToConstPtr< CScriptCameraHint >(hintManager->GetCurrentHint(mgr));
    if (hint != nullptr) {
      const CVector3f hintAxis = hint->GetTransform().GetColumn(kDY);
      const float distance = CVector3f::Dot(ballPosition - GetTranslation(), hintAxis);
      mLookPos = hint->GetTranslation() + distance * hintAxis;
      mLookPosAhead = mLookPos;
      mFixedLookPos = mLookPos;
    }
  }
}

CVector3f CBallCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CPlayer& player = GetPlayer(mgr);
  if (player.GetCameraState() == CPlayer::kCS_Four) {
    const CVector3f firstPersonPos =
        GetCameraManager(mgr).GetFirstPersonCamera()->GetScanObjectIndicatorPosition(mgr);
    float factor = 1.f - player.GetMorphBallTransitionFactor();
    factor = CMath::Clamp(0.f, factor, 1.f);
    if (mState == kBCS_FromBall) {
      factor = 1.f - factor;
    }
    return mLookPos + factor * (firstPersonPos - mLookPos);
  }
  return mLookPos;
}

void CBallCamera::ActivateFailSafe(float dt, CStateManager& mgr) {
  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, dt, mgr);

  const CVector3f position =
      FindDesiredPosition(distance, elevation, Player(mgr).GetMovementDirection(), mgr, true);
  SetTranslation(position);
  TeleportLookAtStuff(mgr);
  TeleportCamera(CTransform4f::LookAt(position, mLookPos, CVector3f::Up()), mgr);
  CameraManager(mgr).SetPlayerCamera(mgr, GetUniqueId());

  mPendingFailsafe = false;
  mObscuredTime = 0.f;
}

void CBallCamera::CheckFailSafe(float dt, CStateManager& mgr) {
  const CCameraManager& cameraManager = CameraManager(mgr);
  if (cameraManager.GetCurrentCameraId(false) != GetUniqueId() &&
      !cameraManager.IsInterpolationCameraActive()) {
    return;
  }
  if (cameraManager.IsInterpolationCameraActive() &&
      cameraManager.GetInterpolationCamera()->GetTargetId() != GetUniqueId()) {
    return;
  }
  if ((mgr.GetUpdateFrameIdx() & 3) != GetInputIndex()) {
    mObscuredTime += dt;
    return;
  }

  const CPlayer& player = Player(mgr);
  const CVector3f ballPos = player.GetBallPosition();
  mPrevClearLOS = mClearLOS;
  CVector3f cameraToBall = ballPos - GetTranslation();
  const float rayLength = cameraToBall.Magnitude();
  cameraToBall.Normalize();

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, GetTranslation(), cameraToBall, rayLength, skLineOfSightFilter,
                    nullptr);
  const CRayCastResult hit = mgr.RayWorldIntersection(
      mObscuringObjectId, GetTranslation(), cameraToBall, rayLength, skLineOfSightFilter, nearList);
  if (!hit.IsValid()) {
    mClearLOS = true;
    mObscuringMaterial = CMaterialList(kMT_NoStepLogic);
  } else {
    mObscuringMaterial = hit.GetMaterial();
    CVector3f upperBallPos = ballPos;
    upperBallPos.SetZ(upperBallPos.GetZ() + player.GetTweakPlayer()->GetBallRadius());
    const bool clearAbove =
        mgr.RayCollideWorld(GetTranslation(), upperBallPos, nearList, skLineOfSightFilter, &player);
    const bool clearBelow = mgr.RayCollideWorld(GetTranslation(), player.GetTranslation(), nearList,
                                                skLineOfSightFilter, &player);
    if (!clearAbove && !clearBelow) {
      mClearLOS = false;
      if (mPrevClearLOS) {
        mSplineIntermediatePos = ballPos;
        if (ShouldResetSpline(mgr) && !mNoSpline && mObscuringMaterial.HasMaterial(kMT_Floor) &&
            mgr.RayCollideWorld(ballPos, ballPos + CVector3f(0.f, 0.f, -2.5f), nearList,
                                skLineOfSightFilter, nullptr)) {
          BuildSpline(mgr);
        }
      }
    }
  }

  if (mClearLOS) {
    mObscuredTime = 0.f;
  } else {
    mObscuredTime += dt;
  }
  mUnobscureMag = CMath::Clamp(0.f, 0.5f * mObscuredTime, 1.f);
  mPendingFailsafe =
      mObscureAvoidance && !mClearLOS && mSplineState == kBSS_Invalid &&
      (mObscuredTime > 2.f || (mTooCloseActorId != kInvalidUniqueId && mObscuredTime > 1.f));
  bool useFailsafe = mPendingFailsafe;
  if ((GetTranslation() - ballPos).Magnitude() < 0.3f + player.GetTweakPlayer()->GetBallRadius()) {
    useFailsafe = true;
  }
  if (mNearbyDoorClosed) {
    mNearbyDoorClosed = false;
    if (hit.IsValid()) {
      useFailsafe = true;
    }
  }
  if (mNearbyDoorClosing) {
    mNearbyDoorClosing = false;
    if (CheckDoorProximity(GetTranslation(), mgr)) {
      useFailsafe = true;
    }
  }
  if (useFailsafe) {
    ActivateFailSafe(dt, mgr);
  }
}

bool CBallCamera::CheckDoorProximity(const CVector3f& position, const CStateManager& mgr) const {
  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  if (door == nullptr || door->IsOpen()) {
    return false;
  }

  const rstl::optional_object< CAABox > bounds = door->GetTouchBounds();
  const CVector3f extent(0.3f, 0.3f, 0.3f);
  if (!bounds || !bounds->DoBoundsOverlap(CAABox(position - extent, position + extent))) {
    return false;
  }

  const CScriptDock* dock =
      TCastToConstPtr< CScriptDock >(mgr.GetObjectById(door->GetConnectedDockID()));
  return dock != nullptr && CMath::AbsF(dock->GetPlane(mgr).GetHeight(position)) < 1.15f;
}

void CBallCamera::DoorClosing(TUniqueId uid) {
  if (uid == mTooCloseActorId) {
    mNearbyDoorClosing = true;
  }
}

void CBallCamera::DoorClosed(TUniqueId uid) {
  if (uid == mTooCloseActorId) {
    mNearbyDoorClosed = true;
  }
}

void CBallCamera::Think(float dt, CStateManager& mgr) {
  CPlayer& player = Player(mgr);
  if (!player.GetPlayerState()->IsPlayerAlive() || gpMain->IsMaxSpeed()) {
    return;
  }

  const TAreaId areaId = mgr.GetNextAreaId();
  mgr.SetActorAreaId(*this, areaId);
  UpdatePlayerMovement(dt, mgr);

  CCollisionActor* collisionActor =
      TCastToPtr< CCollisionActor >(mgr.GetObjectByIdFromListAll(mCollisionActorId));
  if (collisionActor != nullptr) {
    mgr.SetActorAreaId(*collisionActor, areaId);
  }

  const CPlayer::EPlayerCameraState cameraState = player.GetCameraState();
  if (cameraState != CPlayer::kCS_Ball && cameraState != CPlayer::kCS_Four &&
      cameraState != CPlayer::kCS_Transitioning && !mForceProcessing) {
    if (collisionActor != nullptr) {
      collisionActor->SetActive(false);
    }
    return;
  }
  if (collisionActor != nullptr) {
    collisionActor->SetActive(true);
  }

  const CTransform4f oldTransform = GetTransform();
  if (player.GetBombJumpCounter() != 1) {
    UpdateLookAtPosition(dt, mgr, false);
  }
  CheckFailSafe(dt, mgr);
  UpdateObjectTooCloseId(mgr);
  UpdateAnglePerSecond(dt);

  switch (mState) {
  case kBCS_FreeLook:
    if (x204_24_) {
      UpdateUsingFreeLook(dt, mgr);
    } else {
      UpdateUsingColliders(dt, mgr);
    }
    break;
  case kBCS_Default:
  case kBCS_Chase:
  case kBCS_Boost:
    switch (mBehaviour) {
    case kBCB_Default:
    case kBCB_FreezeLookPosition:
    case kBCB_HintBallToCam:
    case kBCB_Unknown6:
    case kBCB_HintLocalOffset:
      if (mSplineState == kBSS_Invalid) {
        UpdateUsingColliders(dt, mgr);
      } else {
        UpdateUsingSpline(dt, mgr);
      }
      break;
    case kBCB_FixedTransform: {
      const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
          CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
      if (hint != nullptr) {
        SetTransform(hint->GetTransform());
      }
      break;
    }
    default:
      break;
    }
    break;
  case kBCS_ToBall:
  case kBCS_FromBall:
    UpdateUsingTransitions(dt, mgr);
    break;
  }

  const CTransform4f nextTransform = ValidateCameraTransform(GetTransform(), oldTransform);
  SetTransform(nextTransform);
  CActor::Think(dt, mgr);
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr) {
  switch (state) {
  case kBCS_ToBall: {
    const CTransform4f xf = CameraManager(mgr).GetFirstPersonCamera()->GetTransform();
    SetTransform(xf);
    TeleportCamera(xf.GetTranslation(), mgr);
    InterpolateFOV(CameraManager(mgr).GetFirstPersonCamera()->GetFov(), 1.f, 0.f, GetUniqueId(),
                   mgr);
    InvalidateSpline();
  }
  case kBCS_Default:
  case kBCS_Chase:
  case kBCS_Boost:
    mgr.SetGameState(CStateManager::kGS_Running);
    break;
  case kBCS_FromBall:
    mgr.SetGameState(CStateManager::kGS_Running);
    InterpolateFOV(CameraManager(mgr).GetFirstPersonFOV(), 1.f, 0.f);
    InvalidateSpline();
    break;
  default:
    break;
  }
  mState = state;
}

void CBallCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()));
  if (player == nullptr) {
    return;
  }

  const CScriptPlayerHint* hint =
      TCastToConstPtr< CScriptPlayerHint >(player->GetPlayerHintManager()->GetCurrentHint(mgr));
  const bool preventFreeLook = hint != nullptr && (hint->GetOverrideFlags() & 0x80000) != 0;
  if (player->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    return;
  }

  const CControlMapper& controls = player->GetControlMapper();
  const CMorphBall& ball = *player->GetMorphBall();
  switch (mState) {
  case kBCS_Chase:
    if (!controls.GetDigitalInput(CControlMapper::kC_ChaseCamera, input) ||
        player->IsInFreeLook()) {
      SetState(kBCS_Default, mgr);
    }
    break;
  case kBCS_Default:
    if (mChaseAllowed && controls.GetPressInput(CControlMapper::kC_ChaseCamera, input)) {
      SetState(kBCS_Chase, mgr);
    }
    break;
  case kBCS_FreeLook:
    if ((!controls.GetDigitalInput(CControlMapper::kC_LookHold1, input) &&
         !controls.GetDigitalInput(CControlMapper::kC_LookHold2, input)) ||
        CMath::AbsF(player->GetMoveSpeed()) >= 0.1f ||
        ball.GetBallState() == CMorphBall::kBS_Spider || preventFreeLook) {
      SetState(kBCS_Default, mgr);
    } else {
      const float left = controls.GetAnalogInput(CControlMapper::kC_LookLeft, input);
      const float right = controls.GetAnalogInput(CControlMapper::kC_LookRight, input);
      const float up = controls.GetAnalogInput(CControlMapper::kC_LookUp, input);
      const float down = controls.GetAnalogInput(CControlMapper::kC_LookDown, input);
      mFreeLookZoomOutInput = controls.GetAnalogInput(CControlMapper::kC_LookZoomOut, input);
      mFreeLookZoomInInput = controls.GetAnalogInput(CControlMapper::kC_LookZoomIn, input);
      mFreeLookDistance += input.DeltaTime() * ((mFreeLookZoomOutInput - mFreeLookZoomInInput) *
                                                gpTweakBall->GetBallCameraFreeLookZoomSpeed());
      mFreeLookDistance =
          CMath::Clamp(gpTweakBall->GetBallCameraFreeLookMinDistance(), mFreeLookDistance,
                       gpTweakBall->GetBallCameraFreeLookMaxDistance());
      mFreeLookYawDelta =
          input.DeltaTime() * ((left - right) * gpTweakBall->GetBallCameraFreeLookSpeed());
      mFreeLookPitchDelta =
          input.DeltaTime() * ((up - down) * gpTweakBall->GetBallCameraFreeLookSpeed());
    }
    break;
  case kBCS_Boost:
    if (!ball.IsBoosting() && ball.GetBallAnimationIndex() != 1) {
      SetState(kBCS_Default, mgr);
    }
    break;
  default:
    break;
  }

  if (mBoostAllowed && mState != kBCS_Boost &&
      (ball.IsBoosting() || ball.GetBoostChargeTimer() > 0.f)) {
    SetState(kBCS_Boost, mgr);
  }
}

void CBallCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_XCRT: {
    mCollisionActorId = mgr.AllocateUniqueId();
    CCollisionActor* actor = rs_new CCollisionActor(mCollisionActorId, GetAreaIdForPersistence(),
                                                    kInvalidUniqueId, true, 0.3f, 1.f);
    if (actor != nullptr) {
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Unknown59), CMaterialList(kMT_Player, kMT_CameraPassthrough)));
      actor->MaterialList() = CMaterialList(kMT_NoPlatformCollision, kMT_ScanPassthrough,
                                            kMT_SeeThrough, kMT_CameraPassthrough);
      actor->SetTranslation(GetTranslation());
      mgr.AddObject(actor);
      actor->SetMovable(false);
      actor->SetLastNonCollidingState(CMotionState(
          GetTranslation(), CNUQuaternion::BuildFromAxisAngle(CVector3f::Forward(), 0.f),
          CVector3f::Zero(), CAxisAngle::Identity()));
      actor->SetDrawEnabled(false);
    }
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(), CMaterialList(kMT_Unknown59, kMT_NoPlatformCollision, kMT_Player,
                                       kMT_Character, kMT_CameraPassthrough)));
    RemoveMaterial(kMT_Unknown59, mgr);
    break;
  }
  case kSM_XDelete:
    mgr.DeleteObjectRequest(mCollisionActorId);
    mCollisionActorId = kInvalidUniqueId;
    break;
  default:
    break;
  }
}

void CBallCamera::OverrideCameraInfo(CStateManager& mgr) {
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
      CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
  if (hint == nullptr) {
    return;
  }
  ResetToTweaks(mgr);
  const CCameraOverrideInfo& info = hint->GetInfo();
  const uint flags = info.GetFlags();
  const uint overrideFlags = info.GetOverrideFlags();
  mBehaviour = info.GetBehaviourType();
  mChaseAllowed = (flags & 0x2) != 0;
  mBoostAllowed = (flags & 0x4) != 0;
  mObscureAvoidance = (flags & 0x8) != 0;
  mVolumeCollider = (flags & 0x10) != 0;
  if (flags & 0x40) {
    mLookAtBall = true;
  }
  mNoElevationInterp = (flags & 0x4000) != 0;
  mDirectElevation = (flags & 0x8000) != 0;
  mOverrideLookDir = (flags & 0x10000) != 0;
  mNoElevationVelClamp = (flags & 0x20000) != 0;
  mNoSpline = (flags & 0x80000) != 0;
  x206_26_ = (flags & 0x100000) != 0;

  if (overrideFlags & 0x1) {
    mTargetMinDistance = info.GetMinDist();
  }
  if (overrideFlags & 0x2) {
    mMaxDistance = info.GetMaxDist();
  }
  if (overrideFlags & 0x4) {
    mBackwardsDistance = info.GetBackwardsDist();
  }
  if (overrideFlags & 0x100) {
    mElevation = info.GetElevation();
  }
  if (overrideFlags & 0x8) {
    mLookAtOffset = info.GetLookAtOffset();
  }
  mClampAttitude = (overrideFlags & 0x20) != 0;
  if (mClampAttitude) {
    mAttitudeRange = info.GetAttitudeRange();
  }
  mClampAzimuth = (overrideFlags & 0x40) != 0;
  if (mClampAzimuth) {
    mAzimuthRange = info.GetAzimuthRange();
  }
  if (overrideFlags & 0x10) {
    InterpolateFOV(info.GetFov(), 1.f, 0.f);
  }
  if (overrideFlags & 0x80) {
    mTargetAnglePerSecond = info.GetAnglePerSecond();
  }

  CPlayer& player = Player(mgr);
  if (flags & 0x200) {
    player.SetControlDirectionInterpolation(info.GetControlInterpDur());
  } else {
    const CScriptPlayerHint* playerHint =
        TCastToConstPtr< CScriptPlayerHint >(player.GetPlayerHintManager()->GetCurrentHint(mgr));
    if (playerHint == nullptr || (playerHint->GetOverrideFlags() & 2) == 0) {
      player.ResetControlDirectionInterpolation();
    }
  }

  switch (mBehaviour) {
  case kBCB_Default:
    if (flags & 0x20) {
      TeleportLookAtStuff(mgr);
      if (flags & 0x40000) {
        const CTransform4f current = CameraManager(mgr).GetCurrentCameraTransform(mgr, false);
        CVector3f direction = player.GetTranslation() - current.GetTranslation();
        direction.SetZ(0.f);
        if (direction.IsMagnitudeSafe()) {
          direction.Normalize();
        } else {
          direction = player.GetMovementDirection();
        }
        TeleportCamera(FindDesiredTransform(direction, mgr), mgr);
      } else {
        TeleportCamera(CTransform4f::LookAt(hint->GetTranslation(), mLookPos, CVector3f::Up()),
                       mgr);
      }
    }
    break;
  case kBCB_FreezeLookPosition:
  case kBCB_HintInitializePosition:
    if (flags & 0x20) {
      TeleportLookAtStuff(mgr);
      float distance = mCurMinDistance;
      float elevation = mElevation;
      ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
      const CVector3f desiredPos =
          FindDesiredPosition(distance, elevation, player.GetMovementDirection(), mgr, false);
      TeleportCamera(CTransform4f::LookAt(desiredPos, mFixedLookPos, CVector3f::Up()), mgr);
    }
    break;
  case kBCB_HintBallToCam: {
    mOverrideBallToCam = info.GetWorldOffset();
    const CVector3f ballPos = player.GetBallPosition();
    CVector3f cameraPos = ballPos + mOverrideBallToCam;
    if (flags & 0x1) {
      CVector3f direction(-mOverrideBallToCam.GetX(), -mOverrideBallToCam.GetY(), 0.f);
      direction.Normalize();
      const CVector2f flatOffset(mOverrideBallToCam.GetX(), mOverrideBallToCam.GetY());
      cameraPos = FindDesiredPosition(flatOffset.Magnitude(), mOverrideBallToCam.GetZ(), direction,
                                      mgr, false);
    }
    TeleportCamera(CTransform4f::LookAt(cameraPos, mLookPos, CVector3f::Up()), mgr);
    break;
  }
  case kBCB_HintLocalOffset: {
    mHintLocalOffset = info.GetWorldOffset();
    mOverrideBallToCam = hint->GetTransform().Rotate(mHintLocalOffset);
    CVector3f cameraPos = player.GetBallPosition() + mOverrideBallToCam;
    if (flags & 0x1) {
      CVector3f direction(-mOverrideBallToCam.GetX(), -mOverrideBallToCam.GetY(), 0.f);
      direction.Normalize();
      const CVector2f flatOffset(mOverrideBallToCam.GetX(), mOverrideBallToCam.GetY());
      cameraPos = FindDesiredPosition(flatOffset.Magnitude(), mOverrideBallToCam.GetZ(), direction,
                                      mgr, false);
    }
    TeleportCamera(CTransform4f::LookAt(cameraPos, mLookPos, CVector3f::Up()), mgr);
    break;
  }
  default:
    break;
  }
  if (flags & 0x20) {
    CameraManager(mgr).SetPlayerCamera(mgr, GetUniqueId());
  }
  if (TCastToConstPtr< CActor >(mgr.GetObjectById(hint->GetCameraTargetId())) != nullptr) {
    SetWatchedObject(hint->GetCameraTargetId());
  }
}

bool CBallCamera::SplineIntersectTest(CMaterialList& intersectMaterial, CStateManager& mgr) const {
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59, kMT_Floor, kMT_Wall),
      CMaterialList(kMT_NoPlatformCollision, kMT_Player, kMT_Character, kMT_CameraPassthrough));
  return GetCameraManager(mgr).CheckSplineCollision(mCamSpline, 0, filter, mgr, intersectMaterial,
                                                    mCamSpline.GetLength() / 12.f, 0.3f);
}

void CBallCamera::InvalidateSpline() { mSplineState = kBSS_Invalid; }
