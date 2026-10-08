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
const CMaterialList skLineOfSightInclude = CMaterialList(kMT_Solid);
const CMaterialList skLineOfSightExclude =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
const CMaterialFilter skLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(skLineOfSightInclude, skLineOfSightExclude);
const CRelAngle skAvoidStepAngle = CRelAngle::FromDegrees(60.f);
} // namespace

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const CTransform4f& xf, float fovY,
                         float nearZ, float farZ, float aspect, int index, int controllerIdx)
: CGameCamera(uid, rstl::string_l("Ball Camera"),
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
  const_cast< CCameraManager& >(GetCameraManager(mgr)).UpdateCameraTriggers(GetUniqueId(), mgr);
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
  SetLookAtOffset(gpTweakBall->GetBallCameraOffset());
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
      const_cast< CCameraManager& >(GetCameraManager(mgr)).SetPlayerCamera(mgr, GetUniqueId());
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
    mSplineState = kBSS_Invalid;
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
  mSmallColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 1, 4.f, nearList, mgr);
  mMediumColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 3, 4.f, nearList, mgr);
  mLargeColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 4, 4.f, nearList, mgr);
  return ApplyColliders();
}

CVector3f CBallCamera::AvoidGeometry(const CTransform4f& xf,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                     CStateManager& mgr) {
  switch (mAvoidGeomCycle) {
  case 0:
    mSmallColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 1, 4.f, nearList, mgr);
    break;
  case 1:
    mMediumColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 3, 4.f, nearList, mgr);
    break;
  case 2:
    mLargeColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 4, 4.f, nearList, mgr);
    break;
  case 3:
    mLargeColliders.UpdateColliders(xf, Player(mgr).GetBallPosition(), 4, 4.f, nearList, mgr);
    break;
  }

  if (++mAvoidGeomCycle >= 4) {
    mAvoidGeomCycle = 0;
  }
  return ApplyColliders();
}

bool CBallCamera::DetectCollision(const CVector3f& from, const CVector3f& to, float radius,
                                  float& distance, const CStateManager& mgr, int controllerIdx) {
  CVector3f delta = to - from;
  float length = delta.Magnitude();
  const float invLength = 1.f / length;
  CVector3f direction = delta * invLength;
  bool clear = true;

  if (length > 1.1920929e-6f) {
    float margin = 2.f * radius;
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    bounds.AccumulateBounds(from);
    bounds.AccumulateBounds(to);
    bounds = CAABox(bounds.GetMinPoint() - CVector3f(margin, margin, margin),
                    bounds.GetMaxPoint() + CVector3f(margin, margin, margin));
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildColliderList(nearList, *mgr.GetPlayer(controllerIdx), bounds);
    CAreaCollisionCache cache(bounds);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (cache.HasCacheOverflowed()) {
      clear = false;
    }
    if (CGameCollision::DetectCollisionBoolean_Cached(
            mgr, cache,
            CCollidableSphere(CSphere(CVector3f::Zero(), radius), CMaterialList(kMT_Solid)),
            CTransform4f::Translate(from),
            CMaterialFilter::MakeIncludeExclude(
                CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough, kMT_Player,
                                                        kMT_Character, kMT_CameraPassthrough)),
            nearList)) {
      distance = -1.f;
      return true;
    }

    TUniqueId hitId = kInvalidUniqueId;
    if (clear) {
      const CCollidableSphere sphere(CSphere(CVector3f::Zero(), radius), CMaterialList(kMT_Solid));
      const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough, kMT_Player,
                                                  kMT_Character, kMT_CameraPassthrough));
      CTransform4f testTransform = CTransform4f::Translate(from);
      const int stepCount = static_cast< uint >(length / 0.5f);
      const float stepScale = 1.f / stepCount;
      const CVector3f step = stepScale * delta;
      for (int i = 0; i < stepCount; ++i) {
        CCollisionInfo hitInfo;
        double hitDistance = step.Magnitude();
        if (CGameCollision::DetectCollision_Cached_Moving(mgr, cache, sphere, testTransform, filter,
                                                          nearList, direction, hitId, hitInfo,
                                                          hitDistance)) {
          clear = false;
          distance = float(hitDistance + i * step.Magnitude());
          break;
        }
        testTransform.SetTranslation(testTransform.GetTranslation() + step);
      }
    }
  }
  return !clear;
}

bool CBallCamera::fn_801a6b20(const CVector3f& from, const CVector3f& direction, CVector3f& result,
                              CStateManager& mgr) {
  const CTransform4f negativeRotation =
      CQuaternion::ZRotation(CRelAngle::FromRadians(-skAvoidStepAngle.AsRadians()))
          .BuildTransform4f();
  const CTransform4f positiveRotation = CQuaternion::ZRotation(skAvoidStepAngle).BuildTransform4f();
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
  const CTransform4f negativeRotation =
      CQuaternion::ZRotation(CRelAngle::FromRadians(-skAvoidStepAngle.AsRadians()))
          .BuildTransform4f();
  const CTransform4f positiveRotation = CQuaternion::ZRotation(skAvoidStepAngle).BuildTransform4f();
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
  CVector3f useDirection = direction;
  if (!direction.IsMagnitudeSafe()) {
    useDirection = CVector3f(0.f, 1.f, 0.f);
  }
  const CTransform4f lookRotation = CTransform4f::LookAt(CVector3f::Zero(), useDirection);

  float constrainedDistance = distance;
  float constrainedElevation = elevation;
  ConstrainElevationAndDistance(constrainedElevation, constrainedDistance, 0.f, mgr);
  CVector3f eyePos = Player(mgr).GetEyePosition();
  if (watchedPlayer != nullptr &&
      watchedPlayer->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    eyePos = GetFixedLookPos();
  }
  if (!mgr.RayCollideWorld(ballPos, eyePos, skLineOfSightFilter, nullptr)) {
    eyePos = ballPos;
  }

  CVector3f desiredOffset(0.f, -constrainedDistance, constrainedElevation);
  desiredOffset[kDZ] -= eyePos.GetZ() - ballPos.GetZ();
  desiredOffset = lookRotation.GetRotation() * desiredOffset;
  CVector3f resultOffset(0.f, distance, constrainedElevation);
  resultOffset[kDZ] -= eyePos.GetZ() - ballPos.GetZ();
  const float minSeekDistance = constrainedDistance;
  float collisionDistance = desiredOffset.Magnitude();
  const bool clear = !DetectCollision(eyePos, eyePos + desiredOffset, 0.3f, collisionDistance, mgr,
                                      GetControllerNumber());
  bool found = false;

  if (!clear && collisionDistance <= 0.f) {
    const CAABox bounds(ballPos.GetX() - distance, ballPos.GetY() - distance,
                        ballPos.GetZ() - constrainedElevation, ballPos.GetX() + distance,
                        ballPos.GetY() + distance, ballPos.GetZ() + constrainedElevation);
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    const CActor* ignored = TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId));
    mgr.BuildNearList(nearList, bounds, skLineOfSightFilter, ignored);
    found = fn_801a67a4(minSeekDistance, eyePos, desiredOffset, nearList, resultOffset, mgr);
    if (!found) {
      CVector3f flatOffset(desiredOffset.ToVec2f(), 0.f);
      found = fn_801a67a4(minSeekDistance, eyePos, flatOffset, nearList, resultOffset, mgr);
      if (!found) {
        CVector3f reflectedOffset = desiredOffset;
        reflectedOffset[kDZ] = -reflectedOffset[kDZ];
        found = fn_801a67a4(minSeekDistance, eyePos, reflectedOffset, nearList, resultOffset, mgr);
      }
    }
  } else {
    const float clearDistance = 0.95f * distance;
    bool movingForward = false;
    if (mBallVelFlat > 1.25f && mBallDeltaFlat.IsMagnitudeSafe() && watchedPlayer != nullptr &&
        (watchedPlayer->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed ||
         watchedPlayer->GetMorphballTransitionState() == CPlayer::kMS_Morphing)) {
      if (CVector3f::Dot(mBallDeltaFlat.AsNormalized(), watched->GetTransform().GetForward()) >
          0.f) {
        movingForward = true;
      }
    }
    if (clear || (!fullTest && (collisionDistance > clearDistance || movingForward))) {
      if (movingForward) {
        resultOffset = desiredOffset;
      } else {
        const float lookDistance = collisionDistance;
        resultOffset = lookDistance * desiredOffset.Normalize();
      }
      found = true;
    } else {
      found = fn_801a6b20(eyePos, desiredOffset, resultOffset, mgr);
      if (!found) {
        CVector3f flatOffset(desiredOffset.ToVec2f(), 0.f);
        found = fn_801a6b20(eyePos, flatOffset, resultOffset, mgr);
        if (!found) {
          CVector3f reflectedOffset = desiredOffset;
          reflectedOffset.SetZ(-reflectedOffset.GetZ());
          found = fn_801a6b20(eyePos, reflectedOffset, resultOffset, mgr);
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
          CVector3f flatOffset(desiredOffset.ToVec2f(), 0.f);
          found = fn_801a67a4(minSeekDistance, eyePos, flatOffset, nearList, resultOffset, mgr);
          if (!found) {
            CVector3f reflectedOffset = desiredOffset;
            reflectedOffset.SetZ(-reflectedOffset.GetZ());
            found =
                fn_801a67a4(minSeekDistance, eyePos, reflectedOffset, nearList, resultOffset, mgr);
          }
        }
      }
    }
  }

  if (!found) {
    mDesiredPosition = GetCameraManager(mgr).GetLastCameraTransform().GetTranslation();
    return mDesiredPosition;
  }
  const CVector3f desiredPosition = eyePos + resultOffset;
  mDesiredPosition = desiredPosition;
  return desiredPosition;
}

