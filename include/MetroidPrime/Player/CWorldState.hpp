#ifndef _CWORLDSTATE
#define _CWORLDSTATE

#include "MetroidPrime/CMapWorldInfo.hpp"
#include "rstl/rc_ptr.hpp"

class CRelayTracker;
class CWorldLayerState;
class CBitStreamReader;
class CBitStreamWriter;

class CWorldState {
public:
  explicit CWorldState(CAssetId worldId);
  CWorldState(CBitStreamReader& in, CAssetId worldId, const CWorldSaveGameInfo& saveWorld);
  ~CWorldState();

  void PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveWorld) const;
  CAssetId GetWorldAssetId() const;
  TAreaId GetCurrentArea() const;
  void SetAreaId(TAreaId areaId);
  CAssetId GetDesiredAreaAssetId() const;
  void SetDesiredAreaAssetId(CAssetId areaId);
  rstl::rc_ptr< CMapWorldInfo > GetMapWorldInfo() const;
  rstl::ncrc_ptr< CMapWorldInfo >& MapWorldInfo();
  rstl::ncrc_ptr< CWorldLayerState >& GetLayerState();
  rstl::ncrc_ptr< CRelayTracker >& RelayTracker(); // Guessed name

private:
  CAssetId mWorldId;
  TAreaId mAreaId;
  rstl::ncrc_ptr< CRelayTracker > mRelayTracker;
  rstl::ncrc_ptr< CMapWorldInfo > mMapWorldInfo;
  CAssetId mDesiredAreaAssetId;
  rstl::ncrc_ptr< CWorldLayerState > mLayerState;
};
CHECK_SIZEOF(CWorldState, 0x24)

#endif // _CWORLDSTATE
