#ifndef _CMAPWORLDINFO
#define _CMAPWORLDINFO

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/bit_vector.hpp"
#include "rstl/pair.hpp"

class CBitStreamReader;
class CBitStreamWriter;
class CWorldSaveGameInfo;

class CMapWorldInfo {
public:
  CMapWorldInfo();
  CMapWorldInfo(CBitStreamReader& in, const CWorldSaveGameInfo& saveInfo, CAssetId worldId);
  void PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveInfo, CAssetId worldId) const;
  bool IsMapped(TAreaId areaId) const;
  bool IsAreaVisited(TAreaId areaId) const;
  bool IsAreaVisible(TAreaId areaId) const;
  bool IsWorldVisible(TAreaId areaId, bool inDarkWorld) const;
  bool IsAnythingSet();
  bool IsDoorVisited(TEditorId objectId) const;
  bool IsObjectUnmapped(TEditorId objectId) const; // Guessed name
  void SetIsMapped(TAreaId areaId, bool mapped);
  void SetAreaVisited(TAreaId areaId, bool visited);
  void SetDoorVisited(TEditorId objectId, bool visited);
  void SetObjectUnmapped(TEditorId objectId, bool unmapped); // Guessed name
  bool GetMapStationUsed() const { return mMapStationUsed; }

private:
  mutable rstl::bit_vector<> mVisitedAreas;
  mutable rstl::bit_vector<> mMappedAreas;
  mutable rstl::vector< rstl::pair< TEditorId, bool > > mVisitedDoors;
  mutable rstl::vector< rstl::pair< TEditorId, bool > > mUnmappedObjects; // Guessed name
  bool mMapStationUsed;
};
CHECK_SIZEOF(CMapWorldInfo, 0x4c)

#endif // _CMAPWORLDINFO
