#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Orbit.hpp"

SLdrTweakPlayer_Orbit::SLdrTweakPlayer_Orbit() {
  orbitCloseMinDistance = 27.0f;
  orbitCloseNormalDistance = 75.0f;
  orbitCloseMaxDistance = 100.0f;
  orbitFarMinDistance = 27.0f;
  orbitFarNormalDistance = 75.0f;
  orbitFarMaxDistance = 100.0f;
  orbitCarcassMinDistance = 2.5f;
  orbitCarcassNormalDistance = 75.0f;
  orbitCarcassMaxDistance = 100.0f;
  orbitMaxAngularChange = 360.0f;
  orbitModeTimer = 0.2f;
  orbitCameraSpeed = 360.0f;
  orbitUpperAngle = 70.0f;
  orbitLowerAngle = 70.0f;
  orbitHorizAngle = 45.0f;
  orbitUpperCameraAngle = 25.0f;
  orbitLowerCameraAngle = 25.0f;
  orbitMaxTargetDistance = 100.0f;
  orbitMaxLockDistance = 100.0f;
  unknown_0x55f7d145 = 0.0f;
  orbitDistanceThreshold = 2.0f;
  orbitZoneWidth = 180;
  orbitZoneHeight = 180;
  unknown_0x58ee9d03 = 320;
  unknown_0xe052fa66 = 224;
  unknown_0xc452b61e = 320;
  unknown_0x7ceed17b = 224;
  orbitScanZoneWidth = 126;
  orbitScanZoneHeight = 44;
  unknown_0xec529a5e = 320;
  unknown_0x54eefd3b = 224;
  unknown_0x73ebdce2 = 320;
  unknown_0xcb57bb87 = 224;
  orbitBoxWidth = 20.0f;
  orbitBoxHeight = 10.0f;
  orbitMinCameraPitchDistance = 3.0f;
  orbitMaxCameraPitchDistance = 6.0f;
  unknown_0x478c15f9 = 0.2f;
  orbitZRange = 0.2f;
  orbitSelectionCloseAngle = 5.0f;
  orbitSelectionMaxAngle = 90.0f;
  unknown_0x90b71b2e = 2.0f;
  orbitPreventionTime = 2.0f;
  orbitDash = true;
  orbitDashUsesTap = true;
  orbitDashTapTime = 0.30000001f;
  orbitDashStickThreshold = 0.40000001f;
  orbitDashDoubleJumpImpulse = 8.0f;
  unknown_0x75a00cfb = 5.0f;
  unknown_0xc4775e5f = 50.0f;
}

SLdrTweakPlayer_Orbit::~SLdrTweakPlayer_Orbit() {}

