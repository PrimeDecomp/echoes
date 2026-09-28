#ifndef _CMAPWORLD
#define _CMAPWORLD

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
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

  class CMapAreaBFSInfo {
  public:
    CMapAreaBFSInfo(int areaIdx, int depth, float surfDepth, float outlineDepth);
    int GetAreaIndex() const { return mAreaIdx; }
    int GetDepth() const { return mDepth; }
    float GetSurfaceDrawDepth() const { return mSurfDrawDepth; }
    float GetOutlineDrawDepth() const { return mOutlineDrawDepth; }

  private:
    int mAreaIdx;
    int mDepth;
    float mSurfDrawDepth;
    float mOutlineDrawDepth;
  };

  class CMapObjectSortInfo {
  public:
    enum EObjectCode {
      kOC_Invalid = -1,
      kOC_Object = 1 << 16,
      kOC_DoorSurface = 2 << 16,
      kOC_Door = 3 << 16,
      kOC_Surface = 4 << 16,
      kOC_Area = 5 << 16 // Guessed name: prepared whole-area rendering.
    };

    CMapObjectSortInfo(float zDist, int areaIdx, EObjectCode type, int idx, CColor surfColor,
                       CColor outlineColor);
    float GetZDistance() const { return mZDist; }
    int GetAreaIndex() const { return mAreaIdx; }
    EObjectCode GetObjectCode() const { return EObjectCode(mTypeAndIdx & 0xffff0000); }
    int GetLocalObjectIndex() const { return mTypeAndIdx & 0xffff; }
    const CColor& GetSurfaceColor() const { return mSurfColor; }
    const CColor& GetOutlineColor() const { return mOutlineColor; }

  private:
    float mZDist;
    int mAreaIdx;
    int mTypeAndIdx;
    CColor mSurfColor;
    CColor mOutlineColor;
  };

  class CMapWorldDrawParms {
  public:
    CMapWorldDrawParms(float alphaSurfVisited, float alphaOlVisited, float alphaSurfUnvisited,
                       float alphaOlUnvisited, float alpha, const CStateManager& mgr,
                       const CTransform4f& modelXf, const CTransform4f& viewXf, const IWorld& world,
                       const CMapWorldInfo& info, float outlineWidthScale, bool sortDoorSurfs,
                       float playerFlash, float hintFlash, float objectScale);

    const IWorld& GetWorld() const { return mWorld; }
    const CMapWorldInfo& GetMapWorldInfo() const { return mMapWorldInfo; }
    const CStateManager& GetStateManager() const { return mMgr; }
    const CTransform4f& GetPlaneProjectionTransform() const { return mModelXf; }
    const CTransform4f& GetCameraTransform() const { return mViewXf; }
    float GetAlphaSurfaceVisited() const { return mAlphaSurfVisited; }
    float GetAlphaOutlineVisited() const { return mAlphaOlVisited; }
    float GetAlphaSurfaceUnvisited() const { return mAlphaSurfUnvisited; }
    float GetAlphaOutlineUnvisited() const { return mAlphaOlUnvisited; }
    float GetAlpha() const { return mAlpha; }
    float GetOutlineWidthScale() const { return mOutlineWidthScale; }
    float GetPlayerAreaFlashIntensity() const { return mPlayerFlashIntensity; }
    float GetHintAreaFlashIntensity() const { return mHintFlashIntensity; }
    float GetObjectScale() const { return mObjectScale; }
    bool GetIsSortDoorSurfaces() const { return mSortDoorSurfs; }

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
    CMapAreaData* NextMapAreaData() const { return mNext; }
    EMapAreaList GetContainingList() const { return mList; }
    void SetContainingList(EMapAreaList list) const { mList = list; }
    void SetNextMapArea(CMapAreaData* next) const { mNext = next; }

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
  void Draw(const CMapWorldDrawParms& parms, int curArea, int otherArea, float depth1, float depth2,
            float darkWorldBlend, bool inMapScreen) const;

private:
  bool IsMapAreaInBFSInfoVector(const CMapAreaData* area,
                                const rstl::vector< CMapAreaBFSInfo >& vec) const;
  void MoveMapAreaToList(CMapAreaData* data, EMapAreaList list);
  void DoBFS(const IWorld& world, int startArea, int areaCount, float surfDepth, float outlineDepth,
             bool checkLoad, rstl::vector< CMapAreaBFSInfo >& bfsInfos) const;
  void DrawAreas(const CMapWorldDrawParms& parms, int selArea,
                 const rstl::vector< CMapAreaBFSInfo >& bfsInfos, float darkWorldBlend,
                 bool inMapScreen) const;
  void ClearTraversedFlags() const;

  rstl::vector< CMapAreaData > mAreas;
  rstl::reserved_vector< CMapAreaData*, 3 > mListHeads;
  mutable rstl::vector< bool > mTraversed;
  mutable CVector3f mWorldSpherePoint;
  mutable float mWorldSphereRadius;
  mutable float mWorldSphereHalfDepth;
};
NESTED_CHECK_SIZEOF(CMapWorld, CMapAreaBFSInfo, 0x10)
NESTED_CHECK_SIZEOF(CMapWorld, CMapObjectSortInfo, 0x14)
NESTED_CHECK_SIZEOF(CMapWorld, CMapAreaData, 0x18)
NESTED_CHECK_SIZEOF(CMapWorld, CMapWorldDrawParms, 0x3c)
CHECK_SIZEOF(CMapWorld, 0x44)

#endif // _CMAPWORLD
