#include "Kyoto/Alloc/IAllocator.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "dolphin/base/PPCArch.h"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "rstl/single_ptr.hpp"
#include "types.h"

#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/CDvdRequest.hpp>
#include <Kyoto/CResFactory.hpp>
#include <Kyoto/Graphics/CGraphicsPalette.hpp>
#include <Kyoto/Math/CMath.hpp>
#include <Kyoto/Streams/CInputStream.hpp>

int CTexture::sCurrentFrameCount = 0;
int CTexture::sTotalAllocatedMemory = 0;
bool CTexture::sMangleMips = false;

#define ROUND_UP_4(v) (((v) + 3) & ~3)

static uint sLoadedTextureStorage[GX_MAX_TEXMAP];
static uint* sLoadedTextures = sLoadedTextureStorage;
static uint sTextureLoadCount;

CTexture::CTexture(ETexelFormat fmt, const short w, const short h, int mips)
: mTexelFormat(fmt)
, mWidth(w)
, mHeight(h)
, mNumMips(mips)
, mBitsPerPixel(TexelFormatBitsPerPixel(fmt))
, mLocked(false)
, mIsPowerOfTwo(false)
, mNoSwap(true)
, mCounted(false)
, mCanLoadObj(true)
, mMemoryAllocated(0)
, mNativeFormat(GX_TF_RGB565)
, mNativeCIFormat(GX_TF_C8)
, mClampMode(kCM_Repeat)
, mFrameAllocated(sCurrentFrameCount) {
  InitBitmapBuffers(fmt, w, h, mips);
  InitTextureObjects();
}

CTexture::CTexture(CInputStream& in, EAutoMipmap automip, EBlackKey blackKey)
: mTexelFormat(kTF_Invalid)
, mWidth(0)
, mHeight(0)
, mNumMips(0)
, mBitsPerPixel(0)
, mLocked(false)
, mIsPowerOfTwo(false)
, mNoSwap(true)
, mCounted(false)
, mCanLoadObj(true)
, mMemoryAllocated(0)
, mNativeFormat(GX_TF_RGB565)
, mNativeCIFormat(GX_TF_C8)
, mClampMode(kCM_Repeat)
, mFrameAllocated(sCurrentFrameCount) {
  mTexelFormat = ETexelFormat(in.Get< uint >());
  mWidth = in.ReadUint16();
  mHeight = in.ReadUint16();
  mNumMips = in.ReadInt32();
  if (IsCITextureFormat(mTexelFormat)) {
    mGraphicsPalette = rs_new CGraphicsPalette(in);
  }
  mBitsPerPixel = TexelFormatBitsPerPixel(mTexelFormat);
  InitBitmapBuffers(mTexelFormat, mWidth, mHeight, mNumMips);
  int bufLen = 0;
  for (int i = 0; i < mNumMips;) {
    int width = ((GetWidth() >> i) + 3) & ~3;
    int height = ((GetHeight() >> i) + 3) & ~3;
    int page = width * height;
    i++;
    bufLen += (GetBitsPerPixel() * page) >> 3;
  }

  uchar* buf = static_cast< uchar* >(mARAMToken.GetMRAMSafe());
  for (int off = 0, len = 0; off < bufLen; off += len) {
    len = bufLen - off;
    if (len > 256) {
      len = 256;
    }

    in.Get(buf + off, len);
    DCFlushRangeNoSync(buf + off, OSRoundUp32B(len));
  }

  if (sMangleMips != false) {
    for (int i = 1; i < mNumMips; ++i) {
      MangleMipmap(i);
    }
  }

  InitTextureObjects();
  PPCSync();
}

CTexture::~CTexture() { UncountMemory(); }

void CTexture::InitTextureObjects() {
  mIsPowerOfTwo =
      CMath::FloorPowerOfTwo(mWidth) == mWidth && CMath::FloorPowerOfTwo(mHeight) == mHeight;

  if (!mIsPowerOfTwo) {
    mClampMode = kCM_Clamp;
  }
  bool hasMips = mNumMips > 1;
  GXTexWrapMode wrap = GXTexWrapMode(mClampMode);
  short width = mWidth;
  short height = mHeight;
  void* buf = mARAMToken.GetMRAMSafe();
  CountMemory();

  if (IsCITextureFormat(mTexelFormat)) {
    GXInitTexObjCI(&mTexObj, buf, width, height, mNativeCIFormat, wrap, wrap, hasMips, 0);
  } else {
    GXInitTexObj(&mTexObj, buf, width, height, mNativeFormat, wrap, wrap, hasMips);
    GXInitTexObjLOD(&mTexObj, mNumMips > 1 ? GX_LIN_MIP_LIN : GX_LINEAR, GX_LINEAR, 0.f,
                    mNumMips - 1.f, 0.f, false, false, mNumMips > 1 ? GX_ANISO_4 : GX_ANISO_1);
  }

  mCanLoadObj = true;
  InvalidateTexmaps();
}

