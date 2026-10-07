#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Text/ScreenText.hpp"
#include "MetaRender/IRenderer.hpp"
#include "rstl/math.hpp"

#include <dolphin/gx.h>
#include <math.h>
#include <stdlib.h>

extern IRenderer* gpRender;

static const CColor& skIdentityColorMultiply = CColor::White();

// Debug name tables; the code using them is dead-stripped in the target, but the strings remain.
static const char* skFilterTypeNames[] = {
    "PassThru   ", "Multiply   ", "Invert     ", "Add        ", "Subtract   ",
    "Blend      ", "WideScreen ", "SceneAdd   ", "NoColor    ",
};
static const char* skFilterShapeNames[] = {
    "FullScreen                      ", "FullScreenHalvesLeftRight       ",
    "FullScreenHalvesTopBottom       ", "FullScreenQuarters              ",
    "CinemaBars                      ", "ScanLinesEven                   ",
    "ScanLinesOdd                    ", "RandomStatic                    ",
    "DialogBox                       ", "CinematicPlaceholderLabel       ",
    "CookieCutterDepthRandomStatic   ",
};
static const char* skBlurTypeNames[] = {"NoBlur  ", "LoBlur  ", "HiBlur  "};

// Guessed names for the original dialog-box settings.
static float sDialogBoxOffsetY = -135.f;
static float sDialogBoxWidth = 600.f;
static float sDialogBoxHeight = 110.f;
static float sDialogBoxBorder = 32.f;

CCameraFilterPass::CCameraFilterPass()
: mCurrentType(kFT_Passthru)
, mNextType(kFT_Passthru)
, mShape(kFS_Fullscreen)
, mDuration(0.f)
, mRemainingTime(0.f)
, mPreviousColor(0xFFFFFFFF)
, mCurrentColor(0xFFFFFFFF)
, mNextColor(0xFFFFFFFF)
, mNextTexture(kInvalidAssetId) {}

void CCameraFilterPass::SetFilter(const EFilterType type, const EFilterShape shape,
                                  const float time, const CColor& color, const CAssetId txtr) {
  if (time == 0.f) {
    mDuration = 0.f;
    mRemainingTime = 0.f;
    mShape = shape;
    mNextType = type;
    mCurrentType = type;
    mNextColor = color;
    mCurrentColor = mNextColor;
    mPreviousColor = mCurrentColor;
    mNextTexture = txtr;

    if (mNextTexture != kInvalidAssetId) {
      mTexture = rs_new TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', txtr)));
    } else {
      mTexture = nullptr;
    }
  } else {
    mNextColor = color;
    mPreviousColor = mCurrentColor;
    mShape = shape;
    mNextTexture = txtr;

    if (mNextTexture != kInvalidAssetId) {
      mTexture = rs_new TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', txtr)));
    }

    mRemainingTime = time;
    mDuration = time;
    mCurrentType = mNextType;
    mNextType = type;

    if (type == kFT_Passthru) {
      if (mCurrentType == kFT_Multiply) {
        mNextColor = skIdentityColorMultiply;
      } else if (mCurrentType == kFT_Add || mCurrentType == kFT_Blend) {
        mNextColor = CColor(mNextColor.GetRed(), mNextColor.GetGreen(), mNextColor.GetBlue(), 0.f);
      }
    } else {
      if (mCurrentType == kFT_Passthru) {
        if (type == kFT_Multiply) {
          mCurrentColor = skIdentityColorMultiply;
        } else if (type == kFT_Add || type == kFT_Blend) {
          mCurrentColor =
              CColor(mNextColor.GetRed(), mNextColor.GetGreen(), mNextColor.GetBlue(), 0.f);
          mPreviousColor = mCurrentColor;
        }
      }
      mCurrentType = mNextType;
    }
  }
}

void CCameraFilterPass::DisableFilter(float time) {
  SetFilter(kFT_Passthru, mShape, time, mNextColor, kInvalidAssetId);
}