void LoadTypedefSLdrTweakPlayer_Orbit(SLdrTweakPlayer_Orbit& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x68738246:
      sldrThis.orbitCloseMinDistance = input.ReadFloat();
      break;
    case 0x991180ef:
      sldrThis.orbitCloseNormalDistance = input.ReadFloat();
      break;
    case 0x398a391b:
      sldrThis.orbitCloseMaxDistance = input.ReadFloat();
      break;
    case 0x495caff1:
      sldrThis.orbitFarMinDistance = input.ReadFloat();
      break;
    case 0xd2fb0fed:
      sldrThis.orbitFarNormalDistance = input.ReadFloat();
      break;
    case 0x18a514ac:
      sldrThis.orbitFarMaxDistance = input.ReadFloat();
      break;
    case 0x31072813:
      sldrThis.orbitCarcassMinDistance = input.ReadFloat();
      break;
    case 0xf0e68dab:
      sldrThis.orbitCarcassNormalDistance = input.ReadFloat();
      break;
    case 0x60fe934e:
      sldrThis.orbitCarcassMaxDistance = input.ReadFloat();
      break;
    case 0xa6332f81:
      sldrThis.orbitMaxAngularChange = input.ReadFloat();
      break;
    case 0x663a6f89:
      sldrThis.orbitModeTimer = input.ReadFloat();
      break;
    case 0xe60bbbbb:
      sldrThis.orbitCameraSpeed = input.ReadFloat();
      break;
    case 0x914977b4:
      sldrThis.orbitUpperAngle = input.ReadFloat();
      break;
    case 0xee0c7156:
      sldrThis.orbitLowerAngle = input.ReadFloat();
      break;
    case 0xc6960be2:
      sldrThis.orbitHorizAngle = input.ReadFloat();
      break;
    case 0x6f6dffdd:
      sldrThis.orbitUpperCameraAngle = input.ReadFloat();
      break;
    case 0x48c63796:
      sldrThis.orbitLowerCameraAngle = input.ReadFloat();
      break;
    case 0x598beb71:
      sldrThis.orbitMaxTargetDistance = input.ReadFloat();
      break;
    case 0x30b2f98e:
      sldrThis.orbitMaxLockDistance = input.ReadFloat();
      break;
    case 0x55f7d145:
      sldrThis.unknown_0x55f7d145 = input.ReadFloat();
      break;
    case 0xf034335c:
      sldrThis.orbitDistanceThreshold = input.ReadFloat();
      break;
    case 0x40ae584e:
      sldrThis.orbitZoneWidth = input.ReadInt32();
      break;
    case 0x111e9dec:
      sldrThis.orbitZoneHeight = input.ReadInt32();
      break;
    case 0x58ee9d03:
      sldrThis.unknown_0x58ee9d03 = input.ReadInt32();
      break;
    case 0xe052fa66:
      sldrThis.unknown_0xe052fa66 = input.ReadInt32();
      break;
    case 0xc452b61e:
      sldrThis.unknown_0xc452b61e = input.ReadInt32();
      break;
    case 0x7ceed17b:
      sldrThis.unknown_0x7ceed17b = input.ReadInt32();
      break;
    case 0x44989f30:
      sldrThis.orbitScanZoneWidth = input.ReadInt32();
      break;
    case 0xa6a7f710:
      sldrThis.orbitScanZoneHeight = input.ReadInt32();
      break;
    case 0xec529a5e:
      sldrThis.unknown_0xec529a5e = input.ReadInt32();
      break;
    case 0x54eefd3b:
      sldrThis.unknown_0x54eefd3b = input.ReadInt32();
      break;
    case 0x73ebdce2:
      sldrThis.unknown_0x73ebdce2 = input.ReadInt32();
      break;
    case 0xcb57bb87:
      sldrThis.unknown_0xcb57bb87 = input.ReadInt32();
      break;
    case 0xd2a8cc1f:
      sldrThis.orbitBoxWidth = input.ReadFloat();
      break;
    case 0xd5cbcec1:
      sldrThis.orbitBoxHeight = input.ReadFloat();
      break;
    case 0x141ed3b9:
      sldrThis.orbitMinCameraPitchDistance = input.ReadFloat();
      break;
    case 0x72fbb5cd:
      sldrThis.orbitMaxCameraPitchDistance = input.ReadFloat();
      break;
    case 0x478c15f9:
      sldrThis.unknown_0x478c15f9 = input.ReadFloat();
      break;
    case 0x93b712ba:
      sldrThis.orbitZRange = input.ReadFloat();
      break;
    case 0x7b4688ce:
      sldrThis.orbitSelectionCloseAngle = input.ReadFloat();
      break;
    case 0x2e0cf3cd:
      sldrThis.orbitSelectionMaxAngle = input.ReadFloat();
      break;
    case 0x90b71b2e:
      sldrThis.unknown_0x90b71b2e = input.ReadFloat();
      break;
    case 0x775761c5:
      sldrThis.orbitPreventionTime = input.ReadFloat();
      break;
    case 0xfa255139:
      sldrThis.orbitDash = input.ReadBool();
      break;
    case 0x8f80e39e:
      sldrThis.orbitDashUsesTap = input.ReadBool();
      break;
    case 0xd290d7b5:
      sldrThis.orbitDashTapTime = input.ReadFloat();
      break;
    case 0x2421b618:
      sldrThis.orbitDashStickThreshold = input.ReadFloat();
      break;
    case 0xb814530b:
      sldrThis.orbitDashDoubleJumpImpulse = input.ReadFloat();
      break;
    case 0x75a00cfb:
      sldrThis.unknown_0x75a00cfb = input.ReadFloat();
      break;
    case 0xc4775e5f:
      sldrThis.unknown_0xc4775e5f = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
