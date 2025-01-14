#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Movement.hpp"

SLdrTweakBall_Movement::SLdrTweakBall_Movement() {
  forwardAccelNormal = 12000.0f;
  forwardAccelAir = 3000.0f;
  forwardAccelIce = 12000.0f;
  forwardAccelOrganic = 12000.0f;
  forwardAccelWater = 8000.0f;
  forwardAccelLava = 8000.0f;
  forwardAccelPhazon = 8000.0f;
  forwardAccelShrubbery = 8000.0f;
  movementFrictionNormal = 0.15000001f;
  movementFrictionAir = 0.15000001f;
  movementFrictionIce = 0.15000001f;
  movementFrictionOrganic = 0.15000001f;
  movementFrictionWater = 0.25f;
  movementFrictionLava = 0.15000001f;
  movementFrictionPhazon = 0.15000001f;
  movementFrictionShrubbery = 0.15000001f;
  forwardMaxSpeedNormal = 25.0f;
  forwardMaxSpeedAir = 10.0f;
  forwardMaxSpeedIce = 25.0f;
  forwardMaxSpeedOrganic = 25.0f;
  forwardMaxSpeedWater = 15.0f;
  forwardMaxSpeedLava = 15.0f;
  forwardMaxSpeedPhazon = 5.0f;
  forwardMaxSpeedShrubbery = 5.0f;
  unknown_0x85ee51ed = 100.0f;
  unknown_0x6d7811f5 = 0.0f;
  ballUpGravity = 20.0f;
  ballDownGravity = 35.0f;
  ballForwardBrakingAccelNormal = 12000.0f;
  ballForwardBrakingAccelAir = 3000.0f;
  ballForwardBrakingAccelIce = 12000.0f;
  ballForwardBrakingAccelOrganic = 12000.0f;
  ballForwardBrakingAccelWater = 12000.0f;
  ballForwardBrakingAccelLava = 12000.0f;
  ballForwardBrakingAccelPhazon = 12000.0f;
  ballForwardBrakingAccelShrubbery = 12000.0f;
  ballGravity = 76.0f;
  ballWaterGravity = 35.0f;
  unknown_0xeeb74968 = 3.2f;
  ballBoostHeight = 0.0f;
  unknown_0xe2bb0298 = 90.0f;
  minimumAlignmentSpeed = 5.0f;
  tireness = 120.0f;
  maxLeanAngle = 30.0f;
  tireToMarbleThresholdSpeed = 5.0f;
  marbleToTireThresholdSpeed = 10.0f;
  forceToLeanGain = 4.0f;
  leanTrackingGain = 4.0f;
}

SLdrTweakBall_Movement::~SLdrTweakBall_Movement() {}

void LoadTypedefSLdrTweakBall_Movement(SLdrTweakBall_Movement& sldrThis, CInputStream& input) {
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
    case 0x85ee51ed:
      sldrThis.unknown_0x85ee51ed = input.ReadFloat();
      break;
    case 0x6d7811f5:
      sldrThis.unknown_0x6d7811f5 = input.ReadFloat();
      break;
    case 0xf80a0655:
      sldrThis.ballUpGravity = input.ReadFloat();
      break;
    case 0x92f30f94:
      sldrThis.ballDownGravity = input.ReadFloat();
      break;
    case 0xb06ac970:
      sldrThis.ballForwardBrakingAccelNormal = input.ReadFloat();
      break;
    case 0xbb3c2629:
      sldrThis.ballForwardBrakingAccelAir = input.ReadFloat();
      break;
    case 0xd27a50f1:
      sldrThis.ballForwardBrakingAccelIce = input.ReadFloat();
      break;
    case 0x60520222:
      sldrThis.ballForwardBrakingAccelOrganic = input.ReadFloat();
      break;
    case 0x0dff9b92:
      sldrThis.ballForwardBrakingAccelWater = input.ReadFloat();
      break;
    case 0xbbaf8577:
      sldrThis.ballForwardBrakingAccelLava = input.ReadFloat();
      break;
    case 0x50f2a114:
      sldrThis.ballForwardBrakingAccelPhazon = input.ReadFloat();
      break;
    case 0x5b3c4299:
      sldrThis.ballForwardBrakingAccelShrubbery = input.ReadFloat();
      break;
    case 0xd83e5eec:
      sldrThis.ballGravity = input.ReadFloat();
      break;
    case 0x9c905747:
      sldrThis.ballWaterGravity = input.ReadFloat();
      break;
    case 0xeeb74968:
      sldrThis.unknown_0xeeb74968 = input.ReadFloat();
      break;
    case 0xbd28af6e:
      sldrThis.ballBoostHeight = input.ReadFloat();
      break;
    case 0xe2bb0298:
      sldrThis.unknown_0xe2bb0298 = input.ReadFloat();
      break;
    case 0xe46fa96a:
      sldrThis.minimumAlignmentSpeed = input.ReadFloat();
      break;
    case 0x4ed87ebd:
      sldrThis.tireness = input.ReadFloat();
      break;
    case 0xca8edeab:
      sldrThis.maxLeanAngle = input.ReadFloat();
      break;
    case 0x96226d18:
      sldrThis.tireToMarbleThresholdSpeed = input.ReadFloat();
      break;
    case 0x191f4f62:
      sldrThis.marbleToTireThresholdSpeed = input.ReadFloat();
      break;
    case 0x54c20211:
      sldrThis.forceToLeanGain = input.ReadFloat();
      break;
    case 0xb90a721e:
      sldrThis.leanTrackingGain = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
