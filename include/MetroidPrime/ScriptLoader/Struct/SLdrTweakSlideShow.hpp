#ifndef _SLDRTWEAKSLIDESHOW
#define _SLDRTWEAKSLIDESHOW

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakSlideShow {
    SLdrTweakSlideShow();
    ~SLdrTweakSlideShow();

    rstl::string instanceName;
    rstl::string pakFile;
    rstl::string font;
    CColor fontColor;
    CColor fontOutlineColor;
    float unknown_0xd398dac2;
    float unknown_0x03757d08;
    float translationMultiplier;
    float scaleMultiplier;
    float slideShowDelay;
    CColor helpFrameColor;
    float helpTransitionTime;
    float slideBlendTime;
    float unknown_0x029d2082;
    float unknown_0xb187cd9b;
    float fadeInTime;
    float fadeOutTime;
    rstl::string unknown_0xc0544bc1;
};

void LoadTypedefSLdrTweakSlideShow(SLdrTweakSlideShow&, CInputStream&);

/* {
  SLdrTweakSlideShow sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
      break;
    case 0x2bd13ab3:
      // sldrThis.pakFile = TODO
      break;
    case 0xfe31fba0:
      // sldrThis.font = TODO
      break;
    case 0x1a96ec67:
      // sldrThis.fontColor = TODO
      break;
    case 0x844ab6b0:
      // sldrThis.fontOutlineColor = TODO
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
      // sldrThis.helpFrameColor = TODO
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
      // sldrThis.unknown_0xc0544bc1 = TODO
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKSLIDESHOW
