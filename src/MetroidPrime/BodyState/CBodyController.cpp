#include "MetroidPrime/BodyState/CBodyController.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/math.hpp"

CBodyController::CBodyController(CActor& actor, float turnSpeed, EBodyType bodyType)
: mActor(&actor)
, mBodyStateInfo(actor, bodyType)
, mRot(CQuaternion::NoRotation())
, mLocomotionType(pas::kLT_Relaxed)
, mFallState(pas::kFS_Zero)
, mBodyType(bodyType)
, mCurAnim(-1)
, mTurnSpeed(turnSpeed)
, mAnimationOver(false)
, mActive(false)
, mFrozen(false)
, mHasBeenFrozen(false)
, mPlayDeathAnims(true)
, mIntoFreezeDur(0.f)
, mFrozenDur(0.f)
, mBreakoutDur(0.f)
, mTimeFrozen(0.f)
, mBackedUpForce(CVector3f::Zero())
, mFireDur(0.f)
, mElectrocutionDur(0.f)
, mTimeOnFire(0.f)
, mTimeElectrocuting(0.f)
, mRestrictedFlyerMoveSpeed(0.f)
, mTimeScale(1.f)
, mFireDamageBuildup(0.f) {
  mBodyStateInfo.SetBodyController(this);
}

void CBodyController::Activate(CStateManager& mgr, pas::EAnimationState state) {
  mActive = true;
  if (state != pas::kAS_Invalid) {
    mBodyStateInfo.SetState(state);
  } else {
    mBodyStateInfo.SetState(pas::EAnimationState(GetPASDatabase().GetDefaultState()));
  }
  mBodyStateInfo.GetCurrentState()->Start(*this, mgr);
  mBodyStateInfo.GetCurrentAdditiveState()->Start(*this, mgr);
}

void CBodyController::Update(float dt, CStateManager& mgr) {
  SetPlaybackRate(mTimeScale);
  if (!mActive) {
    return;
  }

  mAnimationOver = !GetOwner().GetModelData()->GetAnimationData()->IsAnimTimeRemaining(
      dt, rstl::string_l("Whole Body"));
  mCmdMgr.BlendSteeringCmds();
  mRot = CQuaternion::NoRotation();
  UpdateBody(dt, mgr);
  if (!mFrozen) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mActor)) {
      actor->RotateInOneFrameOR(mRot, dt);
    }
  }
  mCmdMgr.Reset();
}

bool CBodyController::HasBodyState(pas::EAnimationState state) const {
  return GetOwner().GetModelData()->GetAnimationData()->GetPASDatabase().HasState(state);
}

pas::EFallState CBodyController::GetFallState() const { return mFallState; }

void CBodyController::SetFallState(pas::EFallState state) { mFallState = state; }

void CBodyController::UpdateBody(float dt, CStateManager& mgr) {
  if (mTimeScale != 1.f) {
    mTimeScale = rstl::min_val(mTimeScale + (1.f / 3.f) * dt, 1.f);
  }
  if (mFireDamageBuildup != 0.f) {
    mFireDamageBuildup = rstl::max_val(mFireDamageBuildup - (1.f / 30.f) * dt, 0.f);
  }

  UpdateFrozenInfo(dt, mgr);
  if (mFireDur > 0.f) {
    if (mTimeOnFire > mFireDur) {
      mTimeOnFire = 0.f;
      mFireDur = 0.f;
    } else {
      mTimeOnFire += dt;
    }
  } else if (mElectrocutionDur > 0.f) {
    if (mTimeElectrocuting > mElectrocutionDur) {
      mTimeElectrocuting = 0.f;
      mElectrocutionDur = 0.f;
    } else {
      mTimeElectrocuting += dt;
    }
  }

  pas::EAnimationState nextState = mBodyStateInfo.GetCurrentState()->UpdateBody(dt, *this, mgr);
  if (nextState != pas::kAS_Invalid) {
    mBodyStateInfo.GetCurrentState()->Shutdown(*this);
    mBodyStateInfo.SetState(nextState);
    mBodyStateInfo.GetCurrentState()->Start(*this, mgr);
  }

  nextState = mBodyStateInfo.GetCurrentAdditiveState()->UpdateBody(dt, *this, mgr);
  if (nextState != pas::kAS_Invalid) {
    mBodyStateInfo.GetCurrentAdditiveState()->Shutdown(*this);
    mBodyStateInfo.SetAdditiveState(nextState);
    mBodyStateInfo.GetCurrentAdditiveState()->Start(*this, mgr);
  }
}

void CBodyController::SetLocomotionType(pas::ELocomotionType type) { mLocomotionType = type; }

void CBodyController::AbortScriptedAnimations() {
  if (GetCurrentStateId() == pas::kAS_Scripted) {
    CBodyStateCmd cmd(kBSC_AbortScripted);
    mCmdMgr.DeliverCmd(cmd);
  }
}

