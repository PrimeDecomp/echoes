#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerControls.hpp"

SLdrTweakPlayerControls::SLdrTweakPlayerControls()
  : instanceName()
  , unknown_0x3c34dfed()
  , unknown_0x168a79f1()
{}

SLdrTweakPlayerControls::~SLdrTweakPlayerControls() {}

void LoadTypedefSLdrTweakPlayerControls(SLdrTweakPlayerControls& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0x3c34dfed:
      LoadTypedefSLdrTweakPlayerControls_UnknownStruct1(sldrThis.unknown_0x3c34dfed, input);
      break;
    case 0x168a79f1:
      LoadTypedefSLdrTweakPlayerControls_UnknownStruct2(sldrThis.unknown_0x168a79f1, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}