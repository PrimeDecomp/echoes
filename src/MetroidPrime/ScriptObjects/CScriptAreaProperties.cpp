#include "MetroidPrime/ScriptObjects/CScriptAreaProperties.hpp"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrAreaAttributes.hpp"

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

void CScriptAreaProperties::AcceptScriptMsg(CStateManager& mgr, CScriptMsg& msg) {
  const EScriptObjectMessage message(msg.GetMessage());
  CEntity::AcceptScriptMsg(mgr, msg);
  if (GetCurrentAreaId() != kInvalidAreaId) {
    switch (message) {
    case kSM_XCRT:
      mgr.SetIsDarkWorld(m_isDarkWorld);
      break;
    case kSM_XALD:
      mgr.World()->Area(GetCurrentAreaId())->SetAreaAttributes(this);
      if (m_environmentEffects) {
        mgr.EnvFxManager()->SetDensity(m_density, 500);
      }
      break;
    case kSM_Play:
      mgr.EnvFxManager()->Play_801620A8();
      break;
    case kSM_Stop:
      mgr.EnvFxManager()->Stop_801620B4();
      break;
    case kSM_XDelete: {
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

CScriptAreaProperties* LoadAreaProperties(CStateManager& mgr, CInputStream& input,
                                          const CEntityInfo& info) {
  SLdrAreaAttributes sldrThis;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(sldrThis.editorProperties, input);
      break;
    case 0x95d4bee7:
      sldrThis.needSky = input.ReadBool();
      break;
    case 0xb24fde1a:
      sldrThis.darkWorld = input.ReadBool();
      break;
    case 0x9d0006ab:
      sldrThis.environmentEffects = input.ReadInt32();
      break;
    case 0x56263e35:
      sldrThis.environmentGroupSound = input.ReadInt32();
      break;
    case 0x64e5fe9f:
      sldrThis.density = input.ReadFloat();
      break;
    case 0xba5f801e:
      sldrThis.normalLighting = input.ReadFloat();
      break;
    case 0xd208c9fa:
      sldrThis.overrideSky = input.ReadInt32();
      break;
    case 0xffeebc46:
      sldrThis.phazonDamage = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return new CScriptAreaProperties(
      mgr.AllocateUniqueId(), EntityInfoWithEditorProperties(info, sldrThis.editorProperties),
      sldrThis.density, sldrThis.normalLighting, 0.0f, 0.0f, sldrThis.needSky, sldrThis.darkWorld,
      sldrThis.environmentEffects, sldrThis.overrideSky, sldrThis.phazonDamage, 0, CColor::Black());
}

CScriptAreaProperties::~CScriptAreaProperties() {}
