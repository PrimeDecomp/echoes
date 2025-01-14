#ifndef _SLDRTWEAKBALL
#define _SLDRTWEAKBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_BoostBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Camera.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_CannonBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_DeathBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Movement.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_ScrewAttack.hpp"
#include "rstl/string.hpp"

struct SLdrTweakBall {
    SLdrTweakBall();
    ~SLdrTweakBall();

    rstl::string instanceName;
    SLdrTweakBall_Movement movement;
    SLdrTweakBall_Camera camera;
    SLdrTweakBall_Misc misc;
    SLdrTweakBall_BoostBall boostBall;
    SLdrTweakBall_CannonBall cannonBall;
    SLdrTweakBall_ScrewAttack screwAttack;
    SLdrTweakBall_DeathBall deathBall;
};

void LoadTypedefSLdrTweakBall(SLdrTweakBall&, CInputStream&);

/* {
  SLdrTweakBall sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
      break;
    case 0x0def1ffb:
      LoadTypedefSLdrTweakBall_Movement(sldrThis.movement, input);
      break;
    case 0x7aac09b9:
      LoadTypedefSLdrTweakBall_Camera(sldrThis.camera, input);
      break;
    case 0x0c67b730:
      LoadTypedefSLdrTweakBall_Misc(sldrThis.misc, input);
      break;
    case 0xcb4ea3bf:
      LoadTypedefSLdrTweakBall_BoostBall(sldrThis.boostBall, input);
      break;
    case 0x5fb9e808:
      LoadTypedefSLdrTweakBall_CannonBall(sldrThis.cannonBall, input);
      break;
    case 0x4b1c7b7d:
      LoadTypedefSLdrTweakBall_ScrewAttack(sldrThis.screwAttack, input);
      break;
    case 0xbb5fc8a4:
      LoadTypedefSLdrTweakBall_DeathBall(sldrThis.deathBall, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKBALL
