#ifndef _CFONT
#define _CFONT

#include "Kyoto/Graphics/CColor.hpp"
#include "types.h"

class CFont {
public:
  CFont(float scale);
  ~CFont();
  int CharWidth(char) const;
  int GetFontSize() const { return mFontSize; }
  void DrawString(const char* str, long x, long y, const CColor& col) const;

private:
  int mFontSize;
  float mScale;
};
CHECK_SIZEOF(CFont, 0x8)

#endif // _CFONT