void CTexture::Load(GXTexMapID tex, EClampMode clamp) const {
  // The low address bits distinguish the three clamp modes in the cache.
  if (reinterpret_cast< uint >(this) + clamp != sLoadedTextures[tex]) {
    if (mCanLoadObj) {
      void* ptr = mARAMToken.GetMRAMSafe();
      CountMemory();
      if (!mGraphicsPalette.null()) {
        mGraphicsPalette->Load();
      } else {
        mCanLoadObj = false;
      }
      GXInitTexObjData(&mTexObj, ptr);
    }

    if (mClampMode != clamp && mIsPowerOfTwo) {
      mClampMode = clamp;
      GXInitTexObjWrapMode(&mTexObj, GXTexWrapMode(mClampMode), GXTexWrapMode(mClampMode));
    }

    GXLoadTexObj(&mTexObj, tex);
    sLoadedTextures[tex] = reinterpret_cast< uint >(this) + clamp;
    mFrameAllocated = sCurrentFrameCount;
    ++sTextureLoadCount;
  }
}

void CTexture::LoadMipLevel(int mip, GXTexMapID tex, EClampMode clamp) const {
  char* ptr = static_cast< char* >(mARAMToken.GetMRAMSafe());
  GXTexObj obj = mTexObj;
  int width = mWidth;
  int height = mHeight;
  int offset = 0;
  GXTexWrapMode wrap = GXTexWrapMode(clamp);
  for (int i = 0; i < mip; i++) {
    int w = ROUND_UP_4(width);
    int h = ROUND_UP_4(height);
    offset += OSRoundUp32B(((mBitsPerPixel * (w * h)) / 8));
    width /= 2;
    height /= 2;
  }

  GXInitTexObj(&obj, ptr + offset, width, height, mNativeFormat, wrap, wrap, false);
  GXInitTexObjLOD(&obj, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  if (!mGraphicsPalette.null()) {
    mGraphicsPalette->Load();
  }

  GXLoadTexObj(&obj, tex);
  sLoadedTextures[tex] = 0;
  mFrameAllocated = sCurrentFrameCount;
}

void CTexture::UnloadBitmapData(CAssetId textureId) const {
  if (!mBitmapReloader.null()) {
    bool loadToARAM = mBitmapReloader->GetShouldBeInARAM();
    mBitmapReloader = rs_new CDumpedBitmapDataReloader(textureId, mMemoryAllocated, loadToARAM);
  } else {
    bool complete = mARAMToken.GetStatus() == CARAMToken::kS_Zero ||
                    mARAMToken.GetStatus() == CARAMToken::kS_Two ||
                    mARAMToken.GetStatus() == CARAMToken::kS_Five;

    mARAMToken = CARAMToken();
    mBitmapReloader = rs_new CDumpedBitmapDataReloader(textureId, mMemoryAllocated, complete);
    mCanLoadObj = true;
  }
}

bool CTexture::TryReloadBitmapData(CResFactory& factory) const {
  if (mBitmapReloader.null()) {
    return true;
  }

  mCanLoadObj = true;
  mBitmapReloader->BeginReloadBitmapData(factory);
  uchar* ptr = static_cast< uchar* >(mBitmapReloader->TryBuildReloadedBitmapData(factory));
  if (ptr != nullptr) {
    bool loadToARAM = mBitmapReloader->GetShouldBeInARAM();
    mBitmapReloader = nullptr;

    mARAMToken.PostConstruct(ptr, mMemoryAllocated, 1);
    const_cast< CTexture& >(*this).InitTextureObjects();

    if (loadToARAM) {
      LoadToARAM();
    }

    return true;
  }
  return false;
}

int CTexture::GetBitmapDataStatus() const {
  if (!mBitmapReloader.null()) {
    if (mBitmapReloader->GetStatus() == 0) {
      return 2;
    }

    return 5;
  }

  switch (mARAMToken.GetStatus()) {
  case CARAMToken::kS_Zero:
    return 1;
  case CARAMToken::kS_One:
    return 0;
  case CARAMToken::kS_Five:
  case CARAMToken::kS_Two:
    return 3;
  case CARAMToken::kS_Three:
  case CARAMToken::kS_Four:
    return 4;
  default:
    return -1;
  }
}

CTexture::CDumpedBitmapDataReloader::CDumpedBitmapDataReloader(CAssetId textureId, uint bitmapSize,
                                                               bool loadToARAM)
: mState(0)
, mTextureId(textureId)
, mResourceSize(0)
, mBitmapSize(bitmapSize)
, mShouldBeInARAM(loadToARAM) {}

void CTexture::CDumpedBitmapDataReloader::BeginReloadBitmapData(CResFactory& factory) {
  if (mState != 0) {
    return;
  }
  SObjectTag tag('TXTR', mTextureId);
  mResourceSize = factory.ResourceSize(tag);
  mData = static_cast< uchar* >(CMemory::Alloc(mResourceSize, IAllocator::kHI_RoundUpLen));
  mRequest = factory.GetResLoader().LoadResourceAsync(tag, reinterpret_cast< char* >(mData.get()));
  mState = 1;
}

void* CTexture::CDumpedBitmapDataReloader::TryBuildReloadedBitmapData(CResFactory& factory) {
  if (mRequest->IsComplete()) {
    mState = 2;
    mRequest = nullptr;

    SObjectTag tag('TXTR', mTextureId);
    rstl::single_ptr< CInputStream > buf =
        factory.GetResLoader().LoadResourceFromMemorySync(tag, mData.get());
    CInputStream& in = *buf;
    ETexelFormat format = ETexelFormat(in.ReadInt32());
    const short w = in.ReadInt16();
    const short h = in.ReadInt16();
    const int numMips = in.ReadInt32();
    const int bitsPerPixel = TexelFormatBitsPerPixel(format);

    if (IsCITextureFormat(format)) {
      CGraphicsPalette tmp(in);
    }

    int bufLen = 0;
    for (int i = 0; i < numMips;) {
      int width = ((w >> i) + 3) & ~3;
      int height = ((h >> i) + 3) & ~3;
      int page = (width * height);
      i++;
      bufLen += (page * bitsPerPixel) >> 3;
    }

    void* ptr = CMemory::Alloc(mBitmapSize, IAllocator::kHI_RoundUpLen);

    for (int off = 0, len = 0; off < bufLen; off += len) {
      len = bufLen - off;
      if (len > 256) {
        len = 256;
      }

      in.Get(static_cast< char* >(ptr) + off, len);
      DCFlushRangeNoSync(static_cast< char* >(ptr) + off, OSRoundUp32B(len));
    }
    PPCSync();
    mData = nullptr;
    return ptr;
  }
  return nullptr;
}

bool CTexture::LoadToMRAM() const {
  if (mARAMToken.GetStatus() == CARAMToken::kS_One) {
    return true;
  }

  mCanLoadObj = true;
  if (mARAMToken.GetStatus() == CARAMToken::kStatus_Unowned) {
    return false;
  }

  mFrameAllocated = sCurrentFrameCount;
  CountMemory();
  return mARAMToken.LoadToMRAM();
}

bool CTexture::LoadToARAM() const {
  mCanLoadObj = true;
  if (mARAMToken.GetStatus() == CARAMToken::kStatus_Unowned) {
    return false;
  }

  if (mNoSwap) {
    return false;
  }

  if (mFrameAllocated < sCurrentFrameCount - 1) {
    bool ret = mARAMToken.LoadToARAM();

    if (mARAMToken.GetStatus() != CARAMToken::kS_One) {
      UncountMemory();
      InvalidateTexmaps();
    }
    return ret;
  }
  return false;
}

bool CTexture::IsARAMTransferInProgress() const {
  if (mNoSwap) {
    return false;
  }
  return mARAMToken.GetStatus() >= CARAMToken::kS_Two &&
         mARAMToken.GetStatus() <= CARAMToken::kS_Five;
}

int CTexture::TexelFormatBitsPerPixel(ETexelFormat fmt) {
  switch (fmt) {
  case kTF_I4:
  case kTF_C4:
  case kTF_CMPR:
    return 4;
  case kTF_I8:
  case kTF_IA4:
  case kTF_C8:
    return 8;
  case kTF_IA8:
  case kTF_C14X2:
  case kTF_RGB565:
  case kTF_RGB5A3:
    return 16;
  case kTF_RGBA8:
    return 32;
  default:
    return 0;
  }
}

void CTexture::InitBitmapBuffers(ETexelFormat fmt, short width, short height, int mips) {
  switch (fmt) {
  case kTF_C4:
    mNativeCIFormat = GX_TF_C4;
    break;
  case kTF_C8:
    mNativeCIFormat = GX_TF_C8;
    break;
  case kTF_C14X2:
    mNativeCIFormat = GX_TF_C14X2;
    break;
  case kTF_I4:
    mNativeFormat = GX_TF_I4;
    break;
  case kTF_I8:
    mNativeFormat = GX_TF_I8;
    break;
  case kTF_IA4:
    mNativeFormat = GX_TF_IA4;
    break;
  case kTF_IA8:
    mNativeFormat = GX_TF_IA8;
    break;
  case kTF_RGB565:
    mNativeFormat = GX_TF_RGB565;
    break;
  case kTF_RGB5A3:
    mNativeFormat = GX_TF_RGB5A3;
    break;
  case kTF_RGBA8:
    mNativeFormat = GX_TF_RGBA8;
    break;
  case kTF_CMPR:
    mNativeFormat = GX_TF_CMPR;
    break;
  default:
    break;
  }

  bool hasMips = mips > 1;
  mMemoryAllocated = GXGetTexBufferSize(
      width, height, HasPalette() ? mNativeCIFormat : mNativeFormat, hasMips, hasMips ? 11 : 0);
  mCanLoadObj = true;
  mARAMToken.PostConstruct(CMemory::Alloc(mMemoryAllocated, IAllocator::kHI_RoundUpLen),
                           mMemoryAllocated, 1);
  CountMemory();
}

void CTexture::UnLock() {
  mLocked = false;
  mCanLoadObj = true;
  CountMemory();
  DCFlushRange(mARAMToken.GetMRAMSafe(), OSRoundUp32B(mMemoryAllocated));
}

CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in,
                                 const CVParamTransfer& xfer) {
  return rs_new CTexture(in, CTexture::kAM_Zero, CTexture::kBK_Zero);
}

