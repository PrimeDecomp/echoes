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
  ushort x0_[5]; // Meanings and identifier types remain unresolved.
  uint xc_;
  rstl::optional_object< TLockedToken< CModel > > x10_;
  uint x20_;
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
