#ifndef _CDARKWORLDINFO
#define _CDARKWORLDINFO

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CModel;
class CTexture;

// Guessed name. Shared dark-world volume parameters copied into transitions.
struct CDarkWorldInfo {
  CDarkWorldInfo(ushort sfx0, ushort sfx1, ushort sfx2, ushort sfx3, ushort sfx4, float xc,
                 CAssetId spotTexture, float x20, const CVector2f& scroll1,
                 const CVector2f& scroll2, const CVector2f& texScale1, const CVector2f& texScale2,
                 CAssetId environment, CAssetId cloud1, CAssetId cloud2, CColor color,
                 CColor additiveColor)
  : x0_(sfx0)
  , x2_(sfx1)
  , x4_(sfx2)
  , x6_(sfx3)
  , x8_(sfx4)
  , xc_(xc)
  , x10_(LoadOptionalTexture(spotTexture))
  , x20_(x20)
  , mScroll1(scroll1)
  , mScroll2(scroll2)
  , mTexScale1(texScale1)
  , mTexScale2(texScale2)
  , mEnvironment(gpSimplePool->GetObj(SObjectTag('TXTR', environment)))
  , mCloud1(gpSimplePool->GetObj(SObjectTag('TXTR', cloud1)))
  , mCloud2(gpSimplePool->GetObj(SObjectTag('TXTR', cloud2)))
  , mColor(color)
  , mAdditiveColor(additiveColor) {}

  // Guessed name.
  static rstl::optional_object< TLockedToken< CTexture > > LoadOptionalTexture(CAssetId id) {
    if (id == kInvalidAssetId) {
      return rstl::optional_object< TLockedToken< CTexture > >();
    }
    return TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', id)));
  }

  // Meanings and identifier types remain unresolved; copied as separate halfwords.
  ushort x0_;
  ushort x2_;
  ushort x4_;
  ushort x6_;
  ushort x8_;
  float xc_;
  rstl::optional_object< TLockedToken< CTexture > > x10_;
  float x20_;
  CVector2f mScroll1;
  CVector2f mScroll2;
  CVector2f mTexScale1;
  CVector2f mTexScale2;
  TLockedToken< CTexture > mEnvironment;
  TLockedToken< CTexture > mCloud1;
  TLockedToken< CTexture > mCloud2;
  CColor mColor;
  CColor mAdditiveColor;
};
CHECK_SIZEOF(CDarkWorldInfo, 0x70)

#endif // _CDARKWORLDINFO
