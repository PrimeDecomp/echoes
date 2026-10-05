#include "MetroidPrime/Player/CPlayer.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/CCameraHintManager.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerBodyController.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CCollisionCache.hpp"
#include <float.h>
#include <math.h>

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerRagDoll.hpp"

static const CMaterialList BallTransitionInclude(kMT_Unknown59);
static const CMaterialList BallTransitionExclude(kMT_NoPlatformCollision, kMT_Player, kMT_Character,
                                                 kMT_CameraPassthrough);
static const CMaterialFilter BallTransitionCollide =
    CMaterialFilter::MakeIncludeExclude(BallTransitionInclude, BallTransitionExclude);

static const float skStrafeDistances[] = {11.8f, 11.8f, 11.8f, 5.f, 6.f, 5.f, 5.f, 6.f};
static const float skDashStrafeDistances[] = {11.8f, 30.f, 22.6f, 10.f, 10.f, 10.f, 10.f, 10.f};
static const float skDashClampSpeeds[] = {11.8f, 18.f, 15.f, 10.f, 10.f, 10.f, 10.f, 10.f};
static const float skOrbitForwardDistances[] = {11.8f, 11.8f, 11.8f, 5.f, 6.f, 5.f, 5.f, 6.f};

CVector3f CPlayer::GetDampedClampedVelocityWR() const {
  const float acceleration = GetAcceleration();
  CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
  if (mOrbitState == kOS_NoOrbit) {
    float friction = GetTweakPlayer()->GetPlayerTranslationFriction(GetSurfaceRestraint());
    if (GetSurfaceRestraint() == kSR_Air) {
      friction =
          3.5f * (CVector2f(localVelocity.GetX(), localVelocity.GetY()).Magnitude() / GetMass());
    }
    friction *= acceleration;
    if (localVelocity.GetY() > 0.f) {
      localVelocity.SetY(CMath::Max(0.f, localVelocity.GetY() - friction));
    } else {
      localVelocity.SetY(CMath::Min(0.f, localVelocity.GetY() + friction));
    }
    if (localVelocity.GetX() > 0.f) {
      localVelocity.SetX(CMath::Max(0.f, localVelocity.GetX() - friction));
    } else {
      localVelocity.SetX(CMath::Min(0.f, localVelocity.GetX() + friction));
    }
  }
  const float maxSpeed = GetTweakPlayer()->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  localVelocity.SetY(CMath::Limit(localVelocity.GetY(), maxSpeed / acceleration));
  if (mMovementState == NPlayer::kMS_OnGround) {
    localVelocity.SetZ(0.f);
  }
  return GetTransform().Rotate(localVelocity);
}

float CPlayer::GetAverageSpeed() const {
  if (mMoveSpeedAvg.GetAverage()) {
    return *mMoveSpeedAvg.GetAverage();
  }
  return mMoveSpeed;
}

float CPlayer::GetAcceleration() const {
  if (mCurAcceleration >= mAccelerationTable.size()) {
    return mAccelerationTable.back();
  }
  return mAccelerationTable[mCurAcceleration];
}

float CPlayer::GetGravity() const {
  if (x126b_26_) {
    if (!gpGameState->GetPlayerState()->HasPowerUp(CPlayerState::kIT_LightSuit)) {
      return GetTweakPlayer()->GetFluidGravAccel();
    }
  } else if (!gpGameState->GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravityBoost) &&
             CheckSubmerged()) {
    return GetTweakPlayer()->GetFluidGravAccel();
  }
  if (mSidewaysDashing) {
    return -100.f;
  }
  return GetTweakPlayer()->GetNormalGravAccel();
}

float CPlayer::GetWeight() const { return GetMass() * -GetGravity(); }

void CPlayer::UpdateBombJumpStuff() {
  if (mBombJumpCount == 0) {
    return;
  }
  if (--mBombJumpCheckDelayFrames > 0) {
    return;
  }
  CVector3f flatVelocity = GetVelocityWR();
  flatVelocity.SetZ(0.f);
  if (mMovementState == NPlayer::kMS_OnGround ||
      (flatVelocity.CanBeNormalized() && flatVelocity.Magnitude() > 6.f)) {
    mBombJumpCount = 0;
  }
}

void CPlayer::UpdateStepCameraZBias(float dt, CStateManager& mgr) {
  float newBias = GetTranslation()[kDZ] + GetUnbiasedEyeHeight();
  bool ridingMovingPlatform = false;
  if (mRidingPlatform != kInvalidUniqueId) {
    if (const CScriptPlatform* platform =
            TCastToConstPtr< CScriptPlatform >(mgr.GetObjectById(mRidingPlatform))) {
      ridingMovingPlatform = platform->IsMotionActive();
    }
  }
  if (mMovementState == NPlayer::kMS_OnGround && !IsMorphBallTransitioning() &&
      !ridingMovingPlatform) {
    const float oldBias = newBias;
    if (!mStepCameraZBiasDirty) {
      const float delta = newBias - mStepCameraZBias;
      const float verticalStep = dt * GetVelocityWR().GetZ();
      float newDelta = 5.f * dt;
      if (delta > 0.f) {
        if (delta > verticalStep && delta > newDelta) {
          if (delta > GetStepUpHeight()) {
            newDelta += delta - GetStepUpHeight();
          }
          newBias = mStepCameraZBias + newDelta;
        }
      } else if (delta < verticalStep && delta < -newDelta) {
        if (delta < -GetStepDownHeight()) {
          newDelta += -delta - GetStepDownHeight();
        }
        newBias = mStepCameraZBias - newDelta;
      }
    }
    SetEyeZBias(newBias - oldBias);
  } else {
    SetEyeZBias(0.f);
  }
  mStepCameraZBias = newBias;
  mStepCameraZBiasDirty = false;
}

bool CPlayer::SidewaysDashAllowed(float strafeInput, float forwardInput,
                                  const CFinalInput& input) const {
  if (mSlidingOnWall || mHitWallDuringMove || mOrbitState != kOS_OrbitObject) {
    return false;
  }
  if (GetTweakPlayer()->GetDashOnButtonRelease()) {
    if (mOrbitState != kOS_NoOrbit && GetTweakPlayer()->GetDashEnabled() &&
        mStartingJumpTimeout > 0.f && !JumpHeld(input) &&
        mDashButtonHoldTime < GetTweakPlayer()->GetDashButtonHoldCancelTime() &&
        CMath::AbsF(strafeInput) >= CMath::AbsF(forwardInput) &&
        CMath::AbsF(strafeInput) > GetTweakPlayer()->GetDashStrafeInputThreshold()) {
      return true;
    }
  } else if (mOrbitState != kOS_NoOrbit && GetTweakPlayer()->GetDashEnabled() &&
             JumpPressed(input) && mStartingJumpTimeout > 0.f &&
             CMath::AbsF(strafeInput) >= CMath::AbsF(forwardInput) &&
             CMath::AbsF(strafeInput) > 0.01f) {
    const CVector3f stickEdge = CalculateLeftStickEdgePosition(strafeInput, forwardInput);
    const float inputMagnitude =
        CMath::SqrtF(strafeInput * strafeInput + forwardInput * forwardInput);
    const float threshold = inputMagnitude / stickEdge.Magnitude();
    if (threshold >= GetTweakPlayer()->GetDashStrafeInputThreshold()) {
      return true;
    }
  }
  return false;
}

void CPlayer::FinishSidewaysDash() {
  if (mSidewaysDashing) {
    mDoneSidewaysDashing = true;
    if (mMovementState != NPlayer::kMS_OnGround) {
      const CVector3f velocity = GetVelocityWR();
      const CVector3f flatVelocity(CVector2f(velocity.GetX(), velocity.GetY()), 0.f);
      const float maxSpeed = skDashClampSpeeds[GetSurfaceRestraint()];
      const float speed = flatVelocity.Magnitude();
      if (speed > maxSpeed) {
        float acceleration = 1.f;
        if (mAccelerationChangeTimer > 0.f) {
          acceleration = GetAcceleration();
        }
        const float scale = (speed - acceleration * (speed - maxSpeed)) / speed;
        SetVelocityWR(CVector3f(scale * velocity.GetX(), scale * velocity.GetY(), velocity.GetZ()));
      }
    }
  }
  mSidewaysDashing = false;
  mStrafeInputAtDash = 0.f;
  mDashTimer = 0.f;
}

void CPlayer::BeginSidewaysDash(float strafeInput, CStateManager& mgr) {
  mSidewaysDashing = true;
  mStrafeInputAtDash = strafeInput;
  mDoneSidewaysDashing = true;
  mDashTimer = 0.f;
  CVector3f velocity = GetVelocityWR();
  if (velocity.GetZ() > 0.f) {
    velocity.SetZ(velocity.GetZ() * 0.1f);
    if (!mSlidingOnWall) {
      SetVelocityWR(velocity);
      mDashSfx =
          CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x5c6, 0x2826), 127,
                                GetSoundPan(kMSP_4), GetCurrentAreaId().Value(), true, false);
      CSfxManager::SetIgnoreAreaLowPass(mDashSfx, true);
      ApplySubmergedPitchBend(mDashSfx);
      mgr.RumbleManager(GetPlayerIndex())->Rumble(mgr, kRFX_PlayerBump, 0.24375f, kRP_One);
    }
  }
  if (CVector3f::Dot(GetTransform().GetRight(), velocity) > 0.f) {
    mBodyController->CommandMgr().DeliverCmd(CPBCDashCmd(1));
  } else {
    mBodyController->CommandMgr().DeliverCmd(CPBCDashCmd(0));
  }
}

