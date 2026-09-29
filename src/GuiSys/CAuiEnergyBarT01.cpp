#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"
#include "rstl/pair.hpp"

CGuiWidget* CAuiEnergyBarT01::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp,
                                     uint version) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  CAssetId tex = in.Get< CAssetId >();

  CAuiEnergyBarT01* ret = rs_new CAuiEnergyBarT01(parms, sp, tex, true);
  ret->ParseBaseInfo(frame, in, parms, version);
  return ret;
}

CAuiEnergyBarT01::CAuiEnergyBarT01(const CGuiWidgetParms& parms, IObjectStore* sp,
                                   CAssetId textureId, bool loadTexture)
: CGuiWidget(parms)
, mTextureId(textureId)
, mEmptyColor(CColor::White())
, mFilledColor(CColor::White())
, mShadowColor(CColor::White())
, mCoordFunc(nullptr)
, mTesselation(1.f)
, mMaxEnergy(0.f)
, mFilledSpeed(1000.f)
, mShadowSpeed(1000.f)
, mShadowDrainDelay(0.f)
, mAlwaysResetDelayTimer(false)
, mWrapping(false)
, mSetEnergy(0.f)
, mFilledEnergy(0.f)
, mShadowEnergy(0.f)
, mShadowDrainDelayTimer(0.f) {
  if (loadTexture) {
    mTexture = sp->GetObj(SObjectTag('TXTR', mTextureId));
    mTexture->Lock();
  }
}

CAuiEnergyBarT01::~CAuiEnergyBarT01() {}

void CAuiEnergyBarT01::SetMaxEnergy(const float maxEnergy) {
  mMaxEnergy = maxEnergy;
  mSetEnergy = rstl::min_val(mSetEnergy, mMaxEnergy);
  mFilledEnergy = rstl::min_val(mFilledEnergy, mMaxEnergy);
  mShadowEnergy = rstl::min_val(mShadowEnergy, mMaxEnergy);
}

void CAuiEnergyBarT01::SetCurrEnergy(const float energy, const ESetMode mode) {
  float e = CMath::Clamp(0.f, energy, mMaxEnergy);

  if (e == mSetEnergy) {
    return;
  }

  if (mAlwaysResetDelayTimer || mFilledEnergy == mShadowEnergy) {
    mShadowDrainDelayTimer = mShadowDrainDelay;
  }

  mWrapping = mode == kSM_Wrapped;
  mSetEnergy = e;
  if (mode == kSM_Instant) {
    mFilledEnergy = mSetEnergy;
  }
}

void CAuiEnergyBarT01::Update(const float dt) {
  if (mShadowDrainDelayTimer > 0.f) {
    mShadowDrainDelayTimer = rstl::max_val(mShadowDrainDelayTimer - dt, 0.f);
  }

  if (mFilledEnergy < mSetEnergy) {
    if (mWrapping) {
      mFilledEnergy -= dt * mFilledSpeed;
      if (mFilledEnergy < 0.f) {
        mFilledEnergy = rstl::max_val(mSetEnergy, mFilledEnergy + mMaxEnergy);
        mWrapping = false;
        mShadowEnergy = mMaxEnergy;
      }
    } else {
      mFilledEnergy = rstl::min_val(mSetEnergy, mFilledEnergy + dt * mFilledSpeed);
    }
  } else if (mFilledEnergy > mSetEnergy) {
    if (mWrapping) {
      mFilledEnergy += dt * mFilledSpeed;
      if (mFilledEnergy > mMaxEnergy) {
        mFilledEnergy = rstl::min_val(mSetEnergy, mFilledEnergy - mMaxEnergy);
        mWrapping = false;
        mShadowEnergy = mFilledEnergy;
      }
    } else {
      mFilledEnergy = rstl::max_val(mSetEnergy, mFilledEnergy - dt * mFilledSpeed);
    }
  }

  if (mShadowEnergy < mFilledEnergy) {
    mShadowEnergy = mFilledEnergy;
  } else if (mShadowEnergy > mFilledEnergy && mShadowDrainDelayTimer == 0.f) {
    mShadowEnergy = rstl::max_val(mFilledEnergy, mShadowEnergy - dt * mShadowSpeed);
  }

  if (mTexture) {
    mTexture->IsLoaded();
  }

  CGuiWidget::Update(dt);
}

void CAuiEnergyBarT01::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(GetWorldTransform());
  if (!mTexture) {
    return;
  }
  if (!mTexture->IsLoaded() || !mCoordFunc) {
    return;
  }
  CTexture* tex = mTexture->GetObject();
  if (!tex) {
    return;
  }

  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetAmbientColor(CColor::White());
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);

  const float filledFraction = mMaxEnergy > 0.f ? mFilledEnergy / mMaxEnergy : 0.f;
  const float shadowFraction = mMaxEnergy > 0.f ? mShadowEnergy / mMaxEnergy : 0.f;
  const CColor& color = GetModifiedColor();
  CColor filledColor = CColor::Modulate(color, mFilledColor.WithAlphaModulatedBy(parms.GetAlpha()));
  CColor shadowColor = CColor::Modulate(color, mShadowColor.WithAlphaModulatedBy(parms.GetAlpha()));
  CColor emptyColor = CColor::Modulate(color, mEmptyColor.WithAlphaModulatedBy(parms.GetAlpha()));

  static const GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};

  for (int i = 0; i < 3; ++i) {
    const float start = i == 0 ? 0.f : i == 1 ? filledFraction : shadowFraction;
    const float end = i == 0 ? filledFraction : i == 1 ? shadowFraction : 1.f;
    const CColor& useColor = i == 0 ? filledColor : i == 1 ? shadowColor : emptyColor;
    if (start == end) {
      continue;
    }

    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    const int segments = rstl::max_val(1, static_cast< int >(floorf((end - start) / mTesselation)));
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    CGX::SetNumTevStages(1);
    CGX::SetTevKColor(GX_KCOLOR0, useColor.GetGXColor());
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                        GX_PTIDENTITY);
    CGX::SetNumTexGens(1);
    CGX::SetNumChans(0);
    CGX::SetVtxDescv(vtxDesc);
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, (segments + 1) * 2);

    rstl::pair< CVector3f, CVector3f > coord = mCoordFunc(start);
    float t = start;
    for (int j = 0; j < segments; ++j) {
      if (j != 0) {
        coord = mCoordFunc(t);
      }
      GXPosition3f32(coord.first.GetX(), coord.first.GetY(), coord.first.GetZ());
      GXTexCoord2f32(t, 0.f);
      GXPosition3f32(coord.second.GetX(), coord.second.GetY(), coord.second.GetZ());
      GXTexCoord2f32(t, 1.f);
      t += mTesselation;
    }

    coord = mCoordFunc(end);
    GXPosition3f32(coord.first.GetX(), coord.first.GetY(), coord.first.GetZ());
    GXTexCoord2f32(end, 0.f);
    GXPosition3f32(coord.second.GetX(), coord.second.GetY(), coord.second.GetZ());
    GXTexCoord2f32(end, 1.f);
    CGX::End();
  }

  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
}

CGuiWidget::EWidgetUsageFlags CAuiEnergyBarT01::GetWidgetUsageFlags() const {
  return static_cast< EWidgetUsageFlags >(kWUF_Draw | kWUF_Update);
}

FourCC CAuiEnergyBarT01::GetWidgetTypeID() const { return 'ENRG'; }
