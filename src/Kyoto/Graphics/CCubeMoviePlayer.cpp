#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include <Kyoto/Graphics/CMoviePlayer.hpp>

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "dolphin/PPCArch.h"
#include "dolphin/ai.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/os.h"
#include "dolphin/thp/THPDec.h"
#include <Kyoto/Graphics/CGX.hpp>
#include <Kyoto/Graphics/CTexture.hpp>
#include <math.h>
#include <rstl/math.hpp>
#include <string.h>

#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTexture.h"

extern bool lbl_804199CC;
extern bool lbl_80419B9D;
extern "C" void* fn_8033D2EC();

static rstl::string SelectMoviePath(const char* path) {
  rstl::string name(path);
  if (lbl_804199CC) {
    rstl::string palName = name + "_pal";
    if (CDvdFile::FileExists(palName.data())) {
      return palName;
    }
  }
  return name;
}

static bool ShouldEnableLockedCache() {
  return reinterpret_cast< void* >(0xe0000000) != fn_8033D2EC();
}

class CInterruptGuard {
  bool x0_enabled;

public:
  CInterruptGuard() : x0_enabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(x0_enabled); }
};

static int sNumReferences = 0;
static CMoviePlayer* sAudioPlayer;
static const short* curAudioBuffer;
static int soundBufferIndex;
static short soundBuffer[2][320] ATTRIBUTE_ALIGN(32);
static bool sAudioEnabled = true;
static uchar sSfxVolume = 127;

struct CMoviePlayer::SIndexLoad {
  rstl::single_ptr< CDvdRequest > x0_headerRequest;
  rstl::single_ptr< CDvdRequest > x4_videoRequest;
  rstl::single_ptr< CDvdRequest > x8_audioRequest;
  rstl::single_ptr< uchar > xc_buffer;
  int x10_state;

  ~SIndexLoad();

  SIndexLoad()
  : xc_buffer(static_cast< uchar* >(CMemory::Alloc(64, IAllocator::kHI_RoundUpLen)))
  , x10_state(0) {}
};
CMoviePlayer::SIndexLoad::~SIndexLoad() {}

CMoviePlayer::CTHPTextureSet::CTHPTextureSet(void* y, void* u, void* v, void* audio)
: x0_y(static_cast< uchar* >(y))
, x8_u(static_cast< uchar* >(u))
, x10_v(static_cast< uchar* >(v))
, x18_audio(static_cast< uchar* >(audio))
, x20_audioSamples(0)
, x24_audioSamplesConsumed(0) {}

CMoviePlayer::CTHPTextureSet::CTHPTextureSet(const CTHPTextureSet& other)
: x0_y(other.x0_y)
, x8_u(other.x8_u)
, x10_v(other.x10_v)
, x18_audio(other.x18_audio)
, x20_audioSamples(other.x20_audioSamples)
, x24_audioSamplesConsumed(other.x24_audioSamplesConsumed) {}

CMoviePlayer::CTHPTextureSet::~CTHPTextureSet() {}

const unsigned char skInterlacePattern[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void MyTHPGXRestore() {
  CGX::SetZMode(TRUE, GX_ALWAYS, FALSE);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

static void MyTHPGXYuv2RgbSetup(bool field, bool deinterlace) {
  GXVtxDescList attr[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetZMode(TRUE, GX_ALWAYS, FALSE);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE, GX_PTIDENTITY);

  if (deinterlace) {
    CGX::SetNumTexGens(2);
    CGX::SetNumTevStages(4);
  } else {
    CGX::SetNumTexGens(3);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, FALSE, GX_PTIDENTITY);

    float n = field ? 0.25f : 0.f;
    float mtx[8] = {0.125f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.25f, n};
    GXLoadTexMtxImm(reinterpret_cast< MtxPtr >(mtx), GX_TEXMTX0, GX_MTX2x4);
    GXTexObj obj;
    GXInitTexObj(&obj, skInterlacePattern, 8, 4, GX_TF_I8, GX_REPEAT, GX_REPEAT, FALSE);
    GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
    GXLoadTexObj(&obj, GX_TEXMAP3);
    CTexture::InvalidateTexmap(GX_TEXMAP3);
    CGX::SetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD2, GX_TEXMAP3, GX_COLOR_NULL);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE4);
    CGX::SetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    CGX::SetAlphaCompare(GX_LESS, 128, GX_AOP_AND, GX_ALWAYS, 0);
    CGX::SetNumTevStages(5);
  }

  CGX::SetVtxDescv(attr);
  GXSetColorUpdate(TRUE);
  GXSetAlphaUpdate(FALSE);
  GXInvalidateTexAll();
  GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_CLR_RGBA, GX_RGBX8, 0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, false, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
  const GXColorS10 color = {-90, 0, -114, 135};
  GXSetTevColorS10(GX_TEVREG0, color);
  const GXColor kColor0 = {0, 0, 226, 88};
  CGX::SetTevKColor(GX_KCOLOR0, kColor0);
  const GXColor kColor1 = {179, 0, 0, 182};
  CGX::SetTevKColor(GX_KCOLOR1, kColor1);
  const GXColor kColor2 = {255, 0, 255, 128};
  CGX::SetTevKColor(GX_KCOLOR2, kColor2);
}

