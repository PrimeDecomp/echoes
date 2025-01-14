#ifndef _SLDRTWEAKPLAYERCONTROLS
#define _SLDRTWEAKPLAYERCONTROLS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct1.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct2.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerControls {
    SLdrTweakPlayerControls();
    ~SLdrTweakPlayerControls();

    rstl::string instanceName;
    SLdrTweakPlayerControls_UnknownStruct1 unknown_0x3c34dfed;
    SLdrTweakPlayerControls_UnknownStruct2 unknown_0x168a79f1;
};

void LoadTypedefSLdrTweakPlayerControls(SLdrTweakPlayerControls&, CInputStream&);

/* {
  SLdrTweakPlayerControls sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
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
} */
#endif // _SLDRTWEAKPLAYERCONTROLS
