#include "GuiSys/CAuiBitmapMeter.hpp"

#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "dolphin/gx/GXVert.h"
#include "rstl/math.hpp"

CAuiBitmapMeter::CAuiBitmapMeter(const CGuiWidgetParms& parms, CSimplePool* pool,
                                 CAssetId textureId,
                                 const rstl::reserved_vector< CVector3f, 4 >& coords,
                                 const rstl::reserved_vector< CVector2f, 4 >& uvs, bool loadTexture)
: CGuiWidget(parms)
, mCoords(coords)
, mUvs(uvs)
, mTextureId(textureId)
, mShadowColor(CColor::White())
, mTargetFraction(0.f)
, mCurrentFraction(0.f)
, mShadowFraction(0.f)
, mIncreaseSpeed(1.f)
, mDecreaseSpeed(1.f)
, mShadowDrainSpeed(0.75f) {
  if (loadTexture) {
    mTexture = TCachedToken< CTexture >(pool->GetObj(SObjectTag('TXTR', mTextureId)));
    mTexture->Lock();
  }
}

CAuiBitmapMeter::~CAuiBitmapMeter() {}

CGuiWidget* CAuiBitmapMeter::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                                    uint version) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  rstl::reserved_vector< CVector3f, 4 > coords(in);
  rstl::reserved_vector< CVector2f, 4 > uvs(in);
  CAssetId textureId = in.Get< CAssetId >();
  CAuiBitmapMeter* widget = rs_new CAuiBitmapMeter(parms, pool, textureId, coords, uvs, true);
  widget->ParseBaseInfo(frame, in, parms, version);
  return widget;
}

void CAuiBitmapMeter::SetTargetFraction(float fraction) { mTargetFraction = fraction; }

void CAuiBitmapMeter::SetCurrentFraction(float fraction) { mCurrentFraction = fraction; }

float CAuiBitmapMeter::GetCurrentFraction() const { return mCurrentFraction; }

float CAuiBitmapMeter::GetShadowFraction() const { return mShadowFraction; }

void CAuiBitmapMeter::SetShadowColor(const CColor& color) { mShadowColor = color; }

void CAuiBitmapMeter::SetIncreaseSpeed(float speed) { mIncreaseSpeed = speed; }

void CAuiBitmapMeter::SetDecreaseSpeed(float speed) { mDecreaseSpeed = speed; }

void CAuiBitmapMeter::Update(float dt) {
  if (mTexture) {
    mTexture->IsLoaded();
  }

  if (mTargetFraction < mCurrentFraction) {
    mCurrentFraction = rstl::max_val(mTargetFraction, mCurrentFraction - mDecreaseSpeed * dt);
  } else {
    mCurrentFraction = rstl::min_val(mTargetFraction, mCurrentFraction + mIncreaseSpeed * dt);
  }

  if (mCurrentFraction < mShadowFraction) {
    mShadowFraction = rstl::max_val(mCurrentFraction, mShadowFraction - mShadowDrainSpeed * dt);
  } else {
    mShadowFraction = mCurrentFraction;
  }

  CGuiWidget::Update(dt);
}

void CAuiBitmapMeter::Draw(const CGuiWidgetDrawParms& parms) const {
  if (!GetIsVisible() || !mTexture || !mTexture->IsLoaded()) {
    return;
  }
  if (close_enough(mCurrentFraction, 0.f) && close_enough(mShadowFraction, 0.f)) {
    return;
  }
  const CTexture* texture = mTexture->GetObject();
  if (!texture) {
    return;
  }

  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetAmbientColor(CColor::White());
  texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  switch (mDrawFlags) {
  case kGMDF_Shadeless:
  case kGMDF_Opaque:
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
    break;
  case kGMDF_Alpha:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    break;
  case kGMDF_Additive:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    break;
  default:
    break;
  }

  const CColor color = GetModifiedColor();
  const CColor currentColor =
      CColor::Modulate(color, GetColor().WithAlphaModulatedBy(parms.GetAlpha()));
  const CColor shadowColor =
      CColor::Modulate(color, mShadowColor.WithAlphaModulatedBy(parms.GetAlpha()));
  CGraphics::SetModelMatrix(GetWorldTransform());
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTevStages(1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  GXVtxDescList desc[] = {{GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);

  for (int bar = 0; bar < 2; ++bar) {
    CGX::SetTevKColor(GX_KCOLOR0, (bar == 0 ? shadowColor : currentColor).GetGXColor());
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    const float fraction = bar == 0 ? mShadowFraction : mCurrentFraction;
    const float remainder = 1.f - fraction;
    for (int i = 0; i < 4; ++i) {
      const CVector3f pos =
          i % 2 == 0 ? mCoords[i] * fraction + mCoords[i + 1] * remainder : mCoords[i];
      const CVector2f uv = i % 2 == 0 ? mUvs[i] * fraction + mUvs[i + 1] * remainder : mUvs[i];
      GXPosition3f32(pos.GetX(), pos.GetY(), pos.GetZ());
      GXTexCoord2f32(uv.GetX(), uv.GetY());
    }
    CGX::End();
  }

  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
}

CGuiWidget::EWidgetUsageFlags CAuiBitmapMeter::GetWidgetUsageFlags() const {
  return static_cast< EWidgetUsageFlags >(kWUF_Draw | kWUF_Update);
}

FourCC CAuiBitmapMeter::GetWidgetTypeID() const { return 'BMTR'; }
