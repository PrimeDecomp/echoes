#ifndef _CRASTERFONT
#define _CRASTERFONT

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

#include "string.h"

class CTexture;
class CDrawStringOptions;
class IObjectStore;

class CFontInfo {
public:
  CFontInfo(bool a, bool b, int c, int fontSize, const char* name)
  : x0_(a), x1_(b), x4_(c), mFontSize(fontSize) {
    strcpy(mName, name);
  }

private:
  bool x0_;
  bool x1_;
  int x4_;
  int mFontSize;
  char mName[64];
};

class CKernPair {
public:
  CKernPair(wchar_t first, wchar_t second, int howMuch)
  : mFirst(first), mSecond(second), mHowMuch(howMuch) {}

  wchar_t GetFirst() const { return mFirst; }
  wchar_t GetSecond() const { return mSecond; }
  int GetHowMuch() const { return mHowMuch; }

private:
  wchar_t mFirst;
  wchar_t mSecond;
  int mHowMuch;
};

class CGlyph {
public:
  CGlyph(int a, int b, int c, float startU, float startV, float endU, float endV, int cellWidth,
         int cellHeight, int baseline, int kernStart)
  : mA(a)
  , mB(b)
  , mC(c)
  , mStartU(startU)
  , mStartV(startV)
  , mEndU(endU)
  , mEndV(endV)
  , mCellWidth(cellWidth)
  , mCellHeight(cellHeight)
  , mBaseline(baseline)
  , mKernStart(kernStart)
   {}

  short GetA() const { return mA; }
  short GetB() const { return mB; }
  short GetC() const { return mC; }
  float GetStartU() const { return mStartU; }
  float GetStartV() const { return mStartV; }
  float GetEndU() const { return mEndU; }
  float GetEndV() const { return mEndV; }
  short GetCellWidth() const { return mCellWidth; }
  short GetCellHeight() const { return mCellHeight; }
  short GetBaseline() const { return mBaseline; }
  short GetKernStart() const { return mKernStart; }
  //short GetLayer() const { return x20_layer; }

private:
  short mA;
  short mB;
  short mC;
  float mStartU;
  float mStartV;
  float mEndU;
  float mEndV;
  short mCellWidth;
  short mCellHeight;
  short mBaseline;
  short mKernStart;
  //short x20_layer;
};

enum EFontMode {
  kFM_None = -1,
  kFM_OneLayer,
  kFM_OneLayerOutline,
  kFM_FourLayers,
  kFM_TwoLayersOutline,
  kFM_TwoLayers,
};

class CRasterFont {
public:
  CRasterFont(CInputStream& in, IObjectStore* store);
  ~CRasterFont();

  EFontMode GetMode() const;
  void GetSize(const CDrawStringOptions&, int&, int&, const wchar_t*, int) const;
  void SetTexture(TToken< CTexture > token) { mTexture = token; }
  bool IsFinishedLoading();

private:
  bool mInitialized;
  int mMonoWidth;
  int mMonoHeight;
  rstl::vector< rstl::pair< wchar_t, CGlyph > > mGlyphs;
  rstl::vector< CKernPair > mKerning;
  EFontMode mMode;
  rstl::optional_object< CFontInfo > mFontInfo;
  rstl::optional_object< TToken< CTexture > > mTexture;
  int mBaseline;
  int mLineMargin;
};
CHECK_SIZEOF(CRasterFont, 0x94)

#endif // _CRASTERFONT
