#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerRes.hpp"

SLdrTweakPlayerRes::SLdrTweakPlayerRes()
  : instanceName()
  , autoMapperIcons()
  , mapScreenIcons()
  , ballTransitionResources()
  , cinematicResources()
  , unknown(-0.0)
{}

SLdrTweakPlayerRes::~SLdrTweakPlayerRes() {}

void LoadTypedefSLdrTweakPlayerRes(SLdrTweakPlayerRes& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0x357741e0:
      LoadTypedefSLdrTweakPlayerRes_AutoMapperIcons(sldrThis.autoMapperIcons, input);
      break;
    case 0x0d5e02a0:
      LoadTypedefSLdrTweakPlayerRes_MapScreenIcons(sldrThis.mapScreenIcons, input);
      break;
    case 0x279852ba:
      LoadTypedefSLdrTBallTransitionResources(sldrThis.ballTransitionResources, input);
      break;
    case 0x5e630608:
      LoadTypedefSLdrTGunResources(sldrThis.cinematicResources, input);
      break;
    case 0x36ad9d19:
      sldrThis.unknown = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}