static void MyTHPYuv2RgbTextureSetup(void* y, void* u, void* v, ushort width, ushort height) {
  GXTexObj yTex;
  GXInitTexObj(&yTex, y, width, height, GX_TF_I8, GX_CLAMP, GX_CLAMP, FALSE);
  GXInitTexObjLOD(&yTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&yTex, GX_TEXMAP0);

  GXTexObj uTex;
  GXInitTexObj(&uTex, u, width / 2, height / 2, GX_TF_I8, GX_CLAMP, GX_CLAMP, FALSE);
  GXInitTexObjLOD(&uTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&uTex, GX_TEXMAP1);

  GXTexObj vTex;
  GXInitTexObj(&vTex, v, static_cast< s16 >(width / 2), height / 2, GX_TF_I8, GX_CLAMP, GX_CLAMP,
               FALSE);
  GXInitTexObjLOD(&vTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&vTex, GX_TEXMAP2);

  CTexture::InvalidateTexmap(GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP1);
  CTexture::InvalidateTexmap(GX_TEXMAP2);
}

CMoviePlayer::CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
                           const bool deinterlace)
: x0_dvdFile(SelectMoviePath(path).data())
, xb4_indexLoad(rs_new SIndexLoad)
, xb8_nextReadSize(0)
, xbc_nextReadOff(0)
, xc0_readSizeWrapped(0)
, xc4_readOffWrapped(0)
, xc8_curLoadFrame(0)
, xcc_requestFrameWrapped(0)
, xd0_curFrame(0)
, xd4_decodedTexSlot(0)
, xd8_drawTexSlot(-1)
, xdc_audioSlot(-1)
, xe0_decodedTexCount(0)
, xe4_frameRem(0.f)
, xe8_prefetchFrame(0)
, xec_prefetchOff(0)
, xf0_prefetchSize(0)
, xfc_playMode(kPM_Playing)
, x100_totalSeconds(0.f)
, x104_curSeconds(0.f)
, x108_preLoadSeconds(preLoadSeconds)
, x10c_preLoadFrames(0)
, x110_24_loop(loop)
, x110_25_deinterlace(deinterlace)
, x110_26_is60Hz(false)
, x110_27_hasAudio(false)
, x110_28_fieldFlip(false)
, x114_cachedBytes(0)
, x118_fieldIndex(0)
, x11c_volume(127) {
  if (sNumReferences == 0 && ShouldEnableLockedCache()) {
    LCEnable();
  }

  static bool sThpInitialized = false;
  if (!sThpInitialized) {
    sThpInitialized = true;
    THPInit();
  }
  ++sNumReferences;
  VerifyCallbackStatus();
  xb4_indexLoad->x0_headerRequest = x0_dvdFile.SyncRead(xb4_indexLoad->xc_buffer.get(), 64);
}

