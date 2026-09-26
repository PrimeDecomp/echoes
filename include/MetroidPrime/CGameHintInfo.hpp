#ifndef _CGAMEHINTINFO
#define _CGAMEHINTINFO

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;

class CGameHintInfo {
public:
  struct SHintLocation {
    CAssetId mMlvlId;
    CAssetId mMreaId;
    TAreaId mAreaId;
    CAssetId mStringId;

    explicit SHintLocation(CInputStream& in);
  };

  class CGameHint {
  public:
    CGameHint(CInputStream& in, int version);

    const rstl::string& GetName() const { return mName; }
    float GetImmediateTime() const { return mImmediateTime; }
    float GetNormalTime() const { return mNormalTime; }
    CAssetId GetStringId() const { return mStringId; }
    float GetTextTime() const { return mTextTime; }
    const rstl::vector< SHintLocation >& GetLocations() const { return mLocations; }

  private:
    rstl::string mName;
    float mImmediateTime;
    float mNormalTime;
    CAssetId mStringId;
    float mTextTime;
    rstl::vector< SHintLocation > mLocations;
  };

  CGameHintInfo(CInputStream& in, int version);

  const rstl::vector< CGameHint >& GetHints() const { return mHints; }

private:
  rstl::vector< CGameHint > mHints;
};
CHECK_SIZEOF(CGameHintInfo, 0x10)
NESTED_CHECK_SIZEOF(CGameHintInfo, SHintLocation, 0x10)
NESTED_CHECK_SIZEOF(CGameHintInfo, CGameHint, 0x30)

#endif // _CGAMEHINTINFO