CTransform4f CBallCamera::FindDesiredTransform(CVector3f direction, CStateManager& mgr) {
  CVector3f dir = direction;
  if (!direction.IsMagnitudeSafe()) {
    dir = CVector3f(0.f, 1.f, 0.f);
  }
  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  CVector3f position = FindDesiredPosition(distance, elevation, dir, mgr, false);
  UpdateLookAtPosition(0.f, mgr, false);
  CTransform4f xf = CTransform4f::LookAt(position, mLookPos);
  return xf;
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
    if ((door == nullptr || (door != nullptr && !door->IsOpen())) &&
        (mState == kBCS_Boost || mState == kBCS_Chase || mBehaviour == kBCB_FreezeLookPosition)) {
      lookDir = player.GetLeaveMorphDirection();
    }
  }
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
    lookDir = player.GetLeaveMorphDirection();
  }

  lookDir.SetZ(0.f);
  if (lookDir.IsMagnitudeSafe()) {
    lookDir.Normalize();
  } else {
    lookDir = -playerToCamera;
  }
  if (playerToCamera.IsMagnitudeSafe()) {
    playerToCamera.Normalize();
  } else {
    return -lookDir;
  }

  float dot = CMath::Limit(CVector3f::Dot(playerToCamera, -lookDir), 1.f);
  const float angle = acosf(dot);
  if (dot >= 0.99999f) {
    return -lookDir;
  }

  float rotation = yawSpeed * dt;
  if (mClearLOS) {
    rotation *= CMath::Clamp(0.f, 1.f - mSpeedFactor, 1.f);
  }
  const float finalRotation = rotation * CMath::Clamp(0.f, angle / dampenAngle, 1.f);
  const CQuaternion quat =
      CQuaternion::LookAt(CUnitVector3f(playerToCamera), CUnitVector3f(-lookDir),
                          CRelAngle::FromRadians(finalRotation));
  return quat.Transform(playerToCamera);
}

void CBallCamera::UpdateTransform(const CVector3f& lookDirection, const CVector3f& position,
                                  float dt, CStateManager& mgr) {
  const CTransform4f oldTransform = GetTransform();
  const CVector3f usePosition = position;
  CVector3f desiredLook = lookDirection;
  if (mOverrideLookDir && CameraManager(mgr).GetHintManager()->HasHint(mgr)) {
    desiredLook =
        CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr)->GetTransform().GetForward();
  }

  CVector3f flatLook = desiredLook;
  flatLook.SetZ(0.f);
  if (!flatLook.IsMagnitudeSafe()) {
    SetTranslation(usePosition);
    return;
  }

  CVector3f currentLook = GetTransform().GetForward();
  if (currentLook.IsMagnitudeSafe()) {
    currentLook.Normalize();
  } else {
    SetTransform(CTransform4f::LookAt(usePosition, usePosition + desiredLook, CVector3f::Up()));
    return;
  }

  const float dot = CMath::Limit(CVector3f::Dot(currentLook, desiredLook), 1.f);
  if (CMath::AbsF(dot) >= 0.99999988f) {
    SetTransform(CTransform4f::LookAt(usePosition, usePosition + desiredLook, CVector3f::Up()));
  } else {
    const float speedFactor = CMath::Clamp(0.f, acosf(dot) / (1.0471976f * dt), 1.f);
    CRelAngle angle = CRelAngle::FromRadians(dt * (mCurAnglePerSecond * speedFactor));
    const float upDot = CMath::Limit(CVector3f::Dot(desiredLook, CVector3f::Up()), 1.f);
    const float absUpDot = CMath::AbsF(upDot);
    float maxAngle = (12.566371f * dt) * (1.f - absUpDot);
    if (mSplineState == kBSS_One) {
      maxAngle = 4.1887903f * dt;
      if (angle.AsRadians() > maxAngle) {
        angle = CRelAngle::FromRadians(maxAngle);
      }
    }
    if (angle.AsRadians() > maxAngle && !Player(mgr).IsMorphBallTransitioning() &&
        absUpDot > 0.999f) {
      angle = CRelAngle::FromRadians(maxAngle);
    }
    switch (mState) {
    case kBCS_Chase:
      if (mChaseAllowed || mBehaviour == kBCB_FreezeLookPosition) {
        angle = CRelAngle::FromRadians(dt * (mChaseAnglePerSecond * speedFactor));
      }
      break;
    case kBCS_Boost:
      angle = CRelAngle::FromRadians(dt * (mChaseAnglePerSecond * speedFactor));
      break;
    default:
      break;
    }

    if (mLookAtBall) {
      mLookAtBall = false;
      const CQuaternion rotation =
          CQuaternion::LookAt(CUnitVector3f(currentLook), CUnitVector3f(desiredLook),
                              CRelAngle::FromRadians(6.2831855f));
      SetTransform(rotation.BuildTransform4f() * GetTransform().GetRotation());
    } else {
      const CQuaternion rotation =
          CQuaternion::LookAt(CUnitVector3f(currentLook), CUnitVector3f(desiredLook), angle);
      SetTransform(rotation.BuildTransform4f() * GetTransform().GetRotation());
    }
  }
  SetTranslation(usePosition);
}

