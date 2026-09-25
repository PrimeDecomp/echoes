#ifndef _CSCRIPTCAMERAWAYPOINT
#define _CSCRIPTCAMERAWAYPOINT

#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

class CScriptCameraWaypoint : public CScriptWaypoint {
public:
  CScriptCameraWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                        const CTransform4f& xf);

  // CEntity
  ~CScriptCameraWaypoint() override;
  CEntity* TypesMatch(int typeId) const override;

  // CScriptWaypoint
  TUniqueId NextWaypoint(CStateManager& mgr) const override;
};
CHECK_SIZEOF(CScriptCameraWaypoint, 0x158)

#endif // _CSCRIPTCAMERAWAYPOINT
