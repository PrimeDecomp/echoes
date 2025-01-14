#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_Scan.hpp"

SLdrTweakTargeting_Scan::SLdrTweakTargeting_Scan()
: unknown(CColor::Green())
, scanLockLockedColor(CColor::Green())
, scanLockUnlockedColor(CColor::Green()) {
  scanLockScale = 1.0f;
  scanLockTransitionTime = 0.30000001f;
  scanLockTranslation = 1.0f;
}

SLdrTweakTargeting_Scan::~SLdrTweakTargeting_Scan() {}

void LoadTypedefSLdrTweakTargeting_Scan(SLdrTweakTargeting_Scan& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xa0857b6e:
      sldrThis.scanLockScale = input.ReadFloat();
      break;
    case 0xa4cce30f:
      sldrThis.scanLockTransitionTime = input.ReadFloat();
      break;
    case 0x8e9bfea3:
      sldrThis.scanLockTranslation = input.ReadFloat();
      break;
    case 0x768110cd:
      sldrThis.unknown = CColor(input);
      break;
    case 0x319f966b:
      sldrThis.scanLockLockedColor = CColor(input);
      break;
    case 0xa81f378c:
      sldrThis.scanLockUnlockedColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