void CBodyController::SetTurnSpeed(float speed) { mTurnSpeed = rstl::max_val(0.f, speed); }

void CBodyController::EnableAnimation(bool enable) {
  GetOwner().ModelData()->AnimationData()->SetIsAnimating(enable);
}

void CBodyController::SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop,
                                          bool noTrans) {
  GetOwner().ModelData()->AnimationData()->SetAnimation(parms, noTrans);
  GetOwner().ModelData()->EnableLooping(loop);
  mCurAnim = parms.GetAnimationId();
}

float CBodyController::GetAnimTimeRemaining() const {
  return GetOwner().GetModelData()->GetAnimationData()->GetAnimTimeRemaining(
      rstl::string_l("Whole Body"));
}

void CBodyController::SetPlaybackRate(float rate) {
  GetOwner().ModelData()->AnimationData()->SetPlaybackRate(rate);
}

void CBodyController::MultiplyPlaybackRate(float scale) {
  GetOwner().ModelData()->AnimationData()->MultiplyPlaybackRate(scale);
}

void CBodyController::SetDeltaRotation(const CQuaternion& rotation) { mRot = mRot * rotation; }

void CBodyController::FaceDirection(const CVector3f& direction, float dt) {
  if (mFrozen) {
    return;
  }

  CVector3f planarDirection = direction;
  planarDirection.SetZ(0.f);
  if (planarDirection.CanBeNormalized()) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mActor)) {
      const CVector3f normalized = planarDirection.AsNormalized();
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(GetOwner().GetTransform().GetForward(), CUnitVector3f::kN_No),
          CUnitVector3f(normalized, CUnitVector3f::kN_No), CRelAngle::FromDegrees(dt * mTurnSpeed));
      const CQuaternion localRotation = CQuaternion::ScalarVector(
          rotation.GetScalar(), GetOwner().GetTransform().TransposeRotate(rotation.GetVector()));
      actor->RotateInOneFrameOR(localRotation, dt);
    }
  }
}

void CBodyController::FaceDirectionOnSurface(const CVector3f& direction,
                                             const CVector3f& currentDirection, float dt) {
  if (mFrozen || !direction.CanBeNormalized() || !currentDirection.CanBeNormalized()) {
    return;
  }

  if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mActor)) {
    const CUnitVector3f current(currentDirection);
    const CVector3f up = actor->GetTransform().GetUp();
    const CVector3f projected = direction - CVector3f::Dot(direction, up) * up;
    if (!projected.CanBeNormalized()) {
      return;
    }

    // The target uses the projected direction without normalizing it.
    const float dot = CVector3f::Dot(projected, current);
    if (!close_enough(dot, 1.f)) {
      const CRelAngle angle = CRelAngle::FromDegrees(dt * mTurnSpeed);
      const CQuaternion rotation =
          dot < -0.99981f ? CQuaternion::AxisAngle(CUnitVector3f(up, CUnitVector3f::kN_No), angle)
                          : CQuaternion::ShortestRotationArcClamped(current, projected, angle);
      const CQuaternion localRotation = CQuaternion::ScalarVector(
          rotation.GetScalar(), GetOwner().GetTransform().TransposeRotate(rotation.GetVector()));
      actor->RotateInOneFrameOR(localRotation, dt);
    }
  }
}

void CBodyController::FaceDirection3D(const CVector3f& direction, const CVector3f& currentDirection,
                                      float dt) {
  if (mFrozen || !direction.CanBeNormalized() || !currentDirection.CanBeNormalized()) {
    return;
  }

  if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mActor)) {
    const CUnitVector3f current(currentDirection);
    const CUnitVector3f desired(direction);
    const float dot = CVector3f::Dot(desired, current);
    if (!close_enough(dot, 1.f)) {
      const CRelAngle angle = CRelAngle::FromDegrees(dt * mTurnSpeed);
      const CQuaternion rotation =
          dot < -0.99981f
              ? CQuaternion::AxisAngle(
                    CUnitVector3f(actor->GetTransform().GetUp(), CUnitVector3f::kN_No), angle)
              : CQuaternion::ShortestRotationArcClamped(current, desired, angle);
      const CQuaternion localRotation = CQuaternion::ScalarVector(
          rotation.GetScalar(), GetOwner().GetTransform().TransposeRotate(rotation.GetVector()));
      actor->RotateInOneFrameOR(localRotation, dt);
    }
  }
}

const CPASDatabase& CBodyController::GetPASDatabase() const {
  return GetOwner().GetModelData()->GetAnimationData()->GetPASDatabase();
}

void CBodyController::PlayBestAnimation(const CPASAnimParmData& parms, CRandom16& random) {
  const rstl::pair< float, int > best = GetPASDatabase().FindBestAnimation(parms, random, -1);
  const CAnimPlaybackParms playback(best.second, -1, 1.f, true);
  SetCurrentAnimation(playback, false, false);
}

