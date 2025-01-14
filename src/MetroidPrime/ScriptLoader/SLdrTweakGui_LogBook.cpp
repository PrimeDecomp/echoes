#include "MetroidPrime/ScriptLoader/SLdrTweakGui_LogBook.hpp"

SLdrTweakGui_LogBook::SLdrTweakGui_LogBook() {
  mainWindowBorderColor = CColor::Black();
  mainWindowTextColor = CColor::Black();
  mainWindowSelectedTextColor = CColor::Black();
  unknown_0xfe98f30e = CColor::Black();
  unknown_0x39e57ac0 = CColor::Black();
  legendBackgroundColor = CColor::Black();
  nodeColor = CColor::Black();
  selectedNodeColor = CColor::Black();
  parentNodeColor = CColor::Black();
  unknown_0x56843943 = CColor::Black();
  selectedTextCursorColor = CColor::Black();
  unknown_0x905ce2c0 = 1.5f;
  textScale = 1.0f;
  selectedTextScale = 1.0f;
  transitionTime = 1.0f;
  rotationSpeed = 2.0f;
  nodeScale = 1.0f;
  selectedNodeScale = 1.0f;
  selectedTextCursorScale = 1.0f;
  scanModelScale = 2.0f;
  unknown_0x075a0734 = CColor::Black();
  unknown_0x8ef43309 = 2.0f;
  fogNear = 11.0f;
  fogFar = 18.0f;
  fogColor = CColor::Black();
  unknown_0xeef15783 = 12.0f;
  unknown_0x78055ab2 = 0.0f;
  unknown_0x09e3197f = 0.0f;
  unknown_0xfaffce1f = 0.0f;
  unknown_0xe5280e7c = 0.0f;
  backgroundSweepColor = CColor::Black();
  backgroundSweepRadius = 0.60000002f;
  backgroundSweepTime = 1.0f;
  scanTextWindowBackgroundColor = CColor::Black();
  scanTextWindowBorderColor = CColor::Black();
  scanTextWindowFontColor = CColor::Black();
  legendWindowBackgroundColor = CColor::Black();
  legendWindowBorderColor = CColor::Black();
  legendWindowFontColor = CColor::Black();
  legendHideTime = 1.0f;
  unknown_0x7f35c573 = CColor::Black();
  unknown_0xe0d77cf1 = CColor::Black();
  unknown_0x903d9b7e = CColor::Black();
  unknown_0x0fdf22fc = CColor::Black();
  unknown_0xbeaf9d6b = CColor::Black();
  unknown_0x7e8247d6 = CColor::Black();
  unknown_0x8b3f7902 = CColor::Black();
  unknown_0xdc1abb82 = CColor::Black();
  unknown_0x306b2b24 = CColor::Black();
  frameColor = CColor::Black();
  unknown_0x1a4b8068 = CColor::Black();
  sliderBackgroundColor = CColor::Black();
  sliderSelectionColor = CColor::Black();
  sliderScale = 0.1f;
  sliderSpeed = 100.0f;
  unknown_0x58795865 = 0.2f;
  unknown_0x35e0bd31 = 0.2f;
  menuOptionColor = CColor::Black();
  menuOptionEnabledArrowColor = CColor::Black();
  menuOptionDisabledArrowColor = CColor::Black();
  menuOptionScale = 1.0f;
  menuOptionArrowScale = 0.1f;
  modelRotationSpeed = 120.0f;
  unknown_0x60914b79 = -80.0f;
  unknown_0x1fd44d9b = 80.0f;
  modelLight1Color = CColor::Black();
  modelLight2Color = CColor::Black();
  modelAmbientLightColor = CColor::Black();
}

SLdrTweakGui_LogBook::~SLdrTweakGui_LogBook() {}