const void* CTexture::GetConstBitMapData(const int mip) const {
  int offset = 0;
  for (int i = 0; i < mip; i++) {
    offset += (GetBitsPerPixel() >> 3) * ((GetWidth() >> i) * (GetHeight() >> i));
  }

  return static_cast< const uchar* >(mARAMToken.GetMRAMSafe()) + offset;
}

void* CTexture::GetBitMapData(int mip) { return const_cast< void* >(GetConstBitMapData(mip)); }

void CTexture::MangleMipmap(int mip) {
  if (mip >= mNumMips) {
    return;
  }

  const uint colors[4] = {
      0x000000FF,
      0x0000FF00,
      0x00FF0000,
      0x0000FFFF,
  };
  const uint color = colors[(mip - 1) & 3];
  ushort rgb565Color = ((color >> 3) & 0x001F) | // B
                       ((color >> 5) & 0x07E0) | // G
                       ((color >> 8) & 0xF800);  // R
  ushort rgb555Color = ((color >> 3) & 0x001F) | // B
                       ((color >> 6) & 0x03E0) | // G
                       ((color >> 9) & 0x7C00);  // R
  ushort rgb4Color = ((color >> 4) & 0x000F) |   // B
                     ((color >> 8) & 0x00F0) |   // G
                     ((color >> 12) & 0x0F00);   // R

  int width = mWidth;
  int height = mHeight;

  int offset = 0;
  for (int i = 0; i < mip; i++) {
    offset += width * height;
    width /= 2;
    height /= 2;
  }

  switch (GetTexelFormat()) {
  case kTF_RGB565: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe());
    for (int i = 0; i < width * height; ++i) {
      ptr[i + offset] = rgb565Color;
    }
    break;
  }
  case kTF_CMPR: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe()) + offset / 4;
    for (int i = 0; i < width * height / 16; ++i, ptr += 4) {
      ptr[0] = rgb565Color;
      ptr[1] = rgb565Color;
      ptr[2] = 0;
      ptr[3] = 0;
    }
    break;
  }
  case kTF_RGB5A3: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe());
    for (int i = 0; i < width * height; ++i) {
      ushort& val = ptr[i + offset];
      if (val & 0x8000) {
        val = rgb555Color | 0x8000;
      } else {
        val = (val & 0xF000) | rgb4Color;
      }
    }
    break;
  }
  }
}

void CTexture::MakeSwappable() const {
  if (!mNoSwap) {
    return;
  }

  mNoSwap = false;
}

void CTexture::CountMemory() const {
  if (mCounted) {
    return;
  }

  mCounted = true;
  sTotalAllocatedMemory += mMemoryAllocated;
}

void CTexture::UncountMemory() const {
  if (!mCounted) {
    return;
  }

  mCounted = false;
  mCanLoadObj = true;
  sTotalAllocatedMemory -= mMemoryAllocated;
}

void CTexture::InvalidateTexmap(GXTexMapID texmap) { sLoadedTextures[texmap] = 0; }

void CTexture::ScheduleDeletion() {
  if (mARAMToken.GetStatus() != CARAMToken::kStatus_Unowned) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mARAMToken.ForceSyncMRAM());
  }
}

void CTexture::InvalidateTexmaps() const {
  for (int i = 0; i < GX_MAX_TEXMAP; ++i) {
    if (sLoadedTextures[i] == reinterpret_cast< uint >(this) + mClampMode) {
      sLoadedTextures[i] = 0;
    }
  }
}
