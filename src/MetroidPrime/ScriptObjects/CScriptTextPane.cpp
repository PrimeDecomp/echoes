#include "MetroidPrime/ScriptObjects/CScriptTextPane.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTextPane.hpp"

CScriptTextPane::CScriptTextPane(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CVector3f& pivotOffset, float width, float height, int extentX, int extentY,
    const CColor& fontColor, const CColor& outlineColor, const CColor& geometryColor, CAssetId font,
    CAssetId defaultString, const rstl::string& defaultStringName, int blendMode, float fadeInTime,
    float fadeOutTime, const CGuiTextProperties& textProperties, const rstl::string& guiLabel,
    bool depthCompare, bool depthUpdate, bool depthBackwards)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mFadeOpacity(info.GetActive() ? 1.f : 0.f)
, mPivotOffset(pivotOffset)
, mWidth(width)
, mHeight(height)
, mTextSupport(font, extentX, extentY, textProperties, fontColor, outlineColor, geometryColor,
               gpSimplePool)
, mBlendMode(blendMode)
, mDefaultString(defaultString == kInvalidAssetId
                     ? rstl::optional_object_null()
                     : rstl::optional_object< TLockedToken< CStringTable > >(
                           gpSimplePool->GetObj(SObjectTag('STRG', defaultString))))
, mStringIndex(0)
, mGuiLabel(guiLabel)
, mFadeInTime(fadeInTime)
, mFadeOutTime(fadeOutTime)
, mRenderScale(1.f)
, mTargetRenderScale(1.f)
, mDepthCompare(depthCompare)
, mDepthUpdate(depthUpdate)
, mDepthBackwards(depthBackwards) {
  switch (blendMode) {
  case 0:
    SetModelFlags(CModelFlags::ColorModulate(geometryColor));
    break;
  case 1:
    SetModelFlags(CModelFlags::AlphaBlended(geometryColor));
    // The original has no break here, so blend mode 1 ends up additive.
  case 2:
    SetModelFlags(CModelFlags::Additive(geometryColor));
    break;
  }

  if (mDefaultString) {
    if (defaultStringName.size() != 0) {
      const int index = (*mDefaultString)->GetStringIndex(defaultStringName.c_str());
      if (index != -1) {
        mStringIndex = index;
      }
    }
    mTextSupport.SetText(rstl::wstring_l((*mDefaultString)->GetString(mStringIndex)));
  }
}

void CScriptTextPane::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_Increment:
    if (!GetActive()) {
      mgr.SendScriptMsg(this, GetUniqueId(), kSM_Activate, kInvalidUniqueId);
      mFadeOpacity = FLT_EPSILON;
    }
    break;
  case kSM_Decrement:
    mFadeOpacity = -(1.f - FLT_EPSILON);
    break;
  default:
    break;
  }
}

void CScriptTextPane::Think(float dt, CStateManager& mgr) {
  if (mFadeOpacity < 0.f) {
    const float opacity = mFadeOpacity + 1.f / mFadeOutTime * dt;
    mFadeOpacity = opacity >= 0.f ? 0.f : opacity;
    if (mFadeOpacity == 0.f) {
      mgr.SendScriptMsg(this, GetUniqueId(), kSM_Deactivate, kInvalidUniqueId);
    }
  } else if (mFadeOpacity < 1.f) {
    const float opacity = mFadeOpacity + 1.f / mFadeInTime * dt;
    mFadeOpacity = opacity >= 1.f ? 1.f : opacity;
  }

  if (mRenderScale < mTargetRenderScale) {
    const float scale = mRenderScale + 8.f * dt;
    mRenderScale = scale >= mTargetRenderScale ? mTargetRenderScale : scale;
  } else if (mRenderScale > mTargetRenderScale) {
    const float scale = mRenderScale - 8.f * dt;
    mRenderScale = mTargetRenderScale >= scale ? mTargetRenderScale : scale;
  }
}

void CScriptTextPane::PreRender(CStateManager& mgr) {
  mTextSupport.SetGeometryColor(
      GetModelFlags().GetColor().WithAlphaModulatedBy(fabs(mFadeOpacity)));
}

void CScriptTextPane::AddToRenderer(const CStateManager& mgr) const { EnsureRendered(mgr); }

void CScriptTextPane::Render(const CStateManager& mgr) const {
  const CVector2f dimensions(mWidth, mHeight);
  const int extentX = mTextSupport.GetTextBoundingWidth();
  const float scaleX = extentX != 0 ? dimensions.GetX() / extentX : 0.f;
  const int extentY = mTextSupport.GetTextBoundingHeight();
  const float scaleY = extentY != 0 ? dimensions.GetY() / extentY : 0.f;
  const CVector3f offset(-1.f * mPivotOffset.GetX() * mRenderScale,
                         -1.f * mPivotOffset.GetY() * mRenderScale,
                         mPivotOffset.GetZ() * mRenderScale);
  const CTransform4f local = CTransform4f::Translate(offset) *
                             CTransform4f::Scale(scaleX * mRenderScale, 1.f, scaleY * mRenderScale);
  const CTransform4f model = GetTransform() * local;
  CGraphics::SetModelMatrix(model);
  CGraphics::SetDepthWriteMode(mDepthCompare, kE_LEqual, mDepthUpdate);
  switch (mBlendMode) {
  case 0:
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
    break;
  case 1:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    break;
  case 2:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    break;
  }
  mTextSupport.Render();
}

bool CScriptTextPane::CanRenderUnsorted(const CStateManager& mgr) const { return mBlendMode == 0; }

void CScriptTextPane::SetRenderScale(float scale) { mTargetRenderScale = scale; }

CEntity* LoadTextPane(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTextPane sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTextPane.inc"

  return rs_new CScriptTextPane(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.pivotOffset, sldrThis.editorProperties.transform.scale.GetX(),
      sldrThis.editorProperties.transform.scale.GetZ(), sldrThis.textProperties.textBoundingWidth,
      sldrThis.textProperties.textBoundingHeight, sldrThis.textProperties.foregroundColor,
      sldrThis.textProperties.outlineColor, sldrThis.textProperties.geometryColor,
      sldrThis.textProperties.defaultFont, sldrThis.defaultString, sldrThis.defaultStringName,
      sldrThis.blend_Mode, sldrThis.fadeInTime, sldrThis.fadeOutTime,
      CGuiTextProperties(sldrThis.textProperties.wrapText,
                         EJustification(sldrThis.textProperties.horizontalJustification),
                         EVerticalJustification(sldrThis.textProperties.verticalJustification)),
      sldrThis.guiLabel, sldrThis.depth_Compare, sldrThis.depth_Update, sldrThis.depth_Backwards);
}

CScriptTextPane::~CScriptTextPane() {}