void CPlayer::ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr) {
  const float strafeInput = StrafeInput(input);
  const float forwardInput = ForwardInput(input, TurnInput(input));
  CVector3f orbitPoint = mOrbitPoint;
  orbitPoint.SetZ(GetTranslation().GetZ());
  const CVector3f orbitToPlayer = GetTranslation() - orbitPoint;
  if (!orbitToPlayer.CanBeNormalized()) {
    return;
  }
  CVector3f useOrbitToPlayer = orbitToPlayer;
  float strafeVelocity = dt * skStrafeDistances[GetSurfaceRestraint()];
  if (JumpHeld(input)) {
    mDashButtonHoldTime += dt;
  }
  if (!mSidewaysDashing) {
    if (SidewaysDashAllowed(strafeInput, forwardInput, input)) {
      BeginSidewaysDash(strafeInput, mgr);
    }
    strafeVelocity *= strafeInput;
  } else {
    mDashTimer += dt;
    if (mMovementState == NPlayer::kMS_OnGround || mDashTimer >= mDashDuration || mSlidingOnWall ||
        mHitWallDuringMove || mOrbitState != kOS_OrbitObject) {
      FinishSidewaysDash();
      strafeVelocity *= strafeInput;
      CSfxManager::SfxStop(mDashSfx);
    } else {
      const ESurfaceRestraints restraint = GetSurfaceRestraint();
      if (mNoStrafeDashBlend) {
        strafeVelocity = dt * (mDashSpeedMultiplier * skDashStrafeDistances[restraint]);
      } else {
        float blend = CMath::Limit(mDashTimer / mStrafeDashBlendDuration, 1.f);
        blend = 1.f - blend;
        const float dashDifference =
            skDashStrafeDistances[GetSurfaceRestraint()] - skStrafeDistances[GetSurfaceRestraint()];
        strafeVelocity = dt * (mDashSpeedMultiplier *
                               (dashDifference * blend + skStrafeDistances[GetSurfaceRestraint()]));
      }
      if (mStrafeInputAtDash < 0.f) {
        strafeVelocity = -strafeVelocity;
      }
    }
  }

  const float angle = strafeVelocity / orbitToPlayer.Magnitude();
  float maxAngle = M_PIF * 2.f / 3.f;
  if (mSidewaysDashing) {
    maxAngle = M_PIF;
  }
  const float limitedAngle = CMath::Limit(angle, maxAngle * dt);
  const CQuaternion rotation = CQuaternion::AxisAngle(
      CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), CRelAngle::FromRadians(limitedAngle));
  useOrbitToPlayer = rotation.Transform(orbitToPlayer);
  orbitPoint += useOrbitToPlayer;
  if (!JumpHeld(input)) {
    mDashButtonHoldTime = 0.f;
  }

  strafeVelocity = dt * (forwardInput * skOrbitForwardDistances[GetSurfaceRestraint()]);
  orbitPoint += strafeVelocity * -useOrbitToPlayer.AsNormalized();
  const CVector2f flatVelocity(GetVelocityWR().GetX(), GetVelocityWR().GetY());
  const float flatVelocityY = flatVelocity.GetY();
  CVector3f newVelocity = (1.f / dt) * (orbitPoint - GetTranslation());
  newVelocity.SetZ(GetVelocityWR().GetZ());
  CVector3f velocityDelta = newVelocity - CVector3f(flatVelocity.GetX(), flatVelocityY, 0.f);
  velocityDelta.SetZ(0.f);
  const float deltaMagnitude = velocityDelta.Magnitude();
  if (deltaMagnitude > FLT_EPSILON) {
    const float acceleration = (270.f * GetAcceleration()) * dt;
    const float accelerationBlend = CMath::Limit(deltaMagnitude / acceleration, 1.f);
    newVelocity = GetVelocityWR() +
                  accelerationBlend * (acceleration * ((1.f / deltaMagnitude) * velocityDelta));
    if (!mSlidingOnWall) {
      SetVelocityWR(newVelocity);
    }
  }
}

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  const float jumpInput = JumpInput(input, mgr);
  float turnInput = TurnInput(input);
  const float forwardInput = ForwardInput(input, turnInput);
  const float strafeInput = StrafeInput(input);
  SetVelocityWR(GetDampedClampedVelocityWR());
  float turnSpeedMultiplier = GetTweakPlayer()->GetTurnSpeedMultiplier();
  if (GetTweakPlayerControls()->GetFreeLookTurnsPlayer()) {
    if (!GetTweakPlayerControls()->GetHoldButtonsForFreeLook() ||
        (GetTweakPlayerControls()->GetHoldButtonsForFreeLook() && mLookButtonHeld)) {
      turnSpeedMultiplier = GetTweakPlayer()->GetFreeLookTurnSpeedMultiplier();
    }
  }
  if (mOrbitState == kOS_NoOrbit ||
      (mLookButtonHeld && mOrbitState != kOS_OrbitObject && mOrbitState != kOS_Grapple)) {
    if (close_enough(turnInput, 0.f)) {
      const float friction = GetTweakPlayer()->GetPlayerRotationFriction(GetSurfaceRestraint());
      SetAngularVelocityOR(
          CAxisAngle(CVector3f(0.f, 0.f, friction * GetAngularVelocityOR().GetVector().GetZ())));
    }
    if (GetAngularVelocityOR().GetVector().GetZ() >
        turnSpeedMultiplier * GetTweakPlayer()->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())) {
      SetAngularVelocityOR(CAxisAngle(
          CVector3f(0.f, 0.f,
                    turnSpeedMultiplier *
                        GetTweakPlayer()->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()))));
    } else if (-GetAngularVelocityOR().GetVector().GetZ() >
               turnSpeedMultiplier *
                   GetTweakPlayer()->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())) {
      SetAngularVelocityOR(CAxisAngle(
          CVector3f(0.f, 0.f,
                    turnSpeedMultiplier *
                        -GetTweakPlayer()->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()))));
    }
  }
  float angularVelocityDelta =
      turnSpeedMultiplier *
      (turnInput * GetTweakPlayer()->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()));
  angularVelocityDelta -= GetAngularVelocityOR().GetVector().GetZ();
  const float turnFraction =
      CMath::Clamp(0.f,
                   CMath::AbsF(angularVelocityDelta) /
                       (turnSpeedMultiplier *
                        GetTweakPlayer()->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())),
                   1.f);
  if (angularVelocityDelta < 0.f) {
    turnInput =
        turnFraction * -GetTweakPlayer()->GetMaxRotationalAcceleration(GetSurfaceRestraint());
  } else {
    turnInput =
        turnFraction * GetTweakPlayer()->GetMaxRotationalAcceleration(GetSurfaceRestraint());
  }
  float forwardForce;
  if (!close_enough(0.f, forwardInput)) {
    const float maxSpeed = GetTweakPlayer()->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
    const float friction = GetTweakPlayer()->GetPlayerTranslationFriction(GetSurfaceRestraint());
    const float mass = GetMass();
    const float acceleration =
        GetTweakPlayer()->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
    float frictionSpeed = friction * mass / (dt * acceleration);
    frictionSpeed *= maxSpeed;
    float desiredSpeed = forwardInput * (maxSpeed - frictionSpeed);
    desiredSpeed += frictionSpeed * (forwardInput > 0.f ? 1.f : -1.f);
    const float forwardFraction = CMath::Clamp(
        -1.f, (desiredSpeed - GetTransform().TransposeRotate(GetVelocityWR()).GetY()) / maxSpeed,
        1.f);
    forwardForce =
        forwardFraction * GetTweakPlayer()->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  } else {
    forwardForce = 0.f;
  }
  forwardForce *= GetAcceleration();
  float strafeForce;
  if (!close_enough(0.f, strafeInput)) {
    const float maxSpeed = GetTweakPlayer()->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
    const float friction = GetTweakPlayer()->GetPlayerTranslationFriction(GetSurfaceRestraint());
    const float mass = GetMass();
    const float acceleration =
        GetTweakPlayer()->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
    float frictionSpeed = friction * mass / (dt * acceleration);
    frictionSpeed *= maxSpeed;
    float desiredSpeed = strafeInput * (maxSpeed - frictionSpeed);
    desiredSpeed += frictionSpeed * (strafeInput > 0.f ? 1.f : -1.f);
    const float strafeFraction = CMath::Clamp(
        -1.f, (desiredSpeed - GetTransform().TransposeRotate(GetVelocityWR()).GetX()) / maxSpeed,
        1.f);
    strafeForce =
        strafeFraction * GetTweakPlayer()->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  } else {
    strafeForce = 0.f;
  }
  if (mOrbitState != kOS_NoOrbit && gkFreeLookPreventsOrbitMovement && mLookButtonHeld) {
    forwardForce = 0.f;
    strafeForce = 0.f;
  }
  if (mOrbitState == kOS_NoOrbit || mLookButtonHeld) {
    const CVector3f force = CVector3f(0.f, forwardForce, 0.f) + CVector3f(0.f, 0.f, jumpInput) +
                            CVector3f(strafeForce, 0.f, 0.f);
    ApplyForceOR(force, CAxisAngle::Identity());
    if (turnInput != 0.f) {
      ApplyForceOR(CVector3f::Zero(),
                   CAxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), turnInput));
    }
    FinishSidewaysDash();
  } else {
    switch (mOrbitState) {
    case kOS_OrbitObject:
    case kOS_OrbitPoint:
    case kOS_OrbitCarcass:
    case kOS_ForcedOrbitObject: {
      bool canDash = true;
      if (InGrappleJumpCooldown()) {
        canDash = false;
      }
      if (canDash) {
        ComputeDash(input, dt, mgr);
      }
    } break;
    case kOS_Grapple:
      break;
    default:
      break;
    }
    const CVector3f force(0.f, 0.f, jumpInput);
    ApplyForceOR(force, CAxisAngle::Identity());
  }
  if (!GetTweakPlayerControls()->GetMoveDuringFreeLook() && (mInFreeLook || mLookButtonHeld)) {
    if (!mSlidingOnWall && mMovementState == NPlayer::kMS_OnGround) {
      const CVector3f reverseVelocity =
          CVector3f::Zero() - CVector3f(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f);
      const float magnitude = reverseVelocity.Magnitude();
      if (magnitude > FLT_EPSILON) {
        const float acceleration = dt * (54.f * GetAcceleration());
        const float damping = CMath::Limit(magnitude / acceleration, 1.f);
        const CVector3f newVelocity =
            GetVelocityWR() + damping * (acceleration * ((1.f / magnitude) * reverseVelocity));
        SetVelocityWR(newVelocity);
      }
    }
  }
  mHitWallDuringMove = false;
  if (mAccelerationChangeTimer > 0.f) {
    mCurAcceleration = 0;
  } else {
    ++mCurAcceleration;
  }
  mAccelerationChangeTimer -= dt;
  mAccelerationChangeTimer = rstl::max_val(0.f, mAccelerationChangeTimer);
}