void CCameraFilterPass::Update(float dt) {
  if (mRemainingTime > 0.f) {
    mRemainingTime = rstl::max_val(0.f, mRemainingTime - dt);
    mCurrentColor = CColor::Lerp(mNextColor, mPreviousColor, mRemainingTime / mDuration);

    if (mRemainingTime == 0.f) {
      mCurrentType = mNextType;
      if (mCurrentType == kFT_Passthru) {
        mTexture = nullptr;
        mNextTexture = kInvalidAssetId;
      }
    }
  }
}

void CCameraFilterPass::DrawFullScreenColoredQuad(const CColor& color) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  gpRender->SetDepthReadWrite(false, false);
  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(CVector3f(lt.GetX() - 1.f, 0.f, 1.f + rb.GetY()));
  gpRender->PrimVertex(CVector3f(lt.GetX() - 1.f, 0.f, lt.GetY() - 1.f));
  gpRender->PrimVertex(CVector3f(1.f + rb.GetX(), 0.f, 1.f + rb.GetY()));
  gpRender->PrimVertex(CVector3f(1.f + rb.GetX(), 0.f, lt.GetY() - 1.f));
  gpRender->EndPrimitive();
}

void CCameraFilterPass::DrawFullScreenTexturedQuad(const CColor& color, const CTexture* tex,
                                                   float lod) {
  const float u = 0.5f - 0.5f * lod;
  const float v = 0.5f + 0.5f * lod;
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  gpRender->SetDepthReadWrite(false, false);
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(u, v);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(u, u);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.f, lt.GetY() - 1.f));
  CGraphics::StreamTexcoord(v, v);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(v, u);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.f, lt.GetY() - 1.f));
  CGraphics::StreamEnd();
}

void CCameraFilterPass::DrawFullScreenTexturedQuadQuarters(const CColor& color, const CTexture* tex,
                                                           float lod) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  gpRender->SetDepthReadWrite(false, false);
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  }
  CGraphics::SetCullMode(kCM_None);
  for (int i = 0; i < 4; ++i) {
    float x = (i & 1) > 0 ? 1.f : -1.f;
    float z = (i & 2) > 0 ? 1.f : -1.f;
    gpRender->SetModelMatrix(CTransform4f::Scale(x, 0.f, z));
    CGraphics::StreamBegin(kP_TriangleStrip);
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(lod, lod);
    CGraphics::StreamVertex(CVector3f(lt.GetX(), 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(lod, 0.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX(), 0.f, 0.f));
    CGraphics::StreamTexcoord(0.f, lod);
    CGraphics::StreamVertex(CVector3f(0.f, 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(0.f, 0.f);
    CGraphics::StreamVertex(CVector3f(0.f, 0.f, 0.f));
    CGraphics::StreamEnd();
  }
  CGraphics::SetCullMode(kCM_Front);
}

void CCameraFilterPass::DrawScanLines(const CColor& color, bool even) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  float offset = even ? 0.f : 2.f;
  int count = static_cast< int >((rb.GetY() - lt.GetY()) / 4.f);
  CGraphics::SetLineWidth(2.f, kTO_One);
  gpRender->BeginLines(count * 2);
  gpRender->PrimColor(color);
  for (int i = 0; i < count; ++i) {
    float fi = 4.f * static_cast< float >(i);
    gpRender->PrimVertex(CVector3f(lt.GetX(), 0.f, fi + lt.GetY() + offset));
    gpRender->PrimVertex(CVector3f(rb.GetX(), 0.f, fi + lt.GetY() + offset));
  }
  gpRender->EndPrimitive();
  CGraphics::SetLineWidth(1.f, kTO_One);
}

float CCameraFilterPass::GetT(bool invert) const {
  float tmp;
  if (mDuration == 0.f) {
    tmp = 1.f;
  } else {
    tmp = 1.f - mRemainingTime / mDuration;
  }
  if (invert) {
    return 1.f - tmp;
  }
  return tmp;
}

void CCameraFilterPass::DrawWideScreen(const CColor& color, const CTexture* tex, float lod) {
  const rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  float barHeight = 44.f * CGraphics::GetPixelAspectRatio();
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  }
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

  {
    CGraphics::StreamBegin(kP_TriangleStrip);
    float v = static_cast< float >(rand() % 16384) / 16384.f;
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(v, 1.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, lt.GetY() - -(barHeight * lod)));
    CGraphics::StreamTexcoord(v, 0.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, lt.GetY()));
    CGraphics::StreamTexcoord(1.f + v, 1.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, lt.GetY() - -(barHeight * lod)));
    CGraphics::StreamTexcoord(1.f + v, 0.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, lt.GetY()));
    CGraphics::StreamEnd();
  }
  {
    CGraphics::StreamBegin(kP_TriangleStrip);
    float v = static_cast< float >(rand() % 16384) / 16384.f;
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(v, 0.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(v, 1.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, rb.GetY() - (barHeight * lod)));
    CGraphics::StreamTexcoord(1.f + v, 0.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(1.f + v, 1.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, rb.GetY() - (barHeight * lod)));
    CGraphics::StreamEnd();
  }
}

void CCameraFilterPass::DrawRandomStatic(const CColor& color, float alpha, bool cookieCutterDepth) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, 0.f, 1.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;

  if (cookieCutterDepth) {
    CGraphics::SetAlphaCompare(kAF_GEqual, CCast::ToUint8((1.f - alpha) * 255.f), kAO_And,
                               kAF_Always, 0);
    gpRender->SetDepthReadWrite(true, true);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::LoadDolphinSpareTexture(
        static_cast< int >(2.f + (rb.GetX() - lt.GetX())),
        static_cast< int >(2.f + (rb.GetY() - lt.GetY())), GX_TF_IA4,
        reinterpret_cast< void* >(((rand() + 0x1f) & ~0x1f) + 0x8000), GX_TEXMAP0);
  } else {
    gpRender->SetDepthReadWrite(false, false);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulateColor);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::LoadDolphinSpareTexture(
        static_cast< int >(2.f + (rb.GetX() - lt.GetX())),
        static_cast< int >(2.f + (rb.GetY() - lt.GetY())), GX_TF_IA4,
        reinterpret_cast< void* >(((rand() + 0x1f) & ~0x1f) + 0x8000), GX_TEXMAP0);
  }

  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.01f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.01f, lt.GetY() - 1.f));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.01f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.01f, lt.GetY() - 1.f));
  CGraphics::StreamEnd();

  if (cookieCutterDepth) {
    CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  }
}

