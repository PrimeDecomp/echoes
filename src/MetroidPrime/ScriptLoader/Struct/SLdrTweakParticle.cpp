#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakParticle.hpp"

SLdrTweakParticle::SLdrTweakParticle() {}

SLdrTweakParticle::~SLdrTweakParticle() {}

void LoadTypedefSLdrTweakParticle(SLdrTweakParticle& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0x2bd13ab3:
      sldrThis.pakFile = rstl::string(input);
      break;
    case 0x5e15868a:
      sldrThis.primaryWeapon = rstl::string(input);
      break;
    case 0xbc401445:
      sldrThis.secondaryWeapon = rstl::string(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
