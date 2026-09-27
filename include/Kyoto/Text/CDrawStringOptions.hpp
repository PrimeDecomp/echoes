#ifndef _CDRAWSTRINGOPTIONS
#define _CDRAWSTRINGOPTIONS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/reserved_vector.hpp"

class CDrawStringOptions {
public:
  CDrawStringOptions();

  void SetTextDirection(ETextDirection dir) { mDirection = dir; }
  ETextDirection GetTextDirection() const { return mDirection; }
  void SetPaletteEntry(int idx, uint color) { mColors[idx] = color; }
  // Guessed names
  void SetCharacterExtraSpace(int spacing) { mCharacterExtraSpace = spacing; }
  int GetCharacterExtraSpace() const { return mCharacterExtraSpace; }

private:
  ETextDirection mDirection;
  rstl::reserved_vector< u32, 16 > mColors;
  int mCharacterExtraSpace; // Guessed name
};

CHECK_SIZEOF(CDrawStringOptions, 0x4c)

#endif // _CDRAWSTRINGOPTIONS
