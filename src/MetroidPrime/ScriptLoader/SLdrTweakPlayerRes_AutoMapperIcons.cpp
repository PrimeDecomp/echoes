#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_AutoMapperIcons.hpp"

SLdrTweakPlayerRes_AutoMapperIcons::SLdrTweakPlayerRes_AutoMapperIcons() {}

SLdrTweakPlayerRes_AutoMapperIcons::~SLdrTweakPlayerRes_AutoMapperIcons() {}

void LoadTypedefSLdrTweakPlayerRes_AutoMapperIcons(SLdrTweakPlayerRes_AutoMapperIcons& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xe7014cda:
      sldrThis.saveStationIcon = rstl::string(input);
      break;
    case 0x33c94749:
      sldrThis.missileStationIcon = rstl::string(input);
      break;
    case 0x9b36949e:
      sldrThis.elevatorIconIcon = rstl::string(input);
      break;
    case 0xafa1b87c:
      sldrThis.portalIcon = rstl::string(input);
      break;
    case 0xfbf479ec:
      sldrThis.unknown_0xfbf479ec = rstl::string(input);
      break;
    case 0x5566b6e4:
      sldrThis.unknown_0x5566b6e4 = rstl::string(input);
      break;
    case 0x51fe3f1f:
      sldrThis.unknown_0x51fe3f1f = rstl::string(input);
      break;
    case 0xa4127a5a:
      sldrThis.unknown_0xa4127a5a = rstl::string(input);
      break;
    case 0xf8403d18:
      sldrThis.translatorDoorIcon = rstl::string(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
