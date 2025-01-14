#include "MetroidPrime/ScriptLoader/SLdrTweakBall_DeathBall.hpp"

SLdrTweakBall_DeathBall::SLdrTweakBall_DeathBall() {
  deathBallDamageDelay = 0.5f;
}

SLdrTweakBall_DeathBall::~SLdrTweakBall_DeathBall() {}

void LoadTypedefSLdrTweakBall_DeathBall(SLdrTweakBall_DeathBall& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xc915686f:
      sldrThis.deathBallDamageDelay = input.ReadFloat();
      break;
    case 0xccccef24:
      LoadTypedefSLdrTDamageInfo(sldrThis.deathBallDamage, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
