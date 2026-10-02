#ifndef _CTEXTRENDERBUFFER
#define _CTEXTRENDERBUFFER

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CTextRenderBuffer {
public:
  enum ECmd {
    kC_CharacterRender,
    kC_ImageRender,
    kC_FontChange,
    kC_PaletteChange,
    kC_Invalid = -1,
  };
  enum EMode { kM_AllocTally, kM_BufferFill };

  struct Primitive {
    Primitive(ECmd cmd, short x, short y, short chr, uint color, signed char index)
    : mColor(color), mCmd(cmd), mX(x), mY(y), mChar(chr), mIndex(index) {}

    uint mColor;
    ECmd mCmd;
    short mX;
    short mY;
    short mChar;
    signed char mIndex;
  };

  struct SFontPalette {
    SFontPalette(EFontMode mode, const rstl::auto_ptr< CGraphicsPalette >& palette0,
                 const rstl::auto_ptr< CGraphicsPalette >& palette1,
                 const rstl::auto_ptr< CGraphicsPalette >& palette2,
                 const rstl::auto_ptr< CGraphicsPalette >& palette3)
    : mMode(mode)
    , mPalette0(palette0)
    , mPalette1(palette1)
    , mPalette2(palette2)
    , mPalette3(palette3) {}

    EFontMode mMode;
    ushort mColors[4];
    rstl::auto_ptr< CGraphicsPalette > mPalette0;
    rstl::auto_ptr< CGraphicsPalette > mPalette1;
    rstl::auto_ptr< CGraphicsPalette > mPalette2;
    rstl::auto_ptr< CGraphicsPalette > mPalette3;
  };

  explicit CTextRenderBuffer(EMode mode);
  void AddPaletteChange(const CGraphicsPalette& palette, EFontMode mode);
  void AddCharacter(const CVector2i& offset, short chr, uint color);
  void AddFontChange(const TToken< CRasterFont >& font);
  void AddImage(const CVector2i& offset, const CFontImageDef& image);
  void* GetOutStream();
  size_t GetCurLen();
  void SetMode(EMode mode);
  void Render(const CColor& color, float time) const;
  int GetNumPrimitives() const { return mPrimitiveOffsets.size(); }
  Primitive GetPrimitive(int index) const;
  void SetPrimitive(const Primitive& prim, int index);
  // Guessed name. Returns cached bounds, rebuilding them when dirty.
  const rstl::pair< CVector2i, CVector2i >& GetTextBounds();

private:
  void VerifyBuffer();
  void AccumulateTextBounds();
  void GetNextAvailablePalette() const;
  int GetMatchingPaletteIndex(EFontMode mode, const CGraphicsPalette& palette) const;
  // Guessed name for the mode/layer palette expansion helper.
  static void GeneratePalette(EFontMode mode, int layer, CGraphicsPalette& dest,
                              const CGraphicsPalette& source);

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
  rstl::pair< CVector2i, CVector2i > mTextBounds;
  bool mTextBoundsDirty;
};

CHECK_SIZEOF(CTextRenderBuffer, 0xb6c)
NESTED_CHECK_SIZEOF(CTextRenderBuffer, Primitive, 0x10)
NESTED_CHECK_SIZEOF(CTextRenderBuffer, SFontPalette, 0x2c)

#endif // _CTEXTRENDERBUFFER