bool CMoviePlayer::ContinueLoading() {
  if (xb4_indexLoad.null()) {
    return false;
  }
  uchar* const buffer = xb4_indexLoad->xc_buffer.get();
  bool hasVideo;
  bool hasAudio;
  switch (xb4_indexLoad->x10_state) {
  case 0:
    if (xb4_indexLoad->x0_headerRequest->IsComplete()) {
      memcpy(&x28_header, buffer, sizeof(THPHeader));
      x28_header.mVersion = CBasics::SwapBytes(static_cast< uint >(x28_header.mVersion));
      x28_header.mBufferSize = CBasics::SwapBytes(static_cast< uint >(x28_header.mBufferSize));
      x28_header.mAudioMaxSamples =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mAudioMaxSamples));
      x28_header.mFrameRate = CBasics::SwapBytes(x28_header.mFrameRate);
      x28_header.mNumFrames = CBasics::SwapBytes(static_cast< uint >(x28_header.mNumFrames));
      x28_header.mFirstFrameSize =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mFirstFrameSize));
      x28_header.mMovieDataSize =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mMovieDataSize));
      x28_header.mCompInfoDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mCompInfoDataOffsets));
      x28_header.mOffsetDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mOffsetDataOffsets));
      x28_header.mMovieDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mMovieDataOffsets));
      x28_header.mFinalFrameDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mFinalFrameDataOffsets));
      if (fabsf(x28_header.mFrameRate - 29.97f) < 0.00001f) {
        x28_header.mFrameRate = 30.f;
      }
      if (fabsf(x28_header.mFrameRate - 59.94f) < 0.00001f) {
        x28_header.mFrameRate = 60.f;
      }
      if (fabsf(x28_header.mFrameRate - 60.f) < 0.00001f) {
        x110_26_is60Hz = true;
      }
      xb4_indexLoad->x0_headerRequest =
          x0_dvdFile.AsyncSeekRead(buffer, 32, kSO_Set, x28_header.mCompInfoDataOffsets);
      ++xb4_indexLoad->x10_state;
    } else {
      return true;
    }
  case 1: {
    if (xb4_indexLoad->x0_headerRequest->IsComplete()) {
      memcpy(&x58_thpComponents, buffer, sizeof(THPFrameCompInfo));
      x58_thpComponents.mNumComponents =
          CBasics::SwapBytes(static_cast< uint >(x58_thpComponents.mNumComponents));
      xb4_indexLoad->x0_headerRequest = nullptr;
      uchar* audioBuffer = buffer + 32;
      int offset = x28_header.mCompInfoDataOffsets + sizeof(THPFrameCompInfo);
      for (uint i = 0; i < x58_thpComponents.mNumComponents; ++i) {
        switch (x58_thpComponents.mFrameComp[i]) {
        case 0:
          xb4_indexLoad->x4_videoRequest = x0_dvdFile.AsyncSeekRead(buffer, 32, kSO_Set, offset);
          offset += sizeof(THPVideoInfo);
          break;
        case 1:
          xb4_indexLoad->x8_audioRequest =
              x0_dvdFile.AsyncSeekRead(audioBuffer, 32, kSO_Set, offset);
          offset += sizeof(THPAudioInfo);
          x110_27_hasAudio = true;
          break;
        }
      }
      ++xb4_indexLoad->x10_state;
    } else {
      return true;
    }
  }
  case 2: {
    bool complete = true;
    hasVideo = xb4_indexLoad->x4_videoRequest.get() != nullptr;
    hasAudio = xb4_indexLoad->x8_audioRequest.get() != nullptr;
    if (hasVideo && !xb4_indexLoad->x4_videoRequest->IsComplete()) {
      complete = false;
    }
    if (hasAudio && !xb4_indexLoad->x8_audioRequest->IsComplete()) {
      complete = false;
    }
    if (!complete) {
      return true;
    }
    if (hasVideo) {
      memcpy(&x6c_videoInfo, buffer, sizeof(THPVideoInfo));
      x6c_videoInfo.mXSize = CBasics::SwapBytes(static_cast< uint >(x6c_videoInfo.mXSize));
      x6c_videoInfo.mYSize = CBasics::SwapBytes(static_cast< uint >(x6c_videoInfo.mYSize));
    }
    if (hasAudio) {
      memcpy(&x78_audioInfo, buffer + 32, sizeof(THPAudioInfo));
      x78_audioInfo.mSndChannels =
          CBasics::SwapBytes(static_cast< uint >(x78_audioInfo.mSndChannels));
      x78_audioInfo.mSndFrequency =
          CBasics::SwapBytes(static_cast< uint >(x78_audioInfo.mSndFrequency));
      x78_audioInfo.mSndNumSamples =
          CBasics::SwapBytes(static_cast< uint >(x78_audioInfo.mSndNumSamples));
    }
  }
  }

  xb4_indexLoad = nullptr;
  x88_textures.reserve(3);
  xbc_nextReadOff = x28_header.mMovieDataOffsets;
  xb8_nextReadSize = x28_header.mFirstFrameSize;
  xc0_readSizeWrapped = x28_header.mFirstFrameSize;
  xc4_readOffWrapped = x28_header.mMovieDataOffsets;
  x100_totalSeconds = x28_header.mNumFrames / x28_header.mFrameRate;
  if (x108_preLoadSeconds < 0.f) {
    x108_preLoadSeconds = x100_totalSeconds;
    x10c_preLoadFrames = x28_header.mNumFrames;
  } else if (x108_preLoadSeconds > 0.f) {
    x10c_preLoadFrames =
        rstl::min_val(static_cast< uint >(x28_header.mNumFrames),
                      static_cast< uint >(x108_preLoadSeconds * x28_header.mFrameRate));
    x108_preLoadSeconds = rstl::min_val(x108_preLoadSeconds, x100_totalSeconds);
  }
  if (x10c_preLoadFrames > 0) {
    xa4_requestQueue.reserve(x10c_preLoadFrames);
  }
  PostDVDReadRequestIfNeeded();
  return false;
}

