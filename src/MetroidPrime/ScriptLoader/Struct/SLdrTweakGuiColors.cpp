#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGuiColors.hpp"

SLdrTweakGuiColors::SLdrTweakGuiColors()
  : instanceName()
  , hudColors()
  , misc()
  , multiplayer()
  , combatHudColorScheme()
  , echoHudColorScheme()
  , scanHudColorScheme()
  , darkHudColorScheme()
  , ballHudColorScheme()
  , combatHud()
  , scanHud()
  , xRayHud()
  , thermalHud()
  , ballHud()
  , turretHud()
{}

SLdrTweakGuiColors::~SLdrTweakGuiColors() {}

void LoadTypedefSLdrTweakGuiColors(SLdrTweakGuiColors& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0xcb737724:
      LoadTypedefSLdrTweakGuiColors_HUDColorsTypedef(sldrThis.hudColors, input);
      break;
    case 0x6756d4de:
      LoadTypedefSLdrTweakGuiColors_Misc(sldrThis.misc, input);
      break;
    case 0x697613e9:
      LoadTypedefSLdrTweakGuiColors_Multiplayer(sldrThis.multiplayer, input);
      break;
    case 0x67c70055:
      LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(sldrThis.combatHudColorScheme, input);
      break;
    case 0x62e0a08f:
      LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(sldrThis.echoHudColorScheme, input);
      break;
    case 0x80becd6e:
      LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(sldrThis.scanHudColorScheme, input);
      break;
    case 0x7de4b297:
      LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(sldrThis.darkHudColorScheme, input);
      break;
    case 0xc0181762:
      LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(sldrThis.ballHudColorScheme, input);
      break;
    case 0x45d7a40f:
      LoadTypedefSLdrTweakGui_HudColorTypedef(sldrThis.combatHud, input);
      break;
    case 0x594b44cf:
      LoadTypedefSLdrTweakGui_HudColorTypedef(sldrThis.scanHud, input);
      break;
    case 0x8f5ebeb9:
      LoadTypedefSLdrTweakGui_HudColorTypedef(sldrThis.xRayHud, input);
      break;
    case 0xf12b1e59:
      LoadTypedefSLdrTweakGui_HudColorTypedef(sldrThis.thermalHud, input);
      break;
    case 0x58cd6373:
      LoadTypedefSLdrTweakGui_HudColorTypedef(sldrThis.ballHud, input);
      break;
    case 0xde139081:
      LoadTypedefSLdrTweakGuiColors_TurretHudTypedef(sldrThis.turretHud, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}