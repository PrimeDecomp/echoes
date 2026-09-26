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
class CStateManager;
class CTransform4f;
class IWorld;

class CMapWorld {
public:
  enum EMapAreaList { kMAL_Loaded, kMAL_Loading, kMAL_Unloaded };

  class CMapWorldDrawParms {
  public:
    CMapWorldDrawParms(float alphaSurfVisited, float alphaOlVisited, float alphaSurfUnvisited,
                      float alphaOlUnvisited, float alpha, const CStateManager& mgr,
                      const CTransform4f& modelXf, const CTransform4f& viewXf, const IWorld& world,
                      const CMapWorldInfo& info, float outlineWidthScale, bool sortDoorSurfs,
                      float playerFlash, float hintFlash, float objectScale);

  private:
    float mAlphaSurfVisited;
    float mAlphaOlVisited;
    float mAlphaSurfUnvisited;
    float mAlphaOlUnvisited;
    float mAlpha;
    float mOutlineWidthScale;
    const CStateManager& mMgr;
    const CTransform4f& mModelXf;
    const CTransform4f& mViewXf;
    const IWorld& mWorld;
    const CMapWorldInfo& mMapWorldInfo;
    float mPlayerFlashIntensity;
    float mHintFlashIntensity;
    float mObjectScale;
    bool mSortDoorSurfs;
  };

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
  CVector3f ConstrainToWorldVolume(const CVector3f& point, const CVector3f& lookVec) const;
  void Draw(const CMapWorldDrawParms& parms, int curArea, int otherArea, float depth1,
            float depth2, bool inMapScreen) const;

private:
  rstl::vector< CMapAreaData > mAreas;
  rstl::reserved_vector< CMapAreaData*, 3 > mListHeads;
  mutable rstl::vector< bool > mTraversed;
  mutable CVector3f mWorldSpherePoint;
  mutable float mWorldSphereRadius;
  mutable float mWorldSphereHalfDepth;
};
NESTED_CHECK_SIZEOF(CMapWorld, CMapAreaData, 0x18)
NESTED_CHECK_SIZEOF(CMapWorld, CMapWorldDrawParms, 0x3c)
CHECK_SIZEOF(CMapWorld, 0x44)

#endif // _CMAPWORLD
