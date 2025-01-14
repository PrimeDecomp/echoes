#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Motion.hpp"

SLdrTweakPlayer_Motion::SLdrTweakPlayer_Motion() {
  forwardAccelNormal = 35000.0f;
  forwardAccelAir = 8000.0f;
  forwardAccelIce = 35000.0f;
  forwardAccelOrganic = 35000.0f;
  forwardAccelWater = 20000.0f;
  forwardAccelLava = 20000.0f;
  forwardAccelPhazon = 20000.0f;
  forwardAccelShrubbery = 20000.0f;
  rotationalAccelNormal = 14000.0f;
  rotationalAccelAir = 14000.0f;
  rotationalAccelIce = 14000.0f;
  rotationalAccelOrganic = 14000.0f;
  rotationalAccelWater = 14000.0f;
  rotationalAccelLava = 14000.0f;
  rotationalAccelPhazon = 14000.0f;
  rotationalAccelShrubbery = 14000.0f;
  movementFrictionNormal = 1.0f;
  movementFrictionAir = 0.75f;
  movementFrictionIce = 1.0f;
  movementFrictionOrganic = 1.0f;
  movementFrictionWater = 1.0f;
  movementFrictionLava = 1.0f;
  movementFrictionPhazon = 1.0f;
  movementFrictionShrubbery = 1.0f;
  rotationFrictionNormal = 0.44999999f;
  rotationFrictionAir = 0.44999999f;
  rotationFrictionIce = 0.44999999f;
  rotationFrictionOrganic = 0.44999999f;
  rotationFrictionWater = 0.44999999f;
  rotationFrictionLava = 0.44999999f;
  rotationFrictionPhazon = 0.44999999f;
  rotationFrictionShrubbery = 0.44999999f;
  rotationMaxSpeedNormal = 2.5f;
  rotationMaxSpeedAir = 2.5f;
  rotationMaxSpeedIce = 2.5f;
  rotationMaxSpeedOrganic = 2.5f;
  rotationMaxSpeedWater = 2.5f;
  rotationMaxSpeedLava = 2.5f;
  rotationMaxSpeedPhazon = 2.5f;
  rotationMaxSpeedShrubbery = 2.5f;
  forwardMaxSpeedNormal = 16.5f;
  forwardMaxSpeedAir = 16.5f;
  forwardMaxSpeedIce = 16.5f;
  forwardMaxSpeedOrganic = 16.5f;
  forwardMaxSpeedWater = 12.5f;
  forwardMaxSpeedLava = 12.5f;
  forwardMaxSpeedPhazon = 12.5f;
  forwardMaxSpeedShrubbery = 12.5f;
  gravitationalAccel = -35.0f;
  fluidGravitationalAccel = -10.0f;
  verticalJumpAccel = 50.0f;
  horizontalJumpAccel = 50.0f;
  verticalDoubleJumpAccel = 60.0f;
  horizontalDoubleJumpAccel = 60.0f;
  waterJumpFactor = 0.37f;
  waterBallJumpFactor = 0.37f;
  lavaJumpFactor = 0.37f;
  lavaBallJumpFactor = 0.37f;
  phazonJumpFactor = 0.37f;
  phazonBallJumpFactor = 0.37f;
  allowedJumpTime = 0.24950001f;
  allowedDoubleJumpTime = 0.1f;
  minDoubleJumpWindow = 0.0f;
  maxDoubleJumpWindow = 2.0f;
  unknown = 0.0f;
  minJumpTime = 0.235f;
  minDoubleJumpTime = 0.1f;
  ledgeFallTime = 0.050000001f;
  doubleJumpImpulse = 8.0f;
  backwardsForceMultiplier = 1.0f;
  bombJumpHeight = 7.9000001f;
  bombJumpRadius = 1.5f;
  gravityBoostTime = 1.5f;
  gravityBoostForce = 9000.0f;
  gravityBoostCancelDampening = 0.30000001f;
  gravityBoostMultipleAllowed = false;
}

SLdrTweakPlayer_Motion::~SLdrTweakPlayer_Motion() {}

