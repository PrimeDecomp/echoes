#include "MetroidPrime/ScriptLoader/SLdrTBallTransitionResources.hpp"

SLdrTBallTransitionResources::SLdrTBallTransitionResources() {}

SLdrTBallTransitionResources::~SLdrTBallTransitionResources() {}

void LoadTypedefSLdrTBallTransitionResources(SLdrTBallTransitionResources& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xd48e4124:
      sldrThis.unknown_0xd48e4124 = rstl::string(input);
      break;
    case 0x01e12c84:
      LoadTypedefSLdrTGunResources(sldrThis.unknown_0x01e12c84, input);
      break;
    case 0xf24b055d:
      LoadTypedefSLdrTGunResources(sldrThis.unknown_0xf24b055d, input);
      break;
    case 0xa342c3a6:
      sldrThis.unknown_0xa342c3a6 = SLdrSpline(input, propertySize);
      break;
    case 0x15b6840d:
      sldrThis.unknown_0x15b6840d = SLdrSpline(input, propertySize);
      break;
    case 0x23fb0e93:
      sldrThis.unknown_0x23fb0e93 = SLdrSpline(input, propertySize);
      break;
    case 0x564262f0:
      sldrThis.unknown_0x564262f0 = SLdrSpline(input, propertySize);
      break;
    case 0x9183a262:
      sldrThis.movementControl = SLdrSpline(input, propertySize);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