float CPlayer::ForwardInput(const CFinalInput& input, float turnInput) const {
  float forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input);
  float backward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input);
  if (mMorphBallState != kMS_Unmorphed || InGrappleJumpCooldown()) {
    backward = 0.f;
  }
  if (!(forward < 0.001f)) {
    forward = CMath::Limit(forward / 0.8f, 1.f);
    if (CMath::AbsF(atan2f(CMath::AbsF(turnInput), forward)) <
        CRelAngle::FromDegrees(50.f).AsRadians()) {
      const CVector3f stick(CMath::AbsF(turnInput), forward, 0.f);
      if (stick.CanBeNormalized()) {
        forward = stick.Magnitude();
      }
    }
  }
  if (!(backward < 0.001f)) {
    backward = CMath::Limit(backward / 0.8f, 1.f);
    if (CMath::AbsF(atan2f(CMath::AbsF(turnInput), backward)) <
        CRelAngle::FromDegrees(50.f).AsRadians()) {
      const CVector3f stick(CMath::AbsF(turnInput), backward, 0.f);
      if (stick.CanBeNormalized()) {
        backward = stick.Magnitude();
      }
    }
  }
  if (!GetTweakPlayerControls()->GetMoveDuringFreeLook()) {
    CVector3f flatVelocity = GetVelocityWR();
    flatVelocity.SetZ(0.f);
    if (mInFreeLook || mLookButtonHeld) {
      if (mMovementState == NPlayer::kMS_OnGround || close_enough(flatVelocity.Magnitude(), 0.f)) {
        return 0.f;
      }
    }
  }
  return CMath::Limit(forward - backward * GetTweakPlayer()->GetBackwardsForceMultiplier(), 1.f);
}

float CPlayer::StrafeInput(const CFinalInput& input) const {
  if (IsMorphBallTransitioning() || mOrbitState == kOS_NoOrbit) {
    return 0.f;
  }
  const float left = mControlMapper.GetAnalogInput(CControlMapper::kC_StrafeLeft, input);
  const float right = mControlMapper.GetAnalogInput(CControlMapper::kC_StrafeRight, input);
  return right - left;
}

float CPlayer::TurnInput(const CFinalInput& input) const {
  float left = mControlMapper.GetAnalogInput(CControlMapper::kC_TurnLeft, input);
  float right = mControlMapper.GetAnalogInput(CControlMapper::kC_TurnRight, input);
  if (GetTweakPlayerControls()->GetFreeLookTurnsPlayer()) {
    if (!GetTweakPlayerControls()->GetHoldButtonsForFreeLook() ||
        (GetTweakPlayerControls()->GetHoldButtonsForFreeLook() && mLookButtonHeld)) {
      if (left < 0.01f && right < 0.01f) {
        left = mControlMapper.GetAnalogInput(CControlMapper::kC_LookLeft, input);
        right = mControlMapper.GetAnalogInput(CControlMapper::kC_LookRight, input);
      }
    }
  } else if (!GetTweakPlayerControls()->GetHoldButtonsForFreeLook() ||
             (GetTweakPlayerControls()->GetHoldButtonsForFreeLook() && mLookButtonHeld)) {
    const float lookLeft = mControlMapper.GetAnalogInput(CControlMapper::kC_LookLeft, input);
    const float lookRight = mControlMapper.GetAnalogInput(CControlMapper::kC_LookRight, input);
    if (lookLeft > 0.01f || lookRight > 0.01f) {
      return 0.f;
    }
  }
  if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_Grapple) {
    return 0.f;
  }
  if (IsMorphBallTransitioning()) {
    return 0.f;
  }
  float turn = left - right;
  if (mOrbitModeTimer > 0.f) {
    turn *= 1.f -
            0.5f * CMath::Clamp(0.f, mOrbitModeTimer / GetTweakPlayer()->GetOrbitModeTimer(), 1.f);
  }
  return CMath::Limit(turn, 1.f);
}

float CPlayer::JumpInput(const CFinalInput& input, CStateManager& mgr) {
  if (!ShouldSampleFailsafe(mgr)) {
    return 0.f;
  }
  if (IsMorphBallTransitioning()) {
    if (JumpPressed(input) && mMorphBallState == kMS_Morphing &&
        mSpawnedMorphBallState == kMS_Morphed && mJumpPresses == 3) {
      mMorphTime = mMorphDuration;
      mMorphBall->SetScrewAttackActive(true);
    }
    return GetGravity() * GetMass();
  }
  if (IsGravityBoostActive() && !JumpHeld(input)) {
    EndGravityBoost(mgr);
  }
  float waterScale = 1.f;
  if (GetFluidCount() != 0 && !mPlayerState->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
    switch (GetSurfaceRestraint()) {
    case kSR_Water:
      waterScale = GetTweakPlayer()->GetWaterJumpFactor();
      break;
    case kSR_Phazon:
      waterScale = GetTweakPlayer()->GetPhazonJumpFactor();
      break;
    case kSR_Lava:
    case kSR_Shrubbery:
      waterScale = GetTweakPlayer()->GetLavaJumpFactor();
      break;
    default:
      break;
    }
  }
  const float verticalJumpAccel = GetTweakPlayer()->GetVerticalJumpAccel();
  const float horizontalJumpAccel = GetTweakPlayer()->GetHorizontalJumpAccel();
  float doubleJumpImpulse = GetTweakPlayer()->GetDoubleJumpImpulse();
  float verticalDoubleJumpAccel = GetTweakPlayer()->GetVerticalDoubleJumpAccel();
  float horizontalDoubleJumpAccel = GetTweakPlayer()->GetHorizontalDoubleJumpAccel();
  if (mSidewaysDashing) {
    doubleJumpImpulse = GetTweakPlayer()->GetSidewaysDoubleJumpImpulse();
    verticalDoubleJumpAccel = GetTweakPlayer()->GetSidewaysVerticalDoubleJumpAccel();
    horizontalDoubleJumpAccel = GetTweakPlayer()->GetSidewaysHorizontalDoubleJumpAccel();
  }
  if (mDistanceUnderWater >= .8f * GetEyeHeight()) {
    doubleJumpImpulse *= waterScale;
  }
  if (mMovementState == NPlayer::kMS_ApplyJump) {
    const float threshold =
        GetTweakPlayer()->GetMaxDoubleJumpWindow() - GetTweakPlayer()->GetMinDoubleJumpWindow();
    if (JumpPressed(input)) {
      if (mSjTimer <= threshold && mSjTimer > 0.f) {
        mLastSpaceJumpPosition = GetTranslation();
        SetMoveState(NPlayer::kMS_Jump, mgr);
        mBodyController->CommandMgr().DeliverCmd(CPlayerBodyStateCmd(kPBSC_DoubleJump));
        if (kDashDoubleJumpBreaksOrbit && mSidewaysDashing) {
          SetOrbitRequestForOtherPlayers(kOR_BoostBall, mgr);
        }
        mDashTimer = 0.f;
        mStrafeInputAtDash = StrafeInput(input);
        if (GetTweakPlayerControls()->GetImpulseDoubleJump()) {
          ApplyImpulseWR(
              CVector3f(0.f, 0.f, (doubleJumpImpulse - GetVelocityWR().GetZ()) * GetMass()),
              CAxisAngle::Identity());
        }
        float inputMagnitude = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input);
        if (inputMagnitude < mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input)) {
          inputMagnitude = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input);
        }
        return waterScale *
               ((verticalDoubleJumpAccel -
                 inputMagnitude * (verticalDoubleJumpAccel - horizontalDoubleJumpAccel)) *
                GetMass());
      }
      const CFirstPersonCamera* camera =
          mgr.GetCameraManager(GetPlayerIndex())->GetFirstPersonCamera();
      if (camera->GetFluidCount() == 0) {
        CVector3f flatVelocity(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f);
        if (flatVelocity.CanBeNormalized()) {
          flatVelocity.Normalize();
        }
        const float forwardDot = CVector3f::Dot(flatVelocity, camera->GetTransform().GetForward());
        if (mPlayerState->HasPowerUp(CPlayerState::kIT_ScrewAttack) && forwardDot > .95f &&
            mJumpPresses == 2 && GetRezbitState() == kRS_None) {
          mTimeSinceScrewAttackRequest = 0.f;
          if (GetTweakPlayerControls()->GetImpulseDoubleJump()) {
            ApplyImpulseWR(
                CVector3f(0.f, 0.f, (doubleJumpImpulse - GetVelocityWR().GetZ()) * GetMass()),
                CAxisAngle::Identity());
          }
          RequestScrewAttackTransition(kMS_Morphed);
        }
      } else if (!IsGravityBoostActive() && GetFluidCount() != 0 &&
                 mPlayerState->HasPowerUp(CPlayerState::kIT_GravityBoost) && JumpHeld(input)) {
        StartGravityBoost(mgr);
      }
    }
    return GetGravity() * GetMass();
  }
  if (JumpHeld(input) ||
      (mMovementState == NPlayer::kMS_Jump && mStartingJumpTimeout >= mMinJumpTimeout)) {
    if (mMovementState == NPlayer::kMS_Jump) {
      float inputMagnitude = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input);
      if (inputMagnitude < mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input)) {
        inputMagnitude = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input);
      }
      return waterScale *
             ((verticalJumpAccel - inputMagnitude * (verticalJumpAccel - horizontalJumpAccel)) *
              GetMass());
    }
    if (JumpPressed(input)) {
      mLastJumpPosition = GetTranslation();
      SetMoveState(NPlayer::kMS_Jump, mgr);
      return waterScale * (verticalJumpAccel * GetMass());
    }
    return 0.f;
  }
  if (mMovementState == NPlayer::kMS_Jump) {
    SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
  return 0.f;
}

