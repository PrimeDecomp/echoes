#include "MetroidPrime/Enemies/CIngSpotPathFindNavigation.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"

CIngSpotPathFindNavigation::CIngSpotPathFindNavigation()
: mPreviousPosition(CVector3f::Zero()), mCurrentWaypoint(-1) {}

void CIngSpotPathFindNavigation::PathFind(CStateManager& mgr, EStateMsg msg, float dt,
                                          CPatterned& actor) {
  switch (msg) {
  case kStateMsg_Activate:
    mPreviousPosition = actor.GetTranslation();
    mCurrentWaypoint = 0;
    break;
  case kStateMsg_Update: {
    const CVector3f position = actor.GetTranslation();
    while (!IsPathOver(actor)) {
      if (const CPathFindPointSearch* path = actor.GetPointSearchPath()) {
        const CVector3f waypoint = path->GetWaypoints()[mCurrentWaypoint];
        const CVector3f toWaypoint = waypoint - position;
        const CVector3f segment = waypoint - mPreviousPosition;
        const bool hasNext = mCurrentWaypoint + 1 < path->GetWaypoints().size();
        const CVector3f& toNext =
            hasNext ? path->GetWaypoints()[mCurrentWaypoint + 1] - position : CVector3f::Zero();

        if (CVector3f::Dot(toWaypoint, segment) > 0.f &&
            (!hasNext || CVector3f::Dot(toWaypoint, toNext) > 0.f) &&
            toWaypoint.IsMagnitudeSafe()) {
          CVector3f direction = toWaypoint;
          if (mCurrentWaypoint > 0) {
            static const float skSampleTimes[] = {0.25f, 0.5f, 0.75f};
            for (uint i = 0; i < 3; ++i) {
              const CVector3f toSample =
                  path->GetSplinePoint(mCurrentWaypoint - 1, skSampleTimes[i]) - position;
              if (CVector3f::Dot(toSample, segment) > 0.f && toSample.IsMagnitudeSafe()) {
                direction = toSample;
                break;
              }
            }
          }
          actor.BodyController()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(direction.AsNormalized(), CVector3f::Zero(), 1.f));
          return;
        }

        ++mCurrentWaypoint;
        mPreviousPosition = waypoint;
      }
    }
    break;
  }
  default:
    break;
  }
}

bool CIngSpotPathFindNavigation::HasPath(const CPatterned& actor) const {
  const CPathFindPointSearch* path = actor.GetPointSearchPath();
  if (path != nullptr) {
    return path->GetWaypoints().size() != 0;
  }
  return false;
}

bool CIngSpotPathFindNavigation::IsPathOver(const CPatterned& actor) const {
  const CPathFindPointSearch* path = actor.GetPointSearchPath();
  if (path != nullptr) {
    return HasPath(actor) && mCurrentWaypoint >= path->GetWaypoints().size();
  }
  return false;
}
