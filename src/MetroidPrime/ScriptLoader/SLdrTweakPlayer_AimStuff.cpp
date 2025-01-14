#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_AimStuff.hpp"

SLdrTweakPlayer_AimStuff::SLdrTweakPlayer_AimStuff() {
  aimMinTime = 0.1f;
  aimMaxTime = 0.25f;
  aimMaxDistance = 100.0f;
  aimMaxAngleLeft = 30.0f;
  aimMaxAngleRight = 30.0f;
  aimMaxAngleUp = 50.0f;
  aimMaxAngleDown = 60.0f;
  aimAnglePerSecond = 110.0f;
  aimThresholdDistance = 5.0f;
  aimTurnAnglePerSecond = 360.0f;
  unknown = 10.0f;
  aimBoxWidth = 20.0f;
  aimBoxHeight = 10.0f;
  aimTargetTimer = 0.0099999998f;
  aimAssistHorizontalAngle = 5.0f;
  aimAssistVerticalAngle = 5.0f;
}

SLdrTweakPlayer_AimStuff::~SLdrTweakPlayer_AimStuff() {}

void LoadTypedefSLdrTweakPlayer_AimStuff(SLdrTweakPlayer_AimStuff& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x27c60d0a:
      sldrThis.aimMinTime = input.ReadFloat();
      break;
    case 0xb7b51de0:
      sldrThis.aimMaxTime = input.ReadFloat();
      break;
    case 0xf77d0359:
      sldrThis.aimMaxDistance = input.ReadFloat();
      break;
    case 0xde887cca:
      sldrThis.aimMaxAngleLeft = input.ReadFloat();
      break;
    case 0xb4b07d5d:
      sldrThis.aimMaxAngleRight = input.ReadFloat();
      break;
    case 0xe5f8c567:
      sldrThis.aimMaxAngleUp = input.ReadFloat();
      break;
    case 0x9774749d:
      sldrThis.aimMaxAngleDown = input.ReadFloat();
      break;
    case 0x133f3002:
      sldrThis.aimAnglePerSecond = input.ReadFloat();
      break;
    case 0x96fab602:
      sldrThis.aimThresholdDistance = input.ReadFloat();
      break;
    case 0x94164a2f:
      sldrThis.aimTurnAnglePerSecond = input.ReadFloat();
      break;
    case 0x54354c80:
      sldrThis.unknown = input.ReadFloat();
      break;
    case 0x5361ce18:
      sldrThis.aimBoxWidth = input.ReadFloat();
      break;
    case 0x4b2e9260:
      sldrThis.aimBoxHeight = input.ReadFloat();
      break;
    case 0x3b9a3789:
      sldrThis.aimTargetTimer = input.ReadFloat();
      break;
    case 0x38dd0b85:
      sldrThis.aimAssistHorizontalAngle = input.ReadFloat();
      break;
    case 0x1157883e:
      sldrThis.aimAssistVerticalAngle = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