void CPlayer::SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr) {
  switch (state) {
  case NPlayer::kMS_Jump:
    if (mMovementState == NPlayer::kMS_ApplyJump) {
      const CSfxHandle sound =
          CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x89, 0x2824), 127,
                                GetSoundPan(kMSP_4), GetCurrentAreaId().Value(), true, false);
      CSfxManager::SetIgnoreAreaLowPass(sound, true);
      ApplySubmergedPitchBend(sound);
      mgr.RumbleManager(GetPlayerIndex())->Rumble(mgr, kRFX_PlayerBump, 0.2015f, kRP_One);
      mStartingJumpTimeout = GetTweakPlayer()->GetAllowedDoubleJumpTime();
      mMinJumpTimeout =
          GetTweakPlayer()->GetAllowedDoubleJumpTime() - GetTweakPlayer()->GetMinDoubleJumpTime();
      mSjTimer = 0.f;
      mTimeSinceDoubleJump = 0.f;
      if (kDoubleJumpBreaksOrbit) {
        SetOrbitRequestForOtherPlayers(kOR_BoostBall, mgr);
      }
    } else if (mMovementState != NPlayer::kMS_Jump) {
      const CSfxHandle sound =
          CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x8a, 0x2825), 127,
                                GetSoundPan(kMSP_4), GetCurrentAreaId().Value(), true, false);
      CSfxManager::SetIgnoreAreaLowPass(sound, true);
      ApplySubmergedPitchBend(sound);
      mAirborneTimer = 0.01f;
      mStartingJumpTimeout = GetTweakPlayer()->GetAllowedJumpTime();
      mMinJumpTimeout = GetTweakPlayer()->GetAllowedJumpTime() - GetTweakPlayer()->GetMinJumpTime();
      if (mPlayerState->GetItemAmount(CPlayerState::kIT_SpaceJumpBoots) != 0) {
        mSjTimer = GetTweakPlayer()->GetMaxDoubleJumpWindow();
      } else {
        mSjTimer = 0.f;
      }
      if (mJumpCameraTimer <= 0.f && mFallCameraTimer <= 0.f && !mInFreeLook && !mLookButtonHeld) {
        mJumpCameraTimer = 0.01f;
        mCancelCameraPitch = false;
      }
    }
    mMovementState = NPlayer::kMS_Jump;
    SetSurfaceRestraint(kSR_Air);
    mTimeSinceJump = 0.f;
    break;
  case NPlayer::kMS_Falling:
    if (mMovementState == NPlayer::kMS_OnGround) {
      mStartingJumpTimeout = GetTweakPlayer()->GetAllowedLedgeTime();
      mMovementState = NPlayer::kMS_Falling;
      mAirborneTimer = 0.01f;
      if (GetTweakPlayerControls()->GetFallingDoubleJump()) {
        mSjTimer = GetTweakPlayer()->GetMaxDoubleJumpWindow();
      } else {
        mSjTimer = 0.f;
      }
    }
    break;
  case NPlayer::kMS_FallingMorphed:
    mMovementState = NPlayer::kMS_FallingMorphed;
    SetSurfaceRestraint(kSR_Normal);
    break;
  case NPlayer::kMS_OnGround:
    mFallingTime = 0.f;
    mMovementState = NPlayer::kMS_OnGround;
    mStartingJumpTimeout = 0.f;
    mSjTimer = 0.f;
    SetSurfaceRestraint(kSR_Normal);
    if (mMorphBallState != kMS_Morphed) {
      AddMaterial(kMT_GroundCollider, mgr);
    }
    mJumpCameraTimer = 0.f;
    mFallCameraTimer = 0.f;
    mCancelCameraPitch = false;
    mJumpPresses = 0;
    if (mGravityBoostEndSfx) {
      CSfxManager::SfxStop(mGravityBoostEndSfx);
    }
    break;
  case NPlayer::kMS_ApplyJump:
    mStartingJumpTimeout = 0.f;
    if (mMovementState != NPlayer::kMS_ApplyJump) {
      mMovementState = NPlayer::kMS_ApplyJump;
      if (mJumpCameraTimer <= 0.f && mFallCameraTimer <= 0.f && !mInFreeLook && !mLookButtonHeld) {
        mFallCameraTimer = 0.01f;
        mCancelCameraPitch = false;
      }
    }
    SetSurfaceRestraint(kSR_Air);
    break;
  }
}

void CPlayer::CalculatePlayerMovementDirection(float dt, const CVector3f& displacement) {
  if (displacement.CanBeNormalized() && displacement.Magnitude() > 0.02f) {
    mTimeMoving += dt;
    mMoveSpeed = CMath::AbsF(displacement.Magnitude() / dt);
    mLookDir = displacement.AsNormalized();
    CVector3f flatDelta = displacement;
    flatDelta.SetZ(0.f);
    if (flatDelta.CanBeNormalized()) {
      mFlatMoveSpeed = CMath::AbsF(flatDelta.Magnitude() / dt);
      flatDelta.Normalize();
      switch (mMorphBallState) {
      case kMS_Morphed:
        if (mFlatMoveSpeed > 0.25f) {
          mMoveDir = flatDelta;
        }
        mGunDir = mMoveDir;
        mLastPosForDirCalc = GetTranslation();
        break;
      case kMS_Unmorphed:
      case kMS_Morphing:
      case kMS_Unmorphing:
        mLookDir = GetTransform().GetForward();
        mMoveDir = mLookDir;
        mMoveDir.SetZ(0.f);
        if (mMoveDir.CanBeNormalized()) {
          mMoveDir.Normalize();
        }
        mGunDir = mMoveDir;
        mLastPosForDirCalc = GetTranslation();
        break;
      }
    } else {
      if (mMorphBallState != kMS_Morphed) {
        mLookDir = GetTransform().GetForward();
        mMoveDir = mLookDir;
        mMoveDir.SetZ(0.f);
        if (mMoveDir.CanBeNormalized()) {
          mMoveDir.Normalize();
        }
        mGunDir = mMoveDir;
        mLastPosForDirCalc = GetTranslation();
      }
      mFlatMoveSpeed = 0.f;
    }
  } else {
    mTimeMoving = 0.f;
    switch (mMorphBallState) {
    case kMS_Morphed:
    case kMS_Morphing:
    case kMS_Unmorphing:
      mLookDir = mMoveDir;
      break;
    default:
      mLookDir = GetTransform().GetForward();
      mMoveDir = mLookDir;
      mMoveDir.SetZ(0.f);
      if (mMoveDir.CanBeNormalized()) {
        mMoveDir.Normalize();
      }
      mGunDir = mMoveDir;
      mLastPosForDirCalc = GetTranslation();
      break;
    }
    mMoveSpeed = 0.f;
    mFlatMoveSpeed = 0.f;
  }
  mMoveDir.SetZ(0.f);
  if (mMoveDir.CanBeNormalized()) {
    mLookDir.Normalize();
  }
}

