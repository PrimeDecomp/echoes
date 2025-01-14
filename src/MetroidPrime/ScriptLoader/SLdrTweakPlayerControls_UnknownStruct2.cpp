#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct2.hpp"

SLdrTweakPlayerControls_UnknownStruct2::SLdrTweakPlayerControls_UnknownStruct2() {
  unknown_0xff1b0413 = true;
  unknown_0xe0c1d958 = true;
  toggleAimPosition = false;
  unknown_0x1f99c6ba = false;
  unknown_0x18eb3ab5 = true;
  unknown_0xbdc01c71 = false;
  fixedVerticalAim = false;
  unknown_0xda97bbcd = false;
  orbitAroundEnemies = true;
  unknown_0xc224d966 = true;
  addGrenadeAlert = true;
  unknown_0x3fb16819 = true;
  unknown_0x4fcf4b70 = false;
  unknown_0x07bb06a6 = false;
  unknown_0x04d8d57b = false;
  unknown_0x5282c47e = true;
  fallingDoubleJump = false;
  impulseDoubleJump = true;
  unknown_0xa796a8b9 = true;
  unknown_0x7c0599c8 = true;
  unknown_0x522ab1ac = true;
}

SLdrTweakPlayerControls_UnknownStruct2::~SLdrTweakPlayerControls_UnknownStruct2() {}

void LoadTypedefSLdrTweakPlayerControls_UnknownStruct2(SLdrTweakPlayerControls_UnknownStruct2& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xff1b0413:
      sldrThis.unknown_0xff1b0413 = input.ReadBool();
      break;
    case 0xe0c1d958:
      sldrThis.unknown_0xe0c1d958 = input.ReadBool();
      break;
    case 0x55c20d58:
      sldrThis.toggleAimPosition = input.ReadBool();
      break;
    case 0x1f99c6ba:
      sldrThis.unknown_0x1f99c6ba = input.ReadBool();
      break;
    case 0x18eb3ab5:
      sldrThis.unknown_0x18eb3ab5 = input.ReadBool();
      break;
    case 0xbdc01c71:
      sldrThis.unknown_0xbdc01c71 = input.ReadBool();
      break;
    case 0x1c30f1a6:
      sldrThis.fixedVerticalAim = input.ReadBool();
      break;
    case 0xda97bbcd:
      sldrThis.unknown_0xda97bbcd = input.ReadBool();
      break;
    case 0x83583abd:
      sldrThis.orbitAroundEnemies = input.ReadBool();
      break;
    case 0xc224d966:
      sldrThis.unknown_0xc224d966 = input.ReadBool();
      break;
    case 0x1fcfaf3f:
      sldrThis.addGrenadeAlert = input.ReadBool();
      break;
    case 0x3fb16819:
      sldrThis.unknown_0x3fb16819 = input.ReadBool();
      break;
    case 0x4fcf4b70:
      sldrThis.unknown_0x4fcf4b70 = input.ReadBool();
      break;
    case 0x07bb06a6:
      sldrThis.unknown_0x07bb06a6 = input.ReadBool();
      break;
    case 0x04d8d57b:
      sldrThis.unknown_0x04d8d57b = input.ReadBool();
      break;
    case 0x5282c47e:
      sldrThis.unknown_0x5282c47e = input.ReadBool();
      break;
    case 0x7304dafa:
      sldrThis.fallingDoubleJump = input.ReadBool();
      break;
    case 0x7a49267d:
      sldrThis.impulseDoubleJump = input.ReadBool();
      break;
    case 0xa796a8b9:
      sldrThis.unknown_0xa796a8b9 = input.ReadBool();
      break;
    case 0x7c0599c8:
      sldrThis.unknown_0x7c0599c8 = input.ReadBool();
      break;
    case 0x522ab1ac:
      sldrThis.unknown_0x522ab1ac = input.ReadBool();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