CMoviePlayer::~CMoviePlayer() {
  CancelReadRequests();
  --sNumReferences;
  VerifyCallbackStatus();
  if (sNumReferences == 0 && ShouldEnableLockedCache()) {
    LCDisable();
  }
  if (sAudioPlayer == this) {
    sAudioPlayer = nullptr;
  }
}

void CMoviePlayer::InitializeTextures() {
  const uint ySize = OSRoundUp32B(x6c_videoInfo.mXSize * x6c_videoInfo.mYSize);
  const uint uvSize = OSRoundUp32B(x6c_videoInfo.mXSize * x6c_videoInfo.mYSize / 4);
  const uint audioSize = x28_header.mAudioMaxSamples * 4;
  for (int i = 0; i < x88_textures.capacity(); ++i) {
    void* y = CMemory::Alloc(ySize, IAllocator::kHI_RoundUpLen);
    void* u = CMemory::Alloc(uvSize, IAllocator::kHI_RoundUpLen);
    void* v = CMemory::Alloc(uvSize, IAllocator::kHI_RoundUpLen);
    void* audio = CMemory::Alloc(audioSize, IAllocator::kHI_RoundUpLen);
    DCFlushRangeNoSync(y, ySize);
    DCFlushRangeNoSync(u, uvSize);
    DCFlushRangeNoSync(v, uvSize);
    DCFlushRangeNoSync(audio, audioSize);
    x88_textures.push_back_unsafe(CTHPTextureSet(y, u, v, audio));
  }

  PPCSync();
  xd4_decodedTexSlot = 0;
  xd8_drawTexSlot = -1;
  xdc_audioSlot = -1;
}

void CMoviePlayer::PostDVDReadRequestIfNeeded() {
  CInterruptGuard interrupts;
  if (xc8_curLoadFrame < x28_header.mNumFrames) {
    bool usedPrefetch = false;
    if (!xf8_prefetchRequest.null()) {
      if (xec_prefetchOff == xbc_nextReadOff && xb8_nextReadSize == xf0_prefetchSize &&
          xe8_prefetchFrame == xc8_curLoadFrame) {
        x98_requestBuffer = rstl::auto_ptr< uchar >(xf4_prefetchBuffer.release());
        xa0_request = xf8_prefetchRequest;
        if (xa0_request->IsComplete()) {
          PrefetchNextFrame();
        }
        usedPrefetch = true;
      } else {
        xf8_prefetchRequest = nullptr;
        xf4_prefetchBuffer = nullptr;
        xe8_prefetchFrame = x28_header.mNumFrames;
      }
    }
    if (!usedPrefetch) {
      x98_requestBuffer = rstl::auto_ptr< uchar >(
          static_cast< uchar* >(CMemory::Alloc(xb8_nextReadSize, IAllocator::kHI_RoundUpLen)));
      rstl::single_ptr< CRealDvdRequest > request(rs_new CRealDvdRequest);
      DVDOpen(const_cast< char* >(x0_dvdFile.GetFilename().data()), &request->FileInfo());
      request->FileInfo().cb.userData = this;
      DVDReadAsyncPrio(&request->FileInfo(), x98_requestBuffer.get(), xb8_nextReadSize,
                       xbc_nextReadOff, DVDCallback, 2);
      xa0_request = request;
    }
  }
}