void LoadTypedefSLdrTweakPlayer_Motion(SLdrTweakPlayer_Motion& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x18d0b2da:
      sldrThis.forwardAccelNormal = input.ReadFloat();
      break;
    case 0x84f61ac5:
      sldrThis.forwardAccelAir = input.ReadFloat();
      break;
    case 0xedb06c1d:
      sldrThis.forwardAccelIce = input.ReadFloat();
      break;
    case 0x56f9f2af:
      sldrThis.forwardAccelOrganic = input.ReadFloat();
      break;
    case 0xd05b643f:
      sldrThis.forwardAccelWater = input.ReadFloat();
      break;
    case 0x122ce118:
      sldrThis.forwardAccelLava = input.ReadFloat();
      break;
    case 0xf848dabe:
      sldrThis.forwardAccelPhazon = input.ReadFloat();
      break;
    case 0x68ac6028:
      sldrThis.forwardAccelShrubbery = input.ReadFloat();
      break;
    case 0xcd4ee9fc:
      sldrThis.rotationalAccelNormal = input.ReadFloat();
      break;
    case 0x3c461db2:
      sldrThis.rotationalAccelAir = input.ReadFloat();
      break;
    case 0x55006b6a:
      sldrThis.rotationalAccelIce = input.ReadFloat();
      break;
    case 0x8421e909:
      sldrThis.rotationalAccelOrganic = input.ReadFloat();
      break;
    case 0x2e41a61d:
      sldrThis.rotationalAccelWater = input.ReadFloat();
      break;
    case 0xdcf5b580:
      sldrThis.rotationalAccelLava = input.ReadFloat();
      break;
    case 0x2dd68198:
      sldrThis.rotationalAccelPhazon = input.ReadFloat();
      break;
    case 0x1a946073:
      sldrThis.rotationalAccelShrubbery = input.ReadFloat();
      break;
    case 0xd4a25028:
      sldrThis.movementFrictionNormal = input.ReadFloat();
      break;
    case 0x2b5cb136:
      sldrThis.movementFrictionAir = input.ReadFloat();
      break;
    case 0x421ac7ee:
      sldrThis.movementFrictionIce = input.ReadFloat();
      break;
    case 0x0586137d:
      sldrThis.movementFrictionOrganic = input.ReadFloat();
      break;
    case 0xaece038b:
      sldrThis.movementFrictionWater = input.ReadFloat();
      break;
    case 0x3637e815:
      sldrThis.movementFrictionLava = input.ReadFloat();
      break;
    case 0x343a384c:
      sldrThis.movementFrictionPhazon = input.ReadFloat();
      break;
    case 0x0caf4624:
      sldrThis.movementFrictionShrubbery = input.ReadFloat();
      break;
    case 0x34b2c148:
      sldrThis.rotationFrictionNormal = input.ReadFloat();
      break;
    case 0xb917ae8a:
      sldrThis.rotationFrictionAir = input.ReadFloat();
      break;
    case 0xd051d852:
      sldrThis.rotationFrictionIce = input.ReadFloat();
      break;
    case 0x48d462b4:
      sldrThis.rotationFrictionOrganic = input.ReadFloat();
      break;
    case 0x6bb67d81:
      sldrThis.rotationFrictionWater = input.ReadFloat();
      break;
    case 0xf4725cad:
      sldrThis.rotationFrictionLava = input.ReadFloat();
      break;
    case 0xd42aa92c:
      sldrThis.rotationFrictionPhazon = input.ReadFloat();
      break;
    case 0x319526db:
      sldrThis.rotationFrictionShrubbery = input.ReadFloat();
      break;
    case 0x4b6eb6ca:
      sldrThis.rotationMaxSpeedNormal = input.ReadFloat();
      break;
    case 0xc107f3db:
      sldrThis.rotationMaxSpeedAir = input.ReadFloat();
      break;
    case 0xa8418503:
      sldrThis.rotationMaxSpeedIce = input.ReadFloat();
      break;
    case 0x4b1d5ccf:
      sldrThis.rotationMaxSpeedOrganic = input.ReadFloat();
      break;
    case 0xddcc44cd:
      sldrThis.rotationMaxSpeedWater = input.ReadFloat();
      break;
    case 0xe8662d92:
      sldrThis.rotationMaxSpeedLava = input.ReadFloat();
      break;
    case 0xabf6deae:
      sldrThis.rotationMaxSpeedPhazon = input.ReadFloat();
      break;
    case 0x3c3f9884:
      sldrThis.rotationMaxSpeedShrubbery = input.ReadFloat();
      break;
    case 0xffd4a030:
      sldrThis.forwardMaxSpeedNormal = input.ReadFloat();
      break;
    case 0x59dfbcb9:
      sldrThis.forwardMaxSpeedAir = input.ReadFloat();
      break;
    case 0x3099ca61:
      sldrThis.forwardMaxSpeedIce = input.ReadFloat();
      break;
    case 0x16c1fddb:
      sldrThis.forwardMaxSpeedOrganic = input.ReadFloat();
      break;
    case 0x6c648931:
      sldrThis.forwardMaxSpeedWater = input.ReadFloat();
      break;
    case 0x4b42f5a9:
      sldrThis.forwardMaxSpeedLava = input.ReadFloat();
      break;
    case 0x1f4cc854:
      sldrThis.forwardMaxSpeedPhazon = input.ReadFloat();
      break;
    case 0xb3408173:
      sldrThis.forwardMaxSpeedShrubbery = input.ReadFloat();
      break;
    case 0x14b78aaf:
      sldrThis.gravitationalAccel = input.ReadFloat();
      break;
    case 0x2c7620d3:
      sldrThis.fluidGravitationalAccel = input.ReadFloat();
      break;
    case 0x0c2c91f7:
      sldrThis.verticalJumpAccel = input.ReadFloat();
      break;
    case 0x938c77d4:
      sldrThis.horizontalJumpAccel = input.ReadFloat();
      break;
    case 0x13c95dfd:
      sldrThis.verticalDoubleJumpAccel = input.ReadFloat();
      break;
    case 0x8e41fed2:
      sldrThis.horizontalDoubleJumpAccel = input.ReadFloat();
      break;
    case 0xb261fa30:
      sldrThis.waterJumpFactor = input.ReadFloat();
      break;
    case 0x6ae560e9:
      sldrThis.waterBallJumpFactor = input.ReadFloat();
      break;
    case 0x03149633:
      sldrThis.lavaJumpFactor = input.ReadFloat();
      break;
    case 0xd7b3f3ea:
      sldrThis.lavaBallJumpFactor = input.ReadFloat();
      break;
    case 0xaf1450a2:
      sldrThis.phazonJumpFactor = input.ReadFloat();
      break;
    case 0x980d701a:
      sldrThis.phazonBallJumpFactor = input.ReadFloat();
      break;
    case 0xa805feae:
      sldrThis.allowedJumpTime = input.ReadFloat();
      break;
    case 0x233e3199:
      sldrThis.allowedDoubleJumpTime = input.ReadFloat();
      break;
    case 0x97f30b95:
      sldrThis.minDoubleJumpWindow = input.ReadFloat();
      break;
    case 0x4c4c5872:
      sldrThis.maxDoubleJumpWindow = input.ReadFloat();
      break;
    case 0x9bb73a0b:
      sldrThis.unknown = input.ReadFloat();
      break;
    case 0x4c8d664c:
      sldrThis.minJumpTime = input.ReadFloat();
      break;
    case 0x1fc20169:
      sldrThis.minDoubleJumpTime = input.ReadFloat();
      break;
    case 0xe7a5d759:
      sldrThis.ledgeFallTime = input.ReadFloat();
      break;
    case 0x7044b295:
      sldrThis.doubleJumpImpulse = input.ReadFloat();
      break;
    case 0xd82380a6:
      sldrThis.backwardsForceMultiplier = input.ReadFloat();
      break;
    case 0x2a2e4100:
      sldrThis.bombJumpHeight = input.ReadFloat();
      break;
    case 0x905545e6:
      sldrThis.bombJumpRadius = input.ReadFloat();
      break;
    case 0x229460be:
      sldrThis.gravityBoostTime = input.ReadFloat();
      break;
    case 0x0e238fd3:
      sldrThis.gravityBoostForce = input.ReadFloat();
      break;
    case 0xdc92a0ac:
      sldrThis.gravityBoostCancelDampening = input.ReadFloat();
      break;
    case 0xe1fefd3c:
      sldrThis.gravityBoostMultipleAllowed = input.ReadBool();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
