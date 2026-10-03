#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"

#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

CPathFindNavigation::CPathFindNavigation()
: mSegmentStart(CVector3f::Zero())
, mPathTarget(CVector3f::Zero())
, mDestinationPosition(CVector3f::Zero())
, mFaceTarget(kInvalidUniqueId)
, mInPosition(false)
, x26_25_(false) {}

void CPathFindNavigation::PathFind(CStateManager& mgr, EStateMsg msg, float dt,
                                   CPatterned& actor) {
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
  // TODO: recover vertical-movement access, destination-facing and body-command steering.
}
