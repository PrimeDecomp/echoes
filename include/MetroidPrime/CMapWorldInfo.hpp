#ifndef _CMAPWORLDINFO
#define _CMAPWORLDINFO

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/bit_vector.hpp"
#include "rstl/pair.hpp"

class CMapWorldInfo {
public:
  CMapWorldInfo();
  bool IsMapped(TAreaId areaId) const;
  bool IsAreaVisited(TAreaId areaId) const;
  bool IsAreaVisible(TAreaId areaId) const;
  bool IsWorldVisible(TAreaId areaId, bool inDarkWorld) const;
  bool IsAnythingSet();
  bool IsDoorVisited(TEditorId objectId) const;
  bool IsObjectUnmapped(TEditorId objectId) const; // Guessed name
  void SetDoorVisited(TEditorId objectId, bool visited);
  bool GetMapStationUsed() const { return mMapStationUsed; }

private:
  mutable rstl::bit_vector<> mVisitedAreas;
  mutable rstl::bit_vector<> mMappedAreas;
  rstl::vector< rstl::pair< TEditorId, bool > > mVisitedDoors;
  rstl::vector< rstl::pair< TEditorId, bool > > mUnmappedObjects; // Guessed name
  bool mMapStationUsed;
};
CHECK_SIZEOF(CMapWorldInfo, 0x4c)

#endif // _CMAPWORLDINFO
