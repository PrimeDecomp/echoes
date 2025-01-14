#include "MetroidPrime/ScriptLoader/SLdrTweakBall_CannonBall.hpp"

SLdrTweakBall_CannonBall::SLdrTweakBall_CannonBall() {}

SLdrTweakBall_CannonBall::~SLdrTweakBall_CannonBall() {}

void LoadTypedefSLdrTweakBall_CannonBall(SLdrTweakBall_CannonBall& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xd51d8d71:
      LoadTypedefSLdrTDamageInfo(sldrThis.cannonBallDamage, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
