#ifndef _SLDRTWEAKPARTICLE
#define _SLDRTWEAKPARTICLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakParticle {
    SLdrTweakParticle();
    ~SLdrTweakParticle();

    rstl::string instanceName;
    rstl::string pakFile;
    rstl::string primaryWeapon;
    rstl::string secondaryWeapon;
};

void LoadTypedefSLdrTweakParticle(SLdrTweakParticle&, CInputStream&);

/* {
  SLdrTweakParticle sldrThis;
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
    case 0x5e15868a:
      // sldrThis.primaryWeapon = TODO
      break;
    case 0xbc401445:
      // sldrThis.secondaryWeapon = TODO
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKPARTICLE
