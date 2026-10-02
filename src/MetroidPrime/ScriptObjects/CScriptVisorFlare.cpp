#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVisorFlare.hpp"

#include "Kyoto/CSimplePool.hpp"

#include "rstl/optional_object.hpp"

CScriptVisorFlare::CScriptVisorFlare(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CVector3f& pos,
                                     CVisorFlare::EBlendMode blendMode, bool distanceScaled,
                                     float fadeTime, float angularFalloff, float rotationScale,
                                     uint darkVisorMode, uint combatVisorMode,
                                     const rstl::vector< CVisorFlare::CFlareDef >& flares,
                                     bool smallOcclusionTest, bool noOcclusionTest)
: CActor(uid, name, info, 0, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_NoStepLogic), CActorParameters::None(), kInvalidUniqueId)
, mFlare(blendMode, distanceScaled, fadeTime, angularFalloff, rotationScale, darkVisorMode,
         combatVisorMode, flares, smallOcclusionTest, noOcclusionTest) {}

void CScriptVisorFlare::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mFlare.Update(dt, GetTranslation(), this, mgr);
  }
}

void CScriptVisorFlare::PreRender(CStateManager& mgr) {
  mFlare.UpdateFrustum(mgr, GetTranslation());
  mgr.RenderLast(GetUniqueId());
}

void CScriptVisorFlare::AddToRenderer(const CStateManager&) const {}

void CScriptVisorFlare::Render(const CStateManager& mgr) const {
  mFlare.Render(GetTranslation(), *this, mgr);
}

static rstl::optional_object< CVisorFlare::CFlareDef > LoadFlareDef(const SLdrFlareDef& def) {
  if (def.texture != kInvalidAssetId) {
    TToken< CTexture > texture = gpSimplePool->GetObj(SObjectTag('TXTR', def.texture));
    texture.Lock();
    return CVisorFlare::CFlareDef(texture, def.position, def.scale, def.color.GetColor_u32());
  }
  return rstl::optional_object_null();
}

CEntity* LoadVisorFlare(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrVisorFlare sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrVisorFlare.inc"

  rstl::vector< CVisorFlare::CFlareDef > flares;
  flares.reserve(5);
  rstl::optional_object< CVisorFlare::CFlareDef > flare1 = LoadFlareDef(sldrThis.flare1);
  rstl::optional_object< CVisorFlare::CFlareDef > flare2 = LoadFlareDef(sldrThis.flare2);
  rstl::optional_object< CVisorFlare::CFlareDef > flare3 = LoadFlareDef(sldrThis.flare3);
  rstl::optional_object< CVisorFlare::CFlareDef > flare4 = LoadFlareDef(sldrThis.flare4);
  rstl::optional_object< CVisorFlare::CFlareDef > flare5 = LoadFlareDef(sldrThis.flare5);
  if (flare1) {
    flares.push_back_unsafe(*flare1);
  }
  if (flare2) {
    flares.push_back_unsafe(*flare2);
  }
  if (flare3) {
    flares.push_back_unsafe(*flare3);
  }
  if (flare4) {
    flares.push_back_unsafe(*flare4);
  }
  if (flare5) {
    flares.push_back_unsafe(*flare5);
  }

  return rs_new CScriptVisorFlare(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                  LdrToEntityInfo(info, sldrThis.editorProperties),
                                  sldrThis.editorProperties.transform.position,
                                  static_cast< CVisorFlare::EBlendMode >(sldrThis.blendMode),
                                  sldrThis.constantScale, sldrThis.fadeTime, sldrThis.fadeFactor,
                                  sldrThis.rotateFactor, 2, sldrThis.combatVisorMode, flares,
                                  sldrThis.unknown_0xa51f243e, sldrThis.noOcclusionTest);
}

CScriptVisorFlare::~CScriptVisorFlare() {}
