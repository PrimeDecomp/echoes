#include "MetroidPrime/Player/CPlayer.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

// NonMatching scaffold. Definitions are in reverse target order for deferred inlining.

CVector3f CPlayer::GetDampedClampedVelocityWR() const {
  // TODO: Recover the remaining target behavior.
  return CVector3f::Zero();
}

float CPlayer::GetAverageSpeed() const {
  rstl::optional_object< float > average = mMoveSpeedAvg.GetAverage();
  return average.valid() ? *average : mMoveSpeed;
}

float CPlayer::GetAcceleration() const {
  if (mCurAcceleration >= mAccelerationTable.size()) {
    return mAccelerationTable.back();
  }
  return mAccelerationTable[mCurAcceleration];
}

float CPlayer::GetGravity() const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::GetWeight() const { return GetMass() * -GetGravity(); }

void CPlayer::UpdateBombJumpStuff() {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateStepCameraZBias(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::SidewaysDashAllowed(float strafeInput, float forwardInput,
                                  const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

void CPlayer::FinishSidewaysDash() {
  // TODO: Clamp excess sideways velocity before finishing the dash.
  if (mSidewaysDashing) {
    mDoneSidewaysDashing = true;
  }
  mSidewaysDashing = false;
  mStrafeInputAtDash = 0.f;
  mDashTimer = 0.f;
}

void CPlayer::fn_801892a0(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Recover the remaining target behavior.
}

float CPlayer::ForwardInput(const CFinalInput& input, float turnInput) const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::StrafeInput(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::TurnInput(const CFinalInput& input) const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::JumpInput(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

void CPlayer::SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::CalculatePlayerMovementDirection(float dt, const CVector3f& displacement) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::CalculateLeaveMorphBallDirection(const CFinalInput& input) {
  // TODO: Recover the remaining target behavior.
}

float CPlayer::GetBallMaxVelocity() const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::GetActualFirstPersonMaxVelocity(float dt) const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::GetActualBallMaxVelocity(float dt) const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
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
  if (mMorphBallState == kMS_Morphed) {
    return GetCollidableSphere();
  }
  return CPhysicsActor::GetCollisionPrimitive();
}

CTransform4f CPlayer::CreateTransformFromMovementDirection() const {
  // TODO: Recover the remaining target behavior.
  return CTransform4f::Identity();
}

void CPlayer::BombJump(const CVector3f& position, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::Teleport(const CTransform4f& xf, CStateManager& mgr, bool resetBallCam) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::CheckSubmerged() const {
  // TODO: Check the actor fluid counter before testing the water depth.
  const float height = mMorphBallState == kMS_Morphed ? 2.f * GetTweakPlayer()->GetBallRadius()
                                                      : 0.5f * GetEyeHeight();
  return height <= mDistanceUnderWater;
}

void CPlayer::UpdateSubmerged(const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
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
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

void CPlayer::fn_80185a88(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::fn_801858cc(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return false;
}

void CPlayer::TransitionToMorphBallState(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::TransitionFromMorphBallState(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80184ba4(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80184a60(float dt, CStateManager& mgr, EPlayerMorphBallState state) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::fn_801843d0(CStateManager& mgr, EPlayerMorphBallState state) {
  // TODO: Recover the remaining target behavior.
  return false;
}

void CPlayer::fn_801842c8(float dt, CStateManager& mgr, EPlayerMorphBallState state) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::fn_80184294(EPlayerMorphBallState state) {
  mScrewAttackTransitionPending = true;
  mScrewAttackTransitionState = state;
  mMorphBall->SetScrewAttackActive(false);
}

void CPlayer::ActivateMorphBallCamera(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::EnterMorphBallState(CStateManager& mgr, EPlayerMorphBallState state) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::LeaveMorphBallState(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateTransitionFilter(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateMorphBallTransition(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::IsGravityBoostActive() const { return mGravityBoostDuration > 0.f; }

void CPlayer::StartGravityBoost(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
void CPlayer::ApplyGravityBoost(float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::EndGravityBoost(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}