void CCameraFilterPass::DrawDialogBox(const CColor& color, const CTexture* texture, float alpha) {
  const rstl::pair< CVector2f, CVector2f > viewport =
      gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const float scaleX = (viewport.second.GetX() - viewport.first.GetX()) / 640.f;
  const float scaleY = (viewport.second.GetY() - viewport.first.GetY()) / 448.f;
  const float halfWidth = 0.5f * sDialogBoxWidth;
  const float halfHeight = 0.5f * sDialogBoxHeight;
  const float innerWidth = halfWidth - sDialogBoxBorder;
  const float innerHeight = halfHeight - sDialogBoxBorder;

  gpRender->SetDepthReadWrite(false, false);
  CTransform4f transform = CTransform4f::Translate(0.f, 0.f, sDialogBoxOffsetY);
  transform = transform * CTransform4f::Scale(scaleX, 1.f, scaleY);
  gpRender->SetModelMatrix(transform);
  if (texture != nullptr) {
    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

  const float x[] = {-halfWidth, -innerWidth, innerWidth, halfWidth};
  const float y[] = {halfHeight, innerHeight, -innerHeight, -halfHeight};
  const float u[] = {0.f, 1.f / 3.f, 1.f - 1.f / 3.f, 1.f};
  const float v[] = {1.f, 1.f - 1.f / 3.f, 1.f / 3.f, 0.f};
  CColor fadedColor = color;
  fadedColor.SetAlpha(static_cast< uchar >(alpha * static_cast< float >(color.GetAlphau8())));
  CGraphics::StreamBegin(kP_Quads);
  CGraphics::StreamColor(fadedColor);
  for (uint column = 0; column < 3; ++column) {
    for (uint row = 0; row < 3; ++row) {
      CGraphics::StreamTexcoord(u[column], v[row]);
      CGraphics::StreamVertex(x[column], 0.f, y[row]);
      CGraphics::StreamTexcoord(u[column], v[row + 1]);
      CGraphics::StreamVertex(x[column], 0.f, y[row + 1]);
      CGraphics::StreamTexcoord(u[column + 1], v[row + 1]);
      CGraphics::StreamVertex(x[column + 1], 0.f, y[row + 1]);
      CGraphics::StreamTexcoord(u[column + 1], v[row]);
      CGraphics::StreamVertex(x[column + 1], 0.f, y[row]);
    }
  }
  CGraphics::StreamEnd();
}

void CCameraFilterPass::DrawCinematicPlaceholderLabel(const CColor& color, const CTexture* texture,
                                                      float alpha) {
  const CViewport viewport = CGraphics::GetViewport();
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  ScreenText::DrawString(rstl::string(rstl::string::literal_t(), "CINEMATIC PLACEHOLDER"),
                         static_cast< int >(viewport.mHalfWidth) - 112,
                         static_cast< int >(viewport.mHalfHeight), *gpDefaultFont);
}

void CCameraFilterPass::Draw() const {
  float t = GetT(mNextType == kFT_Passthru);
  CTexture* tex = mTexture.null() ? nullptr : **mTexture;
  DrawFilter(mCurrentType, mShape, mCurrentColor, tex, t);
}

void CCameraFilterPass::DrawFilter(EFilterType type, EFilterShape shape, const CColor& color,
                                   const CTexture* tex, float lod) {
  if (type == kFT_Passthru) {
    return;
  }
  switch (type) {
  case kFT_Passthru:
  default:
    return;
  case kFT_Multiply:
    gpRender->SetBlendMode_ColorMultiply();
    break;
  case kFT_Invert:
    gpRender->SetBlendMode_InvertDst();
    break;
  case kFT_Add:
    gpRender->SetBlendMode_AdditiveAlpha();
    break;
  case kFT_Blend:
    gpRender->SetBlendMode_AlphaBlended();
    break;
  case kFT_Subtract:
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
    break;
  case kFT_Widescreen:
    return;
  case kFT_SceneAdd:
    gpRender->SetBlendMode_AdditiveDestColor();
    break;
  case kFT_NoColor:
    gpRender->SetBlendMode_NoColorWrite();
    break;
  }
  DrawFilterShape(shape, color, tex, lod);
  gpRender->SetBlendMode_AlphaBlended();
}

float CCameraFilterPass::GetWidthScale() const { return 1.f; }

float CCameraFilterPass::GetHeightScale() const {
  if (mShape == kFS_CinemaBars && mCurrentType != kFT_Passthru) {
    const float t = GetT(mNextType == kFT_Passthru);
    return (1.f - 88.f / 448.f) * t + (1.f - t);
  }
  return 1.f;
}

void CCameraFilterPass::DrawFilterShape(EFilterShape shape, const CColor& color,
                                        const CTexture* tex, float lod) {
  switch (shape) {
  case kFS_ScanLinesEven:
    DrawScanLines(color, true);
    break;
  case kFS_ScanLinesOdd:
    DrawScanLines(color, false);
    break;
  case kFS_Fullscreen:
  case kFS_FullscreenHalvesLeftRight:
  case kFS_FullscreenHalvesTopBottom:
  default:
    if (tex != nullptr) {
      DrawFullScreenTexturedQuad(color, tex, lod);
    } else {
      DrawFullScreenColoredQuad(color);
    }
    break;
  case kFS_FullscreenQuarters:
    if (tex != nullptr) {
      DrawFullScreenTexturedQuadQuarters(color, tex, lod);
    } else {
      DrawFullScreenColoredQuad(color);
    }
    break;
  case kFS_CinemaBars:
    DrawWideScreen(color, tex, lod);
    break;
  case kFS_RandomStatic:
    DrawRandomStatic(color, 1.f, false);
    break;
  case kFS_DialogBox:
    DrawDialogBox(color, tex, lod);
    break;
  case kFS_CinematicPlaceholderLabel:
    DrawCinematicPlaceholderLabel(color, tex, lod);
    break;
  case kFS_CookieCutterDepthRandomStatic:
    DrawRandomStatic(color, lod, true);
    break;
  }
}

CCameraBlurPass::CCameraBlurPass()
: mCurrentType(kBT_NoBlur)
, mNextType(kBT_NoBlur)
, mPreviousValue(0.f)
, mCurrentValue(0.f)
, mNextValue(0.f)
, mDuration(0.f)
, mRemainingTime(0.f)
, mUsePersistent(false)
, mNoPersistentCopy(false) {}

void CCameraBlurPass::Update(float dt) {
  if (mRemainingTime > 0.f) {
    mRemainingTime = rstl::max_val(0.f, mRemainingTime - dt);
    const float t = mRemainingTime / mDuration;
    mCurrentValue = mNextValue * (1.f - t) + mPreviousValue * t;
    if (mRemainingTime == 0.f) {
      if (mNextType == kBT_NoBlur && mCurrentType != kBT_NoBlur && mUsePersistent) {
        FreePersistentFbTexture();
      }
      mCurrentType = mNextType;
    }
  }
}

void CCameraBlurPass::SetBlur(EBlurType type, float amount, float duration, bool usePersistentFb) {
  if (duration == 0.f) {
    mRemainingTime = 0.f;
    mDuration = 0.f;
    mNextValue = amount;
    mCurrentValue = amount;
    mPreviousValue = amount;
    if (mCurrentType == kBT_NoBlur) {
      if (type != kBT_NoBlur && usePersistentFb) {
        AllocatePersistentFbTexture();
      }
    } else if (type == kBT_NoBlur && mUsePersistent) {
      FreePersistentFbTexture();
    }
    mNextType = type;
    mCurrentType = type;
    mUsePersistent = usePersistentFb;
  } else {
    mUsePersistent = usePersistentFb;
    mDuration = duration;
    mRemainingTime = duration;
    mPreviousValue = mCurrentValue;
    mNextValue = amount;
    if (type != mNextType) {
      if (mCurrentType == kBT_NoBlur) {
        if (mUsePersistent) {
          AllocatePersistentFbTexture();
        }
        mCurrentType = type;
      }
      mNextType = type;
    }
  }
}

void CCameraBlurPass::DisableBlur(float duration) {
  SetBlur(kBT_NoBlur, 0.f, duration, mUsePersistent);
}

void CCameraBlurPass::Draw() const {
  if (mCurrentType == kBT_NoBlur) {
    return;
  }
  const CViewport& viewport = CGraphics::GetViewport();
  const int width = viewport.mWidth >> 1;
  const int height = viewport.mHeight >> 1;
  void* buffer = CGraphics::GetDolphinSpareBuffer();
  if (!mNoPersistentCopy || !mUsePersistent) {
    GetFbCopy(GX_TF_RGB565);
    mNoPersistentCopy = true;
  }
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, buffer,
                                     CGraphics::kSpareBufferTexMapID);

  const float left = static_cast< float >(viewport.mLeft);
  const float top = static_cast< float >(viewport.mTop);
  const float right = left + static_cast< float >(width);
  const float bottom = top + static_cast< float >(height);
  CGraphics::SetOrtho(left, right, bottom, top, -1.f, 1.f);
  gpRender->SetDepthReadWrite(false, false);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_AlphaBlended();

  const GXVtxDescList descriptors[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(descriptors);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(7);
  CGX::SetNumTevStages(7);
  for (int i = 0; i < 7; ++i) {
    const GXTevStageID stage = static_cast< GXTevStageID >(i);
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    CGX::SetStandardTevColorAlphaOp(stage);
    CGX::SetTevKColorSel(stage, GX_TEV_KCSEL_K0);
  }
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  for (int i = 0; i < 8; ++i) {
    CGX::SetTevOrder(static_cast< GXTevStageID >(i), static_cast< GXTexCoordID >(i),
                     CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  }

  const float alpha = mUsePersistent ? 1.f : rstl::min_val(1.f, 0.5f * mCurrentValue);
  const CColor weight(1.f / 7.f, 1.f / 7.f, 1.f / 7.f, alpha);
  CGX::SetTevKColor(GX_KCOLOR0, weight.GetGXColor());
  for (int i = 0; i < 7; ++i) {
    const float angle = M_2PIF * static_cast< float >(i - 1) / 6.f;
    const float dx = i == 0 ? 0.f : (mCurrentValue / static_cast< float >(width)) * cosf(angle);
    const float dy = i == 0 ? 0.f : (mCurrentValue / static_cast< float >(height)) * sinf(angle);
    const float matrix[2][4] = {{1.f, 0.f, 0.f, dx}, {0.f, 1.f, 0.f, dy}};
    CGX::LoadTexMtxImm(matrix, GX_TEXMTX0 + i * 3, GX_MTX2x4);
  }
  for (int i = 0; i < 8; ++i) {
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(i), GX_TG_MTX2x4, GX_TG_TEX0,
                        static_cast< GXTexMtx >(GX_TEXMTX0 + i * 3), GX_FALSE, GX_PTIDENTITY);
  }
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, buffer, GX_TEXMAP0);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(left - 1.f, 0.f, bottom + 1.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(left - 1.f, 0.f, top - 1.f);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(right + 1.f, 0.f, bottom + 1.f);
  GXTexCoord2f32(1.f, 0.f);
  GXPosition3f32(right + 1.f, 0.f, top - 1.f);
  GXTexCoord2f32(1.f, 1.f);
  CGX::End();
  for (int i = 0; i < 8; ++i) {
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(i), GX_TG_MTX2x4,
                        static_cast< GXTexGenSrc >(GX_TG_TEX0 + i), GX_IDENTITY, GX_FALSE,
                        GX_PTIDENTITY);
  }
  gpRender->SetBlendMode_AlphaBlended();
  gpRender->SetDepthReadWrite(true, true);
}

void CCameraBlurPass::GetFbCopy(GXTexFmt format) {
  const CViewport& viewport = CGraphics::GetViewport();
  const int width = viewport.mWidth;
  const int height = viewport.mHeight;
  GXSetTexCopySrc(static_cast< ushort >(viewport.mLeft), static_cast< ushort >(viewport.mTop),
                  static_cast< ushort >(width), static_cast< ushort >(height));
  if (format == GX_TF_RGB565) {
    GXGetTexBufferSize(width / 2, height / 2, GX_TF_RGB565, GX_FALSE, 0);
  } else {
    GXGetTexBufferSize(width, height, GX_TF_I8, GX_FALSE, 0);
  }
  if (format == GX_TF_RGB565) {
    GXSetTexCopyDst(width / 2, height / 2, format, GX_TRUE);
  } else {
    GXSetTexCopyDst(width, height, format, GX_FALSE);
  }
  GXCopyTex(CGraphics::GetDolphinSpareBuffer(), GX_FALSE);
  GXPixModeSync();
}

void CCameraBlurPass::AllocatePersistentFbTexture() {}

void CCameraBlurPass::FreePersistentFbTexture() { mNoPersistentCopy = false; }
