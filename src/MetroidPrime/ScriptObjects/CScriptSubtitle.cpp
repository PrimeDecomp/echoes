#include "MetroidPrime/ScriptObjects/CScriptSubtitle.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSubtitle.hpp"

CScriptSubtitle::CScriptSubtitle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 int positionX, int positionY, int extentX, int extentY,
                                 const CColor& fontColor, const CColor& outlineColor,
                                 const CColor& geometryColor, CAssetId font, CAssetId stringTable,
                                 int initialStringIndex, float fadeInTime, float fadeOutTime,
                                 const CGuiTextProperties& textProperties)
: CActor(uid, name, info, 0, CTransform4f::Identity(), CModelData::CModelDataNull(),
         CMaterialList(), CActorParameters::None(), kInvalidUniqueId)
, mPositionX(positionX)
, mPositionY(positionY)
, mTextSupport(font, extentX, extentY, textProperties, fontColor, outlineColor, geometryColor,
               gpSimplePool)
, mGeometryColor(geometryColor)
, mStringTable(gpSimplePool->GetObj(SObjectTag('STRG', stringTable)))
, mStringIndex(initialStringIndex)
, mFadeInTime(fadeInTime)
, mFadeOutTime(fadeOutTime)
, mFadeOpacity(0.f)
, mTargetFadeOpacity(0.f) {
  if (mStringIndex < 0 || mStringIndex >= mStringTable->GetStringCount()) {
    mStringIndex = CMath::Clamp(0, mStringIndex, mStringTable->GetStringCount());
  }
  RefreshText();
  SetModelFlags(CModelFlags::AlphaBlended(mGeometryColor));
}

void CScriptSubtitle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }
  switch (msg.GetMessage()) {
  case kSM_Increment:
    SetStringIndex(mgr, mStringIndex + 1);
    break;
  case kSM_Decrement:
    SetStringIndex(mgr, mStringIndex - 1);
    break;
  case kSM_SetToZero:
    SetStringIndex(mgr, 0);
    break;
  case kSM_SetToMax:
    SetStringIndex(mgr, mStringTable->GetStringCount() - 1);
    break;
  case kSM_Start:
    mTargetFadeOpacity = 1.f;
    break;
  case kSM_Stop:
    mTargetFadeOpacity = 0.f;
    break;
  default:
    break;
  }
}

void CScriptSubtitle::Think(float dt, CStateManager& mgr) {
  if (mTargetFadeOpacity > mFadeOpacity) {
    mFadeOpacity = mFadeInTime == 0.f
                       ? mTargetFadeOpacity
                       : rstl::min_val(mFadeOpacity + dt / mFadeInTime, mTargetFadeOpacity);
  } else if (mTargetFadeOpacity < mFadeOpacity) {
    mFadeOpacity = mFadeOutTime == 0.f
                       ? mTargetFadeOpacity
                       : rstl::max_val(mFadeOpacity - dt / mFadeOutTime, mTargetFadeOpacity);
  }
}

void CScriptSubtitle::PreRender(CStateManager& mgr) {
  mgr.RenderLastAfterCameraFilters(GetUniqueId());
}

void CScriptSubtitle::Render(const CStateManager& mgr) const {
  const_cast< CGuiTextSupport& >(mTextSupport)
      .SetGeometryColor(GetModelFlags().GetColorRef().WithAlphaModulatedBy(mFadeOpacity));
  if (mFadeOpacity > 0.f) {
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    gpRender->SetDepthReadWrite(false, false);
    switch (static_cast< signed char >(GetModelFlags().GetTrans())) {
    case CModelFlags::kT_Opaque:
    case CModelFlags::kT_One:
    case CModelFlags::kT_Two:
    case CModelFlags::kT_Blend:
    case CModelFlags::kT_Additive2:
      gpRender->SetBlendMode_AlphaBlended();
      break;
    case CModelFlags::kT_Additive:
      gpRender->SetBlendMode_AdditiveAlpha();
      break;
    default:
      break;
    }
    const CViewport& viewport = CGraphics::GetViewport();
    const CTransform4f xf = CTransform4f::Translate(
        (static_cast< float >(viewport.mWidth) / 640.f) * mPositionX, 0.f,
        (static_cast< float >(viewport.mHeight) / 448.f) * (448.f - mPositionY));
    gpRender->SetModelMatrix(xf);
    mTextSupport.Render();
  }
}

void CScriptSubtitle::SetStringIndex(CStateManager& mgr, int index) {
  if (index == mStringIndex) {
    return;
  }
  const int maxIndex = mStringTable->GetStringCount() - 1;
  if (index == 0) {
    SendScriptMsgs(kSS_Zero, mgr);
  } else if (index == maxIndex) {
    SendScriptMsgs(kSS_MaxReached, mgr);
  }
  mStringIndex = CMath::Clamp(0, index, maxIndex);
  RefreshText();
}

void CScriptSubtitle::RefreshText() {
  const wchar_t* text = mStringTable->GetString(mStringIndex);
  mTextSupport.SetText(rstl::wstring_l(text));
}

// Guessed loader name.
CEntity* LoadSubtitle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSubtitle sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSubtitle.inc"

  if (sldrThis.stringTable == kInvalidAssetId) {
    return nullptr;
  }
  return rs_new CScriptSubtitle(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.textPositionX,
      sldrThis.textPositionY, sldrThis.textProperties.textBoundingWidth,
      sldrThis.textProperties.textBoundingHeight, sldrThis.textProperties.foregroundColor,
      sldrThis.textProperties.outlineColor, sldrThis.textProperties.geometryColor,
      sldrThis.textProperties.defaultFont, sldrThis.stringTable, sldrThis.initialStringIndex,
      sldrThis.fadeInTime, sldrThis.fadeOutTime,
      CGuiTextProperties(
          sldrThis.textProperties.wrapText,
          static_cast< EJustification >(sldrThis.textProperties.horizontalJustification),
          static_cast< EVerticalJustification >(sldrThis.textProperties.verticalJustification)));
}

CScriptSubtitle::~CScriptSubtitle() {}
