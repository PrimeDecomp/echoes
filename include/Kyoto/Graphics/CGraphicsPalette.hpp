#ifndef _CGRAPHICSPALETTE
#define _CGRAPHICSPALETTE

#include "types.h"

#include "dolphin/gx.h"

#include <rstl/single_ptr.hpp>

enum EPaletteFormat {
  kPF_IA8 = GX_TL_IA8,
  kPF_RGB565 = GX_TL_RGB565,
  kPF_RGB5A3 = GX_TL_RGB5A3,
};

class CInputStream;

class CGraphicsPalette {
public:
  CGraphicsPalette(EPaletteFormat format, int numEntries);
  CGraphicsPalette(CInputStream& in);
  ~CGraphicsPalette();

  ushort* GetPaletteData() { return mEntries.get(); }
  const ushort* GetPaletteData() const { return mEntries.get(); }
  void Load() const;
  void UnLock();

private:
  static uint sCurrentFrameCount;
  EPaletteFormat mFmt;
  mutable uint mFrameLoaded;
  uint mEntryCount;
  rstl::single_ptr<ushort> mEntries;
  GXTlutObj mTlutObj;
  bool mLocked;
};

#endif // _CGRAPHICSPALETTE
