#include "GuiSys/CGuiTextPane.hpp"

#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

bool CGuiTextPane::sDrawPaneRects = false;

CGuiTextPane::SFontInfo::SFontInfo(int extentX, int extentY, const CColor& fontColor,
                                   const CColor& outlineColor, CAssetId fontId)
: mExtentX(extentX)
, mExtentY(extentY)
, mFontColor(fontColor)
, mOutlineColor(outlineColor)
, mFontId(fontId) {}

CGuiTextPane::SFontInfo::SFontInfo(CInputStream& in)
: mExtentX(in.ReadInt32())
, mExtentY(in.ReadInt32())
, mFontColor(in)
, mOutlineColor(in)
, mFontId(in.ReadInt32()) {}

CGuiWidget* CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                                 uint version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  float width = in.ReadFloat();
  float height = in.ReadFloat();
  CVector3f scaleCenter(in);

  CGuiTextPane* pane;
  if (version < 2) {
    CAssetId fontId = in.ReadInt32();
    bool wordWrap = in.ReadBool();
    in.ReadBool(); // Legacy horizontal-text setting is no longer used.
    EJustification justification = static_cast< EJustification >(in.ReadInt32());
    EVerticalJustification vertical = static_cast< EVerticalJustification >(in.ReadInt32());
    CColor fontColor(in);
    CColor outlineColor(in);
    int extentX = static_cast< int >(in.ReadFloat());
    int extentY = static_cast< int >(in.ReadFloat());
    CAssetId alternateId = in.ReadInt32();
    int alternateX = in.ReadInt32();
    int alternateY = in.ReadInt32();

    SFontInfo font(extentX, extentY, fontColor, outlineColor, fontId);
    SFontInfo alternate(alternateX, alternateY, fontColor, outlineColor, alternateId);
    CGuiTextProperties properties(wordWrap, justification, vertical);
    pane = rs_new CGuiTextPane(parms, pool, width, height, scaleCenter, properties, font, alternate,
                               false);
  } else {
    bool wordWrap = in.ReadBool();
    EJustification justification = static_cast< EJustification >(in.ReadInt32());
    EVerticalJustification vertical = static_cast< EVerticalJustification >(in.ReadInt32());
    SFontInfo font(in);
    SFontInfo alternate(in);
    bool scaleToViewport = version >= 3 ? in.ReadBool() : false;

    CGuiTextProperties properties(wordWrap, justification, vertical);
    pane = rs_new CGuiTextPane(parms, pool, width, height, scaleCenter, properties, font, alternate,
                               scaleToViewport);
  }
  pane->ParseBaseInfo(frame, in, parms, version);
  pane->InitializeBuffers();
  pane->TextSupport().SetText(rstl::string_l(""));
  return pane;
}

CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* pool, float width,
                           float height, const CVector3f& scaleCenter,
                           const CGuiTextProperties& properties, const SFontInfo& font,
                           const SFontInfo& alternateFont, bool scaleToViewport)
: CGuiPane(parms, width, height, scaleCenter)
, mTextSupport(font.mFontId, font.mExtentX, font.mExtentY, properties, font.mFontColor,
               font.mOutlineColor, CColor::White(), pool)
, mFontInfo(font)
, mAlternateFontInfo(alternateFont)
, mDrawShadow(false)
, mScaleToViewport(scaleToViewport) {}

CGuiTextPane::~CGuiTextPane() {}

