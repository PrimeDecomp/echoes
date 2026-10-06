#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

CPathFindNavigation::CPathFindNavigation()
: mSegmentStart(CVector3f::Zero())
, mPathTarget(CVector3f::Zero())
, mDestinationPosition(CVector3f::Zero())
, mFaceTarget(kInvalidUniqueId)
, mInPosition(false)
, mUseLocomotionFacing(false) {}

void CPathFindNavigation::PathFind(CStateManager& mgr, EStateMsg msg, float dt, CPatterned& actor) {
  if (actor.GetSearchPath() == nullptr) {
    return;
  }

  switch (msg) {
  case kStateMsg_Activate:
    StartPathFind(mgr, actor);
    break;
  case kStateMsg_Update:
    if (!actor.GetSearchPath()->IsOver()) {
      const CVector3f position = actor.GetTranslation() + 0.3f * CVector3f::Up();
      ApproachDest(mgr, actor);

      const float scale = actor.GetModelData()->GetScale().GetY();
      CVector3f point = position + scale * actor.GetTransform().GetForward();
      actor.GetSearchPath()->GetSplinePointWithLookahead(point, position, 3.f * scale);
      mPathTarget = point;
      if (actor.GetSearchPath()->SegmentOver(position) || mInPosition) {
        actor.GetSearchPath()->Advance();
        mInPosition = false;
        mSegmentStart = actor.GetTranslation();
      }
    }
    break;
  default:
    break;
  }
}

void CPathFindNavigation::SetDestination(const CVector3f& position) {
  mDestinationPosition = position;
  mPathTarget = mDestinationPosition;
}

void CPathFindNavigation::StartPathFind(CStateManager& mgr, CPatterned& actor) {
  if (actor.GetSearchPath()->Search(actor.GetTranslation(), mPathTarget) ==
      CPathFindSearch::kR_Success) {
    mSegmentStart = actor.GetTranslation();
    mPathTarget = actor.GetSearchPath()->GetPoint();
    mInPosition = false;
    ApproachDest(mgr, actor);
  }
}

void CPathFindNavigation::ApproachDest(CStateManager& mgr, CPatterned& actor) {
  CVector3f move = mPathTarget - actor.GetTranslation();
  CVector3f face = CVector3f::Zero();
  if (mFaceTarget != kInvalidUniqueId) {
    if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mFaceTarget))) {
      face = target->GetTranslation() - actor.GetTranslation();
    }
  }
  if (!actor.GetVerticalMovement()) {
    move.SetZ(0.f);
    face.SetZ(0.f);
  }

  const CVector3f path = mPathTarget - mSegmentStart;
  if (CVector3f::Dot(path, move) <= 0.f) {
    mInPosition = true;
  } else if (CVector3f::Dot(move, move) < 9.f) {
    move = path;
  }

  if (!mInPosition) {
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    if (mUseLocomotionFacing || actor.BodyController()->GetBodyType() == kBT_AiMovedFlyer ||
        actor.BodyController()->GetBodyType() == kBT_4WayBlended) {
      actor.BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    } else if (mFaceTarget == kInvalidUniqueId ||
               !actor.BodyController()->HasBodyState(pas::kAS_Step)) {
      actor.BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else {
      const pas::EStepDirection step = actor.FindBestStepDirection(move);
      if (step != pas::kSD_Forward) {
        actor.BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(step, pas::kStep_Normal));
      } else {
        actor.BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
      actor.BodyController()->CommandMgr().SetTargetVector(face);
    }
  } else {
    actor.BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(actor.GetTransform().GetForward(), CVector3f::Zero(), 1.f));
  }
}