void CBallCamera::UpdatePlayerMovement(float dt, CStateManager& mgr) {
  const CPlayer& player = Player(mgr);
  mMaxBallVel = CMath::AbsF(player.GetActualBallMaxVelocity(dt));
  CVector3f ballPos = player.GetBallPosition();
  mBallDelta = ballPos - mPrevBallPos;
  mBallDeltaFlat = mBallDelta;
  mBallDeltaFlat.SetZ(0.f);
  const CVector3f& velocity = player.GetVelocityWR();
  const CVector2f flatVelocity(velocity.GetX(), velocity.GetY());
  mBallVelFlat = flatVelocity.Magnitude();
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
    if (CMath::AbsF(CMath::FastArcCosR(dot)) > 1.7453293f) {
      mObtuseDirection = true;
    }
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
  if (mElevation < 2.f) {
    return position;
  }
  CVector3f pos = position;
  if (!mClearLOS && mObscuringMaterial.HasMaterial(kMT_Floor)) {
    mElevInterpTimer = 1.f;
    pos.SetZ(GetTranslation().GetZ());
    mElevInterpStart = GetTranslation().GetZ();
  } else if (mElevInterpTimer > 0.f) {
    mElevInterpTimer -= dt;
    float timer = CMath::Clamp(0.f, mElevInterpTimer, 1.f);
    float delta = pos.GetZ() - mElevInterpStart;
    pos.SetZ(delta * (1.f - timer) + mElevInterpStart);
  }
  return pos;
}

const bool CBallCamera::ShouldResetSpline(CStateManager& mgr) const {
  bool ret = false;
  if (mState != kBCS_ToBall &&
      Player(mgr).GetMorphBall()->GetBallState() != CMorphBall::kBS_Spider &&
      mSplineState == kBSS_Invalid) {
    switch (mBehaviour) {
    case kBCB_Unknown4:
    case kBCB_Unknown5:
    case kBCB_Unknown6:
    case kBCB_Unknown7:
    case kBCB_Unknown8:
    case kBCB_Unknown9:
    case kBCB_FixedTransform:
      break;
    default:
      ret = true;
      break;
    }
  }
  return ret;
}

void CBallCamera::BuildSpline(CStateManager& mgr) {
  const CVector3f ballPos = Player(mgr).GetBallPosition();
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  TUniqueId intersectId = kInvalidUniqueId;
  const CVector3f down(0.f, 0.f, -1.f);
  mgr.BuildNearList(nearList, ballPos, down, 20.f, skLineOfSightFilter, nullptr);
  CRayCastResult hit =
      mgr.RayWorldIntersection(intersectId, ballPos, down, 20.f, skLineOfSightFilter, nearList);
  float downFactor;
  if (hit.IsValid()) {
    downFactor = CMath::Clamp(0.f, hit.GetTime() / 20.f, 1.f);
  } else {
    downFactor = 1.f;
  }

  mSplineState = kBSS_One;
  mReevalSplineEnd = true;
  mCamBehindFloorOrWall = false;
  mCamSpline.ResetKnots(4);
  mCamSpline.ResetControlPoints(4);
  mCamSpline.AddKnotAndControlPoint(GetTranslation());

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  CVector3f knot1 = mSplineIntermediatePos;
  knot1.SetZ(GetTranslation().GetZ());
  mCamSpline.AddKnotAndControlPoint(knot1);

  CVector3f delta = mSplineIntermediatePos - GetTranslation();
  delta *= 0.5f + downFactor;
  CVector3f knot2 = knot1 + delta;
  mgr.BuildNearList(nearList, knot1, delta.AsNormalized(), delta.Magnitude(), skLineOfSightFilter,
                    nullptr);
  hit = mgr.RayWorldIntersection(intersectId, knot1, delta.AsNormalized(), delta.Magnitude(),
                                 skLineOfSightFilter, nearList);
  if (hit.IsValid()) {
    knot2 = hit.GetPoint();
    if (intersectId != kInvalidUniqueId) {
      const CActor* hitActor = TCastToConstPtr< CActor >(mgr.ObjectById(intersectId));
      if (hitActor != nullptr && hitActor->GetMaterialList().HasMaterial(kMT_Floor)) {
        knot2[kDZ] += elevation;
      }
    } else if (hit.GetMaterial().HasMaterial(kMT_Floor)) {
      knot2[kDZ] += elevation;
    }
  }
  mCamSpline.AddKnotAndControlPoint(knot2);

  CVector3f toBall = ballPos - knot2;
  toBall.SetZ(0.f);
  if (toBall.IsMagnitudeSafe()) {
    toBall.Normalize();
  } else {
    toBall = Player(mgr).GetMovementDirection();
  }

  CVector3f knot3 = knot2;
  knot3 -= downFactor * delta;
  knot3.SetZ(knot2.GetZ() + (0.25f + downFactor) * delta.GetZ());
  const CVector3f secondDelta = knot3 - knot2;

  mgr.BuildNearList(nearList, knot2, secondDelta.AsNormalized(), secondDelta.Magnitude(),
                    skLineOfSightFilter, nullptr);
  hit = mgr.RayWorldIntersection(intersectId, knot2, secondDelta.AsNormalized(),
                                 secondDelta.Magnitude(), skLineOfSightFilter, nearList);
  if (hit.IsValid()) {
    knot3 = hit.GetPoint();
    if (intersectId != kInvalidUniqueId) {
      const CActor* hitActor = TCastToConstPtr< CActor >(mgr.ObjectById(intersectId));
      if (hitActor != nullptr && hitActor->GetMaterialList().HasMaterial(kMT_Floor)) {
        knot3[kDZ] += elevation;
      }
    } else if (hit.GetMaterial().HasMaterial(kMT_Floor)) {
      knot3[kDZ] += elevation;
    }
  }
  mCamSpline.AddKnotAndControlPoint(knot3);

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
    if (!SplineIntersectTest(hitMaterial, mgr) &&
        (hitMaterial.HasMaterial(kMT_Floor) || hitMaterial.HasMaterial(kMT_Wall))) {
      mCamBehindFloorOrWall = true;
      mCollisionExcludeList = CMaterialList();
    }
  }

  mSplineCtrl = 0.5f * downFactor + 0.5f;
  mSplineCtrlRange = mSplineCtrl;
  mSplineEndPosition = mCamSpline.GetControlPoint(mCamSpline.GetControlPointCount() - 1);
  x498_ = 0.f;
  mCamSpline.SetSplineType(CMotionSpline::kST_Bezier);
  mCamSpline.SetDuration(mSplineCtrlRange);
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
  CVector3f result = CVector3f::Zero();
  bool found = false;
  for (int i = 0; i < nearList.size(); ++i) {
    const CScriptRepulsor* repulsor =
        TCastToConstPtr< CScriptRepulsor >(mgr.GetObjectById(nearList[i]));
    if (repulsor == nullptr || !repulsor->GetActive() ||
        (repulsor->GetFlags() & CScriptRepulsor::kF_RepelPlayer) == 0) {
      continue;
    }

    CVector3f repulsorDirection((repulsor->GetTranslation() - GetTranslation()).ToVec2f(), 0.f);
    CVector3f ballDirection((ballPos - GetTranslation()).ToVec2f(), 0.f);
    const float radius = repulsor->GetRadius();
    found = true;
    if (!ballDirection.CanBeNormalized() || repulsorDirection.Magnitude() >= radius ||
        !(CVector3f::Dot(ballDirection, repulsorDirection) > 0.f)) {
      continue;
    }

    const CLine line(GetTranslation(), CUnitVector3f(ballDirection));
    const CVector3f closestPoint = line.GetClosestPoint(repulsor->GetTranslation());
    if (repulsorDirection.Magnitude() >= distance) {
      continue;
    }
    const float strength = repulsor->GetStrength();
    const float falloff = 1.f - CMath::Clamp(0.f, repulsorDirection.Magnitude() / radius, 1.f);
    CVector3f pushDirection((closestPoint - repulsor->GetTranslation()).ToVec2f(), 0.f);
    if (CMath::AbsF(CVector3f::Dot(pushDirection, ballDirection)) > 0.999f) {
      pushDirection = CVector3f(pushDirection.GetY(), -pushDirection.GetX(), 0.f);
    }
    if (repulsor->GetFlags() & CScriptRepulsor::kF_UseForwardVector) {
      pushDirection = repulsor->GetTransform().GetForward();
    }
    result += falloff * (strength * (dt * pushDirection.AsNormalized()));
  }
  position = result;
  return found;
}

