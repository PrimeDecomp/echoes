#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakBall.hpp"

SLdrTweakBall::SLdrTweakBall() {}

SLdrTweakBall::~SLdrTweakBall() {}

void LoadTypedefSLdrTweakBall(SLdrTweakBall& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
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
}
