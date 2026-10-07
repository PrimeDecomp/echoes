#include "GuiSys/CGuiModel.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameModelDatabase.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

// Guessed names: shared rendering state for a frame's model-widget batch.
static int sDrawFlags = 7;
static bool sDrawing;
static CColor sDrawColor = CColor::Black();

void CGuiModel::UpdateDrawState(int drawFlags, CColor color) {
  if (sDrawFlags != drawFlags) {
    sDrawColor = color;
    sDrawFlags = drawFlags;
    CCubeMaterial::ResetTransparencyKColor();
    CCubeMaterial::ResetCachedMaterials();
  }

  if (!(color == sDrawColor)) {
    if (sDrawFlags == kGMDF_Alpha || sDrawFlags == kGMDF_Additive || drawFlags == kGMDF_Additive ||
        drawFlags == kGMDF_Alpha) {
      sDrawFlags = drawFlags;
      sDrawColor = color;
      CCubeMaterial::ResetTransparencyKColor();
      CCubeMaterial::ResetCachedMaterials();
    } else {
      const int kColor = CCubeMaterial::GetTransparencyKColor();
      if (kColor != -1) {
        CGX::SetTevKColor(static_cast< GXTevKColorID >(kColor), color.GetGXColor());
        sDrawColor = color;
      }
    }
  }
}

void CGuiModel::BeginDraw() {
  CCubeMaterial::ResetCachedMaterials();
  CCubeMaterial::ResetTransparencyKColor();
  UpdateDrawState(-1, CColor::Black());
  sDrawing = true;
}

void CGuiModel::EndDraw() { sDrawing = false; }

CGuiModel* CGuiModel::Create(CGuiFrame* frame, CInputStream& in, uint version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  const CAssetId modelId = in.Get< CAssetId >();
  const int modelIndex = in.Get< int >();
  const uint lightMask = in.Get< uint >();

  CGuiModel* model = rs_new CGuiModel(parms, modelId, lightMask, modelIndex);
  model->ParseBaseInfo(frame, in, parms, version);
  return model;
}

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, CAssetId modelId, uint lightMask, int modelIndex)
: CGuiWidget(parms), mModelId(modelId), mModelIndex(modelIndex), mLightMask(lightMask) {}

CGuiModel::~CGuiModel() {}

void CGuiModel::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(GetWorldTransform());
  if (GetIsVisible()) {
    if (!sDrawing) {
      CCubeMaterial::ResetCachedMaterials();
      CCubeMaterial::ResetTransparencyKColor();
      UpdateDrawState(-1, CColor::Black());
    }

    const CColor color = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
    UpdateDrawState(mDrawFlags, color);
    GetParentFrame()->EnableLights(mLightMask);
    const bool cullChanged = mCullFaces;
    if (cullChanged) {
      CGraphics::SetCullMode(kCM_Front);
    }

    CModelFlags flags = CModelFlags::Normal();
    bool doDraw = true;
    switch (mDrawFlags) {
    case kGMDF_Shadeless:
      flags = CModelFlags::Normal();
      break;
    case kGMDF_Opaque:
      flags = CModelFlags::ColorModulate(color);
      break;
    case kGMDF_Alpha:
      flags = CModelFlags::AlphaBlended(color).DepthCompareUpdate(mDepthTest, mDepthWrite);
      flags = CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_Unknown400);
      break;
    case kGMDF_Additive:
      flags = CModelFlags::Additive(color).DepthCompareUpdate(mDepthTest, mDepthWrite);
      flags = CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_Unknown400);
      break;
    case kGMDF_AlphaAdditiveOverdraw:
      flags = CModelFlags::AlphaBlended(color).DepthCompareUpdate(mDepthTest, false);
      if (mDepthGreater) {
        flags = flags.DepthBackwards();
      }
      DrawModel(flags);
      flags = CModelFlags::AdditiveRGB(color).DepthCompareUpdate(mDepthTest, mDepthWrite);
      if (mDepthGreater) {
        flags = flags.DepthBackwards();
      }
      DrawModel(flags);
      doDraw = false;
      break;
    case kGMDF_ClearAlpha:
      CGX::SetAlphaUpdate(true);
      CGX::SetDstAlpha(true, 0);
      CGX::SetColorUpdate(true);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1_8);
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                          GX_PTIDENTITY);
      CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
      CGX::SetNumTevStages(1);
      CGX::SetNumTexGens(1);
      CGX::SetNumChans(0);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
      CGX::SetZMode(false, GX_ALWAYS, false);
      GetParentFrame()->GetModelDatabase()->GetModel(mModelIndex)->DrawFlat(kSS_All);
      CGX::SetAlphaUpdate(false);
      CGX::SetColorUpdate(true);
      doDraw = false;
      break;
    case kGMDF_DoubleColor:
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
      CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
      CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                          GX_PTIDENTITY);
      CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
      CGX::SetNumTevStages(1);
      CGX::SetNumTexGens(1);
      CGX::SetNumChans(0);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTCLR, GX_BL_SRCCLR, GX_LO_CLEAR);
      CGX::SetZMode(false, GX_ALWAYS, false);
      GetParentFrame()->GetModelDatabase()->GetModel(mModelIndex)->DrawFlat(kSS_All);
      doDraw = false;
      break;
    default:
      doDraw = false;
      break;
    }

    if (doDraw) {
      if (mDepthGreater) {
        flags = flags.DepthBackwards();
      }
      DrawModel(flags);
    }

    if (cullChanged) {
      CGraphics::SetCullMode(kCM_None);
    }
  }

  CGuiWidget::Draw(parms);
}

void CGuiModel::DrawModel(const CModelFlags& flags) const {
  GetParentFrame()->GetModelDatabase()->Draw(mModelIndex, flags);
}

CGuiWidget::EWidgetUsageFlags CGuiModel::GetWidgetUsageFlags() const { return kWUF_PreDraw; }

FourCC CGuiModel::GetWidgetTypeID() const { return 'MODL'; }
