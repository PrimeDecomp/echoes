#ifndef _CMAPWORLD
#define _CMAPWORLD

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CMapArea;
class CMapWorldInfo;
class IWorld;

class CMapWorld {
public:
  enum EMapAreaList { kMAL_Loaded, kMAL_Loading, kMAL_Unloaded };

  class CMapAreaData {
  public:
    CMapAreaData(CAssetId areaRes, EMapAreaList list, CMapAreaData* next);
    CMapArea* GetMapArea() const;
    bool IsLoaded() const;
    void Lock();
    void Unlock();

  private:
    CAssetId mAreaRes;
    mutable TCachedToken< CMapArea > mArea;
    mutable EMapAreaList mList;
    mutable CMapAreaData* mNext;
  };

  explicit CMapWorld(CInputStream& in);
  ~CMapWorld();

  uint GetNumAreas() const { return mAreas.size(); }
  CMapArea* GetMapArea(int areaId) const;
  void SetWhichMapAreasLoaded(const IWorld& world, int start, int count);
  int GetCurrentMapAreaDepth(const IWorld& world, int areaId) const;
  rstl::vector< int > GetVisibleAreas(const IWorld& world, const CMapWorldInfo& info) const;
  bool IsMapAreaValid(const IWorld& world, int areaId, bool checkLoad) const;
  bool IsMapAreasStreaming() const;
  void RecalculateWorldSphere(const CMapWorldInfo& info, const IWorld& world) const;

private:
  rstl::vector< CMapAreaData > mAreas;
  rstl::reserved_vector< CMapAreaData*, 3 > mListHeads;
  mutable rstl::vector< bool > mTraversed;
  mutable CVector3f mWorldSpherePoint;
  mutable float mWorldSphereRadius;
  mutable float mWorldSphereHalfDepth;
};
NESTED_CHECK_SIZEOF(CMapWorld, CMapAreaData, 0x18)
CHECK_SIZEOF(CMapWorld, 0x44)

#endif // _CMAPWORLD
