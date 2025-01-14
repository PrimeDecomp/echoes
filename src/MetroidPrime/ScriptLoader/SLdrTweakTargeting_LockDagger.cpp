#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_LockDagger.hpp"

SLdrTweakTargeting_LockDagger::SLdrTweakTargeting_LockDagger() {
  lockDaggerNormalScale = 1.0f;
  unknown = 0.80000001f;
  lockDaggerColor = CColor::Black();
  lockDagger0Angle = 0.0f;
  lockDagger1Angle = 120.0f;
  lockDagger2Angle = 240.0f;
}

SLdrTweakTargeting_LockDagger::~SLdrTweakTargeting_LockDagger() {}

void LoadTypedefSLdrTweakTargeting_LockDagger(SLdrTweakTargeting_LockDagger& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x469550e8:
      sldrThis.lockDaggerNormalScale = input.ReadFloat();
      break;
    case 0x7b48e6f9:
      sldrThis.unknown = input.ReadFloat();
      break;
    case 0x53c2c9fc:
      sldrThis.lockDaggerColor = CColor(input);
      break;
    case 0xa29cdf22:
      sldrThis.lockDagger0Angle = input.ReadFloat();
      break;
    case 0x631200e2:
      sldrThis.lockDagger1Angle = input.ReadFloat();
      break;
    case 0xfaf066e3:
      sldrThis.lockDagger2Angle = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
