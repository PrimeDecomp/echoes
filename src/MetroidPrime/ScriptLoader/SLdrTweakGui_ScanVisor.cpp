#include "MetroidPrime/ScriptLoader/SLdrTweakGui_ScanVisor.hpp"

SLdrTweakGui_ScanVisor::SLdrTweakGui_ScanVisor() {
  inactiveColor = CColor::Black();
  inactiveExternalColor = CColor::Black();
  nonCriticalColor = CColor::Black();
  criticalColor = CColor::Black();
  burnInColor = CColor::Black();
  highlightColor = CColor::Black();
  criticalHighlightColor = CColor::Black();
  unknown_0xe8f5018b = CColor::Black();
  unknown_0xba1ae1e5 = CColor::Black();
  unknown_0xb39d450e = CColor::Black();
  unknown_0x1042455b = CColor::Black();
  unknown_0xd72435ad = CColor::Black();
  unknown_0x75cdc913 = CColor::Black();
  sweepBarColor = CColor::Black();
  burnInTime = 1.0f;
  fadeOutTime = 0.30000001f;
}

SLdrTweakGui_ScanVisor::~SLdrTweakGui_ScanVisor() {}

void LoadTypedefSLdrTweakGui_ScanVisor(SLdrTweakGui_ScanVisor& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x972271b9:
      sldrThis.inactiveColor = CColor(input);
      break;
    case 0xa908c775:
      sldrThis.inactiveExternalColor = CColor(input);
      break;
    case 0xee1f1df6:
      sldrThis.nonCriticalColor = CColor(input);
      break;
    case 0x43445ae7:
      sldrThis.criticalColor = CColor(input);
      break;
    case 0xf48fd559:
      sldrThis.burnInColor = CColor(input);
      break;
    case 0x7a6412f6:
      sldrThis.highlightColor = CColor(input);
      break;
    case 0xf45f7d17:
      sldrThis.criticalHighlightColor = CColor(input);
      break;
    case 0xe8f5018b:
      sldrThis.unknown_0xe8f5018b = CColor(input);
      break;
    case 0xba1ae1e5:
      sldrThis.unknown_0xba1ae1e5 = CColor(input);
      break;
    case 0xb39d450e:
      sldrThis.unknown_0xb39d450e = CColor(input);
      break;
    case 0x1042455b:
      sldrThis.unknown_0x1042455b = CColor(input);
      break;
    case 0xd72435ad:
      sldrThis.unknown_0xd72435ad = CColor(input);
      break;
    case 0x75cdc913:
      sldrThis.unknown_0x75cdc913 = CColor(input);
      break;
    case 0x997ec38d:
      sldrThis.sweepBarColor = CColor(input);
      break;
    case 0x00b83f02:
      sldrThis.burnInTime = input.ReadFloat();
      break;
    case 0x7c269ebc:
      sldrThis.fadeOutTime = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
