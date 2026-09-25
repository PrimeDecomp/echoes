#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

void CPatterned::Start(CStateManager&, EStateMsg, float) {}

void CPatterned::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
}

void CPatterned::Dead(CStateManager&, EStateMsg, float) {
  // TODO: Submit the death command; begin fading and change materials when the body permits it.
}

float CPatterned::GetFadeOnDeathTime() const { return mFadeOnDeathTime; }

void CPatterned::PathFind(CStateManager&, EStateMsg, float) {
  // TODO: Initialize the search, advance its waypoints, and steer toward the next segment.
}

void CPatterned::fn_801524fc(CStateManager&) {
  // TODO: Search from the current position, select the next waypoint and start approaching it.
}

bool CPatterned::OffLine(CStateManager&, const CTriggerData& data) const {
  const CVector3f fromStart = GetTranslation() - mReflectedDestPos;
  CVector3f segment = mDestPos - mReflectedDestPos;
  float distanceSquared = fromStart.MagSquared();
  if (CVector3f::Dot(segment, fromStart) > 0.f) {
    segment.Normalize();
    const CVector3f fromEnd = GetTranslation() - mDestPos;
    const float along = CVector3f::Dot(segment, fromStart);
    distanceSquared = (fromStart - along * segment).MagSquared();
    if (CVector3f::Dot(segment, fromEnd) > 0.f) {
      distanceSquared = fromEnd.MagSquared();
    }
  }
  return distanceSquared > data.GetFloat() * data.GetFloat();
}

bool CPatterned::InRange(CStateManager& mgr, const CTriggerData&) const {
  const float range = 0.5f * (mMinAttackRange + mMaxAttackRange);
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() < range * range;
}

bool CPatterned::TooClose(CStateManager& mgr, const CTriggerData&) const {
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() <
         mMinAttackRange * mMinAttackRange;
}

bool CPatterned::InMaxRange(CStateManager& mgr, const CTriggerData&) const {
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() <
         mMaxAttackRange * mMaxAttackRange;
}

bool CPatterned::InDetectionRange(CStateManager& mgr, const CTriggerData&) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CVector3f delta = mgr.GetPlayer(i)->GetTranslation() - GetTranslation();
    if (delta.MagSquared() < mDetectionRange * mDetectionRange &&
        (mDetectionHeightRange <= 0.f ||
         delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange)) {
      return true;
    }
  }
  return false;
}

bool CPatterned::Leash(CStateManager&, const CTriggerData&) const {
  return mCurPlayerLeashTime > mPlayerLeashTime &&
         (mLatestLeashPosition - GetTranslation()).MagSquared() > mLeashRadius * mLeashRadius;
}

bool CPatterned::SpotPlayer(CStateManager& mgr, const CTriggerData&) const {
  const CVector3f eye = GetGunEyePos();
  const CVector3f forward = GetTransform().GetForward();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CVector3f delta = mgr.GetPlayer(i)->GetAimPosition(mgr, 0.f) - eye;
    const float forwardDistance = CVector3f::Dot(delta, forward);
    if (forwardDistance > 0.f &&
        delta.MagSquared() * mDetectionAngle < forwardDistance * forwardDistance) {
      return true;
    }
  }
  return false;
}

bool CPatterned::IsOnScreen(const CStateManager&) const {
  // TODO: Project the bounding-box center through the first player's current camera.
  return false;
}

bool CPatterned::PlayerSpot(CStateManager&, const CTriggerData&) const {
  // TODO: Check the first player's morph state, screen projection and visibility ray.
  return false;
}

bool CPatterned::Landed(CStateManager&, const CTriggerData&) const {
  const bool landed = mOnGround && !mPrevOnGround;
  mPrevOnGround = mOnGround;
  return landed;
}

bool CPatterned::PathOver(CStateManager&, const CTriggerData&) const {
  // TODO: Check the search result and last waypoint after vertical/ground movement.
  return false;
}

bool CPatterned::PathFound(CStateManager&, const CTriggerData&) const {
  // TODO: Test whether GetSearchPath() exists and reports a successful search.
  return false;
}

bool CPatterned::PathShagged(CStateManager&, const CTriggerData&) const {
  // TODO: Check search failure and excessive deviation from the active path segment.
  return false;
}

bool CPatterned::NoPathNodes(CStateManager&, const CTriggerData&) const {
  // TODO: Query whether the search has any usable nodes at the actor's position.
  return false;
}

bool CPatterned::Attacked(CStateManager&, const CTriggerData&) const {
  return mHitByPlayerProjectile;
}

bool CPatterned::HasPatrolPath(CStateManager& mgr, const CTriggerData&) const {
  return GetConnectedObject(mgr, kSS_Patrol, kSM_Follow) != kInvalidUniqueId;
}

bool CPatterned::InPosition(CStateManager&, const CTriggerData&) const { return mInPosition; }

bool CPatterned::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return GetAnimOver(mgr, data);
}

bool CPatterned::Stuck(CStateManager&, const CTriggerData&) const {
  return mPredictedLeashTime > 0.2f;
}

bool CPatterned::Delay(CStateManager&, const CTriggerData& data) const {
  return mStateMachine->GetTime() > data.GetFloat();
}

bool CPatterned::RandomDelay(CStateManager&, const CTriggerData& data) const {
  const TStateMachineState< CPatterned >& state =
      static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine);
  return state.GetTime() > data.GetFloat() * state.GetRandom();
}

bool CPatterned::FixedDelay(CStateManager&, const CTriggerData&) const {
  return mStateMachine->GetTime() > mStateMachine->GetDelay();
}

bool CPatterned::CodeTrigger(CStateManager&, const CTriggerData&) const {
  return static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine).GetCodeTrigger();
}

bool CPatterned::Random(CStateManager&, const CTriggerData& data) const {
  return static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine).GetRandom() <
         data.GetFloat();
}

bool CPatterned::FixedRandom(CStateManager&, const CTriggerData&) const {
  const TStateMachineState< CPatterned >& state =
      static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine);
  return state.GetRandom() < state.GetFixedRandom();
}

void CPatterned::SetAttackTarget(CStateManager&, TUniqueId) {}

void CPatterned::ApproachDest(CStateManager&) {
  // TODO: Choose locomotion/step commands using the destination segment and body type.
}

TUniqueId CPatterned::GetConnectedObject(CStateManager&, EScriptObjectState,
                                         EScriptObjectMessage) const {
  // TODO: Randomly choose among at most eight active, matching script connections.
  return kInvalidUniqueId;
}

pas::EStepDirection CPatterned::FindBestStepDirection(const CVector3f& direction) const {
  const CVector3f local = GetTransform().TransposeRotate(direction);
  const float angle = CVector3f::GetAngleDiff(local, CVector3f::Forward());
  if (angle < CMath::Deg2Rad(45.f)) {
    return pas::kSD_Forward;
  }
  if (angle > CMath::Deg2Rad(135.f)) {
    return pas::kSD_Backward;
  }
  return CVector3f::Dot(local, CVector3f::Right()) > 0.f ? pas::kSD_Right : pas::kSD_Left;
}

void CPatterned::RotateToPoint(const CVector3f&, float, float) {
  // TODO: Clamp the horizontal turn by dt, turn speed and the body controller's time scale.
}

void CPatterned::ApplyScreenShake(CStateManager&, const CVector3f&, TUniqueId) {
  // TODO: Resolve the shaker (or Footstep/Attach connection), copy its shake and set its position.
}
