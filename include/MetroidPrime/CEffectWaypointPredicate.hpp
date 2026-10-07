#ifndef _CEFFECTWAYPOINTPREDICATE
#define _CEFFECTWAYPOINTPREDICATE

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

// Guessed name. Effect and time-keyframe paths only accept script waypoints.
class CEffectWaypointPredicate : public CValidEntityPredicate {
public:
  ~CEffectWaypointPredicate() override {}

  bool IsValid(const CStateManager& mgr, TUniqueId id) const override {
    return TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id)) != nullptr;
  }
};

#endif // _CEFFECTWAYPOINTPREDICATE