void CBodyController::LoopBestAnimation(const CPASAnimParmData& parms, CRandom16& random) {
  const rstl::pair< float, int > best = GetPASDatabase().FindBestAnimation(parms, random, -1);
  const CAnimPlaybackParms playback(best.second, -1, 1.f, true);
  SetCurrentAnimation(playback, true, false);
}

void CBodyController::Freeze(float intoFreezeDuration, float frozenDuration,
                             float breakoutDuration) {
  mIntoFreezeDur = intoFreezeDuration;
  mFrozenDur = frozenDuration;
  mBreakoutDur = breakoutDuration;
  mFrozen = true;
  mHasBeenFrozen = true;

  CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(GetOwner());
  mBackedUpForce = actor->GetConstantForceWR();
  actor->SetConstantForceWR(CVector3f::Zero());
  actor->SetMomentumWR(CVector3f::Zero());
  mFireDur = 0.f;
  mTimeOnFire = 0.f;
  mTimeFrozen = 0.f;
}

void CBodyController::FrozenBreakout() {
  if (mFrozen) {
    const float timeToBreakout = mIntoFreezeDur + mFrozenDur;
    if (mTimeFrozen < timeToBreakout) {
      mTimeFrozen = timeToBreakout;
    }
  }
}

void CBodyController::UnFreeze() {
  SetPlaybackRate(mTimeScale);
  mFrozen = false;
  mIntoFreezeDur = 0.f;
  mFrozenDur = 0.f;
  mBreakoutDur = 0.f;
  mTimeFrozen = 0.f;
  mActor->SetVolume(127);

  CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(GetOwner());
  actor->SetConstantForceWR(mBackedUpForce);
  actor->SetVelocityWR((1.f / actor->GetMass()) * mBackedUpForce);
}

float CBodyController::GetPercentageFrozen() const {
  const float totalTime = mBreakoutDur + (mIntoFreezeDur + mFrozenDur);
  if (totalTime == 0.f || mTimeFrozen > totalTime) {
    return 0.f;
  }

  float result = 1.f;
  if (mTimeFrozen <= mIntoFreezeDur && mIntoFreezeDur > 0.f) {
    return mTimeFrozen / mIntoFreezeDur;
  }
  if (mTimeFrozen >= totalTime - mBreakoutDur && mBreakoutDur > 0.f) {
    result = 1.f - (mTimeFrozen - (mFrozenDur + mIntoFreezeDur)) / mBreakoutDur;
  }
  return result;
}

void CBodyController::SetOnFire(float duration) {
  mFireDur = duration;
  mTimeOnFire = 0.f;
  if (IsFrozen()) {
    UnFreeze();
  }
}

void CBodyController::DouseFlames() {
  if (mFireDur > 0.f) {
    mFireDur = 0.f;
    mTimeOnFire = 0.f;
  }
}

void CBodyController::SetElectrocuting(float duration) {
  if (!IsElectrocuting()) {
    CBCAdditiveReactionCmd reaction(pas::kART_Electrocution, 1.f, true);
    CommandMgr().DeliverCmd(reaction);
  }
  mElectrocutionDur = duration;
  mTimeElectrocuting = 0.f;
  if (IsFrozen()) {
    UnFreeze();
  } else if (IsOnFire()) {
    DouseFlames();
  }
}

void CBodyController::DouseElectrocuting() {
  mElectrocutionDur = 0.f;
  mTimeElectrocuting = 0.f;
  CBodyStateCmd cmd(kBSC_StopReaction);
  mCmdMgr.DeliverCmd(cmd);
}

void CBodyController::UpdateFrozenInfo(float dt, CStateManager& mgr) {
  if (!mFrozen) {
    return;
  }

  const float totalTime = mIntoFreezeDur + mFrozenDur + mBreakoutDur;
  if (mTimeFrozen > totalTime &&
      mBodyStateInfo.GetCurrentAdditiveStateId() != pas::kAS_AdditiveReaction) {
    UnFreeze();
    if (mActor) {
      mActor->SendScriptMsgs(kSS_UnFrozen, mgr);
    }
    return;
  }

  if (mTimeFrozen <= totalTime) {
    float unfrozen = 1.f;
    if (mTimeFrozen < totalTime - mBreakoutDur) {
      unfrozen = 1.f - GetPercentageFrozen();
    }
    MultiplyPlaybackRate(unfrozen * mTimeScale);
    mTimeFrozen += dt;
    GetOwner().SetVolume(static_cast< uchar >(127.f * unfrozen));
    if (mTimeFrozen > totalTime && HasIceBreakoutState()) {
      mCmdMgr.DeliverCmd(CBCAdditiveReactionCmd(pas::kART_IceBreakout, 1.f, false));
    }
  }
}
