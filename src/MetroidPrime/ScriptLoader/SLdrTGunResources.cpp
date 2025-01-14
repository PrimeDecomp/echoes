#include "MetroidPrime/ScriptLoader/SLdrTGunResources.hpp"

SLdrTGunResources::SLdrTGunResources() {}

SLdrTGunResources::~SLdrTGunResources() {}

void LoadTypedefSLdrTGunResources(SLdrTGunResources& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x2705318d:
      sldrThis.powerBeam = rstl::string(input);
      break;
    case 0x7cc2879f:
      sldrThis.iceBeam = rstl::string(input);
      break;
    case 0x382765b0:
      sldrThis.waveBeam = rstl::string(input);
      break;
    case 0xcb269ac8:
      sldrThis.plasmaBeam = rstl::string(input);
      break;
    case 0xa3890335:
      sldrThis.phazonBeam = rstl::string(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
