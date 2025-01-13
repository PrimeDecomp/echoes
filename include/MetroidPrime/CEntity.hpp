#ifndef _CENTITY
#define _CENTITY

#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CStateManager;

class CEntity {
public:
  virtual ~CEntity();
  virtual CEntity* TypesMatch(int typeId) const;
  virtual void PreThink(float dt, CStateManager& mgr);
  virtual void Think(float dt, CStateManager& mgr);
  virtual void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&);
  virtual void SendActive(CStateManager& mgr, bool active);
  virtual void SetActive(const bool active);

  CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, uint inGrave);

  void SendScriptMsgs(EScriptObjectState state, CStateManager& mgr, TUniqueId uid,
                      EScriptObjectMessage msg);
  // static inline void SendScriptMsg(CStateManager& mgr, CEntity* to, TUniqueId sender,
  //                                  EScriptObjectMessage msg) {
  //   mgr.SendScriptMsg(to, sender, msg);
  // }
  TUniqueId GetUniqueId() const { return m_uid; }
  TEditorId GetEditorId() const { return m_editorId; }
  TAreaId GetAreaIdForPersistence() const;
  TAreaId GetCurrentAreaId() const { return m_areaId; }
  const bool GetActive() const { return m_active; }
  bool IsScriptingBlocked() const { return m_scriptingBlocked; }

  // might be fake?
  rstl::vector< SConnection >& ConnectionList() { return m_conns; }
  const rstl::vector< SConnection >& GetConnectionList() const { return m_conns; }

  static rstl::vector< SConnection > NullConnectionList;
  static CEntityInfo NullEntityInfo;

  TUniqueId FindConnectedObject(const CStateManager&, EScriptObjectState,
                                EScriptObjectMessage) const;
  TUniqueId FindConnectedObject_if(const CStateManager&, EScriptObjectState, EScriptObjectMessage,
                                   const CValidEntityPredicate&) const;
  void /* TODO */ FindConnectedObjects(const CStateManager&, EScriptObjectState,
                                       EScriptObjectMessage) const;
  void /* TODO */ FindConnectedObjects_if(const CStateManager&, EScriptObjectState,
                                          EScriptObjectMessage, const CValidEntityPredicate&) const;
  void /* TODO */ CheckConnectedObject(const CStateManager&, EScriptObjectState,
                                       EScriptObjectMessage) const;
  void /* TODO */ CheckConnectedObject_if(const CStateManager&, EScriptObjectState,
                                          EScriptObjectMessage, const CValidEntityPredicate&) const;

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
