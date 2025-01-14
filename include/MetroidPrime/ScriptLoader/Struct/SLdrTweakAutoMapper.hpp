#ifndef _SLDRTWEAKAUTOMAPPER
#define _SLDRTWEAKAUTOMAPPER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_Base.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_DoorColors.hpp"
#include "rstl/string.hpp"

struct SLdrTweakAutoMapper {
    SLdrTweakAutoMapper();
    ~SLdrTweakAutoMapper();

    rstl::string instanceName;
    SLdrTweakAutoMapper_Base base;
    SLdrTweakAutoMapper_DoorColors doorColors;
};

void LoadTypedefSLdrTweakAutoMapper(SLdrTweakAutoMapper&, CInputStream&);

/* {
  SLdrTweakAutoMapper sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
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
} */
#endif // _SLDRTWEAKAUTOMAPPER