void CMoviePlayer::DVDCallback(s32 result, DVDFileInfo* info) {
  if (result == 10) {
    return;
  }
  DCInvalidateRange(info->cb.addr, info->cb.length);
  lbl_80419B9D = true;
  static_cast< CMoviePlayer* >(info->cb.userData)->HandleDVDInterrupt(info);
}

void CMoviePlayer::HandleDVDInterrupt(DVDFileInfo* info) {
  if (!xf8_prefetchRequest.null() || xa0_request.null() || &xa0_request->FileInfo() != info) {
    return;
  }
  PrefetchNextFrame();
}

void CMoviePlayer::PrefetchNextFrame() {
  xe8_prefetchFrame = xc8_curLoadFrame + 1;
  if (xe8_prefetchFrame != x28_header.mNumFrames) {
    xf0_prefetchSize = *reinterpret_cast< const uint* >(x98_requestBuffer.get());
    xec_prefetchOff = xbc_nextReadOff + xb8_nextReadSize;
    xf4_prefetchBuffer =
        static_cast< uchar* >(CMemory::Alloc(xf0_prefetchSize, IAllocator::kHI_RoundUpLen));
    rstl::single_ptr< CRealDvdRequest > request(rs_new CRealDvdRequest);
    DVDOpen(const_cast< char* >(x0_dvdFile.GetFilename().data()), &request->FileInfo());
    request->FileInfo().cb.userData = this;
    DVDReadAsyncPrio(&request->FileInfo(), xf4_prefetchBuffer.get(), xf0_prefetchSize,
                     xec_prefetchOff, DVDCallback, 2);
    xf8_prefetchRequest = request;
  }
}

void CMoviePlayer::ReadCompleted() {
  CInterruptGuard interrupts;
  xa0_request = nullptr;
  if (xc8_curLoadFrame == xa4_requestQueue.size() && x10c_preLoadFrames > xc8_curLoadFrame) {
    xa4_requestQueue.push_back_unsafe(x98_requestBuffer);
    x114_cachedBytes += xb8_nextReadSize;
  }
  xbc_nextReadOff += xb8_nextReadSize;
  xb8_nextReadSize = CBasics::SwapBytes(*reinterpret_cast< const uint* >(x98_requestBuffer.get()));
  ++xc8_curLoadFrame;
  if (xc8_curLoadFrame == x10c_preLoadFrames) {
    if (xc8_curLoadFrame == x28_header.mNumFrames) {
      xc0_readSizeWrapped = x28_header.mFirstFrameSize;
      xc4_readOffWrapped = x28_header.mMovieDataOffsets;
    } else {
      xc0_readSizeWrapped = xb8_nextReadSize;
      xc4_readOffWrapped = xbc_nextReadOff;
    }
  }
  if (xc8_curLoadFrame >= x28_header.mNumFrames && x110_24_loop) {
    xbc_nextReadOff = xc4_readOffWrapped;
    xb8_nextReadSize = xc0_readSizeWrapped;
    xc8_curLoadFrame = x10c_preLoadFrames;
  }
}