bool CBallCamera::fn_801a36f0(float distance, float dt, CVector3f& position, CStateManager& mgr) {
  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  position = CVector3f::Zero();
  bool found = false;
  if (door != nullptr && door->IsOpen() && !door->IsHorizontal()) {
    const CScriptDock* dock =
        TCastToConstPtr< CScriptDock >(mgr.GetObjectById(door->GetConnectedDockID()));
    if (dock != nullptr) {
      const bool ballSide = dock->GetPlane(mgr).IsFacing(Player(mgr).GetBallPosition());
      const bool cameraSide = dock->GetPlane(mgr).IsFacing(GetTranslation());
      if ((ballSide && !cameraSide) || (!ballSide && cameraSide)) {
        const CLine planeLine(dock->GetTranslation(), dock->GetPlane(mgr).GetNormal());
        CVector3f fromCamera(
            CVector3f(planeLine.GetClosestPoint(GetTranslation()) - GetTranslation()).ToVec2f(),
            0.f);
        if (fromCamera.CanBeNormalized()) {
          const float strength = CMath::Clamp(0.f, fromCamera.Magnitude() / 5.f, 1.f);
          position = strength * (40.f * (dt * fromCamera.AsNormalized()));
          found = true;
        }
      }
    }
  }
  return found;
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
  if (Player(mgr).GetBombJumpCounter() == 2) {
    CVector3f lookDir = mLookPos - GetTranslation();
    if (mLookAtBall) {
      lookDir = ballPos - GetTranslation();
    }

    if (lookDir.IsMagnitudeSafe()) {
      lookDir.Normalize();
      UpdateTransform(lookDir, GetTranslation(), dt, mgr);
    }
    return;
  }

  const CPlayer* player = &Player(mgr);
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed && !mAvoidGeometryFull) {
    return;
  }

  const CTransform4f oldXf = GetTransform();
  const CVector3f oldPos = GetTranslation();
  x3b0_ = mSmallColliders.CountObscuredColliders();
  x3b4_ = mMediumColliders.CountObscuredColliders();
  x3b8_ = mLargeColliders.CountObscuredColliders();

  CVector3f ballToCamFlat = GetTransform().GetTranslation() - ballPos;
  CVector3f posAtBallLevel(0.f, 0.f, ballToCamFlat.GetZ());
  ballToCamFlat[kDZ] = 0.f;
  float ballToCamFlatMag = 0.f;
  if (ballToCamFlat.IsMagnitudeSafe()) {
    ballToCamFlatMag = ballToCamFlat.Magnitude();
  } else {
    ballToCamFlat = -player->GetMovementDirection();
  }

  posAtBallLevel = GetTransform().GetTranslation() - posAtBallLevel;
  CTransform4f ballToUnderCamLook = CTransform4f::Identity();
  if (CVector3f(posAtBallLevel - ballPos).CanBeNormalized()) {
    ballToUnderCamLook = CTransform4f::LookAt(ballPos, posAtBallLevel, CVector3f::Up());
  }

  float distance = mBallCameraSpring.ApplyDistanceSpring(mCurMinDistance, ballToCamFlatMag,
                                                         dt * (3.f + mSpeedFactor));
  CVector3f camToBall = ballPos - GetTransform().GetTranslation();
  camToBall[kDZ] = 0.f;
  if (camToBall.IsMagnitudeSafe()) {
    camToBall.Normalize();
    float dot = CVector3f::Dot(camToBall, player->GetMovementDirection());
    dot = CMath::Limit(dot, 1.f);
    if (CMath::AbsF(acosf(dot)) > (150.f * (M_PIF / 180.f))) {
      CVector3f velocity = player->GetVelocityWR();
      if (velocity.IsMagnitudeSafe()) {
        distance = mBallCameraSpring.ApplyDistanceSpring(
            mCurMinDistance + mSpeedFactor * (mBackwardsDistance - mCurMinDistance),
            ballToCamFlatMag, 3.f * dt);
      }
    }
  }

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
    if (mPrevClearLOS && player->GetMoveSpeed() < 1.2f) {
      targetColliderMag = 0.25f;
    }
    mColliderMag += 2.f * ((targetColliderMag - mColliderMag) * dt);
    mSmallColliders.UpdateCollidersDistances(mColliderMag * (7.f * 0.33f),
                                             mColliderMag * (7.f * 0.33f), -M_PIF / 2.f);
    mMediumColliders.UpdateCollidersDistances(mColliderMag * (7.f * 0.66f),
                                              mColliderMag * (7.f * 0.66f), -M_PIF / 2.f);
    mLargeColliders.UpdateCollidersDistances(mColliderMag * 7.f, mColliderMag * 7.f, -M_PIF / 2.f);
  }

  mCollidersAABB = mLargeColliders.CalculateCollidersBoundingBox();
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, mCollidersAABB, skLineOfSightFilter,
                    TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId)));

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
    if (ballToCam.IsMagnitudeSafe()) {
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
      if (ballToCamDir.IsMagnitudeSafe()) {
        ballToCamDir.Normalize();
      } else {
        ballToCamDir = -player->GetMovementDirection();
      }
      TUniqueId intersectId = kInvalidUniqueId;
      const CRayCastResult result = mgr.RayWorldIntersection(
          intersectId, ballPos, ballToCamDir, distance, skLineOfSightFilter, nearList);
      const float hitTime = result.GetTime();
      if (result.IsValid()) {
        desiredBallToCam = 0.9f * (hitTime * ballToCamDir);
      }
    }
    interpolateElevation = false;
    break;
  }
  }

  const float desiredDistance = desiredBallToCam.Magnitude();
  CVector3f desiredCamPos = ballPos + desiredBallToCam;
  float collDist = 0.f;
  bool noCollision = !DetectCollision(ballPos, ballPos + desiredBallToCam, 0.3f, collDist, mgr,
                                      GetControllerNumber());
  if (!noCollision) {
    const float collisionDistance = collDist;
    if (collisionDistance >= 1.f) {
      const CVector3f& normalizedDir = desiredBallToCam.AsNormalized();
      desiredBallToCam = collisionDistance * normalizedDir;
      desiredCamPos = ballPos + desiredBallToCam;
    } else {
      desiredCamPos = GetTranslation();
      desiredBallToCam = desiredCamPos - ballPos;
    }
  }

  CTransform4f lookXf = CTransform4f::LookAt(desiredCamPos, mLookPos, CVector3f::Up());
  CTransform4f oldLookXf = CTransform4f::LookAt(GetTranslation(), mLookPos, CVector3f::Up());
  mNextLookXf = lookXf;
  lookXf = oldLookXf;

  CVector3f colliderPointLocal = CVector3f::Zero();
  CVector3f volumeOffset = CVector3f::Zero();
  bool volumeCollision = fn_801a39d0(desiredDistance, dt, volumeOffset, mgr);
  CVector3f doorOffset = CVector3f::Zero();
  bool doorCollision = fn_801a36f0(desiredDistance, dt, doorOffset, mgr);
  if (!volumeCollision && !doorCollision) {
    if (mAvoidGeometryFull || !mClearLOS) {
      colliderPointLocal = AvoidGeometryFull(lookXf, nearList, mgr);
    } else {
      colliderPointLocal = AvoidGeometry(lookXf, nearList, mgr);
    }
  }

  CVector3f oldBallToCamFlat((GetTranslation() - ballPos).ToVec2f(), 0.f);
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
  CVector3f camDelta = rotatedColliderPoint + desiredCamPos - ballPos;
  if (camDelta.IsMagnitudeSafe()) {
    camDelta.Normalize();
  }
  CVector3f desiredPos = ballPos + distance * camDelta;

  if (mBehaviour == kBCB_Unknown6) {
    desiredPos = CameraManager(mgr).GetPathCamera()->GetTranslation();
  }

  if (volumeCollision || doorCollision) {
    if (mgr.RayCollideWorld(mDampedPos, mDampedPos + volumeOffset + doorOffset, skLineOfSightFilter,
                            this)) {
      mDampedPos += volumeOffset + doorOffset;
    } else {
      volumeCollision = false;
      doorCollision = false;
    }
  }

  camDelta = mDampedPos - desiredPos;
  float dampDeltaMag = camDelta.Magnitude();
  if (camDelta.IsMagnitudeSafe()) {
    camDelta.Normalize();
  }
  float springDist = mBallCameraCentroidSpring.ApplyDistanceSpring(0.f, dampDeltaMag, dt);
  mDampedPos = desiredPos + springDist * camDelta;

  if (volumeCollision || doorCollision) {
    mDampedPos += volumeOffset + doorOffset;
  }

  CVector3f posDelta = oldPos - mDampedPos;
  float posDeltaMag = posDelta.Magnitude();
  if (posDelta.IsMagnitudeSafe()) {
    posDelta.Normalize();
  }

  float springMag = mBallCameraCentroidDistanceSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt);

  CVector3f finalPos = mDampedPos + springMag * posDelta;
  const bool ridingPlatform = player->GetRidingPlatform() != kInvalidUniqueId;
  const CMorphBall* morphBall = player->GetMorphBall();
  if (morphBall->GetBallState() != CMorphBall::kBS_Spider && !mNoElevationVelClamp &&
      !ridingPlatform && morphBall->GetBallState() != CMorphBall::kBS_ScrewAttack) {
    const uint framesSinceFloor = mgr.GetUpdateFrameIdx() - morphBall->GetLastFloorCollisionFrame();
    if (player->GetVelocityWR().GetZ() > 8.f && framesSinceFloor >= 2) {
      CVector3f delta = finalPos - oldPos;
      delta[kDZ] = CMath::Limit(delta.GetZ(), 0.1f * dt);
      finalPos = oldPos + delta;
    }
    if (framesSinceFloor < 2 && player->GetPlayerMovementState() != NPlayer::kMS_OnGround) {
      finalPos.SetZ(oldPos.GetZ());
    }
  }

  if (morphBall->GetBallState() == CMorphBall::kBS_ScrewAttack) {
    finalPos.SetZ(oldPos.GetZ());
    const float ceilingZ = player->GetLastSpaceJumpPosition().GetZ() + 5.f;
    float targetZ = ceilingZ;
    rstl::reserved_vector< TUniqueId, 1024 > blockers;
    TUniqueId hitId = kInvalidUniqueId;
    CVector3f forward(GetTransform().GetForward().ToVec2f(), 0.f);
    if (forward.CanBeNormalized()) {
      forward.Normalize();
      CVector3f castPos(finalPos.GetX(), finalPos.GetY(), ceilingZ + 0.6f);
      for (;;) {
        mgr.BuildNearList(blockers, castPos, forward, 10.f, skLineOfSightFilter, nullptr);
        const CRayCastResult hit =
            mgr.RayWorldIntersection(hitId, castPos, forward, 10.f, skLineOfSightFilter, blockers);
        if (!hit.IsValid()) {
          break;
        }
        targetZ -= 0.6f;
        castPos.SetZ(castPos.GetZ() - 0.6f);
      }
    }
    const float upwardDistance = targetZ - finalPos.GetZ();
    if (upwardDistance > 0.f) {
      mgr.BuildNearList(blockers, finalPos, CVector3f::Up(), upwardDistance, skLineOfSightFilter,
                        nullptr);
      const CRayCastResult hit = mgr.RayWorldIntersection(
          hitId, finalPos, CVector3f::Up(), upwardDistance, skLineOfSightFilter, blockers);
      if (hit.IsValid()) {
        targetZ = rstl::min_val(targetZ, hit.GetPoint().GetZ() - 0.6f);
      }
    }
    const float deltaZ = targetZ - finalPos.GetZ();
    const float step = 8.f * dt * CMath::Clamp(0.f, CMath::AbsF(deltaZ / 0.25f), 1.f);
    if (!close_enough(deltaZ, 0.05f)) {
      finalPos.SetZ(finalPos.GetZ() + (deltaZ < 0.f ? -step : step));
    } else {
      finalPos.SetZ(ceilingZ);
    }
  }

  if (mClearLOS && morphBall->GetBallState() != CMorphBall::kBS_ScrewAttack) {
    float movementFactor = 0.f;
    if (player->GetVelocityWR().Magnitude() > 15.f) {
      movementFactor = CMath::Limit((player->GetVelocityWR().Magnitude() - 15.f) / 15.f, 1.f);
    } else if (mObtuseDirection) {
      movementFactor = mSpeedFactor;
    }
    CVector3f flatDelta = posDelta;
    flatDelta.SetZ(0.f);
    float alignment = 0.f;
    if (flatDelta.CanBeNormalized() && player->GetLeaveMorphDirection().CanBeNormalized()) {
      alignment = CMath::AbsF(CMath::Limit(
          CVector3f::Dot(player->GetLeaveMorphDirection(), flatDelta.AsNormalized()), 1.f));
    }
    if (ridingPlatform) {
      finalPos.SetZ(finalPos.GetZ() + mBallDelta.GetZ());
      mDampedPos.SetZ(mDampedPos.GetZ() + mBallDelta.GetZ());
      mLookPos.SetZ(mLookPos.GetZ() + mBallDelta.GetZ());
      mLookPosAhead.SetZ(mLookPosAhead.GetZ() + mBallDelta.GetZ());
      mFixedLookPos.SetZ(mFixedLookPos.GetZ() + mBallDelta.GetZ());
    } else {
      movementFactor *= alignment;
      finalPos += movementFactor * mBallDeltaFlat;
    }
  }

  if (morphBall->GetBallState() != CMorphBall::kBS_ScrewAttack) {
    if (interpolateElevation && mState != kBCS_ToBall) {
      finalPos = InterpolateCameraElevation(finalPos, dt);
    }
    if (mNoElevationInterp) {
      finalPos[kDZ] = elevation + ballPos.GetZ();
    }

    if (CameraManager(mgr).GetHintManager()->HasHint(mgr)) {
      const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(
          CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr));
      if (hint != nullptr && (hint->GetInfo().GetFlags() & (1u << 22))) {
        finalPos.SetZ(hint->GetTranslation().GetZ());
      }
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
          finalPos = GetTranslation();
        }
      }
    }
  }

  const float backupZ = finalPos.GetZ();
  finalPos = MoveCollisionActor(finalPos, dt, mgr);
  if (mClearLOS && mShortMoveCount > 0) {
    finalPos[kDZ] = backupZ;
    finalPos = MoveCollisionActor(finalPos, dt, mgr);
  }

  CVector3f lookDir = mLookPos - finalPos;
  if (mLookAtBall) {
    lookDir = ballPos - finalPos;
  }
  if (lookDir.IsMagnitudeSafe()) {
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
  CVector3f result = currentVelocity;
  CVector3f delta = newVelocity - currentVelocity;
  if (delta.IsMagnitudeSafe()) {
    float t = CMath::Limit(delta.Magnitude() / (rate * dt), 1.f);
    result += t * (dt * (rate * delta.AsNormalized()));
  } else {
    result = newVelocity;
  }
  return result;
}