void LoadTypedefSLdrTweakGui_LogBook(SLdrTweakGui_LogBook& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xba5a65c8:
      sldrThis.mainWindowBorderColor = CColor(input);
      break;
    case 0xbff0e37b:
      sldrThis.mainWindowTextColor = CColor(input);
      break;
    case 0x28b49a88:
      sldrThis.mainWindowSelectedTextColor = CColor(input);
      break;
    case 0xfe98f30e:
      sldrThis.unknown_0xfe98f30e = CColor(input);
      break;
    case 0x39e57ac0:
      sldrThis.unknown_0x39e57ac0 = CColor(input);
      break;
    case 0xa6b633fa:
      sldrThis.legendBackgroundColor = CColor(input);
      break;
    case 0xac80013b:
      sldrThis.nodeColor = CColor(input);
      break;
    case 0x5946b439:
      sldrThis.selectedNodeColor = CColor(input);
      break;
    case 0xc5aa728c:
      sldrThis.parentNodeColor = CColor(input);
      break;
    case 0x56843943:
      sldrThis.unknown_0x56843943 = CColor(input);
      break;
    case 0x6bd4783a:
      sldrThis.selectedTextCursorColor = CColor(input);
      break;
    case 0x905ce2c0:
      sldrThis.unknown_0x905ce2c0 = input.ReadFloat();
      break;
    case 0x5fa64c77:
      sldrThis.textScale = input.ReadFloat();
      break;
    case 0xaa60f975:
      sldrThis.selectedTextScale = input.ReadFloat();
      break;
    case 0x1832935e:
      sldrThis.transitionTime = input.ReadFloat();
      break;
    case 0x165943f6:
      sldrThis.nodeCollapseMotion = SLdrSpline(input, propertySize);
      break;
    case 0x9ad04dd7:
      sldrThis.selectedNodeCollapseMotion = SLdrSpline(input, propertySize);
      break;
    case 0x08719a9c:
      sldrThis.nodeExpandMotion = SLdrSpline(input, propertySize);
      break;
    case 0x11cd076f:
      sldrThis.rotationSpeed = input.ReadFloat();
      break;
    case 0xb71677d0:
      sldrThis.nodeScale = input.ReadFloat();
      break;
    case 0x42d0c2d2:
      sldrThis.selectedNodeScale = input.ReadFloat();
      break;
    case 0x70420ed1:
      sldrThis.selectedTextCursorScale = input.ReadFloat();
      break;
    case 0xc0cb3936:
      sldrThis.scanModelScale = input.ReadFloat();
      break;
    case 0x925b9d4a:
      sldrThis.unknown_0x925b9d4a = SLdrSpline(input, propertySize);
      break;
    case 0x075a0734:
      sldrThis.unknown_0x075a0734 = CColor(input);
      break;
    case 0x8ef43309:
      sldrThis.unknown_0x8ef43309 = input.ReadFloat();
      break;
    case 0x4194622e:
      sldrThis.fogNear = input.ReadFloat();
      break;
    case 0x3935a756:
      sldrThis.fogFar = input.ReadFloat();
      break;
    case 0xe578c0dd:
      sldrThis.fogColor = CColor(input);
      break;
    case 0xeef15783:
      sldrThis.unknown_0xeef15783 = input.ReadFloat();
      break;
    case 0x78055ab2:
      sldrThis.unknown_0x78055ab2 = input.ReadFloat();
      break;
    case 0x09e3197f:
      sldrThis.unknown_0x09e3197f = input.ReadFloat();
      break;
    case 0xfaffce1f:
      sldrThis.unknown_0xfaffce1f = input.ReadFloat();
      break;
    case 0xe5280e7c:
      sldrThis.unknown_0xe5280e7c = input.ReadFloat();
      break;
    case 0x647d9f91:
      sldrThis.backgroundSweepColor = CColor(input);
      break;
    case 0x7120f18f:
      sldrThis.backgroundSweepRadius = input.ReadFloat();
      break;
    case 0x4def660d:
      sldrThis.backgroundSweepTime = input.ReadFloat();
      break;
    case 0xc7c1967b:
      sldrThis.scanTextWindowBackgroundColor = CColor(input);
      break;
    case 0xf74bbad2:
      sldrThis.scanTextWindowBorderColor = CColor(input);
      break;
    case 0xe31421e2:
      sldrThis.scanTextWindowFontColor = CColor(input);
      break;
    case 0x0b8f0ddb:
      sldrThis.legendWindowBackgroundColor = CColor(input);
      break;
    case 0x0a7a4155:
      sldrThis.legendWindowBorderColor = CColor(input);
      break;
    case 0x527a0b10:
      sldrThis.legendWindowFontColor = CColor(input);
      break;
    case 0x7927ca67:
      sldrThis.legendHideTime = input.ReadFloat();
      break;
    case 0x7f35c573:
      sldrThis.unknown_0x7f35c573 = CColor(input);
      break;
    case 0xe0d77cf1:
      sldrThis.unknown_0xe0d77cf1 = CColor(input);
      break;
    case 0x903d9b7e:
      sldrThis.unknown_0x903d9b7e = CColor(input);
      break;
    case 0x0fdf22fc:
      sldrThis.unknown_0x0fdf22fc = CColor(input);
      break;
    case 0xbeaf9d6b:
      sldrThis.unknown_0xbeaf9d6b = CColor(input);
      break;
    case 0x7e8247d6:
      sldrThis.unknown_0x7e8247d6 = CColor(input);
      break;
    case 0x8b3f7902:
      sldrThis.unknown_0x8b3f7902 = CColor(input);
      break;
    case 0xdc1abb82:
      sldrThis.unknown_0xdc1abb82 = CColor(input);
      break;
    case 0x306b2b24:
      sldrThis.unknown_0x306b2b24 = CColor(input);
      break;
    case 0xa485372c:
      sldrThis.frameColor = CColor(input);
      break;
    case 0x1a4b8068:
      sldrThis.unknown_0x1a4b8068 = CColor(input);
      break;
    case 0xcc2f25f1:
      sldrThis.sliderBackgroundColor = CColor(input);
      break;
    case 0xcb8d5caf:
      sldrThis.sliderSelectionColor = CColor(input);
      break;
    case 0x3f44254f:
      sldrThis.sliderScale = input.ReadFloat();
      break;
    case 0x7087c377:
      sldrThis.sliderSpeed = input.ReadFloat();
      break;
    case 0x58795865:
      sldrThis.unknown_0x58795865 = input.ReadFloat();
      break;
    case 0x35e0bd31:
      sldrThis.unknown_0x35e0bd31 = input.ReadFloat();
      break;
    case 0xe7c283be:
      sldrThis.menuOptionColor = CColor(input);
      break;
    case 0xdabcb730:
      sldrThis.menuOptionEnabledArrowColor = CColor(input);
      break;
    case 0x9c2195d7:
      sldrThis.menuOptionDisabledArrowColor = CColor(input);
      break;
    case 0xfc54f555:
      sldrThis.menuOptionScale = input.ReadFloat();
      break;
    case 0xef549530:
      sldrThis.menuOptionArrowScale = input.ReadFloat();
      break;
    case 0xfa7a92b8:
      sldrThis.modelRotationSpeed = input.ReadFloat();
      break;
    case 0x60914b79:
      sldrThis.unknown_0x60914b79 = input.ReadFloat();
      break;
    case 0x1fd44d9b:
      sldrThis.unknown_0x1fd44d9b = input.ReadFloat();
      break;
    case 0x06c93afa:
      // sldrThis.modelLight1Position = TODO
      break;
    case 0x33924afb:
      sldrThis.modelLight1Color = CColor(input);
      break;
    case 0x17b45083:
      // sldrThis.modelLight2Position = TODO
      break;
    case 0xaa702cfa:
      sldrThis.modelLight2Color = CColor(input);
      break;
    case 0x62ff6d72:
      sldrThis.modelAmbientLightColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