void CMoviePlayer::DecodeFromRead(const void* ptr) {
  uchar work[4096 + 32];
  void* alignedWork = reinterpret_cast< void* >((reinterpret_cast< uintptr_t >(work) + 31) & ~31);
  if (x88_textures.empty()) {
    InitializeTextures();
  }
  CTHPTextureSet& texture = x88_textures[xd4_decodedTexSlot];
  const uint* sizes = static_cast< const uint* >(ptr) + 2;
  const uchar* data = static_cast< const uchar* >(ptr) + 8 + x58_thpComponents.mNumComponents * 4;
  uint offset = 0;
  texture.SetAudioSamplesConsumed(0);
  texture.SetAudioSamples(0);
  for (uint i = 0; i < x58_thpComponents.mNumComponents; ++i) {
    if (x58_thpComponents.mFrameComp[i] == 0) {
      THPVideoDecode(const_cast< uchar* >(data + offset), texture.Y(), texture.U(), texture.V(),
                     alignedWork);
    } else if (x58_thpComponents.mFrameComp[i] == 1) {
      const uint samples = THPAudioDecode(static_cast< short* >(texture.Audio()),
                                          const_cast< uchar* >(data + offset), 0);
      const BOOL interrupts = OSDisableInterrupts();
      texture.SetAudioSamples(samples);
      texture.SetAudioSamplesConsumed(0);
      OSRestoreInterrupts(interrupts);
    }
    offset += CBasics::SwapBytes(*sizes++);
  }
  if (++xd4_decodedTexSlot == x88_textures.size()) {
    xd4_decodedTexSlot = 0;
  }
}

void CMoviePlayer::Update(float dt) {
  if (xc8_curLoadFrame < x10c_preLoadFrames) {
    if (!xa0_request.null() && xa0_request->IsComplete()) {
      ReadCompleted();
      if (xc8_curLoadFrame >= xa4_requestQueue.size() && xc8_curLoadFrame < x10c_preLoadFrames &&
          xa4_requestQueue.size() < x28_header.mNumFrames) {
        PostDVDReadRequestIfNeeded();
      }
    }
  } else if (!xa0_request.null()) {
    const bool canDecode = xcc_requestFrameWrapped >= xa4_requestQueue.size() &&
                           xc8_curLoadFrame >= xa4_requestQueue.size();
    if (xe0_decodedTexCount < 2 && canDecode && xa0_request->IsComplete()) {
      ReadCompleted();
      rstl::auto_ptr< uchar > buffer;
      if (xc8_curLoadFrame < x28_header.mNumFrames) {
        buffer = x98_requestBuffer;
      } else {
        buffer = rstl::auto_ptr< uchar >(x98_requestBuffer.get());
        buffer.release();
      }
      PostDVDReadRequestIfNeeded();
      DecodeFromRead(buffer.get());
      ++xe0_decodedTexCount;
      ++xcc_requestFrameWrapped;
      if (xcc_requestFrameWrapped >= x28_header.mNumFrames && x110_24_loop) {
        xcc_requestFrameWrapped = 0;
      }
    }
  }
  if (xa0_request.null() && xfc_playMode == kPM_Playing &&
      xa4_requestQueue.size() < x28_header.mNumFrames) {
    PostDVDReadRequestIfNeeded();
  }
  if (xe0_decodedTexCount < 2 && xfc_playMode == kPM_Playing &&
      xcc_requestFrameWrapped < x10c_preLoadFrames) {
    const int frame = rstl::min_val(xcc_requestFrameWrapped, xa4_requestQueue.size() - 1);
    if (frame == -1) {
      return;
    }
    DecodeFromRead(xa4_requestQueue[frame].get());
    ++xe0_decodedTexCount;
    ++xcc_requestFrameWrapped;
    if (xcc_requestFrameWrapped >= x28_header.mNumFrames && x110_24_loop) {
      xcc_requestFrameWrapped = 0;
    }
  }
  if (xe0_decodedTexCount > 0 && xfc_playMode == kPM_Playing) {
    x104_curSeconds += dt;
    if (x110_24_loop) {
      x104_curSeconds = CMath::ModF(x104_curSeconds, x100_totalSeconds);
    } else {
      x104_curSeconds = rstl::min_val(x104_curSeconds, x100_totalSeconds);
    }
    float remainder = xe4_frameRem - dt;
    const float frameDt = 1.f / x28_header.mFrameRate;
    if (remainder <= 0.f) {
      if (!x110_28_fieldFlip) {
        if (++xd8_drawTexSlot >= x88_textures.size()) {
          xd8_drawTexSlot = 0;
        }
        const BOOL interrupts = OSDisableInterrupts();
        if (xdc_audioSlot == -1) {
          xdc_audioSlot = 0;
        }
        OSRestoreInterrupts(interrupts);
        --xe0_decodedTexCount;
        ++xd0_curFrame;
        if (xd0_curFrame == x28_header.mNumFrames && x110_24_loop) {
          xd0_curFrame = 0;
        }
        remainder += frameDt;
        x118_fieldIndex = 0;
      } else {
        remainder += dt;
        x110_28_fieldFlip = false;
      }
    }
    xe4_frameRem = remainder;
  }
}

