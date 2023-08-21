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

  CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, int active);

  void SendScriptMsgs(EScriptObjectState state, CStateManager& mgr, TUniqueId, EScriptObjectMessage msg);
  // static inline void SendScriptMsg(CStateManager& mgr, CEntity* to, TUniqueId sender,
  //                                  EScriptObjectMessage msg) {
  //   mgr.SendScriptMsg(to, sender, msg);
  // }
  TUniqueId GetUniqueId() const { return m_uid; }
  TEditorId GetEditorId() const { return m_editorId; }
  TAreaId GetAreaId() const;
  TAreaId GetCurrentAreaId() const { return m_areaId; }
  const bool GetActive() const { return m_active; }
  bool IsScriptingBlocked() const { return m_scriptingBlocked; }

  // might be fake?
  rstl::vector< SConnection >& ConnectionList() { return m_conns; }
  const rstl::vector< SConnection >& GetConnectionList() const { return m_conns; }

  static rstl::vector< SConnection > NullConnectionList;
  static CEntityInfo NullEntityInfo;

  TUniqueId SearchForSomething(CStateManager&, EScriptObjectState, EScriptObjectMessage);

private:
  TAreaId m_areaId;  // x4
  TUniqueId m_uid;  // x8
  TEditorId m_editorId;  // xc
  rstl::vector< SConnection > m_conns;  // x10
  uint m_active : 1; // x20
  uint m_notInArea : 1;
  uint m_inGraveyard : 4;
  uint m_scriptingBlocked : 1;
  uint m_entityUnknown : 1;
};
CHECK_SIZEOF(CEntity, 0x24)

#endif // _CENTITY