CVector3f CBallCamera::ComputeVelocity(CVector3f currentVelocity, CVector3f positionDelta,
                                       float dt) {
  CVector3f velocity = positionDelta;
  float magnitude = velocity.Magnitude();
  if (mClampVelTimer > 0.f && velocity.IsMagnitudeSafe() && !mObtuseDirection) {
    velocity = velocity.AsNormalized() * CMath::Limit(magnitude, mClampVelRange);
  }
  return velocity;
}

void CBallCamera::UpdateAnglePerSecond(float dt) {
  float delta = mTargetAnglePerSecond - mCurAnglePerSecond;
  if (CMath::AbsF(delta) >= 0.0017453292f) {
    const float limited = CMath::Limit(delta / M_PIF, 1.f);
    mCurAnglePerSecond += limited * (10.471975f * dt);
  } else {
    mCurAnglePerSecond = mTargetAnglePerSecond;
  }
}

CVector3f CBallCamera::ClampElevationToWater(CVector3f position, CStateManager& mgr) const {
  const CScriptWater* water =
      TCastToConstPtr< CScriptWater >(mgr.GetObjectById(Player(mgr).InFluidId()));
  if (water == nullptr) {
    water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
  }
  CVector3f pos = position;
  if (water != nullptr) {
    const float waterZ = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
    const float deltaZ = position.GetZ() - waterZ;
    if (position.GetZ() >= waterZ && deltaZ <= 0.25f) {
      pos.SetZ(waterZ + 0.25f);
    } else if (position.GetZ() < waterZ && deltaZ >= -0.12f) {
      pos.SetZ(waterZ - 0.12f);
    }
  }
  return pos;
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
  actor->AddMaterial(kMT_Solid, mgr);
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
  actor->RemoveMaterial(kMT_Solid, mgr);
  return actor->GetTranslation();
}

void CBallCamera::UpdateLookAtPosition(float dt, CStateManager& mgr, bool teleport) {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()));
  if (player == nullptr) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetWatchedObject()))) {
      mLookPos = actor->GetScanObjectIndicatorPosition(mgr);
    }
    return;
  }

  if (player->GetBombJumpCounter() == 1) {
    const CScriptDoor* door =
        TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(GetTooCloseActorId()));
    if (door != nullptr && !door->IsOpen()) {
      return;
    }
  }

  const CVector3f ballPosition = player->GetBallPosition();
  CVector3f movementDirection = player->GetMovementDirection();
  movementDirection.Normalize();
  CVector3f offset(mSpeedFactor * mLookAtOffset.GetX(), mSpeedFactor * mLookAtOffset.GetY(),
                   mLookAtOffset.GetZ());
  const CTransform4f moveRotation = player->CreateTransformFromMovementDirection().GetRotation();
  if (mBallDeltaFlat.IsMagnitudeSafe()) {
    offset = moveRotation * offset;
  }

  const CVector3f previousLook = mLookPos;
  CVector3f lookAhead = ballPosition + offset;
  bool cameraTransitioning = false;
  if (CameraManager(mgr).IsBallCameraTransitioning(mgr)) {
    cameraTransitioning = true;
    if (CameraManager(mgr).HintManager()->HasHint(mgr) &&
        CameraManager(mgr).HintManager()->GetBestHintState() != nullptr) {
      const CScriptTrigger* sender = TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(
          CameraManager(mgr).HintManager()->GetBestHintState()->GetFirstSender()));
      if (sender != nullptr && (sender->GetTriggerFlags() & 0x10000006) == 0x10000000) {
        cameraTransitioning = false;
      }
    }
  }

  if (cameraTransitioning) {
    offset.SetZ(GetTranslation().GetZ() - 2.f);
    lookAhead.SetZ(offset.GetZ());
  }
  mLookPosAhead = lookAhead;
  mFixedLookPos = ballPosition + CVector3f(0.f, 0.f, offset.GetZ());

  if (!teleport) {
    CVector3f lookDelta = previousLook - lookAhead;
    const float lookDeltaMagnitude = lookDelta.Magnitude();
    if (lookDelta.IsMagnitudeSafe()) {
      lookDelta.Normalize();
    }
    float speedingTime = mSpeedingTime / 3.f;
    float springScale = 1.f;
    springScale += 2.f * CMath::Clamp(0.f, speedingTime, 1.f);
    const float springDistance =
        mBallCameraLookAtSpring.ApplyDistanceSpring(0.f, lookDeltaMagnitude, dt * springScale);
    if (springDistance > 0.0001f) {
      lookAhead += springDistance * lookDelta;
    }
    lookDelta = lookAhead - previousLook;
  }
  mLookPos = lookAhead;

  if (mDirectElevation) {
    mLookPos.SetZ(ballPosition.GetZ() + mLookAtOffset.GetZ());
    mLookPosAhead.SetZ(mLookPos.GetZ());
    mFixedLookPos.SetZ(mLookPos.GetZ());
  }
  if (player->IsMorphBallTransitioning()) {
    mLookPos = mLookPosAhead;
    mLookPosAhead = mLookPos;
    mFixedLookPos = mLookPos;
  }

  if (mOverrideLookDir && mBehaviour != kBCB_Unknown4 && mBehaviour != kBCB_Unknown5 &&
      CameraManager(mgr).HintManager()->HasHint(mgr) && !cameraTransitioning) {
    const CTransform4f hintTransform =
        CameraManager(mgr).HintManager()->GetCurrentHint(mgr)->GetTransform();
    const CVector3f toBall = Player(mgr).GetBallPosition() - GetTranslation();
    const float distance = CVector3f::Dot(toBall, hintTransform.GetForward());
    mLookPos = hintTransform.GetTranslation() + distance * hintTransform.GetForward();
    mLookPosAhead = mLookPos;
    mFixedLookPos = mLookPos;
  }
}

