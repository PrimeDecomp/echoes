#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

// Guessed name; this predicate has no instance fields beyond its inherited vptr.
class CValidPassiveKeyframePredicate : public CValidEntityPredicate {
public:
  // CValidEntityPredicate
  ~CValidPassiveKeyframePredicate() override {}
  bool IsValid(const CStateManager& mgr, TUniqueId id) const override;
};
CHECK_SIZEOF(CValidPassiveKeyframePredicate, 0x4)

bool CValidPassiveKeyframePredicate::IsValid(const CStateManager& mgr, TUniqueId id) const {
  const CScriptActorKeyframe* keyframe =
      TCastToConstPtr< CScriptActorKeyframe >(mgr.GetObjectById(id));
  return keyframe != nullptr && keyframe->GetActive() && keyframe->IsPassive();
}

CWaypointNavigation::CWaypointNavigation()
: mMoveSpeed(1.f)
, mPauseRemainingTime(0.f)
, mDestination(kInvalidUniqueId)
, mLastDestination(kInvalidUniqueId)
, mPatrolState(kPS_Invalid)
, mSegmentStart(CVector3f::Zero())
, mDestinationPosition(CVector3f::Zero())
, mFaceVector(CVector3f::Zero())
, mWobbleSteering(0.f)
, mInPosition(false)
, mHorizontalMovement(false) {}

void CWaypointNavigation::Update(float dt) {
  float t = mPauseRemainingTime;
  if (t > 0.f) {
    t -= dt;
  }
  mPauseRemainingTime = t;
}

void CWaypointNavigation::Patrol(CStateManager& mgr, EStateMsg msg, float, CPatterned& actor) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mLastDestination == kInvalidUniqueId) {
      mDestination = actor.GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
      mMoveSpeed = 1.f;
      if (mDestination != kInvalidUniqueId) {
        if (const CScriptAIWaypoint* waypoint =
                TCastToConstPtr< CScriptAIWaypoint >(mgr.GetObjectById(mDestination))) {
          mMoveSpeed = waypoint->GetSpeed();
        }
      }
    } else {
      mDestination = mLastDestination;
    }
    mSegmentStart = actor.GetTranslation();
    mInPosition = false;
    mPatrolState = kPS_Moving;
    mPauseRemainingTime = 0.f;
    break;
  case kStateMsg_Update:
    switch (mPatrolState) {
    case kPS_Moving:
      if (mInPosition && mDestination != kInvalidUniqueId) {
        if (const CScriptAIWaypoint* waypoint =
                TCastToConstPtr< CScriptAIWaypoint >(mgr.GetObjectById(mDestination))) {
          if (waypoint->GetPause() > 0.f) {
            mPauseRemainingTime = waypoint->GetPause();
            mPatrolState = kPS_Paused;
          }
        }
      }
      if (mDestination == kInvalidUniqueId) {
        mPatrolState = kPS_Done;
      }
      UpdateDest(mgr, actor);
      ApproachDest(mgr, actor);
      break;
    case kPS_Paused:
      if (mPauseRemainingTime <= 0.f) {
        mPatrolState = kPS_Moving;
      }
      break;
    case kPS_Done:
      if (mDestination != kInvalidUniqueId) {
        mPatrolState = kPS_Moving;
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mLastDestination = mDestination;
    mPatrolState = kPS_Invalid;
    break;
  }
}

void CWaypointNavigation::ApproachDest(CStateManager&, CPatterned& actor) {
  CVector3f move = mDestinationPosition - actor.GetTranslation();
  if (!actor.GetVerticalMovement() || mHorizontalMovement) {
    move.SetZ(0.f);
  }

  const CVector3f segment = mDestinationPosition - mSegmentStart;
  if (CVector3f::Dot(segment, move) <= 0.f) {
    mInPosition = true;
  } else if (move.MagSquared() < 9.f) {
    move = segment;
  }

  if (!mInPosition) {
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    CVector3f velocity = mMoveSpeed * move;
    ApplyWobbleSteering(velocity);
    actor.BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(velocity, mFaceVector, 1.f));
  } else {
    CVector3f velocity = actor.GetBodyController()->GetCommandMgr().GetPreviousMoveVector();
    ApplyWobbleSteering(velocity);
    actor.BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(velocity, mFaceVector, 1.f));
  }
}

void CWaypointNavigation::UpdateDest(CStateManager& mgr, CPatterned& actor) {
  if (mInPosition && mDestination != kInvalidUniqueId) {
    mLastDestination = mDestination;
    if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestination))) {
      UpdateActorKeyframe(mgr, actor);
      mDestination = waypoint->NextWaypoint(mgr);
      if (mDestination != kInvalidUniqueId) {
        mSegmentStart = actor.GetTranslation();
        mInPosition = false;
        if (const CScriptWaypoint* next =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestination))) {
          if (const CScriptAIWaypoint* aiWaypoint =
                  TCastToConstPtr< CScriptAIWaypoint >(waypoint)) {
            mMoveSpeed = aiWaypoint->GetSpeed();
            if (aiWaypoint->GetFlags() & 2) {
              CBodyStateCmdMgr& cmdMgr = actor.BodyController()->CommandMgr();
              cmdMgr.DeliverCmd(CBCJumpCmd(next->GetTranslation(), pas::kJT_Normal,
                                           pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
            } else if (aiWaypoint->GetFlags() & 4) {
              const TUniqueId nextId = next->NextWaypoint(mgr);
              if (nextId != kInvalidUniqueId) {
                if (const CScriptWaypoint* end =
                        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(nextId))) {
                  CBodyStateCmdMgr& cmdMgr = actor.BodyController()->CommandMgr();
                  cmdMgr.DeliverCmd(CBCJumpCmd(next->GetTranslation(), end->GetTranslation()));
                }
              }
            }
          }
        }
      }
      mgr.SendScriptMsg(waypoint, actor.GetUniqueId(), static_cast< EScriptObjectMessage >('ARRV'),
                        actor.GetUniqueId());
    }
  }
  if (mDestination != kInvalidUniqueId) {
    if (const CActor* destination = TCastToConstPtr< CActor >(mgr.GetObjectById(mDestination))) {
      mDestinationPosition = destination->GetTranslation();
    }
  }
}

void CWaypointNavigation::UpdateActorKeyframe(CStateManager& mgr, CPatterned& actor) {
  if (const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestination))) {
    const rstl::vector< TUniqueId > ids = waypoint->FindConnectedObjects_if(
        mgr, kSS_Arrived, kSM_Action, CValidPassiveKeyframePredicate());
    for (int i = 0; i < ids.size(); ++i) {
      CScriptActorKeyframe* keyframe = static_cast< CScriptActorKeyframe* >(mgr.ObjectById(ids[i]));
      keyframe->UpdateEntity(actor.GetUniqueId(), mgr);
    }
  }
}

void CWaypointNavigation::ApplyWobbleSteering(CVector3f& movement) const {
  if (mWobbleSteering == 0.f) {
    return;
  }

  CVector3f rotated;
  if (mWobbleSteering < 0.f) {
    rotated = CVector3f(movement.GetY(), -movement.GetX(), movement.GetZ());
  } else {
    rotated = CVector3f(-movement.GetY(), movement.GetX(), movement.GetZ());
  }
  movement = CVector3f::Lerp(movement, rotated, CMath::AbsF(mWobbleSteering));
}

void CWaypointNavigation::ConfigureWobbleSteering(bool clockwise, float strength) {
  if (clockwise == true) {
    mWobbleSteering = strength;
  } else {
    mWobbleSteering = -1.f * strength;
  }
}
