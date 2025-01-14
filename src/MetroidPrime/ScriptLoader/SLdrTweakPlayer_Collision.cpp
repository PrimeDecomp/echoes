#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Collision.hpp"

SLdrTweakPlayer_Collision::SLdrTweakPlayer_Collision() {
  playerHeight = 2.7f;
  playerRadius = 0.5f;
  stepUpHeight = 1.0f;
  stepDownHeight = 0.80000001f;
  ballRadius = 0.69999999f;
}

SLdrTweakPlayer_Collision::~SLdrTweakPlayer_Collision() {}

void LoadTypedefSLdrTweakPlayer_Collision(SLdrTweakPlayer_Collision& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xd0f345b2:
      sldrThis.playerHeight = input.ReadFloat();
      break;
    case 0x6a884154:
      sldrThis.playerRadius = input.ReadFloat();
      break;
    case 0xd9355674:
      sldrThis.stepUpHeight = input.ReadFloat();
      break;
    case 0x88ea81db:
      sldrThis.stepDownHeight = input.ReadFloat();
      break;
    case 0x0e2f537f:
      sldrThis.ballRadius = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