void CGuiTextPane::Draw(const CGuiWidgetDrawParms& parms) const {
  if (sDrawPaneRects) {
    CGuiPane::Draw(CGuiWidgetDrawParms(0.2f * parms.GetAlpha(), parms.GetCameraOffset()));
  }
  if (!GetIsVisible()) {
    return;
  }

  const float* vertices = GetVtxBuf();
  CVector2f dimensions = GetDimensions();
  float width = mTextSupport.GetTextBoundingWidth() == 0
                    ? 0.f
                    : dimensions.GetX() / mTextSupport.GetTextBoundingWidth();
  float height = mTextSupport.GetTextBoundingHeight() == 0
                     ? 0.f
                     : dimensions.GetY() / mTextSupport.GetTextBoundingHeight();
  CTransform4f local =
      CTransform4f::Translate(CVector3f(vertices[0], vertices[1], vertices[2]) + GetScaleCenter()) *
      CTransform4f::Scale(width, 1.f, height);
  CTransform4f model = GetWorldTransform() * local;
  CColor color = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());

  if (mDrawShadow) {
    CGraphics::SetModelMatrix(model * CTransform4f::Translate(2.f, 0.f, -2.f));
    mTextSupport.SetGeometryColor(CColor::Black().WithAlphaOf(0.5f * color.GetAlpha()));
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    mTextSupport.Render();
  }
  CGraphics::SetModelMatrix(model);
  mTextSupport.SetGeometryColor(color);
  CGraphics::SetDepthWriteMode(mDepthTest, kE_LEqual, mDepthWrite);

  switch (mDrawFlags) {
  case kGMDF_Shadeless:
  case kGMDF_Opaque:
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
    mTextSupport.Render();
    break;
  case kGMDF_Alpha:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    mTextSupport.Render();
    break;
  case kGMDF_Additive:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    mTextSupport.Render();
    break;
  case kGMDF_AlphaAdditiveOverdraw: {
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    mTextSupport.Render();
    uchar alpha = color.GetAlphau8();
    CColor alphaColor(alpha, alpha, alpha, static_cast< uchar >(255));
    mTextSupport.SetGeometryColor(CColor::Modulate(color, alphaColor));
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    mTextSupport.Render();
    break;
  }
  }
}

void CGuiTextPane::ScaleDimensions(const CVector3f& scale) {}

void CGuiTextPane::SetDimensions(const CVector2f& dim, bool initVBO) {
  CGuiPane::SetDimensions(dim, initVBO);
  if (initVBO) {
    InitializeBuffers();
  }
}

rstl::vector< SObjectTag > CGuiTextPane::GetFontAssets() const {
  return rstl::vector< SObjectTag >(1, SObjectTag('FONT', mTextSupport.GetFontID()));
}

void CGuiTextPane::Update(float dt) {
  CGuiWidget::Update(dt);
  mTextSupport.Update(dt);
}

void CGuiTextPane::Initialize() {
  if (!mScaleToViewport) {
    return;
  }

  CGuiCamera* camera = GetParentFrame()->GetFrameCamera();
  if (camera != nullptr) {
    CVector3f position = GetWorldTransform().GetTranslation();
    CVector2f dimensions = GetDimensions();
    CVector3f farCorner = position + CVector3f(dimensions.GetX(), 0.f, dimensions.GetY());
    CVector3f start = camera->ConvertToScreenSpace(position);
    CVector3f end = camera->ConvertToScreenSpace(farCorner);
    CVector2f startPixel(0.5f * (640.f * start.GetX()), 0.5f * (448.f * start.GetY()));
    CVector2f endPixel(0.5f * (640.f * end.GetX()), 0.5f * (448.f * end.GetY()));
    int extentX =
        static_cast< int >(GetWorldTransform().Get00() * (endPixel.GetX() - startPixel.GetX()));
    int extentY =
        static_cast< int >(GetWorldTransform().Get22() * (endPixel.GetY() - startPixel.GetY()));
    mTextSupport.SetExtentX(extentX);
    mTextSupport.SetExtentY(extentY);
  }
}

CGuiWidget::EWidgetUsageFlags CGuiTextPane::GetWidgetUsageFlags() const {
  return static_cast< EWidgetUsageFlags >(kWUF_Draw | kWUF_Update);
}

FourCC CGuiTextPane::GetWidgetTypeID() const { return 'TXPN'; }
