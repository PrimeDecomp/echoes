#ifndef _CTEXTRENDERBUFFER
#define _CTEXTRENDERBUFFER

#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CRasterFont;
class CTextRenderBuffer {
public:
  enum EMode { kM_AllocTally, kM_BufferFill };

  struct SFontPalette {
    int x0;
    uint x4;
    uint x8;
    rstl::auto_ptr< CGraphicsPalette > mPalettes[4];
  };

  explicit CTextRenderBuffer(EMode mode);
  ~CTextRenderBuffer();
  void AddFontChange(const TToken< CRasterFont >& font);

private:
  EMode mMode;
  rstl::vector< TToken< CRasterFont > > mFonts;
  rstl::vector< CFontImageDef > mImages;
  rstl::vector< int > mPrimitiveOffsets;
  rstl::vector< signed char > mBytecode;
  uint mBlobSize;
  uint mCurrentBytecodeOffset;
  mutable char mActiveFont;
  mutable char mActivePalette;
  mutable char mQueuedFont;
  mutable char mQueuedPalette;
  mutable rstl::reserved_vector< SFontPalette, 64 > mPalettes;
  mutable int mNextPalette;
  CVector2i xb58;
  CVector2i xb60;
  bool xb68;
};

CHECK_SIZEOF(CTextRenderBuffer, 0xb6c)

#endif // _CTEXTRENDERBUFFER
