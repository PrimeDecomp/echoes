#include "MetroidPrime/ScriptObjects/CScriptTextPane.hpp"

#include "MetroidPrime/ScriptLoader.hpp"

CEntity* CScriptTextPane::TypesMatch(int typeId) const {}

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
, mDefaultString()
, mStringIndex(0)
, mGuiLabel(guiLabel)
, mFadeInTime(fadeInTime)
, mFadeOutTime(fadeOutTime)
, mRenderScale(1.f)
, mTargetRenderScale(1.f)
, mDepthCompare(depthCompare)
, mDepthUpdate(depthUpdate)
, mDepthBackwards(depthBackwards) {}

void CScriptTextPane::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {}

void CScriptTextPane::Think(float dt, CStateManager& mgr) {}

void CScriptTextPane::PreRender(CStateManager& mgr) {}

void CScriptTextPane::AddToRenderer(const CStateManager& mgr) const {}

void CScriptTextPane::Render(const CStateManager& mgr) const {}

bool CScriptTextPane::CanRenderUnsorted(const CStateManager& mgr) const {}

void CScriptTextPane::SetRenderScale(float scale) {}

CEntity* LoadTextPane(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}

CScriptTextPane::~CScriptTextPane() {}
