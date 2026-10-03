#include "MetroidPrime/ScriptObjects/CScriptSubtitle.hpp"

#include "MetroidPrime/ScriptLoader.hpp"

CScriptSubtitle::~CScriptSubtitle() {}

// Guessed loader name.
CEntity* LoadSubtitle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}

void CScriptSubtitle::RefreshText() {}

void CScriptSubtitle::SetStringIndex(CStateManager& mgr, int index) {}

void CScriptSubtitle::Render(const CStateManager& mgr) const {}

void CScriptSubtitle::PreRender(CStateManager& mgr) {}

void CScriptSubtitle::Think(float dt, CStateManager& mgr) {}

void CScriptSubtitle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {}

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
, mTargetFadeOpacity(0.f) {}
