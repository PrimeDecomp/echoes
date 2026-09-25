#ifndef _CSCRIPTWAYPOINT
#define _CSCRIPTWAYPOINT

#include "MetroidPrime/CActor.hpp"

// Guessed name: the fieldless Echoes waypoint base, corroborated by the Wii waypoint export.
class CScriptWaypoint : public CActor {
public:
  CScriptWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf);

  // CEntity
  ~CScriptWaypoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  // CScriptWaypoint
  virtual TUniqueId NextWaypoint(CStateManager& mgr) const;
  virtual TUniqueId FollowWaypoint(CStateManager& mgr) const;
};
CHECK_SIZEOF(CScriptWaypoint, 0x158)

#endif // _CSCRIPTWAYPOINT
