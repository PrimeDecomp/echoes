#include "MetroidPrime/Player/CPlayer.hpp"

// NonMatching scaffold. Definitions are in reverse target order for deferred inlining.

bool CPlayer::ValidateOrbitTargetIdAndPointer(TUniqueId target, const CStateManager& mgr) const {
  // TODO: Check that the id resolves to an actor.
  return false;
}

int CPlayer::ValidateCurrentOrbitTargetId(CStateManager& mgr) {
  // TODO: Recover orbit-validation result values and target checks.
  return 0;
}

int CPlayer::ValidateOrbitTargetId(TUniqueId target, CStateManager& mgr) const {
  // TODO: Recover the remaining target behavior.
  return 0;
}

float CPlayer::GetOrbitMaxTargetDistance() const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

float CPlayer::GetOrbitMaxLockDistance() const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

void CPlayer::UpdateOrbitTarget(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitOrientation(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ActivateOrbitSource(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitInput(const CFinalInput& input, float dt, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitZone() {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitModeTimer(float dt) {
  if (mOrbitState == kOS_NoOrbit && mOrbitModeTimer > 0.f) {
    mOrbitModeTimer -= dt;
  } else {
    mOrbitModeTimer = 0.f;
  }
}

void CPlayer::UpdateOrbitPreventionTimer(float dt) {
  if (mOrbitPreventionTimer > 0.f) {
    mOrbitPreventionTimer -= dt;
  }
}

void CPlayer::AddOrbitDisableSource(CStateManager& mgr, TUniqueId id) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::RemoveOrbitDisableSource(TUniqueId id) {
  for (rstl::reserved_vector< TUniqueId, 5 >::iterator it = mOrbitDisableSources.begin();
       it != mOrbitDisableSources.end(); ++it) {
    if (*it == id) {
      mOrbitDisableSources.erase(it);
      break;
    }
  }
}

bool CPlayer::CheckOrbitDisableSourceList() const { return !mOrbitDisableSources.empty(); }

bool CPlayer::CheckOrbitDisableSourceList(const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::WithinOrbitScreenEllipse(const CVector3f& screenPosition,
                                       EPlayerZoneInfo zone) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::WithinOrbitScreenBox(const CVector3f& screenPosition, EPlayerZoneInfo zone,
                                   EPlayerZoneType type) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

void CPlayer::FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& candidates,
                                   rstl::reserved_vector< TUniqueId, 64 >& objects,
                                   EPlayerZoneInfo zone, EPlayerZoneType type, CStateManager& mgr,
                                   bool offScreen) {
  // TODO: Recover the remaining target behavior.
}

TUniqueId CPlayer::FindBestOrbitableObject(const rstl::reserved_vector< TUniqueId, 64 >& objects,
                                           EPlayerZoneInfo zone, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return kInvalidUniqueId;
}

void CPlayer::UpdateOrbitableObjects(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

TUniqueId CPlayer::FindOrbitTargetId(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return kInvalidUniqueId;
}

// Guessed name
TUniqueId CPlayer::CheckEnemyAgainstOrbitZone(TUniqueId target, EPlayerZoneInfo zone,
                                              EPlayerZoneType type, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return kInvalidUniqueId;
}

TUniqueId CPlayer::FindAimTargetId(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return kInvalidUniqueId;
}

// Guessed name
void CPlayer::UpdateAimCandidates(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::ValidateObjectForMode(TUniqueId target, CStateManager& mgr) const {
  // TODO: Recover the remaining target behavior.
  return false;
}

bool CPlayer::ValidateAimTargetId(TUniqueId target, CStateManager& mgr, float dt) {
  // TODO: Recover the remaining target behavior.
  return false;
}

void CPlayer::UpdateAimTargetTimer(float dt) {
  if (mAimTarget != kInvalidUniqueId && mAimTargetTimer > 0.f) {
    mAimTargetTimer -= dt;
  }
}

void CPlayer::UpdateAimTarget(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::SetOrbitPosition(float distance) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitFixedPosition() {
  mOrbitPoint = GetEyePosition() + GetTransform().Rotate(mOrbitVector);
}

void CPlayer::UpdateOrbitZPosition() {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateOrbitPosition(float distance, const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::SetOrbitTargetId(TUniqueId target, const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::SetOrbitState(EPlayerOrbitState state, const CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

CVector3f CPlayer::GetHUDOrbitTargetPosition() const {
  // TODO: Add the camera-bob translation.
  return mOrbitPoint;
}

float CPlayer::CalculateOrbitMinDistance(EPlayerOrbitType type) const {
  // TODO: Recover the remaining target behavior.
  return 0.f;
}

void CPlayer::OrbitPoint(EPlayerOrbitType type, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::OrbitCarcass(CStateManager& mgr) {
  if (mOrbitState == kOS_OrbitObject) {
    mOrbitType = kOT_Default;
    SetOrbitState(kOS_OrbitCarcass, mgr);
  }
}

void CPlayer::PreventFallingCameraPitch() {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::InGrappleJumpCooldown() const {
  return mMovementState != NPlayer::kMS_OnGround &&
         (mGrappleJumpTimeout > 0.f || (mJumpCameraTimer == 0.f && mOrbitState == kOS_NoOrbit));
}

void CPlayer::fn_8011eac4(EPlayerOrbitRequest request, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::SetOrbitRequestForTarget(TUniqueId target, EPlayerOrbitRequest request,
                                       CStateManager& mgr) {
  if ((mOrbitState == kOS_OrbitObject || mOrbitState == kOS_Grapple ||
       mOrbitState == kOS_ForcedOrbitObject) &&
      target == mOrbitTargetId) {
    SetOrbitRequest(request, mgr);
  }
}

void CPlayer::SetOrbitRequest(EPlayerOrbitRequest request, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::BreakGrapple(EPlayerOrbitRequest request, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::BeginGrapple(CVector3f& direction, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::ApplyGrappleJump(CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
}

bool CPlayer::ValidateFPPosition(CVector3f position, CStateManager& mgr) {
  // TODO: Recover the remaining target behavior.
  return false;
}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Recover the remaining target behavior.
}

void CPlayer::UpdateGrappleArmTransform(const CVector3f& offset, CStateManager& mgr, float dt) {
  // TODO: Recover the remaining target behavior.
}

CVector3f CPlayer::fn_8011ca08() const {
  // TODO: Project the camera direction from the eye at the orbit distance.
  return GetEyePosition();
}

void CPlayer::fn_8011c3c0() {
  // TODO: Recover the remaining target behavior.
}

// Guessed name
// Guessed name
// Guessed name
void* CPlayer::GetReflectionTextureData() const { return mReflectionTextureData; }

void* CPlayer::GetIndirectTextureData() const { return mIndirectTextureData; }

void* CPlayer::GetMaskTextureData() const { return mMaskTextureData; }
