#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

#include "TGameTypes.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"

class CStateManager {
public:
  typedef rstl::map< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  TUniqueId AllocateUniqueId();
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void ShowPausedHUDMemo(CAssetId strg, float time);

  void SendScriptMsg_fn_80037100(const CScriptMsg&);

  CEntity* ObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;
};

#endif // _CSTATEMANAGER
