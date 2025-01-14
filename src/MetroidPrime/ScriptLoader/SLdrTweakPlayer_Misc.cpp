#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Misc.hpp"

SLdrTweakPlayer_Misc::SLdrTweakPlayer_Misc() {
  eyeOffset = 0.2f;
  normalTurnFactor = 1.0f;
  freeLookTurnFactor = 1.0f;
  freeLookMaxX = 100.0f;
  freeLookMaxZ = 70.0f;
  freeLookSpeed = 100.0f;
  freeLookSnapSpeed = 200.0f;
  freeLookFadeAngle = 5.0f;
  freeLookMinAngle = 0.1f;
  freeLookCenteredTime = 0.25f;
  freeLookDampenFactor = 80.0f;
  nullAnalogScales = false;
  unknown = 5.0f;
  leftAnalogMax = 1.0f;
  rightAnalogMax = 1.0f;
}

SLdrTweakPlayer_Misc::~SLdrTweakPlayer_Misc() {}

void LoadTypedefSLdrTweakPlayer_Misc(SLdrTweakPlayer_Misc& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xb49b008e:
      sldrThis.eyeOffset = input.ReadFloat();
      break;
    case 0xb0291470:
      sldrThis.normalTurnFactor = input.ReadFloat();
      break;
    case 0x287892ab:
      sldrThis.freeLookTurnFactor = input.ReadFloat();
      break;
    case 0x79f8ef93:
      sldrThis.freeLookMaxX = input.ReadFloat();
      break;
    case 0x34304e98:
      sldrThis.freeLookMaxZ = input.ReadFloat();
      break;
    case 0xba27556f:
      sldrThis.freeLookSpeed = input.ReadFloat();
      break;
    case 0xd7d7828e:
      sldrThis.freeLookSnapSpeed = input.ReadFloat();
      break;
    case 0x0e0c431e:
      sldrThis.freeLookFadeAngle = input.ReadFloat();
      break;
    case 0x2c1da0ec:
      sldrThis.freeLookMinAngle = input.ReadFloat();
      break;
    case 0xe11788e4:
      sldrThis.freeLookCenteredTime = input.ReadFloat();
      break;
    case 0xc982754e:
      sldrThis.freeLookDampenFactor = input.ReadFloat();
      break;
    case 0xfb5c81a9:
      sldrThis.nullAnalogScales = input.ReadBool();
      break;
    case 0xfb909bc3:
      sldrThis.unknown = input.ReadFloat();
      break;
    case 0xf1f038de:
      sldrThis.leftAnalogMax = input.ReadFloat();
      break;
    case 0x2b1f5094:
      sldrThis.rightAnalogMax = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
