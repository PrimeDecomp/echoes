#include "MetroidPrime/ScriptLoader/SLdrTweakGui_EchoVisor.hpp"

SLdrTweakGui_EchoVisor::SLdrTweakGui_EchoVisor() {
  echoBaseColor = CColor::Black();
  echoOutlineColor = CColor::Black();
  echoDamageColor = CColor::Black();
  unknown_0xa288fe51 = CColor::Black();
  unknown_0xf316c617 = 1;
  unknown_0xb56d2286 = 0.0f;
  unknown_0xc5c2d8fc = 100.0f;
  unknown_0x47d948b1 = 800.0f;
  unknown_0x5708b903 = 500.0f;
  unknown_0xafc8c571 = 2.0f;
  unknown_0x28bffdc5 = 0.89999998f;
  echoRingColor = CColor::Black();
  unknown_0x99a65ca7 = 10.0f;
  unknown_0x51f19dea = 20.0f;
}

SLdrTweakGui_EchoVisor::~SLdrTweakGui_EchoVisor() {}

void LoadTypedefSLdrTweakGui_EchoVisor(SLdrTweakGui_EchoVisor& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x905fa7e4:
      sldrThis.echoBaseColor = CColor(input);
      break;
    case 0x62ca2ddd:
      sldrThis.echoOutlineColor = CColor(input);
      break;
    case 0xdc4c2a5f:
      sldrThis.echoDamageColor = CColor(input);
      break;
    case 0xa288fe51:
      sldrThis.unknown_0xa288fe51 = CColor(input);
      break;
    case 0xf316c617:
      sldrThis.unknown_0xf316c617 = input.ReadInt32();
      break;
    case 0xb56d2286:
      sldrThis.unknown_0xb56d2286 = input.ReadFloat();
      break;
    case 0xc5c2d8fc:
      sldrThis.unknown_0xc5c2d8fc = input.ReadFloat();
      break;
    case 0x47d948b1:
      sldrThis.unknown_0x47d948b1 = input.ReadFloat();
      break;
    case 0x5708b903:
      sldrThis.unknown_0x5708b903 = input.ReadFloat();
      break;
    case 0xafc8c571:
      sldrThis.unknown_0xafc8c571 = input.ReadFloat();
      break;
    case 0x28bffdc5:
      sldrThis.unknown_0x28bffdc5 = input.ReadFloat();
      break;
    case 0x66351d36:
      sldrThis.echoRingColor = CColor(input);
      break;
    case 0x99a65ca7:
      sldrThis.unknown_0x99a65ca7 = input.ReadFloat();
      break;
    case 0x51f19dea:
      sldrThis.unknown_0x51f19dea = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
