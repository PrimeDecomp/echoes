#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_Charge_Gauge.hpp"

SLdrTweakTargeting_Charge_Gauge::SLdrTweakTargeting_Charge_Gauge() {
  unknown_0xd032c2a1 = 0.0f;
  unknown_0xa118e250 = 90.0f;
  unknown_0xdb1ac8ee = 150.0f;
  unknown_0xecd100f8 = 210.0f;
  chargeGaugeScale = 1.0f;
  chargeGaugeColor = CColor::Black();
  unknown_0xed78e6eb = 14;
  unknown_0x2c3d9e27 = 8.5f;
}

SLdrTweakTargeting_Charge_Gauge::~SLdrTweakTargeting_Charge_Gauge() {}

void LoadTypedefSLdrTweakTargeting_Charge_Gauge(SLdrTweakTargeting_Charge_Gauge& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xd032c2a1:
      sldrThis.unknown_0xd032c2a1 = input.ReadFloat();
      break;
    case 0xa118e250:
      sldrThis.unknown_0xa118e250 = input.ReadFloat();
      break;
    case 0xdb1ac8ee:
      sldrThis.unknown_0xdb1ac8ee = input.ReadFloat();
      break;
    case 0xecd100f8:
      sldrThis.unknown_0xecd100f8 = input.ReadFloat();
      break;
    case 0x49f8161f:
      sldrThis.chargeGaugeScale = input.ReadFloat();
      break;
    case 0x526e60f4:
      sldrThis.chargeGaugeColor = CColor(input);
      break;
    case 0xed78e6eb:
      sldrThis.unknown_0xed78e6eb = input.ReadInt32();
      break;
    case 0x2c3d9e27:
      sldrThis.unknown_0x2c3d9e27 = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