void CPlayer::CalculateLeaveMorphBallDirection(const CFinalInput& input) {
  if (mMorphBallState != kMS_Morphed || mMorphBall->InScrewAttackMode()) {
    mLeaveMorphDir = mMoveDir;
  } else {
    const float forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input);
    const float backward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input);
    const float left = mControlMapper.GetAnalogInput(CControlMapper::kC_TurnLeft, input);
    const float right = mControlMapper.GetAnalogInput(CControlMapper::kC_TurnRight, input);
    if (forward > 0.3f || backward > 0.3f || left > 0.3f || right > 0.3f) {
      if (GetVelocityWR().Magnitude() > 0.5f) {
        mLeaveMorphDir = mMoveDir;
      }
    }
  }
}

float CPlayer::GetBallMaxVelocity() const {
  return gpTweakBall->GetBallTranslationMaxSpeed(GetSurfaceRestraint());
}

float CPlayer::GetActualFirstPersonMaxVelocity(float dt) const {
  const float friction = GetTweakPlayer()->GetPlayerTranslationFriction(GetSurfaceRestraint());
  const float frictionForce = friction * GetMass();
  const float maxSpeed = GetTweakPlayer()->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  const float acceleration =
      GetTweakPlayer()->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  return -(frictionForce * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

float CPlayer::GetActualBallMaxVelocity(float dt) const {
  const float friction = gpTweakBall->GetBallTranslationFriction(GetSurfaceRestraint());
  const float frictionForce = friction * GetMass();
  const float maxSpeed = gpTweakBall->GetBallTranslationMaxSpeed(GetSurfaceRestraint());
  const float acceleration = gpTweakBall->GetMaxBallTranslationAcceleration(GetSurfaceRestraint());
  return -(frictionForce * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

void CPlayer::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                           CStateManager& mgr) {
  if (mMorphBallState != kMS_Unmorphed) {
    mMorphBall->CollidedWith(id, list, mgr);
  }
}

CTransform4f CPlayer::GetPrimitiveTransform() const {
  return CPhysicsActor::GetPrimitiveTransform();
}

const CCollidableSphere* CPlayer::GetCollidableSphere() const {
  return &mMorphBall->GetCollidableSphere();
}

const CCollisionPrimitive* CPlayer::GetCollisionPrimitive() const {
  switch (mMorphBallState) {
  case kMS_Morphed:
    return GetCollidableSphere();
  case kMS_Unmorphed:
    return CPhysicsActor::GetCollisionPrimitive();
  case kMS_Morphing:
  case kMS_Unmorphing:
    return CPhysicsActor::GetCollisionPrimitive();
  default:
    return CPhysicsActor::GetCollisionPrimitive();
  }
}

CTransform4f CPlayer::CreateTransformFromMovementDirection() const {
  CVector3f direction = mMoveDir;
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  } else {
    direction = CVector3f(0.f, 1.f, 0.f);
  }
  const CVector3f right(direction.GetY(), -direction.GetX(), 0.f);
  return CTransform4f::FromColumns(right, direction, CVector3f::Up(), GetTranslation());
}

void CPlayer::BombJump(const CVector3f& position, CStateManager& mgr) {
  if (mMorphBallState == kMS_Morphed &&
      mMorphBall->GetBombJumpState() != CMorphBall::kBJS_BombJumpDisabled) {
    const float extent = GetTweakPlayer()->GetBallRadius();
    const CVector3f toBall = GetTranslation() + CVector3f(0.f, 0.f, extent) - position;
    const float maxDistance = GetTweakPlayer()->GetBombJumpRadius();
    if (toBall.MagSquared() < maxDistance * maxDistance &&
        CVector3f::Dot(CVector3f(0.f, 0.f, 1.f), toBall) >= -extent) {
      float velocity = sqrt(2.f * fabsf(GetTweakPlayer()->GetNormalGravAccel()) *
                            GetTweakPlayer()->GetBombJumpHeight());
      if (CheckSubmerged() && !mPlayerState->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
        velocity = sqrt(2.f * fabsf(GetTweakPlayer()->GetFluidGravAccel()) *
                        GetTweakPlayer()->GetBombJumpHeight());
      }
      mgr.RumbleManager(GetPlayerIndex())->Rumble(mgr, kRFX_PlayerBump, 0.3f, kRP_One);
      mAirborneTimer = 0.01f;
      const CVector3f newVelocity(0.f, 0.f, velocity);
      SetVelocityWR(newVelocity);
      mMorphBall->SetDamageTimer(0.1f);
      mMorphBall->CancelBoosting();
      mMorphBall->SetDisableSpiderBallTime(.4f);
      if (mMorphBall->GetCloseToCollisionTime() > .1f) {
        mGun->AddBombReloadTime(.4f);
      }
      if (mBombJumpCount > 0) {
        if (mBombJumpCount > 2) {
          mBombJumpCount = 0;
          mBombJumpCheckDelayFrames = 0;
        } else {
          ++mBombJumpCount;
        }
      } else {
        const CBallCamera* camera = mCameraManager->GetBallCamera();
        if (camera->GetTooCloseActorId() != kInvalidUniqueId &&
            camera->GetTooCloseActorDistance() < 5.f) {
          mBombJumpCount = 1;
          mBombJumpCheckDelayFrames = 2;
        }
      }
      ApplySubmergedPitchBend(CSfxManager::AddEmitter(0x87, GetTranslation(),
                                                      GetCurrentAreaId().Value(), false, false));
    }
  }
}

void CPlayer::Teleport(const CTransform4f& transform, CStateManager& mgr,
                       const bool resetBallCamera) {
  CVector3f direction = transform.GetForward();
  direction.SetZ(0.f);
  CPhysicsActor::Stop();
  if (CCollisionCache* cache = GetCollisionCache()) {
    cache->SetBounds(CAABox::MakeNullBox());
    cache->Reset();
  }
  if (direction.CanBeNormalized()) {
    direction.Normalize();
    SetTransform(CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up()));
    SetTranslation(transform.GetTranslation());
    mLookDir = direction;
    mMoveDir = direction;
    mGunDir = direction;
    mLastPosForDirCalc = transform.GetTranslation();
    mMoveSpeed = 0.f;
    mFlatMoveSpeed = 0.f;
    mTimeMoving = 0.f;
    mMoveSpeedAvg.clear();
    mControlDir = direction;
    mControlDirFlat = direction;
    mGravityBoostUsed = false;
    mGravityBoostDuration = 0.f;
    mGravityBoostSfx = CSfxHandle();
  } else {
    SetTranslation(transform.GetTranslation());
  }
  mStepCameraZBiasDirty = true;
  SetEyeZBias(0.f);
  SetLastNonCollidingState(GetMotionState());
  SetMoveState(NPlayer::kMS_OnGround, mgr);
  CTransform4f eyeTransform = GetTransform();
  eyeTransform.SetTranslation(GetEyePosition());
  mCameraManager->FirstPersonCamera()->Reset(eyeTransform, mgr);
  if (resetBallCamera) {
    mCameraManager->BallCamera()->Reset(eyeTransform, mgr);
  }
  ForceGunOrientation(GetTransform(), mgr);
  SetOrbitRequest(kOR_Respawn, mgr);
  SetOrbitRequestForOtherPlayers(kOR_Respawn, mgr);
}

bool CPlayer::CheckSubmerged() const {
  if (GetFluidCount() == 0) {
    return false;
  }
  const float ballHeight = 2.f * GetTweakPlayer()->GetBallRadius();
  const float eyeHeight = 0.5f * GetEyeHeight();
  float height = eyeHeight;
  if (mMorphBallState == kMS_Morphed) {
    height = ballHeight;
  }
  return mDistanceUnderWater >= height;
}

void CPlayer::UpdateSubmerged(const CStateManager& mgr) {
  x126b_26_ = false;
  mDistanceUnderWater = 0.f;
  if (GetFluidCount() != 0) {
    mKnockBackManager.DouseFlames();
    if (const CScriptWater* water =
            TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
      mDistanceUnderWater = -water->GetWRSurfacePlane().GetHeight(GetTranslation());
      x126b_26_ = water->GetFluidPlane().GetFluidType() == 2;
    }
  }
}

float CPlayer::GetStepDownHeight() const {
  if (mMovementState == NPlayer::kMS_Jump) {
    return -1.f;
  }
  if (mMovementState == NPlayer::kMS_ApplyJump) {
    return 0.1f;
  }
  return CPhysicsActor::GetStepDownHeight();
}

float CPlayer::GetStepUpHeight() const {
  if (mMovementState == NPlayer::kMS_Jump || mMovementState == NPlayer::kMS_ApplyJump) {
    return 0.3f;
  }
  return CPhysicsActor::GetStepUpHeight();
}

float CPlayer::GetUnbiasedEyeHeight() const {
  return mFpBounds.GetMaxPoint().GetZ() - GetTweakPlayer()->GetEyeOffset();
}

float CPlayer::GetEyeHeight() const { return mEyeZBias + GetUnbiasedEyeHeight(); }

CVector3f CPlayer::GetEyePosition() const {
  return GetTranslation() + CVector3f(0.f, 0.f, GetEyeHeight());
}

CVector3f CPlayer::GetBallPosition() const {
  return GetTranslation() + CVector3f(0.f, 0.f, GetTweakPlayer()->GetBallRadius());
}

void CPlayer::SetEyeZBias(float bias) { mEyeZBias = bias; }

float CPlayer::UpdateCameraBob(float dt, CStateManager& mgr) {
  float magnitude = 0.f;
  CPlayerCameraBob::ECameraBobState state;
  const CVector3f velocity = GetVelocityWR();
  if (mOrbitState == kOS_NoOrbit) {
    const float forwardSpeed = CVector3f::Dot(velocity, GetTransform().GetForward());
    state = CPlayerCameraBob::kCBS_Walk;
    magnitude = CMath::AbsF(forwardSpeed / GetActualFirstPersonMaxVelocity(dt));
    if (magnitude < 0.01f) {
      state = CPlayerCameraBob::kCBS_WalkNoBob;
      magnitude = 0.f;
    }
  } else {
    state = CPlayerCameraBob::kCBS_Orbit;
    const float rightSpeed = CVector3f::Dot(velocity, GetTransform().GetRight());
    const float forwardSpeed = CVector3f::Dot(velocity, GetTransform().GetForward());
    const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
    const float strafeSpeed = skStrafeDistances[GetSurfaceRestraint()];
    const float maxMagnitude = CMath::SqrtF(strafeSpeed * strafeSpeed + maxSpeed * maxSpeed);
    magnitude = CMath::SqrtF(rightSpeed * rightSpeed + forwardSpeed * forwardSpeed) / maxMagnitude;
    magnitude *= CPlayerCameraBob::GetOrbitBobScale();
    magnitude = rstl::min_val(CPlayerCameraBob::GetMaxOrbitBobScale(), magnitude);
    if (magnitude < 0.01f) {
      magnitude = 0.f;
    }
  }
  if (mMovementState != NPlayer::kMS_OnGround) {
    state = CPlayerCameraBob::kCBS_InAir;
    magnitude = 0.f;
  } else if (magnitude < 0.01f) {
    if (mGun->GetFiring() != 0) {
      state = CPlayerCameraBob::kCBS_GunFireNoBob;
      magnitude = 0.f;
    } else if (CMath::AbsF(GetAngularVelocityOR().GetAngle()) > 0.1f) {
      state = CPlayerCameraBob::kCBS_TurningNoBob;
      magnitude = 0.f;
    }
  }
  if (mInFreeLook || mLookButtonHeld) {
    state = CPlayerCameraBob::kCBS_FreeLookNoBob;
    magnitude = 0.f;
  }
  if (mOrbitState == kOS_Grapple) {
    state = CPlayerCameraBob::kCBS_GrappleNoBob;
    magnitude = 0.f;
  }
  if (mScanState == kSS_ScanComplete) {
    magnitude = 0.f;
  }
  if (mDoneSidewaysDashing) {
    state = CPlayerCameraBob::kCBS_FreeLookNoBob;
    magnitude *= 0.1f;
    if (mMovementState == NPlayer::kMS_OnGround) {
      mDoneSidewaysDashing = false;
    }
  }
  if (mCameraManager->IsInCinematicCamera()) {
    magnitude = 0.f;
  }
  magnitude *= mCameraManager->GetCameraBobMagnitude();
  mCameraBob->SetPlayerVelocity(velocity);
  mCameraBob->SetState(state, mgr);
  mCameraBob->SetBobMagnitude(magnitude);
  const float timeScaleRange = 1.f - CPlayerCameraBob::GetSlowSpeedPeriodScale();
  mCameraBob->SetBobTimeScale(timeScaleRange * magnitude +
                              CPlayerCameraBob::GetSlowSpeedPeriodScale());
  mCameraBob->Update(dt, mgr, *this);
  return magnitude;
}

void CPlayer::SetIntoBallReadyAnimation(float dt, EPlayerMorphBallState state) {
  CPlayerBodyStateCmdMgr& commandMgr = mBodyController->CommandMgr();
  if (state == kMS_Morphed) {
    commandMgr.DeliverCmd(CPBCMorphToScrewAttackCmd(2, 1));
  } else if (mMovementState == NPlayer::kMS_ApplyJump) {
    commandMgr.DeliverCmd(CPBCMorphToBallCmd(0, 0));
  } else {
    const CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
    const CVector3f flatVelocity(CVector2f(localVelocity.GetX(), localVelocity.GetY()), 0.f);
    const float speed = flatVelocity.Magnitude();
    if (speed <= 1.f) {
      commandMgr.DeliverCmd(CPBCMorphToBallCmd(0, 1));
    } else {
      const float angle =
          CMath::Rad2Deg(CMath::WrapTwoPi(atan2f(-flatVelocity.GetX(), flatVelocity.GetY())));
      const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
      if (angle >= 225.f && angle <= 315.f) {
        commandMgr.DeliverCmd(CPBCMorphToBallCmd(0, 4));
      } else if (speed >= 0.5f * maxSpeed) {
        commandMgr.DeliverCmd(CPBCMorphToBallCmd(0, 3));
      } else {
        commandMgr.DeliverCmd(CPBCMorphToBallCmd(0, 2));
      }
    }
  }
}

bool CPlayer::UpdatePlayerRagDoll(float dt, CStateManager& mgr) {
  bool updated = false;
  if (mRagDoll.get()) {
    if (!mRagDoll->IsPrimed()) {
      mRagDoll->Prime(mgr, GetTransform(), *ModelData());
      const CVector3f position = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(position);
      AnimationData()->SetPlaybackRate(0.f);
    } else {
      float waterTop = -0.5f * FLT_MAX;
      if (InFluidId() != kInvalidUniqueId) {
        if (const CScriptWater* water =
                TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
          if (water->GetActive()) {
            waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
          }
        }
      }
      mRagDoll->Update(mgr, dt, waterTop);
      updated = true;
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }
    if (mRagDoll->IsOver() && !mRagDoll->WillContinueSmallMovements()) {
      SetMomentumWR(CVector3f::Zero());
      Stop();
    }
  }
  return updated;
}

void CPlayer::UpdatePlayerBodyController(float dt, CStateManager& mgr) {
  CPlayerBodyStateCmdMgr& commandMgr = mBodyController->CommandMgr();
  if (mPlayerState->IsPlayerAlive()) {
    switch (mMorphBallState) {
    case kMS_Unmorphed:
      if (mOrbitState == kOS_Grapple || mGrappleState != kGS_None) {
        if (mGrappleState == kGS_JumpOff) {
          commandMgr.DeliverCmd(CPBCJumpCmd(0, 0));
        } else {
          commandMgr.DeliverCmd(CPBCGrappleCmd(0));
        }
      } else if (mMovementState == NPlayer::kMS_ApplyJump || mMovementState == NPlayer::kMS_Jump) {
        if (mBodyController->IsFastLocomotion()) {
          commandMgr.DeliverCmd(CPBCJumpCmd(1, 0));
        } else {
          commandMgr.DeliverCmd(CPBCJumpCmd(0, 0));
        }
      } else {
        mBodyController->SetLocomotionMode(1);
        commandMgr.DeliverCmd(
            CPBCLocomotionCmd(GetDampedClampedVelocityWR(), GetTransform().GetForward()));
      }
      break;
    case kMS_Unmorphing:
      if (GetAverageSpeed() > 1.f) {
        commandMgr.DeliverCmd(CPlayerBodyStateCmd(kPBSC_ContinueLocomotion));
      }
      break;
    default:
      break;
    }
    if (mgr.IsMultiplayer() &&
        (mInFreeLook || (mOrbitState != kOS_NoOrbit && mOrbitState != kOS_Grapple))) {
      if (!mBodyController->IsMoving()) {
        mBodyController->SetLocomotionMode(6);
      }
      commandMgr.DeliverCmd(CPBCAimCmd(GetFirstPersonCameraTransform().GetForward()));
    }
  }
  mBodyController->Update(dt, mgr);
}

void CPlayer::SetOutOfBallReadyAnimation(float dt, CStateManager& mgr) {
  CPlayerBodyStateCmdMgr& commandMgr = mBodyController->CommandMgr();
  if (mMovementState != NPlayer::kMS_OnGround) {
    const CVector3f ballPos = GetBallPosition();
    if (mgr.RayCollideWorld(ballPos, ballPos + CVector3f(0.f, 0.f, -7.f), BallTransitionCollide,
                            this)) {
      commandMgr.DeliverCmd(CPBCMorphToPlayerCmd(1, 0));
      return;
    }
  }
  const CVector3f flatVelocity(CVector2f(GetVelocityWR().GetX(), GetVelocityWR().GetY()), 0.f);
  if (flatVelocity.CanBeNormalized()) {
    const float speed = flatVelocity.Magnitude();
    const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
    if (speed <= 0.2f * maxSpeed) {
      commandMgr.DeliverCmd(CPBCMorphToPlayerCmd(1, 1));
    } else {
      const CVector3f cameraForward = mCameraManager->GetBallCamera()->GetTransform().GetForward();
      if (CVector3f::Dot(mMoveDir, cameraForward) < -0.5f) {
        commandMgr.DeliverCmd(CPBCMorphToPlayerCmd(1, 1));
      } else if (speed < maxSpeed) {
        commandMgr.DeliverCmd(CPBCMorphToPlayerCmd(1, 2));
      } else {
        commandMgr.DeliverCmd(CPBCMorphToPlayerCmd(1, 3));
      }
    }
  } else {
    commandMgr.DeliverCmd(CPBCMorphToPlayerCmd(1, 1));
  }
}

void CPlayer::PrepareToEnterMorphBallState(float dt, CStateManager& mgr) {
  CBallCamera* ballCamera = mCameraManager->BallCamera();
  mCameraManager->TransferCameraState(*mCameraManager->FirstPersonCamera(), *ballCamera, mgr);
  mCameraManager->SetPlayerCamera(mgr, ballCamera->GetUniqueId());
  const bool immediate = !ballCamera->TransitionToMorphBallState(mgr);
  const bool hasInitialHint =
      static_cast< const CCameraHintManager* >(mCameraManager->GetHintManager())
          ->HasBallCameraInitialPositionHint(mgr);
  if (!hasInitialHint) {
    if (!immediate) {
      mCameraManager->HintManager()->Reset(mgr);
      ballCamera->SetState(CBallCamera::kBCS_ToBall, mgr);
      SetCameraState(kCS_Four, mgr);
      mCameraManager->TransferCameraState(*mCameraManager->FirstPersonCamera(),
                                          *mCameraManager->BallCamera(), mgr);
      const CTransform4f xf = mCameraManager->GetFirstPersonCamera()->GetTransform();
      ballCamera->SetTransform(xf);
      ballCamera->ResetLookAtPosition();
      ballCamera->TeleportCamera(xf.GetTranslation(), mgr);
      mCameraManager->HintManager()->Reset(mgr);
      if (mSpawnedMorphBallState == kMS_Morphed) {
        ballCamera->TeleportCamera(xf, mgr);
      }
    } else {
      ActivateMorphBallCamera(mgr);
    }
  } else if (mSpawnedMorphBallState != kMS_Morphed && !mMorphBall->InScrewAttackMode()) {
    if (immediate) {
      mCameraManager->HintManager()->Reset(mgr);
      return;
    }
    ballCamera->SetState(CBallCamera::kBCS_ToBall, mgr);
    SetCameraState(kCS_Four, mgr);
    mCameraManager->TransferCameraState(*mCameraManager->FirstPersonCamera(),
                                        *mCameraManager->BallCamera(), mgr);
    const CTransform4f xf = mCameraManager->GetFirstPersonCamera()->GetTransform();
    ballCamera->SetTransform(xf);
    ballCamera->ResetLookAtPosition();
    ballCamera->TeleportCamera(xf.GetTranslation(), mgr);
    mCameraManager->HintManager()->Reset(mgr);
  } else {
    ballCamera->SetState(CBallCamera::kBCS_ToBall, mgr);
    SetCameraState(kCS_Four, mgr);
    mCameraManager->TransferCameraState(*mCameraManager->FirstPersonCamera(),
                                        *mCameraManager->BallCamera(), mgr);
    const CTransform4f xf = mCameraManager->GetFirstPersonCamera()->GetTransform();
    ballCamera->SetTransform(xf);
    ballCamera->ResetLookAtPosition();
    ballCamera->TeleportCamera(xf.GetTranslation(), mgr);
    ballCamera->TeleportCamera(xf, mgr);
    ballCamera->ResetToTweaks(mgr);
    mCameraManager->SetPlayerCamera(mgr, ballCamera->GetUniqueId());
    mCameraManager->SetCurrentCameraId(ballCamera->GetUniqueId());
  }
}

void CPlayer::BeginMorphTransition(float dt, CStateManager& mgr, EPlayerMorphBallState state) {
  SetIntoBallReadyAnimation(dt, state);
  SetMomentumWR(CVector3f::Zero());
  SetMorphBallState(kMS_Morphing, state);
  SetCameraState(kCS_Four, mgr);
  mLookDir = GetTransform().GetForward();
  mMoveDir = mLookDir;
  mMoveDir.SetZ(0.f);
  if (mMoveDir.CanBeNormalized()) {
    mMoveDir.Normalize();
  } else {
    mLookDir = CVector3f(0.f, 1.f, 0.f);
    mMoveDir = CVector3f(0.f, 1.f, 0.f);
  }
  PrepareToEnterMorphBallState(dt, mgr);
  SetOrbitRequest(kOR_EnterMorphBall, mgr);
  mGun->HolsterGun(mgr);
  mGravityBoostUsed = false;
}

bool CPlayer::PrepareToLeaveMorphBallState(float dt, CStateManager& mgr, EPlayerMorphBallState state) {
  CBallCamera* ballCamera = mCameraManager->BallCamera();
  ballCamera->TeleportCamera(mCameraManager->GetCurrentCameraTransform(mgr, false), mgr);
  if (mCameraManager->IsInterpolationCameraActive()) {
    ballCamera->SetLookAtPosition(GetTranslation() + 5.f * GetTransform().GetForward());
  } else {
    ballCamera->SetLookAtPosition(
        mCameraManager->GetCurrentCamera(mgr, false)->GetScanObjectIndicatorPosition(mgr));
  }
  CBallCamera* transitionCamera = mCameraManager->BallCamera();
  transitionCamera->SetState(CBallCamera::kBCS_FromBall, mgr);
  CVector3f camToPlayer = GetTranslation() - ballCamera->GetTranslation();
  camToPlayer.SetZ(0.f);
  if (camToPlayer.CanBeNormalized()) {
    camToPlayer.Normalize();
    CVector3f direction = mLeaveMorphDir;
    if (state == kMS_Morphed) {
      direction = mMoveDir;
    }
    CVector3f lookFlat = mLookDir;
    lookFlat.SetZ(0.f);
    if ((!lookFlat.CanBeNormalized() || lookFlat.Magnitude() < .1f) && state != kMS_Morphed) {
      direction = camToPlayer;
    }
    if (mOutOfBallLookAtHint) {
      if (const CScriptPlayerHint* hint =
              TCastToConstPtr< CScriptPlayerHint >(GetPlayerHintManager()->GetCurrentHint(mgr))) {
        CVector3f delta = hint->GetTranslation() - GetTranslation();
        delta.SetZ(0.f);
        if (delta.CanBeNormalized()) {
          direction = delta.AsNormalized();
        }
      }
    }
    if (IsOutOfBallLookAtHintActor()) {
      if (const CScriptPlayerHint* hint =
              TCastToConstPtr< CScriptPlayerHint >(GetPlayerHintManager()->GetCurrentHint(mgr))) {
        if (const CActor* actor =
                TCastToConstPtr< CActor >(mgr.GetObjectById(hint->GetActorId()))) {
          CVector3f delta = actor->GetOrbitPosition(mgr) - GetTranslation();
          delta.SetZ(0.f);
          if (delta.CanBeNormalized()) {
            direction = delta.AsNormalized();
          }
        }
      }
    }
    if (acosf(CMath::Limit(CVector3f::Dot(camToPlayer, direction), 1.f)) < M_PIF / 1.2f ||
        IsOutOfBallLookAtHintActor() || state == kMS_Morphed) {
      SetTransform(CTransform4f::LookAt(GetTranslation(), CVector3f(GetTranslation() + direction)));
    } else {
      SetTransform(
          CTransform4f::LookAt(GetTranslation(), CVector3f(GetTranslation() + camToPlayer)));
      UpdateArmAndGunTransforms(.01f, mgr);
    }
  } else {
    SetTransform(CreateTransformFromMovementDirection());
  }
  const TUniqueId closeActorId = mCameraManager->GetBallCamera()->GetTooCloseActorId();
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(closeActorId))) {
    if (ballCamera->GetTooCloseActorDistance() < 20.f &&
        ballCamera->GetTooCloseActorDistance() > 1.f) {
      CVector3f delta = actor->GetTranslation() - GetTranslation();
      delta.SetZ(0.f);
      CVector3f camDelta = actor->GetTranslation() - ballCamera->GetTranslation();
      camDelta.SetZ(0.f);
      if (delta.CanBeNormalized() && camDelta.CanBeNormalized()) {
        delta.Normalize();
        CVector3f cameraLook = ballCamera->GetTransform().GetForward();
        cameraLook.SetZ(0.f);
        cameraLook.Normalize();
        camDelta.Normalize();
        if (CVector3f::Dot(delta, camDelta) >= .3f && CVector3f::Dot(camDelta, cameraLook) >= .7f) {
          SetTransform(CTransform4f::LookAt(GetTranslation(), GetTranslation() + delta));
        }
      }
    }
  }
  const bool immediate = !transitionCamera->TransitionFromMorphBallState(mgr);
  mCameraManager->SetPlayerCamera(mgr, transitionCamera->GetUniqueId());
  if (immediate) {
    mTransitionFilterTimer = .95f;
    LeaveMorphBallState(mgr);
  }
  return immediate;
}