void CMoviePlayer::DrawFrame(const CVector3f& v1, const CVector3f& v2, const CVector3f& v3,
                             const CVector3f& v4) {
  if (xd8_drawTexSlot == -1) {
    return;
  }
  CGraphics::SetUseVideoFilter(x110_25_deinterlace);
  const BOOL interrupts = OSDisableInterrupts();
  sAudioPlayer = this;
  OSRestoreInterrupts(interrupts);
  CTHPTextureSet& texture = x88_textures[xd8_drawTexSlot];
  const bool field = CGraphics::GetDolphinLastFrameAbove();
  MyTHPGXYuv2RgbSetup(field, x110_25_deinterlace || x110_26_is60Hz);
  MyTHPYuv2RgbTextureSetup(texture.Y(), texture.U(), texture.V(), x6c_videoInfo.mXSize,
                           x6c_videoInfo.mYSize);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT7, 4);
  GXPosition3f32(v1.GetX(), v1.GetY(), v1.GetZ());
  GXTexCoord2u16(0, 0);
  GXPosition3f32(v3.GetX(), v3.GetY(), v3.GetZ());
  GXTexCoord2u16(0, 1);
  GXPosition3f32(v4.GetX(), v4.GetY(), v4.GetZ());
  GXTexCoord2u16(1, 1);
  GXPosition3f32(v2.GetX(), v2.GetY(), v2.GetZ());
  GXTexCoord2u16(1, 0);
  CGX::End();
  MyTHPGXRestore();
  if (x118_fieldIndex == 0 && !field && !x110_26_is60Hz) {
    x110_28_fieldFlip = true;
  }
  ++x118_fieldIndex;
}

void CMoviePlayer::SetPlayMode(const EPlayMode mode) { xfc_playMode = mode; }

float CMoviePlayer::GetTotalSeconds() const { return x100_totalSeconds; }

float CMoviePlayer::GetPlayedSeconds() const { return x104_curSeconds + xe4_frameRem; }

bool CMoviePlayer::GetIsFullyCached() const {
  return xa4_requestQueue.size() >= x10c_preLoadFrames;
}

bool CMoviePlayer::GetIsMovieFinishedPlaying() const {
  return !x110_24_loop && xd0_curFrame == x28_header.mNumFrames;
}

void CMoviePlayer::Rewind() {
  CancelReadRequests();
  x98_requestBuffer = rstl::auto_ptr< uchar >(nullptr);
  xf4_prefetchBuffer = nullptr;
  xb8_nextReadSize = x28_header.mFirstFrameSize;
  xbc_nextReadOff = x28_header.mMovieDataOffsets;
  xc0_readSizeWrapped = x28_header.mFirstFrameSize;
  xc4_readOffWrapped = x28_header.mMovieDataOffsets;
  xc8_curLoadFrame = 0;
  xcc_requestFrameWrapped = 0;
  xd0_curFrame = 0;
  xd4_decodedTexSlot = 0;
  xd8_drawTexSlot = -1;
  xdc_audioSlot = -1;
  xe0_decodedTexCount = 0;
  xe4_frameRem = 0.f;
  x104_curSeconds = 0.f;
  x88_textures.clear();
}

void CMoviePlayer::StaticMyAudioCallback() {
  if (sAudioPlayer != nullptr && sAudioPlayer->x110_27_hasAudio) {
    curAudioBuffer = static_cast< const short* >(OSPhysicalToCached(AIGetDMAStartAddr()));
    soundBufferIndex ^= 1;
    short* buffer = soundBuffer[soundBufferIndex];
    AIInitDMA(reinterpret_cast< uintptr_t >(buffer), sizeof(soundBuffer[0]));
    const BOOL interrupts = OSEnableInterrupts();
    if (curAudioBuffer != nullptr) {
      DCInvalidateRange(const_cast< short* >(curAudioBuffer), sizeof(soundBuffer[0]));
    }
    sAudioPlayer->MixAudio(buffer, curAudioBuffer, 160);
    DCFlushRange(buffer, sizeof(soundBuffer[0]));
    OSRestoreInterrupts(interrupts);
  }
}

