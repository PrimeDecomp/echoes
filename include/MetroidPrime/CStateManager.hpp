#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

#include "TGameTypes.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"

class CWorld;
class CEnvFxManager;

class CStateManager {
public:
  typedef rstl::map< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  TUniqueId AllocateUniqueId();
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void SetIsDarkWorld(bool);

  void SendScriptMsg_fn_80037100(const CScriptMsg&);

  CEntity* ObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;

  CWorld* World() { return m_world; }
  const CWorld* GetWorld() const { return m_world; }
  CEnvFxManager* EnvFxManager() { return m_envFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return m_envFxManager; }

private:
  char pad1[0x1604];
  CWorld* m_world; // 0x1604
  char pad2[40];
  CEnvFxManager* m_envFxManager; // 0x1630
};
// CHECK_OFFSETOF(CStateManager, m_world, 0x1604)
// CHECK_OFFSETOF(CStateManager, m_envFxManager, 0x1630)

#endif // _CSTATEMANAGER
