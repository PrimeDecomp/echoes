#ifndef _CMAPUNIVERSE
#define _CMAPUNIVERSE

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CFactoryFnReturn;
class CVParamTransfer;
class CMapArea;
class CStateManager;

class CMapUniverse {
public:
  class CMapObjectSortInfo {
  public:
    CMapObjectSortInfo(float zDistance, int worldIndex, int areaIndex, int objectIndex,
                       CColor surfaceColor, CColor outlineColor);

    float GetZDistance() const { return mZDistance; }
    int GetWorldIndex() const { return mWorldIndex; }
    int GetAreaIndex() const { return mAreaIndex; }
    int GetObjectIndex() const { return mObjectIndex; }
    CColor GetSurfaceColor() const { return mSurfaceColor; }
    CColor GetOutlineColor() const { return mOutlineColor; }

  private:
    float mZDistance;
    int mWorldIndex;
    int mAreaIndex;
    int mObjectIndex;
    CColor mSurfaceColor;
    CColor mOutlineColor;
  };

  class CMapUniverseDrawParms {
  public:
    CMapUniverseDrawParms(float alpha, int worldIdx, CAssetId worldId, int closestHex,
                          float flashPulse, const CStateManager& mgr, const CTransform4f& model,
                          const CTransform4f& view, bool teleportMode);

    float GetAlpha() const { return mAlpha; }
    int GetFocusWorldIndex() const { return mFocusWorldIndex; }
    CAssetId GetFocusWorldRes() const { return mFocusWorldRes; }
    int GetFocusAreaIndex() const { return mFocusAreaIndex; }
    float GetFlashPulse() const { return mFlashPulse; }
    const CStateManager& GetStateManager() const { return mStateManager; }
    const CTransform4f& GetPaneProjectionTransform() const { return mPaneProjectionTransform; }
    const CTransform4f& GetCameraTransform() const { return mCameraTransform; }
    bool GetTeleportMode() const { return mTeleportMode; } // Guessed name

  private:
    float mAlpha;
    int mFocusWorldIndex;
    CAssetId mFocusWorldRes;
    int mFocusAreaIndex;
    float mFlashPulse;
    const CStateManager& mStateManager;
    const CTransform4f& mPaneProjectionTransform;
    const CTransform4f& mCameraTransform;
    bool mTeleportMode; // Guessed name
  };

  class CMapAreaData {
  public:
    explicit CMapAreaData(CInputStream& in);

    const CTransform4f& GetTransform() const { return mTransform; }

  private:
    CTransform4f mTransform;
  };

  class CMapWorldData {
  public:
    CMapWorldData(CInputStream& in, uint version);

    CAssetId GetWorldAssetId() const { return mWorldAssetId; }
    const rstl::string& GetWorldLabel() const { return mLabel; }
    const CTransform4f& GetWorldTransform() const { return mTransform; }
    const CMapAreaData& GetMapAreaData(int idx) const { return mAreaDatas[idx]; }
    int GetNumMapAreaDatas() const { return mAreaDatas.size(); }
    const CColor& GetSurfaceColorSelected() const { return mSurfColorSelected; }
    const CColor& GetOutlineColorSelected() const { return mOutlineColorSelected; }
    const CColor& GetSurfaceColorUnselected() const { return mSurfColorUnselected; }
    const CColor& GetOutlineColorUnselected() const { return mOutlineColorUnselected; }
    const CVector3f& GetWorldCenterPoint() const { return mCenterPoint; }

  private:
    rstl::string mLabel;
    CAssetId mWorldAssetId;
    CTransform4f mTransform;
    rstl::vector< CMapAreaData > mAreaDatas;
    CColor mSurfColorSelected;
    CColor mOutlineColorSelected;
    CColor mSurfColorUnselected;
    CColor mOutlineColorUnselected;
    CVector3f mCenterPoint;
  };

  CMapUniverse(CInputStream& in, uint version);
  ~CMapUniverse();
  void Draw(const CMapUniverseDrawParms& parms, const CVector3f& pos, float depth1,
            float depth2) const;

  const CMapWorldData& GetMapWorldData(int idx) const { return mWorldDatas[idx]; }
  const CMapWorldData& GetMapWorldDataByWorldId(CAssetId id);
  int GetNumMapWorldDatas() const { return mWorldDatas.size(); }
  const CVector3f& GetMapUniverseCenterPoint() const { return mUniverseCenter; }
  float GetMapUniverseRadius() const { return mUniverseRadius; }

private:
  CAssetId mHexagonId;
  mutable TCachedToken< CMapArea > mHexagonToken;
  rstl::vector< CMapWorldData > mWorldDatas;
  CVector3f mUniverseCenter;
  float mUniverseRadius;
};
NESTED_CHECK_SIZEOF(CMapUniverse, CMapAreaData, 0x30)
NESTED_CHECK_SIZEOF(CMapUniverse, CMapObjectSortInfo, 0x18)
NESTED_CHECK_SIZEOF(CMapUniverse, CMapUniverseDrawParms, 0x24)
NESTED_CHECK_SIZEOF(CMapUniverse, CMapWorldData, 0x70)
CHECK_SIZEOF(CMapUniverse, 0x30)

CFactoryFnReturn FMapUniverseFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer);

#endif // _CMAPUNIVERSE
