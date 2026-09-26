#ifndef _CMAPUNIVERSE
#define _CMAPUNIVERSE

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CMapArea;

class CMapUniverse {
public:
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

  const CMapWorldData& GetMapWorldData(int idx) const { return mWorldDatas[idx]; }
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
NESTED_CHECK_SIZEOF(CMapUniverse, CMapWorldData, 0x70)
CHECK_SIZEOF(CMapUniverse, 0x30)

#endif // _CMAPUNIVERSE
