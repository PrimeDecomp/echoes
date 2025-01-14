#include "MetroidPrime/ScriptLoader/SLdrTweakGui_ScannableObjectDownloadTimes.hpp"

SLdrTweakGui_ScannableObjectDownloadTimes::SLdrTweakGui_ScannableObjectDownloadTimes() {
  fast = 1.0f;
  slow = 3.0f;
}

SLdrTweakGui_ScannableObjectDownloadTimes::~SLdrTweakGui_ScannableObjectDownloadTimes() {}

void LoadTypedefSLdrTweakGui_ScannableObjectDownloadTimes(SLdrTweakGui_ScannableObjectDownloadTimes& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xea8b1d10:
      sldrThis.fast = input.ReadFloat();
      break;
    case 0xb1338beb:
      sldrThis.slow = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
