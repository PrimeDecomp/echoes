#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"

#include "Collision/CRayCastResult.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/algorithm.hpp"

// Guessed name. This predicate has no instance fields beyond its inherited vptr.
class CValidActiveEntityPredicate : public CValidEntityPredicate {
public:
  // CValidEntityPredicate
  ~CValidActiveEntityPredicate() override {}
  bool IsValid(const CStateManager& mgr, TUniqueId id) const override;
};
CHECK_SIZEOF(CValidActiveEntityPredicate, 4)

bool CValidActiveEntityPredicate::IsValid(const CStateManager& mgr, TUniqueId id) const {
  const CEntity* entity = mgr.GetObjectById(id);
  return entity != nullptr && entity->GetActive();
}

void ScriptCameraSpline::CollectWaypoints(const CEntity& entity, EScriptObjectState state,
                                          EScriptObjectMessage message,
                                          rstl::vector< CVector3f >& positions,
                                          rstl::vector< CQuaternion >& orientations,
                                          CStateManager& mgr) {
  if (state == kSS_InvalidState) {
    return;
  }

  const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(
      entity.FindConnectedObject_if(mgr, state, message, CValidActiveEntityPredicate())));
  rstl::vector< TUniqueId > visited;
  uint count = 0;

  visited.clear();
  visited.reserve(4);
  if (waypoint != nullptr) {
    while (waypoint != nullptr) {
      if (rstl::find(visited.begin(), visited.end(), waypoint->GetUniqueId()) != visited.end()) {
        break;
      }

      if (visited.size() == visited.capacity()) {
        visited.reserve(visited.size() * 2);
      }
      visited.push_back_unsafe(waypoint->GetUniqueId());
      ++count;

      waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(waypoint->NextWaypoint(mgr)));
    }

    positions.clear();
    positions.reserve(count);
    orientations.clear();
    orientations.reserve(count);
    for (uint i = 0; i < count; ++i) {
      const CScriptWaypoint* point = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(visited[i]));
      positions.push_back_unsafe(point->GetTranslation());
      orientations.push_back_unsafe(CQuaternion::FromMatrix(point->GetTransform()));
    }
  }
}

void ScriptCameraSpline::Initialise(const CEntity& entity, EScriptObjectState positionState,
                                    EScriptObjectMessage positionMessage,
                                    EScriptObjectState targetState,
                                    EScriptObjectMessage targetMessage, CStateManager& mgr,
                                    CSpline& spline) {
  rstl::vector< CVector3f > positions;
  rstl::vector< CQuaternion > orientations;
  rstl::vector< CVector3f > targets;
  rstl::vector< CQuaternion > targetOrientations;
  CollectWaypoints(entity, positionState, positionMessage, positions, orientations, mgr);
  CollectWaypoints(entity, targetState, targetMessage, targets, targetOrientations, mgr);
  spline.Initialise(positions, orientations, targets);
}

float ScriptCameraSpline::ClampLength(const CMotionSpline& spline, const CVector3f& position,
                                      bool checkObstructions, const CMaterialFilter& filter,
                                      const CStateManager& mgr) {
  if (spline.GetKnotCount() == 0) {
    return 0.f;
  }
  if (spline.IsClosedLoop()) {
    return 0.f;
  }

  const CVector3f first = spline.GetKnot(0);
  const CVector3f last = spline.GetKnot(spline.GetKnotCount() - 1);
  const CVector3f firstDelta = position - first;
  const CVector3f lastDelta = position - last;
  const float firstDistance = firstDelta.Magnitude();
  const float lastDistance = lastDelta.Magnitude();
  if (!firstDelta.IsMagnitudeSafe()) {
    return 0.f;
  }
  if (!lastDelta.IsMagnitudeSafe()) {
    return spline.GetLength();
  }

  if (checkObstructions) {
    const CRayCastResult firstHit =
        mgr.RayStaticIntersection(first, firstDelta.AsNormalized(), firstDelta.Magnitude(), filter);
    const CRayCastResult lastHit =
        mgr.RayStaticIntersection(last, lastDelta.AsNormalized(), lastDelta.Magnitude(), filter);
    if (firstHit.IsValid()) {
      return spline.GetLength();
    }
    if (lastHit.IsValid()) {
      return 0.f;
    }
  }
  return firstDistance < lastDistance ? 0.f : spline.GetLength();
}