void CPlayer::BeginUnmorphTransition(float dt, CStateManager& mgr, EPlayerMorphBallState state) {
  SetMorphBallState(kMS_Unmorphing, state);
  SetOutOfBallReadyAnimation(dt, mgr);
  mMorphBall->LeaveMorphBallState(mgr);
  const bool immediate = PrepareToLeaveMorphBallState(dt, mgr, state);
  ForceGunOrientation(GetTransform(), mgr);
  mGun->DrawGun(mgr);
  ClearForcesAndTorques();
  SetAngularVelocityWR(CAxisAngle::Identity());
  AddMaterial(kMT_GroundCollider, mgr);
  SetMomentumWR(CVector3f::Zero());
  if (!immediate) {
    SetCameraState(kCS_Four, mgr);
  }
}

void CPlayer::RequestScrewAttackTransition(EPlayerMorphBallState state) {
  mScrewAttackTransitionPending = true;
  mScrewAttackTransitionState = state;
  mMorphBall->SetScrewAttackActive(false);
}

void CPlayer::ActivateMorphBallCamera(CStateManager& mgr) {
  SetCameraState(kCS_Two, mgr);
  mCameraManager->BallCamera()->SetState(CBallCamera::kBCS_Default, mgr);
}

void CPlayer::EnterMorphBallState(CStateManager& mgr, EPlayerMorphBallState state) {
  SetMorphBallState(kMS_Morphed, state);
  RemoveMaterial(kMT_GroundCollider, mgr);
  if (gkMorphBallOrbitMode != 0) {
    SetOrbitRequestForOtherPlayers(kOR_BoostBall, mgr);
  }
  const float radius = GetTweakPlayer()->GetBallRadius();
  SetAngularVelocityOR(
      CAxisAngle::FromVector(CVector3f(-GetVelocityWR().Magnitude() / radius, 0.f, 0.f)));
  const CMorphBall::EBallState states[] = {CMorphBall::kBS_Normal, CMorphBall::kBS_ScrewAttack};
  mMorphBall->EnterMorphBallState(mgr, states[mSpawnedMorphBallState]);
  mMorphBall->TakeDamage(-1.f);
  mMorphBall->SetDamageTimer(0.f);
  mPlayerState->StartTransitionToVisor(CPlayerState::kPV_Combat);
  mGun->Reset(mgr);
}

