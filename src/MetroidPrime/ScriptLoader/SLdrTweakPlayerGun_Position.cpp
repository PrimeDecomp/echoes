#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Position.hpp"

SLdrTweakPlayerGun_Position::SLdrTweakPlayerGun_Position() {
  unknown = 0.30000001f;
  x = 0.25f;
  y = 0.30000001f;
  z = -0.34999999f;
}

SLdrTweakPlayerGun_Position::~SLdrTweakPlayerGun_Position() {}

void LoadTypedefSLdrTweakPlayerGun_Position(SLdrTweakPlayerGun_Position& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x1547d77b:
      sldrThis.unknown = input.ReadFloat();
      break;
    case 0xa1677d4e:
      sldrThis.x = input.ReadFloat();
      break;
    case 0x6a3baeeb:
      sldrThis.y = input.ReadFloat();
      break;
    case 0xecafdc45:
      sldrThis.z = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
