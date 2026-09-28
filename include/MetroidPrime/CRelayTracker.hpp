#ifndef _CRELAYTRACKER
#define _CRELAYTRACKER

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CBitStreamReader;
class CBitStreamWriter;
class CWorldSaveGameInfo;

// Inherited name; the target layout corresponds to Prime's CScriptMailbox.
class CRelayTracker {
public:
  CRelayTracker();
  CRelayTracker(CBitStreamReader& in, const CWorldSaveGameInfo& saveWorld);
  ~CRelayTracker();
  void PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveWorld) const;

private:
  rstl::reserved_vector< TEditorId, 512 > mRelays;
};
CHECK_SIZEOF(CRelayTracker, 0x804)

#endif // _CRELAYTRACKER
