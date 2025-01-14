#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_MapScreenIcons.hpp"

SLdrTweakPlayerRes_MapScreenIcons::SLdrTweakPlayerRes_MapScreenIcons() {}

SLdrTweakPlayerRes_MapScreenIcons::~SLdrTweakPlayerRes_MapScreenIcons() {}

void LoadTypedefSLdrTweakPlayerRes_MapScreenIcons(SLdrTweakPlayerRes_MapScreenIcons& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x2c770bb8:
      sldrThis.lStickN = rstl::string(input);
      break;
    case 0x49aec38c:
      sldrThis.lStickU = rstl::string(input);
      break;
    case 0xa86af8c2:
      sldrThis.lStickUl = rstl::string(input);
      break;
    case 0xbbe81a91:
      sldrThis.lStickL = rstl::string(input);
      break;
    case 0x187bc977:
      sldrThis.lStickDl = rstl::string(input);
      break;
    case 0x880754f6:
      sldrThis.lStickD = rstl::string(input);
      break;
    case 0x2f9a2ee4:
      sldrThis.lStickDr = rstl::string(input);
      break;
    case 0x8c09fd02:
      sldrThis.lStickR = rstl::string(input);
      break;
    case 0x9f8b1f51:
      sldrThis.lStickUr = rstl::string(input);
      break;
    case 0x0b07de8d:
      sldrThis.cStickN = rstl::string(input);
      break;
    case 0x6ede16b9:
      sldrThis.cStickU = rstl::string(input);
      break;
    case 0xfefe4c34:
      sldrThis.cStickUl = rstl::string(input);
      break;
    case 0x9c98cfa4:
      sldrThis.cStickL = rstl::string(input);
      break;
    case 0x4eef7d81:
      sldrThis.cStickDl = rstl::string(input);
      break;
    case 0xaf7781c3:
      sldrThis.cStickD = rstl::string(input);
      break;
    case 0x790e9a12:
      sldrThis.cStickDr = rstl::string(input);
      break;
    case 0xab792837:
      sldrThis.cStickR = rstl::string(input);
      break;
    case 0xc91faba7:
      sldrThis.cStickUr = rstl::string(input);
      break;
    case 0x40c21e1e:
      sldrThis.lTriggerOut = rstl::string(input);
      break;
    case 0x120368b8:
      sldrThis.lTriggerIn = rstl::string(input);
      break;
    case 0x16b77ff5:
      sldrThis.rTriggerOut = rstl::string(input);
      break;
    case 0xd4a6a08d:
      sldrThis.rTriggerIn = rstl::string(input);
      break;
    case 0x272f08b4:
      sldrThis.startButtonOut = rstl::string(input);
      break;
    case 0x225f0e23:
      sldrThis.startButtonIn = rstl::string(input);
      break;
    case 0x1c208ab1:
      sldrThis.aButtonOut = rstl::string(input);
      break;
    case 0x43fdc303:
      sldrThis.aButtonIn = rstl::string(input);
      break;
    case 0x35e83e43:
      sldrThis.bButtonOut = rstl::string(input);
      break;
    case 0x5280a97a:
      sldrThis.bButtonIn = rstl::string(input);
      break;
    case 0x277cbaf1:
      sldrThis.xButtonOut = rstl::string(input);
      break;
    case 0x71ed4b23:
      sldrThis.xButtonIn = rstl::string(input);
      break;
    case 0x89142b60:
      sldrThis.yButtonOut = rstl::string(input);
      break;
    case 0xc81690cb:
      sldrThis.yButtonIn = rstl::string(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
