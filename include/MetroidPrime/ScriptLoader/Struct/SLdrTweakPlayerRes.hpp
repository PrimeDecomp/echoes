#ifndef _SLDRTWEAKPLAYERRES
#define _SLDRTWEAKPLAYERRES

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTBallTransitionResources.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTGunResources.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_AutoMapperIcons.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_MapScreenIcons.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerRes {
    SLdrTweakPlayerRes();
    ~SLdrTweakPlayerRes();

    rstl::string instanceName;
    SLdrTweakPlayerRes_AutoMapperIcons autoMapperIcons;
    SLdrTweakPlayerRes_MapScreenIcons mapScreenIcons;
    SLdrTBallTransitionResources ballTransitionResources;
    SLdrTGunResources cinematicResources;
    float unknown;
};

void LoadTypedefSLdrTweakPlayerRes(SLdrTweakPlayerRes&, CInputStream&);

/* {
  SLdrTweakPlayerRes sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
      break;
    case 0x357741e0:
      LoadTypedefSLdrTweakPlayerRes_AutoMapperIcons(sldrThis.autoMapperIcons, input);
      break;
    case 0x0d5e02a0:
      LoadTypedefSLdrTweakPlayerRes_MapScreenIcons(sldrThis.mapScreenIcons, input);
      break;
    case 0x279852ba:
      LoadTypedefSLdrTBallTransitionResources(sldrThis.ballTransitionResources, input);
      break;
    case 0x5e630608:
      LoadTypedefSLdrTGunResources(sldrThis.cinematicResources, input);
      break;
    case 0x36ad9d19:
      sldrThis.unknown = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKPLAYERRES
