#include "MetroidPrime/Cameras/CBallCamera.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"

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
  SetTransformAlt(xf);
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
  // TODO: combine world and dynamic collision tests for the selected player's camera.
  return false;
}

bool CBallCamera::fn_801a6b20(const CVector3f& from, const CVector3f& direction, CVector3f& result,
                              CStateManager& mgr) {
  // TODO: recover the near-list collision search and its original name.
  return false;
}

bool CBallCamera::fn_801a67a4(float radius, const CVector3f& from, const CVector3f& direction,
                              const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                              CVector3f& result, CStateManager& mgr) {
  // TODO: recover this collision-search helper and its original name.
  return false;
}

CVector3f CBallCamera::FindDesiredPosition(float distance, float elevation, CVector3f direction,
                                           CStateManager& mgr, bool fullTest) {
  // TODO: recover spherical collision searches and water/door placement constraints.
  return GetTranslation();
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
  // TODO: select a nearby door and retain its ID, distance and closing state.
}

bool CBallCamera::ConstrainElevationAndDistance(float& elevation, float& distance, float dt,
                                                CStateManager& mgr) {
  // TODO: apply camera-hint and nearby-door distance/elevation constraints.
  return false;
}

CVector3f CBallCamera::ConstrainYawAngle(const CPlayer& player, float yawSpeed, float dampenAngle,
                                         float dt, CStateManager& mgr) {
  // TODO: recover door-dependent movement direction and damped yaw rotation.
  return CVector3f::Zero();
}

void CBallCamera::UpdateTransform(const CVector3f& lookDirection, const CVector3f& position,
                                  float dt, CStateManager& mgr) {
  // TODO: recover hint orientation and state-dependent angular limits.
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
  // TODO: construct Echoes motion-spline control points and collision exclusions.
}

void CBallCamera::UpdateUsingSpline(float dt, CStateManager& mgr) {
  // TODO: advance the motion spline and recover collision-tested camera placement.
}

bool CBallCamera::fn_801a39d0(float distance, float dt, CVector3f& position, CStateManager& mgr) {
  // TODO: recover the volume-avoidance search and its original name.
  return false;
}

bool CBallCamera::fn_801a36f0(float distance, float dt, CVector3f& position, CStateManager& mgr) {
  // TODO: recover the door/dock-plane avoidance search and its original name.
  return false;
}

void CBallCamera::UpdateUsingColliders(float dt, CStateManager& mgr) {
  // TODO: combine collider avoidance, door/volume constraints, splines and camera hints.
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
      SetTransformAlt(CTransform4f::LookAt(position, mLookPos));
    }
  }
}

void CBallCamera::UpdateUsingTransitions(float dt, CStateManager& mgr) {
  // TODO: dispatch morph transitions and synchronize the first-person camera transform.
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
  // TODO: use the player's or camera's fluid actor to avoid the water surface.
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
  // TODO: recover hint-local, fixed-target and movement-predicted look positions.
}

CVector3f CBallCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  // TODO: interpolate with the first-person indicator during camera transitions.
  return mLookPos;
}

void CBallCamera::ActivateFailSafe(float dt, CStateManager& mgr) {
  // TODO: choose and apply a collision-tested recovery transform.
}

void CBallCamera::CheckFailSafe(float dt, CStateManager& mgr) {
  // TODO: track obscuration, doors and prolonged short collision moves.
}

bool CBallCamera::CheckDoorProximity(const CVector3f& position, const CStateManager& mgr) const {
  // TODO: test the nearby door's bounds and plane against the camera position.
  return false;
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
  // TODO: recover update dispatch, camera-hint gating and collision-actor synchronization.
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr) {
  // TODO: restore camera transforms, FOV interpolation and manager state on transitions.
  mState = state;
}

void CBallCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  // TODO: recover player-hint gating and the per-player chase, boost and free-look inputs.
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
  // TODO: apply Echoes camera-hint overrides and delegated-camera state.
}

bool CBallCamera::SplineIntersectTest(CMaterialList& intersectMaterial, CStateManager& mgr) const {
  // TODO: test the motion spline against the camera manager's collision query.
  return false;
}

void CBallCamera::InvalidateSpline() { mSplineState = kBSS_Invalid; }
