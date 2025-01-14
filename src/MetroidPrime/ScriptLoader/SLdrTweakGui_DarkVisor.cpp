#include "MetroidPrime/ScriptLoader/SLdrTweakGui_DarkVisor.hpp"

SLdrTweakGui_DarkVisor::SLdrTweakGui_DarkVisor() {
  darkWorldBaseColor = CColor::Black();
  darkVisorStaticColor = CColor::Black();
  darkVisorPaletteModulate = CColor::Black();
  darkVisorBlurSpeed = 0.0625f;
  darkVisorFrameTop = 100;
  darkVisorFrameHeight = 248;
  darkVisorFrameLeft = 50;
  darkVisorFrameWidth = 540;
}

SLdrTweakGui_DarkVisor::~SLdrTweakGui_DarkVisor() {}

void LoadTypedefSLdrTweakGui_DarkVisor(SLdrTweakGui_DarkVisor& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xc571a8c6:
      sldrThis.darkWorldBaseColor = CColor(input);
      break;
    case 0x93db3352:
      sldrThis.darkVisorStaticColor = CColor(input);
      break;
    case 0x92f8c63a:
      sldrThis.darkVisorPaletteModulate = CColor(input);
      break;
    case 0xaf902412:
      sldrThis.darkVisorBlurSpeed = input.ReadFloat();
      break;
    case 0xfc913090:
      sldrThis.darkVisorFrameTop = input.ReadInt32();
      break;
    case 0xd275ceee:
      sldrThis.darkVisorFrameHeight = input.ReadInt32();
      break;
    case 0xaa8047a0:
      sldrThis.darkVisorFrameLeft = input.ReadInt32();
      break;
    case 0x2773fbb3:
      sldrThis.darkVisorFrameWidth = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
