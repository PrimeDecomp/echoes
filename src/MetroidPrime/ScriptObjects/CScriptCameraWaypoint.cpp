#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

// Guessed name
class CValidCameraWaypointPredicate : public CValidEntityPredicate {
public:
  // CValidEntityPredicate
  ~CValidCameraWaypointPredicate() override;
  bool IsValid(const CStateManager& mgr, TUniqueId id) const override;
};

bool CValidCameraWaypointPredicate::IsValid(const CStateManager& mgr, TUniqueId id) const {
  return TCastToConstPtr< CScriptCameraWaypoint >(mgr.GetObjectById(id)) != nullptr;
}

CScriptCameraWaypoint::CScriptCameraWaypoint(TUniqueId uid, const rstl::string& name,
                                             const CEntityInfo& info, const CTransform4f& xf)
: CScriptWaypoint(uid, name, info, xf) {}

CScriptCameraWaypoint::~CScriptCameraWaypoint() {}

CValidCameraWaypointPredicate::~CValidCameraWaypointPredicate() {}

TUniqueId CScriptCameraWaypoint::NextWaypoint(CStateManager& mgr) const {
  return CheckConnectedObject_if(mgr, static_cast< EScriptObjectState >('ARRV'),
                                 static_cast< EScriptObjectMessage >('NEXT'),
                                 CValidCameraWaypointPredicate());
}
