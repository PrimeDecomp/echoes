#ifndef _CWEAPONASSETINFO
#define _CWEAPONASSETINFO

#include "MetroidPrime/TGameTypes.hpp"

class CWeaponAssetInfo {
public:
  explicit CWeaponAssetInfo(CAssetId a = kInvalidAssetId, CAssetId b = kInvalidAssetId,
                            CAssetId c = kInvalidAssetId, CAssetId d = kInvalidAssetId,
                            CAssetId e = kInvalidAssetId, CAssetId f = kInvalidAssetId,
                            CAssetId g = kInvalidAssetId, CAssetId h = kInvalidAssetId)
  : mCount(8) {
    mAssets[0] = a;
    mAssets[1] = b;
    mAssets[2] = c;
    mAssets[3] = d;
    mAssets[4] = e;
    mAssets[5] = f;
    mAssets[6] = g;
    mAssets[7] = h;
  }

  CAssetId GetAsset(int index) const { return mAssets[index]; }

private:
  int mCount;
  CAssetId mAssets[8];
};
CHECK_SIZEOF(CWeaponAssetInfo, 0x24)

#endif // _CWEAPONASSETINFO
