#include "MetroidPrime/ScriptLoader/SLdrTweakBall_ScrewAttack.hpp"

SLdrTweakBall_ScrewAttack::SLdrTweakBall_ScrewAttack() {
  screwAttackGravity = 40.0f;
  unknown_0xcb77fb28 = 4.5f;
  unknown_0x3fdeb046 = 4.5f;
  unknown_0x691b244d = 5;
  screwAttackVerticalJumpVelocity = 20.0f;
  screwAttackHorizontalJumpVelocity = 20.0f;
  unknown_0x3d03d8a6 = 10.0f;
  unknown_0xf1f2498f = 0.75f;
  unknown_0x4b0aba1c = 0.75f;
  screwAttackWallJumpMaxTime = 0.1f;
  screwAttackWallJumpVerticalVelocity = 20.0f;
  screwAttackWallJumpHorizontalVelocity = 20.0f;
  screwAttackWallJumpGravity = 40.0f;
}

SLdrTweakBall_ScrewAttack::~SLdrTweakBall_ScrewAttack() {}

void LoadTypedefSLdrTweakBall_ScrewAttack(SLdrTweakBall_ScrewAttack& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7eab4bab:
      sldrThis.screwAttackGravity = input.ReadFloat();
      break;
    case 0xcb77fb28:
      sldrThis.unknown_0xcb77fb28 = input.ReadFloat();
      break;
    case 0x3fdeb046:
      sldrThis.unknown_0x3fdeb046 = input.ReadFloat();
      break;
    case 0x691b244d:
      sldrThis.unknown_0x691b244d = input.ReadInt32();
      break;
    case 0x591da5b4:
      sldrThis.screwAttackVerticalJumpVelocity = input.ReadFloat();
      break;
    case 0xe428f231:
      sldrThis.screwAttackHorizontalJumpVelocity = input.ReadFloat();
      break;
    case 0x3d03d8a6:
      sldrThis.unknown_0x3d03d8a6 = input.ReadFloat();
      break;
    case 0xf1f2498f:
      sldrThis.unknown_0xf1f2498f = input.ReadFloat();
      break;
    case 0x4b0aba1c:
      sldrThis.unknown_0x4b0aba1c = input.ReadFloat();
      break;
    case 0x14351140:
      sldrThis.screwAttackWallJumpMaxTime = input.ReadFloat();
      break;
    case 0x5dec33c5:
      sldrThis.screwAttackWallJumpVerticalVelocity = input.ReadFloat();
      break;
    case 0xb8431384:
      sldrThis.screwAttackWallJumpHorizontalVelocity = input.ReadFloat();
      break;
    case 0x38f84011:
      sldrThis.screwAttackWallJumpGravity = input.ReadFloat();
      break;
    case 0xb372ecab:
      LoadTypedefSLdrTDamageInfo(sldrThis.screwAttackDamage, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
