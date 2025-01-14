#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakAutoMapper.hpp"

SLdrTweakAutoMapper::SLdrTweakAutoMapper() {}

SLdrTweakAutoMapper::~SLdrTweakAutoMapper() {}

void LoadTypedefSLdrTweakAutoMapper(SLdrTweakAutoMapper& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0x18b2f423:
      LoadTypedefSLdrTweakAutoMapper_Base(sldrThis.base, input);
      break;
    case 0xc5368863:
      LoadTypedefSLdrTweakAutoMapper_DoorColors(sldrThis.doorColors, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
