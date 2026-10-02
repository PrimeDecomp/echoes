#ifndef _CSCRIPTMAILBOX
#define _CSCRIPTMAILBOX

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CBitStreamReader;
class CBitStreamWriter;
class CStateManager;
class CWorldSaveGameInfo;

class CScriptMailbox {
public:
  CScriptMailbox();
  CScriptMailbox(CBitStreamReader& in, const CWorldSaveGameInfo& saveWorld);
  ~CScriptMailbox();
  void PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveWorld) const;
  void SendMsgs(const TAreaId& areaId, CStateManager& mgr);
  void AddMsg(const TEditorId& id);
  void RemoveMsg(const TEditorId& id);

private:
  rstl::reserved_vector< TEditorId, 512 > mRelays;
};
CHECK_SIZEOF(CScriptMailbox, 0x804)

#endif // _CSCRIPTMAILBOX
