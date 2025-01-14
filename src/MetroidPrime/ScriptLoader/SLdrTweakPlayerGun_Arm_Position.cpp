#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Arm_Position.hpp"

SLdrTweakPlayerGun_Arm_Position::SLdrTweakPlayerGun_Arm_Position() {
}

SLdrTweakPlayerGun_Arm_Position::~SLdrTweakPlayerGun_Arm_Position() {}

void LoadTypedefSLdrTweakPlayerGun_Arm_Position(SLdrTweakPlayerGun_Arm_Position& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x3c9366ac:
      // sldrThis.normal = TODO
      break;
    case 0x66b1d066:
      // sldrThis.grappling = TODO
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