CVector3f CBallCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CPlayer& player = GetPlayer(mgr);
  if (player.GetCameraState() == CPlayer::kCS_MorphBallTransition) {
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
  const CTransform4f xf = CTransform4f::LookAt(position, mLookPos, CVector3f::Up());
  TeleportCamera(xf, mgr);
  const_cast< CCameraManager& >(GetCameraManager(mgr)).SetPlayerCamera(mgr, GetUniqueId());

  mPendingFailsafe = false;
  mObscuredTime = 0.f;
  const_cast< CCameraManager& >(GetCameraManager(mgr)).StartScreenFlash();
}

void CBallCamera::CheckFailSafe(float dt, CStateManager& mgr) {
  if (CameraManager(mgr).GetCurrentCameraId(false) != GetUniqueId() &&
      !CameraManager(mgr).IsInterpolationCameraActive()) {
    return;
  }
  if (CameraManager(mgr).IsInterpolationCameraActive() &&
      CameraManager(mgr).GetInterpolationCamera()->GetTargetId() != GetUniqueId()) {
    return;
  }
  if ((mgr.GetUpdateFrameIdx() & 3) != GetControllerNumber()) {
    mObscuredTime += dt;
    return;
  }

  const CVector3f ballPos = Player(mgr).GetBallPosition();
  mPrevClearLOS = mClearLOS;
  CVector3f cameraToBall = ballPos - GetTranslation();
  const float rayLength = cameraToBall.Magnitude();
  cameraToBall.Normalize();

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, GetTranslation(), cameraToBall, rayLength, skLineOfSightFilter,
                    nullptr);
  const CRayCastResult hit = mgr.RayWorldIntersection(
      mObscuringObjectId, GetTranslation(), cameraToBall, rayLength, skLineOfSightFilter, nearList);
  const CPlayer& player = *mgr.GetPlayer(GetControllerNumber());
  if (!hit.IsValid()) {
    mClearLOS = true;
    mObscuringMaterial = CMaterialList(kMT_NoStepLogic);
  } else {
    mObscuringMaterial = hit.GetMaterial();
    CVector3f upperBallPos = ballPos;
    upperBallPos.SetZ(upperBallPos.GetZ() + player.GetTweakPlayer()->GetBallRadius());
    const CVector3f lowerBallPos = player.GetTranslation();
    const bool clearAbove =
        mgr.RayCollideWorld(GetTranslation(), upperBallPos, nearList, skLineOfSightFilter, &player);
    const bool clearBelow =
        mgr.RayCollideWorld(GetTranslation(), lowerBallPos, nearList, skLineOfSightFilter, &player);
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
    ShouldResetSpline(mgr);
  }
  mUnobscureMag = CMath::Clamp(0.f, 0.5f * mObscuredTime, 1.f);
  if (mObscureAvoidance &&
      (mObscuredTime > 2.f || (mTooCloseActorId != kInvalidUniqueId && mObscuredTime > 1.f)) &&
      !mClearLOS && mSplineState == kBSS_Invalid) {
    mPendingFailsafe = true;
  } else {
    mPendingFailsafe = false;
  }
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
      TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionActorId));
  if (collisionActor != nullptr) {
    mgr.SetActorAreaId(*collisionActor, areaId);
  }

  const CPlayer::EPlayerCameraState cameraState = player.GetCameraState();
  if (cameraState != CPlayer::kCS_Ball && cameraState != CPlayer::kCS_MorphBallTransition &&
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
    case kBCB_FixedTransform:
      SetTransform(mFixedTransform);
      break;
    case kBCB_Unknown7:
      mLookPos += mBallDelta;
      mLookPosAhead += mBallDelta;
      mFixedLookPos += mBallDelta;
      break;
    default:
      break;
    }
    break;
  case kBCS_ToBall:
  case kBCS_FromBall:
    UpdateUsingTransitions(dt, mgr);
    break;
  }

  const CTransform4f nextTransform = ValidateCameraTransform(GetTransform(), oldTransform, dt);
  SetTransform(nextTransform);
  CActor::Think(dt, mgr);
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr) {
  switch (state) {
  case kBCS_ToBall: {
    const CTransform4f xf = CameraManager(mgr).GetFirstPersonCamera()->GetTransform();
    SetTransform(xf);
    TeleportCamera(xf.GetTranslation(), mgr);
    const CFirstPersonCamera* fpCam = CameraManager(mgr).GetFirstPersonCamera();
    InterpolateFOV(fpCam->GetFov(), 1.f, 0.f, GetUniqueId(), mgr);
    InvalidateSpline();
  }
  case kBCS_Default:
    mgr.SetGameState(CStateManager::kGS_Running);
    break;
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
  const EScriptObjectMessage message = msg.GetMessage();
  CGameCamera::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create: {
    mCollisionActorId = mgr.AllocateUniqueId();
    CCollisionActor* actor = rs_new CCollisionActor(mCollisionActorId, GetAreaIdForPersistence(),
                                                    kInvalidUniqueId, true, 0.3f, 1.f);
    if (actor != nullptr) {
      CMaterialList include(kMT_Solid);
      CMaterialList exclude(kMT_Player, kMT_CameraPassthrough);
      CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(include, exclude);
      actor->SetMaterialFilter(filter);
      actor->MaterialList() = CMaterialList(kMT_ProjectilePassthrough, kMT_ScanPassthrough,
                                            kMT_SeeThrough, kMT_CameraPassthrough);
      actor->SetTranslation(GetTranslation());
      mgr.AddObject(actor);
      actor->SetMovable(false);
      actor->SetLastNonCollidingState(CMotionState(
          GetTranslation(), CNUQuaternion::BuildFromAxisAngle(CVector3f::Forward(), 0.f),
          CVector3f::Zero(), CAxisAngle::Identity()));
      actor->SetEnableRender(false);
    }

    CMaterialList include;
    CMaterialList exclude(kMT_Solid, kMT_ProjectilePassthrough, kMT_Player, kMT_Character,
                          kMT_CameraPassthrough);
    CMaterialFilter selfFilter = CMaterialFilter::MakeIncludeExclude(include, exclude);
    SetMaterialFilter(selfFilter);
    RemoveMaterial(kMT_Solid, mgr);
    break;
  }
  case kSM_Delete:
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
  mBehaviour = info.GetBehaviourType();
  if ((info.GetFlags() & 0x2) != 0) {
    mChaseAllowed = true;
  } else {
    mChaseAllowed = false;
  }
  if ((info.GetFlags() & 0x4) != 0) {
    mBoostAllowed = true;
  } else {
    mBoostAllowed = false;
  }
  if ((info.GetFlags() & 0x8) != 0) {
    mObscureAvoidance = true;
  } else {
    mObscureAvoidance = false;
  }
  if ((info.GetFlags() & 0x10) != 0) {
    mVolumeCollider = true;
  } else {
    mVolumeCollider = false;
  }
  if (info.GetFlags() & 0x40) {
    mLookAtBall = true;
  }
  if ((info.GetFlags() & 0x4000) != 0) {
    mNoElevationInterp = true;
  } else {
    mNoElevationInterp = false;
  }
  if ((info.GetFlags() & 0x8000) != 0) {
    mDirectElevation = true;
  } else {
    mDirectElevation = false;
  }
  if ((info.GetFlags() & 0x10000) != 0) {
    mOverrideLookDir = true;
  } else {
    mOverrideLookDir = false;
  }
  if ((info.GetFlags() & 0x20000) != 0) {
    mNoElevationVelClamp = true;
  } else {
    mNoElevationVelClamp = false;
  }
  if ((info.GetFlags() & 0x80000) != 0) {
    mNoSpline = true;
  } else {
    mNoSpline = false;
  }
  if ((info.GetFlags() & 0x100000) != 0) {
    x206_26_ = true;
  } else {
    x206_26_ = false;
  }

  if (info.GetOverrideFlags() & 0x1) {
    mTargetMinDistance = info.GetMinDist();
  }
  if (info.GetOverrideFlags() & 0x2) {
    mMaxDistance = info.GetMaxDist();
  }
  if (info.GetOverrideFlags() & 0x4) {
    mBackwardsDistance = info.GetBackwardsDist();
  }
  if (info.GetOverrideFlags() & 0x100) {
    mElevation = info.GetElevation();
  }
  if (info.GetOverrideFlags() & 0x8) {
    mLookAtOffset = info.GetLookAtOffset();
  }
  if ((info.GetOverrideFlags() & 0x20) != 0) {
    mClampAttitude = true;
    mAttitudeRange = info.GetAttitudeRange();
  } else {
    mClampAttitude = false;
  }
  if ((info.GetOverrideFlags() & 0x40) != 0) {
    mClampAzimuth = true;
    mAzimuthRange = info.GetAzimuthRange();
  } else {
    mClampAzimuth = false;
  }
  if (info.GetOverrideFlags() & 0x10) {
    InterpolateFOV(info.GetFov(), 1.f, 0.f);
  }
  if (info.GetOverrideFlags() & 0x80) {
    mTargetAnglePerSecond = info.GetAnglePerSecond();
  }

  if (info.GetFlags() & 0x200) {
    Player(mgr).SetControlDirectionInterpolation(info.GetControlInterpDur());
  } else {
    const CScriptPlayerHint* playerHint = TCastToConstPtr< CScriptPlayerHint >(
        Player(mgr).GetPlayerHintManager()->GetCurrentHint(mgr));
    if (playerHint != nullptr) {
      if ((playerHint->GetOverrideFlags() & 2) == 0) {
        Player(mgr).ResetControlDirectionInterpolation();
      }
    } else {
      Player(mgr).ResetControlDirectionInterpolation();
    }
  }

  switch (mBehaviour) {
  case kBCB_HintBallToCam: {
    const CVector3f ballToCam = info.GetWorldOffset();
    mOverrideBallToCam = ballToCam;
    const CVector3f ballPos = Player(mgr).GetBallPosition();
    CVector3f cameraPos = ballPos + ballToCam;
    if (info.GetFlags() & 0x1) {
      const float distance = ballToCam.ToVec2f().Magnitude();
      const CVector3f direction = -CVector3f(ballToCam.ToVec2f(), 0.f).AsNormalized();
      cameraPos = FindDesiredPosition(distance, ballToCam.GetZ(), direction, mgr, false);
    }
    const CTransform4f cameraXf = CTransform4f::LookAt(cameraPos, mLookPos, CVector3f::Up());
    TeleportCamera(cameraXf, mgr);
    break;
  }
  case kBCB_HintLocalOffset: {
    mHintLocalOffset = info.GetWorldOffset();
    const CVector3f ballToCam = hint->GetTransform().Rotate(mHintLocalOffset);
    mOverrideBallToCam = ballToCam;
    CVector3f cameraPos = Player(mgr).GetBallPosition() + ballToCam;
    if (info.GetFlags() & 0x1) {
      const float distance = ballToCam.ToVec2f().Magnitude();
      const CVector3f direction = -CVector3f(ballToCam.ToVec2f(), 0.f).AsNormalized();
      cameraPos = FindDesiredPosition(distance, ballToCam.GetZ(), direction, mgr, false);
    }
    const CTransform4f cameraXf = CTransform4f::LookAt(cameraPos, mLookPos, CVector3f::Up());
    TeleportCamera(cameraXf, mgr);
    break;
  }
  case kBCB_Default:
    if (info.GetFlags() & 0x20) {
      if (info.GetFlags() & 0x40000) {
        const CTransform4f current = CameraManager(mgr).GetCurrentCameraTransform(mgr, false);
        CVector3f direction = Player(mgr).GetTranslation() - current.GetTranslation();
        direction.SetZ(0.f);
        if (direction.IsMagnitudeSafe()) {
          direction.Normalize();
        } else {
          direction = Player(mgr).GetMovementDirection();
        }
        TeleportCamera(FindDesiredTransform(direction, mgr), mgr);
      } else {
        const CTransform4f cameraXf =
            CTransform4f::LookAt(hint->GetTranslation(), mLookPos, CVector3f::Up());
        TeleportCamera(cameraXf, mgr);
      }
    }
    break;
  case kBCB_FreezeLookPosition:
  case kBCB_HintInitializePosition:
    if (info.GetFlags() & 0x20) {
      float distance = mCurMinDistance;
      float elevation = mElevation;
      ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
      const CVector3f desiredPos =
          FindDesiredPosition(distance, elevation, Player(mgr).GetMovementDirection(), mgr, false);
      const CTransform4f cameraXf = CTransform4f::LookAt(desiredPos, mLookPos, CVector3f::Up());
      TeleportCamera(cameraXf, mgr);
    }
    break;
  default:
    break;
  }
  if (info.GetFlags() & 0x20) {
    CameraManager(mgr).SetPlayerCamera(mgr, GetUniqueId());
  }
  if (TCastToConstPtr< CActor >(mgr.GetObjectById(hint->GetCameraTargetId())) != nullptr) {
    SetWatchedObject(hint->GetCameraTargetId());
  }
}

bool CBallCamera::SplineIntersectTest(CMaterialList& intersectMaterial, CStateManager& mgr) const {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialList include(kMT_Solid, kMT_Floor, kMT_Wall);
  const CMaterialList exclude(kMT_ProjectilePassthrough, kMT_Player, kMT_Character,
                              kMT_CameraPassthrough);
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(include, exclude);
  return GetCameraManager(mgr).CheckSplineCollision(mCamSpline, 0, filter, mgr, intersectMaterial,
                                                    mCamSpline.GetLength() / 12.f, 0.3f);
}

void CBallCamera::InvalidateSpline() { mSplineState = kBSS_Invalid; }
