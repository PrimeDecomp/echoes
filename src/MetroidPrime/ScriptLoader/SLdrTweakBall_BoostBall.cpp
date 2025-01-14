#include "MetroidPrime/ScriptLoader/SLdrTweakBall_BoostBall.hpp"

SLdrTweakBall_BoostBall::SLdrTweakBall_BoostBall() {
  boostBallDrainTime = 0.33000001f;
  boostBallMinChargeTime = 0.25f;
  boostBallMinRelativeSpeedForDamage = 10.0f;
  boostBallChargeTime1 = 0.25f;
  boostBallChargeTime2 = 0.5f;
  boostBallMaxChargeTime = 1.0f;
  boostBallIncrementalSpeed1 = 35.0f;
  boostBallIncrementalSpeed2 = 35.0f;
  boostBallIncrementalSpeed3 = 35.0f;
  unknown_0xbe605660 = 1.7f;
  unknown_0x6d210beb = 16.0f;
  unknown_0xfdc6649d = 32.0f;
  unknown_0x340be92f = 8.0f;
}

SLdrTweakBall_BoostBall::~SLdrTweakBall_BoostBall() {}

void LoadTypedefSLdrTweakBall_BoostBall(SLdrTweakBall_BoostBall& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7f336c73:
      sldrThis.boostBallDrainTime = input.ReadFloat();
      break;
    case 0x0d23dbe7:
      sldrThis.boostBallMinChargeTime = input.ReadFloat();
      break;
    case 0x729e3cb3:
      sldrThis.boostBallMinRelativeSpeedForDamage = input.ReadFloat();
      break;
    case 0xef7a8e16:
      sldrThis.boostBallChargeTime1 = input.ReadFloat();
      break;
    case 0x69eefcb8:
      sldrThis.boostBallChargeTime2 = input.ReadFloat();
      break;
    case 0x5e85c303:
      sldrThis.boostBallMaxChargeTime = input.ReadFloat();
      break;
    case 0x890d4ae5:
      sldrThis.boostBallIncrementalSpeed1 = input.ReadFloat();
      break;
    case 0x0f99384b:
      sldrThis.boostBallIncrementalSpeed2 = input.ReadFloat();
      break;
    case 0xc4c5ebee:
      sldrThis.boostBallIncrementalSpeed3 = input.ReadFloat();
      break;
    case 0xbe605660:
      sldrThis.unknown_0xbe605660 = input.ReadFloat();
      break;
    case 0x17e38e7e:
      LoadTypedefSLdrTDamageInfo(sldrThis.boostBallDamage, input);
      break;
    case 0x6d210beb:
      sldrThis.unknown_0x6d210beb = input.ReadFloat();
      break;
    case 0xfdc6649d:
      sldrThis.unknown_0xfdc6649d = input.ReadFloat();
      break;
    case 0x340be92f:
      sldrThis.unknown_0x340be92f = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
