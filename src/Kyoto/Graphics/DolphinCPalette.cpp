#include "Kyoto/Graphics/CGraphicsPalette.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "dolphin/gx/GXTexture.h"
#include "dolphin/os/OSCache.h"

uint CGraphicsPalette::sCurrentFrameCount = 0;

static inline GXTlutFmt format_to_format(EPaletteFormat format) {
  return static_cast< GXTlutFmt >(format);
}

CGraphicsPalette::CGraphicsPalette(EPaletteFormat format, int numEntries)
: mFmt(format)
, mEntryCount(numEntries)
, mEntries(static_cast< ushort* >(
      CMemory::Alloc(numEntries * sizeof(ushort), IAllocator::kHI_RoundUpLen)))
, mLocked(false) {
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
}

CGraphicsPalette::CGraphicsPalette(CInputStream& in)
: mFmt(EPaletteFormat(in.ReadInt32()))
, mEntryCount(in.ReadInt16() * in.ReadInt16())
, mEntries(static_cast< ushort* >(
      CMemory::Alloc(mEntryCount * sizeof(ushort), IAllocator::kHI_RoundUpLen)))
, mLocked(false) {
  in.Get(reinterpret_cast< uchar* >(mEntries.get()), mEntryCount * sizeof(ushort));
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
}

CGraphicsPalette::~CGraphicsPalette() {
  const uint frameDiff = sCurrentFrameCount - mFrameLoaded;
  if (frameDiff < 2) {
    CFrameDelayedKiller::ScheduleDeletion(frameDiff > 0
                                              ? CFrameDelayedKiller::kWhichFrame_ThisFrame
                                              : CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mEntries.release());
  }
}

void CGraphicsPalette::Load() const {
  GXLoadTlut(&mTlutObj, GX_TLUT0);
  mFrameLoaded = sCurrentFrameCount;
}

void CGraphicsPalette::UnLock() {
  DCStoreRange(mEntries.get(), mEntryCount * sizeof(ushort));
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
  mLocked = false;
}