void CPlayer::LeaveMorphBallState(CStateManager& mgr) {
  AddMaterial(kMT_GroundCollider, mgr);
  AddMaterial(kMT_Orbit, kMT_Target, mgr);
  SetMomentumWR(CVector3f::Zero());
  SetMorphBallState(kMS_Unmorphed, kMS_Unmorphed);
  SetHudDisable(FLT_EPSILON, 0.f, 2.f);
  SetHudDisable(FLT_EPSILON, 0.f, 2.f);
  mFreeLookYawAngle = 0.f;
  mHorizFreeLookAngleVel = 0.f;
  mFreeLookPitchAngle = 0.f;
  mVertFreeLookAngleVel = 0.f;
  mMorphBall->LeaveMorphBallState(mgr);
  mMorphBall->SetBallState(CMorphBall::kBS_Normal);
  mCameraManager->TransferCameraState(*mCameraManager->BallCamera(),
                                      *mCameraManager->FirstPersonCamera(), mgr);
  mCameraManager->SetPlayerCamera(mgr, mCameraManager->GetFirstPersonCamera()->GetUniqueId());
  mCameraManager->BallCamera()->SetState(CBallCamera::kBCS_Default, mgr);
  mCameraManager->BallCamera()->SetFovAndTarget(CCameraManager::GetDefaultThirdPersonVerticalFOV());
  SetCameraState(kCS_FirstPerson, mgr);
  mCameraManager->FirstPersonCamera()->DeferBallTransitionProcessing();
  mCameraManager->FirstPersonCamera()->PreThink(0.f, mgr);
  ForceGunOrientation(GetTransform(), mgr);
  mGun->DrawGun(mgr);
  if (!mgr.IsMultiplayer()) {
    AnimationData()->GetParticleDB().DestroyAllActiveParticles();
  }
}