void CMoviePlayer::MixAudio(short* out, const short* in, unsigned long samples) {
  const short* input = in;
  if (xdc_audioSlot == -1) {
    if (in != nullptr) {
      memcpy(out, in, samples * 4);
    } else {
      memset(out, 0, samples * 4);
    }
    return;
  }
  const uchar volume = rstl::min_val(127, x11c_volume * sSfxVolume * 100 >> 14);
  const ushort attenuation =
      sAudioEnabled ? static_cast< ushort >(CAudioSys::GetScaledVolume(volume)) : 0;
  for (int frame = 0; samples != 0 && frame < 3; ++frame) {
    CTHPTextureSet& texture = x88_textures[xdc_audioSlot];
    uint count = texture.GetAudioSamples() - texture.GetAudioSamplesConsumed();
    if (count > samples) {
      count = samples;
    } else {
      if (++xdc_audioSlot == x88_textures.size()) {
        xdc_audioSlot = 0;
      }
    }
    const uint consumed = texture.GetAudioSamplesConsumed();
    const short* audio = static_cast< short* >(texture.Audio()) + consumed * 2;
    texture.SetAudioSamplesConsumed(count + consumed);
    if (in != nullptr) {
      for (uint i = 0; i < count * 2; ++i) {
        int sample = *input + ((attenuation * *audio) >> 15);
        if (sample < -32768) {
          sample = -32768;
        } else if (sample > 32767) {
          sample = 32767;
        }
        *out = sample;
        ++out;
        ++input;
        ++audio;
      }
    } else {
      for (uint i = 0; i < count * 2; ++i) {
        int sample = (attenuation * *audio) >> 15;
        if (sample < -32768) {
          sample = -32768;
        } else if (sample > 32767) {
          sample = 32767;
        }
        *out = sample;
        ++out;
        ++audio;
      }
    }
    samples -= count;
  }
  if (samples != 0) {
    if (in != nullptr) {
      memcpy(out, input, samples * 4);
    } else {
      memset(out, 0, samples * 4);
    }
  }
}

void CMoviePlayer::VerifyCallbackStatus() {
  if (sNumReferences > 0) {
    CStaticAudioPlayer::RunDMACallback(StaticMyAudioCallback);
  } else {
    CStaticAudioPlayer::CancelDMACallback(StaticMyAudioCallback);
  }
}

uint CMoviePlayer::GetWidth() const { return x6c_videoInfo.mXSize; }

uint CMoviePlayer::GetHeight() const { return x6c_videoInfo.mYSize; }

void CMoviePlayer::SetAudioEnabled(bool enabled) { sAudioEnabled = enabled; }

bool CMoviePlayer::GetAudioEnabled() { return sAudioEnabled; }

void CMoviePlayer::SetSfxVolume(uchar volume) { sSfxVolume = rstl::min_val(uchar(127), volume); }

CMoviePlayer::EPlayMode CMoviePlayer::GetPlayMode() const { return xfc_playMode; }

void CMoviePlayer::DrawFrame(int left, int right, int bottom, int top) {
  const float l = left;
  const float r = right;
  const float b = bottom;
  const float t = top;
  CVector3f v1(l, 0.f, t);
  CVector3f v2(r, 0.f, t);
  CVector3f v3(l, 0.f, b);
  CVector3f v4(r, 0.f, b);
  DrawFrame(v1, v2, v3, v4);
}

void CMoviePlayer::CancelReadRequests() {
  rstl::single_ptr< CRealDvdRequest > prefetch;
  rstl::single_ptr< CRealDvdRequest > request;
  {
    CInterruptGuard interrupts;
    prefetch = xf8_prefetchRequest;
    request = xa0_request;
  }
  if (!prefetch.null()) {
    prefetch->PostCancelRequest();
    prefetch = nullptr;
  }
  if (!request.null()) {
    request->PostCancelRequest();
    request = nullptr;
  }
}
