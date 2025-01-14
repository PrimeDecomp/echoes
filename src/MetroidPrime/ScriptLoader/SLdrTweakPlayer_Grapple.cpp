#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Grapple.hpp"

SLdrTweakPlayer_Grapple::SLdrTweakPlayer_Grapple() {
  grappleDistance = 25.0f;
  grappleBeamLength = 10.0f;
  grappleSwingTime = 3.3f;
  grappleMaxVelocity = 23.0f;
  grappleCameraSpeed = 90.0f;
  grapplePullCloseDistance = 0.5f;
  grapplePullDampenDistance = 2.0f;
  grapplePullVelocity = 30.0f;
  grapplePullCameraSpeed = 90.0f;
  grappleTurnRate = 35.0f;
  grappleJumpForce = 13.0f;
  grappleReleaseTime = 1.0f;
  grappleControlScheme = 2;
  grappleHoldOrbitButton = true;
  grappleTurnControlsReversed = true;
}

SLdrTweakPlayer_Grapple::~SLdrTweakPlayer_Grapple() {}

void LoadTypedefSLdrTweakPlayer_Grapple(SLdrTweakPlayer_Grapple& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xa726316b:
      sldrThis.grappleDistance = input.ReadFloat();
      break;
    case 0x33e79b51:
      sldrThis.grappleBeamLength = input.ReadFloat();
      break;
    case 0x9dd3e88b:
      sldrThis.grappleSwingTime = input.ReadFloat();
      break;
    case 0xfb605ba4:
      sldrThis.grappleMaxVelocity = input.ReadFloat();
      break;
    case 0xfe98b8e9:
      sldrThis.grappleCameraSpeed = input.ReadFloat();
      break;
    case 0x9210a25e:
      sldrThis.grapplePullCloseDistance = input.ReadFloat();
      break;
    case 0xbe0c8b5e:
      sldrThis.grapplePullDampenDistance = input.ReadFloat();
      break;
    case 0x2603a0be:
      sldrThis.grapplePullVelocity = input.ReadFloat();
      break;
    case 0x5b98a3bd:
      sldrThis.grapplePullCameraSpeed = input.ReadFloat();
      break;
    case 0x87d4b5d6:
      sldrThis.grappleTurnRate = input.ReadFloat();
      break;
    case 0xb7f82a9f:
      sldrThis.grappleJumpForce = input.ReadFloat();
      break;
    case 0x395478a8:
      sldrThis.grappleReleaseTime = input.ReadFloat();
      break;
    case 0x93c013c9:
      sldrThis.grappleControlScheme = input.ReadInt32();
      break;
    case 0x8eeed636:
      sldrThis.grappleHoldOrbitButton = input.ReadBool();
      break;
    case 0xe1eb12e2:
      sldrThis.grappleTurnControlsReversed = input.ReadBool();
      break;
    case 0xae1fc47c:
      LoadTypedefSLdrTweakPlayer_GrappleBeam(sldrThis.beam, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
