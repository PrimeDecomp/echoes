#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakSlideShow.hpp"

SLdrTweakSlideShow::SLdrTweakSlideShow()
  : instanceName()
  , pakFile()
  , font()
  , fontColor({'R': 0.0, 'G': 0.0, 'B': 0.0})
  , fontOutlineColor({'R': 0.0, 'G': 0.0, 'B': 0.0})
  , unknown_0xd398dac2(50.0)
  , unknown_0x03757d08(50.0)
  , translationMultiplier(10.0)
  , scaleMultiplier(10.0)
  , slideShowDelay(3.0)
  , helpFrameColor({'R': 0.0, 'G': 0.0, 'B': 0.0})
  , helpTransitionTime(1.0)
  , slideBlendTime(0.5)
  , unknown_0x029d2082(3.0)
  , unknown_0xb187cd9b(0.5)
  , fadeInTime(0.001)
  , fadeOutTime(2.0)
  , unknown_0xc0544bc1()
{}

SLdrTweakSlideShow::~SLdrTweakSlideShow() {}

void LoadTypedefSLdrTweakSlideShow(SLdrTweakSlideShow& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0x2bd13ab3:
      sldrThis.pakFile = rstl::string(input);
      break;
    case 0xfe31fba0:
      sldrThis.font = rstl::string(input);
      break;
    case 0x1a96ec67:
      sldrThis.fontColor = CColor(input);
      break;
    case 0x844ab6b0:
      sldrThis.fontOutlineColor = CColor(input);
      break;
    case 0xd398dac2:
      sldrThis.unknown_0xd398dac2 = input.ReadFloat();
      break;
    case 0x03757d08:
      sldrThis.unknown_0x03757d08 = input.ReadFloat();
      break;
    case 0x59276e14:
      sldrThis.translationMultiplier = input.ReadFloat();
      break;
    case 0x3ce7a013:
      sldrThis.scaleMultiplier = input.ReadFloat();
      break;
    case 0x278c0893:
      sldrThis.slideShowDelay = input.ReadFloat();
      break;
    case 0xd75d29f8:
      sldrThis.helpFrameColor = CColor(input);
      break;
    case 0x27515c7b:
      sldrThis.helpTransitionTime = input.ReadFloat();
      break;
    case 0xae47dc81:
      sldrThis.slideBlendTime = input.ReadFloat();
      break;
    case 0x029d2082:
      sldrThis.unknown_0x029d2082 = input.ReadFloat();
      break;
    case 0xb187cd9b:
      sldrThis.unknown_0xb187cd9b = input.ReadFloat();
      break;
    case 0x90aa341f:
      sldrThis.fadeInTime = input.ReadFloat();
      break;
    case 0x7c269ebc:
      sldrThis.fadeOutTime = input.ReadFloat();
      break;
    case 0xc0544bc1:
      sldrThis.unknown_0xc0544bc1 = rstl::string(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}