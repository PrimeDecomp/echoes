#ifndef _CENTITY
#define _CENTITY

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/CEntityInfo.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CStateManager;

class CEntity {
public:
  virtual ~CEntity();
  virtual CEntity* TypesMatch(EEntityType);
  virtual void PreThink(float dt, CStateManager& mgr);
  virtual void Think(float dt, CStateManager& mgr);
  virtual void AcceptScriptMsg(CStateManager& mgr, CScriptMsg&);
  virtual void SetActive(const bool active);

  CEntity(TUniqueId id, const CEntityInfo& info, bool active, const rstl::string& name);

  void SendScriptMsgs(EScriptObjectState state, CStateManager& mgr, EScriptObjectMessage msg);
  // static inline void SendScriptMsg(CStateManager& mgr, CEntity* to, TUniqueId sender,
  //                                  EScriptObjectMessage msg) {
  //   mgr.SendScriptMsg(to, sender, msg);
  // }
  TUniqueId GetUniqueId() const { return uid; }
  TEditorId GetEditorId() const { return editorId; }
  TAreaId GetAreaId() const;
  TAreaId GetCurrentAreaId() const { return areaId; }
  const bool GetActive() const { return active; }
  // bool IsScriptingBlocked() const { return scriptingBlocked; }

  // might be fake?
  rstl::vector< SConnection >& ConnectionList() { return conns; }
  const rstl::vector< SConnection >& GetConnectionList() const { return conns; }

  static rstl::vector< SConnection > NullConnectionList;

  TUniqueId SearchForSomething(CStateManager&, EScriptObjectState, EScriptObjectMessage);

private:
  TAreaId areaId;
  TUniqueId uid;
  TEditorId editorId;
  rstl::vector< SConnection > conns;
  bool active : 1;
  bool notInArea : 1;
  bool inGraveyard : 1;
  bool scriptingBlocked : 1;
};
CHECK_SIZEOF(CEntity, 0x24)

#endif // _CENTITY
