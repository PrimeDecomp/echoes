#include "MetroidPrime/ScriptObjects/CScriptAreaProperties.hpp"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAreaAttributes.hpp"

CScriptAreaProperties::CScriptAreaProperties(TUniqueId uid, const CEntityInfo& info, float density,
                                             float normalLightning, uint hasSkyBox,
                                             bool isDarkWorld, uint environmentEffects,
                                             CAssetId skyBoxAssetId, int phazonDamage, int unk1,
                                             float unk2, float unk3, const CColor& color)

: CEntity(uid, info, "AreaAttributes", false)
, m_hasSkybox(hasSkyBox)
, m_isDarkWorld(isDarkWorld)
, m_environmentEffects(environmentEffects)
, m_density(density)
, m_normalLightning(normalLightning)
, m_skyBoxAssetId(skyBoxAssetId)
, m_phazonDamage(phazonDamage)
, skyBoxModel(hasSkyBox ? rstl::optional_object< TLockedToken< CModel > >(
                              gpSimplePool->GetObj(SObjectTag('CMDL', skyBoxAssetId)))
                        : rstl::optional_object_null())
, x4c(unk1)
, x50(unk2)
, x54(unk3)
, m_color(color) {}

void CScriptAreaProperties::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message(msg.GetMessage());
  CEntity::AcceptScriptMsg(mgr, msg);
  if (GetCurrentAreaId() != kInvalidAreaId) {
    switch (message) {
    case kSM_Create:
      mgr.SetIsDarkWorld(m_isDarkWorld);
      break;
    case kSM_AreaLoaded:
      mgr.World()->Area(GetCurrentAreaId())->SetAreaAttributes(this);
      if (m_environmentEffects) {
        mgr.EnvFxManager()->FadeDensity(m_density, 500);
      }
      break;
    case kSM_Play:
      mgr.EnvFxManager()->PlayRainSounds();
      break;
    case kSM_Stop:
      mgr.EnvFxManager()->StopRainSounds();
      break;
    case kSM_Delete: {
      if (mgr.World()->Area(GetCurrentAreaId())->GetPhase() == 0x10) {
        mgr.World()->Area(GetCurrentAreaId())->SetAreaAttributes(nullptr);
      }
      break;
    }
    default:
      break;
    }
  }
}

CEntity* LoadAreaProperties(CStateManager& mgr, CInputStream& input,
                                          CEntityInfo& info) {
  SLdrAreaAttributes sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrAreaAttributes.inc"

  return new CScriptAreaProperties(
      mgr.AllocateUniqueId(), LdrToEntityInfo(info, sldrThis.editorProperties),
      sldrThis.density, sldrThis.normalLighting, 0.0f, 0.0f, sldrThis.needSky, sldrThis.darkWorld,
      sldrThis.environmentEffects, sldrThis.overrideSky, sldrThis.phazonDamage, 0, CColor::Black());
}

CScriptAreaProperties::~CScriptAreaProperties() {}