void CPlayer::UpdateTransitionFilter(float dt, CStateManager& mgr) {
  const uint player = mgr.MaskUIdNumPlayers(GetUniqueId());
  CCameraFilterPass& filter = mgr.CameraFilterPass(player, 8);
  if (mTransitionFilterTimer > 0.f) {
    mTransitionFilterTimer += dt;
    if (mTransitionFilterTimer > 1.25f) {
      mTransitionFilterTimer = 0.f;
      filter.DisableFilter(0.f);
    } else if (mTransitionFilterTimer >= .95f) {
      const float time = mTransitionFilterTimer - .95f;
      float alpha;
      if (time < .1f) {
        alpha = .3f * time / .1f;
      } else if (time < .15f) {
        alpha = .3f;
      } else {
        alpha = .3f * (1.f - CMath::Limit((time - .15f) / .15f, 1.f));
      }
      filter.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_ScanLinesEven, 0.f,
                       CColor(0xffdf8900u).WithAlphaOf(alpha), kInvalidAssetId);
    }
  } else {
    filter.DisableFilter(0.f);
  }
}

void CPlayer::UpdateMorphBallTransition(float dt, CStateManager& mgr) {
  const EPlayerMorphBallState morphState = mMorphBallState;
  if (mgr.IsMultiplayer() || !mBodyController->IsLocomotionActive() ||
      !mgr.GetCameraManager(mPlayerIndex)->IsInFPCamera()) {
    const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    if (mBodyController->IsMorphTransitionActive()) {
      MoveInOneFrameOR(deltas.GetOffsetDelta(), dt);
      RotateInOneFrameOR(deltas.GetOrientationDelta(), dt);
    }
  }
  mMorphTime = rstl::min_val(mMorphDuration, mMorphTime + dt);
  switch (morphState) {
  case kMS_Unmorphing: {
    const CAABox bounds = GetCollisionPrimitive()->CalculateAABox(GetPrimitiveTransform());
    bounds.GetCenterPoint();
    ClearForcesAndTorques();
    SetAngularVelocityWR(CAxisAngle::Identity());
    const bool cinematic = mCameraManager->IsInCinematicCamera();
    if (mMorphTime >= mMorphDuration || cinematic) {
      mTransitionFilterTimer = rstl::max_val(mTransitionFilterTimer, .95f);
      CVector3f displacement = CVector3f::Zero();
      if (CanLeaveMorphBallState(mgr, displacement)) {
        SetTranslation(GetTranslation() + displacement);
        LeaveMorphBallState(mgr);
      } else {
        mMorphTime = mMorphDuration - mMorphTime;
        BeginMorphTransition(dt, mgr, kMS_Unmorphed);
      }
    }
    break;
  }
  case kMS_Morphing: {
    ClearForcesAndTorques();
    SetAngularVelocityWR(CAxisAngle::Identity());
    const bool cinematic = mCameraManager->IsInCinematicCamera();
    if (mMorphTime >= mMorphDuration || cinematic) {
      if (CanEnterMorphBallState()) {
        ActivateMorphBallCamera(mgr);
        EnterMorphBallState(mgr, mSpawnedMorphBallState);
      } else {
        mMorphTime = mMorphDuration - mMorphTime;
        BeginUnmorphTransition(dt, mgr, kMS_Unmorphed);
      }
    }
    if (GetMorphBallTransitionFactor() >= .5f && !mMorphBall->IsMorphBallTransitionFlashValid()) {
      mMorphBall->ResetMorphBallTransitionFlash();
    }
    break;
  }
  default:
    break;
  }
}

bool CPlayer::IsGravityBoostActive() const { return mGravityBoostDuration > 0.f; }

void CPlayer::StartGravityBoost(CStateManager& mgr) {
  if ((GetTweakPlayer()->GetGravityBoostMultipleAllowed() || !mGravityBoostUsed) &&
      !IsGravityBoostActive() &&
      mgr.GetCameraManager(GetPlayerIndex())->GetFirstPersonCamera()->GetFluidCount() != 0 &&
      mMorphBallState == kMS_Unmorphed && GetFluidCount() != 0) {
    mGravityBoostDuration = GetTweakPlayer()->GetGravityBoostTime();
    CVector3f velocity = GetVelocityWR();
    velocity.SetZ(velocity.GetZ() * 0.1f);
    SetVelocityWR(velocity);
    mGravityBoostSfx =
        CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x123, 0x30e), 127,
                              GetSoundPan(kMSP_4), GetCurrentAreaId().Value(), true, true);
    CSfxManager::SetIgnoreAreaLowPass(mGravityBoostSfx, true);
    ApplySubmergedPitchBend(mGravityBoostSfx);
    mGravityBoostUsed = true;
  }
}

// Guessed name
void CPlayer::ApplyGravityBoost(float dt, CStateManager& mgr) {
  if (mGravityBoostDuration > 0.f) {
    mGravityBoostDuration -= dt;
    if (mGravityBoostDuration <= 0.f ||
        mgr.GetCameraManager(GetPlayerIndex())->GetFirstPersonCamera()->GetFluidCount() == 0 ||
        mMorphBallState != kMS_Unmorphed) {
      EndGravityBoost(mgr);
    } else {
      ApplyForceOR(CVector3f(0.f, 0.f, GetTweakPlayer()->GetGravityBoostForce()),
                   CAxisAngle::Identity());
    }
  }
}

void CPlayer::EndGravityBoost(CStateManager& mgr) {
  CVector3f velocity = GetVelocityWR();
  velocity.SetZ(velocity.GetZ() * GetTweakPlayer()->GetGravityBoostCancelDampening());
  SetVelocityWR(velocity);
  mGravityBoostDuration = 0.f;
  if (mGravityBoostSfx) {
    CSfxManager::SfxStop(mGravityBoostSfx);
  }
  mGravityBoostEndSfx =
      CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x360, 0x35f), 127,
                            GetSoundPan(kMSP_4), GetCurrentAreaId().Value(), true, false);
  CSfxManager::SetIgnoreAreaLowPass(mGravityBoostEndSfx, true);
  ApplySubmergedPitchBend(mGravityBoostEndSfx);
}
