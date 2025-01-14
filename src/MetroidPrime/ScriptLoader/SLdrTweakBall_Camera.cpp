#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Camera.hpp"

SLdrTweakBall_Camera::SLdrTweakBall_Camera() {
  ballCameraAnglePerSecond = 1200.0f;
  // ballCameraOffset = {'X': 0.0, 'Y': 0.0, 'Z': 0.0};
  ballCameraMinSpeedDistance = 6.0f;
  ballCameraMaxSpeedDistance = 5.0f;
  ballCameraBackwardsDistance = 10.0f;
  ballCameraElevation = 30.0f;
  ballCameraSpringConstant = 10.0f;
  ballCameraSpringMax = 0.0099999998f;
  ballCameraSpringTardis = 1.0f;
  ballCameraCentroidSpringConstant = 80.0f;
  ballCameraCentroidSpringMax = 4.0f;
  ballCameraCentroidSpringTardis = 1.0f;
  ballCameraCentroidDistanceSpringConstant = 30.0f;
  ballCameraCentroidDistanceSpringMax = 8.0f;
  ballCameraCentroidDistanceSpringTardis = 2.5f;
  ballCameraLookAtSpringConstant = 20.0f;
  ballCameraLookAtSpringMax = 10.0f;
  ballCameraLookAtSpringTardis = 1.5f;
  ballCameraTransitionTime = 0.75f;
  ballCameraFreeLookSpeed = 80.0f;
  ballCameraFreeLookZoomSpeed = 10.0f;
  ballCameraFreeLookMinDistance = 5.0f;
  ballCameraFreeLookMaxDistance = 6.0f;
  ballCameraFreeLookMaxVertAngle = 89.0f;
  unknown_0x144db504 = 3.0f;
  unknown_0xee5bea64 = 25.0f;
  ballCameraChaseDistance = 4.0f;
  ballCameraChaseElevation = 10.0f;
  ballCameraChaseYawSpeed = 60.0f;
  ballCameraChaseDampenAngle = 90.0f;
  ballCameraChaseAnglePerSecond = 1200.0f;
  ballCameraChaseSpringConstant = 20.0f;
  ballCameraChaseSpringMax = 5.0f;
  ballCameraChaseSpringTardis = 5.5f;
  ballCameraBoostDistance = 4.0f;
  ballCameraBoostElevation = 10.0f;
  ballCameraBoostYawSpeed = 80.0f;
  ballCameraBoostDampenAngle = 90.0f;
  ballCameraBoostAnglePerSecond = 2400.0f;
  ballCameraBoostSpringConstant = 20.0f;
  ballCameraBoostSpringMax = 5.0f;
  ballCameraBoostSpringTardis = 5.5f;
  ballCameraControlDistance = 3.0f;
  ballCameraLookAtMinHeight = 4.0f;
  unknown_0x50f77df0 = 0.40000001f;
}

SLdrTweakBall_Camera::~SLdrTweakBall_Camera() {}

void LoadTypedefSLdrTweakBall_Camera(SLdrTweakBall_Camera& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x8ae409fc:
      sldrThis.ballCameraAnglePerSecond = input.ReadFloat();
      break;
    case 0xc7bf307c:
      // sldrThis.ballCameraOffset = TODO
      break;
    case 0xf88297b3:
      sldrThis.ballCameraMinSpeedDistance = input.ReadFloat();
      break;
    case 0xebc77118:
      sldrThis.ballCameraMaxSpeedDistance = input.ReadFloat();
      break;
    case 0x0f52074c:
      sldrThis.ballCameraBackwardsDistance = input.ReadFloat();
      break;
    case 0x31ff1a31:
      sldrThis.ballCameraElevation = input.ReadFloat();
      break;
    case 0xc54a2a64:
      sldrThis.ballCameraSpringConstant = input.ReadFloat();
      break;
    case 0x587695fa:
      sldrThis.ballCameraSpringMax = input.ReadFloat();
      break;
    case 0x6617065c:
      sldrThis.ballCameraSpringTardis = input.ReadFloat();
      break;
    case 0xda3010af:
      sldrThis.ballCameraCentroidSpringConstant = input.ReadFloat();
      break;
    case 0xd2e267a2:
      sldrThis.ballCameraCentroidSpringMax = input.ReadFloat();
      break;
    case 0xd550adbc:
      sldrThis.ballCameraCentroidSpringTardis = input.ReadFloat();
      break;
    case 0xfa4ae412:
      sldrThis.ballCameraCentroidDistanceSpringConstant = input.ReadFloat();
      break;
    case 0xd0efedbf:
      sldrThis.ballCameraCentroidDistanceSpringMax = input.ReadFloat();
      break;
    case 0x7e7391e1:
      sldrThis.ballCameraCentroidDistanceSpringTardis = input.ReadFloat();
      break;
    case 0x6ab01e0b:
      sldrThis.ballCameraLookAtSpringConstant = input.ReadFloat();
      break;
    case 0xf9fd14b7:
      sldrThis.ballCameraLookAtSpringMax = input.ReadFloat();
      break;
    case 0x46a7ebcc:
      sldrThis.ballCameraLookAtSpringTardis = input.ReadFloat();
      break;
    case 0x6efa8159:
      sldrThis.ballCameraTransitionTime = input.ReadFloat();
      break;
    case 0xae74c22f:
      sldrThis.ballCameraFreeLookSpeed = input.ReadFloat();
      break;
    case 0xc040d23b:
      sldrThis.ballCameraFreeLookZoomSpeed = input.ReadFloat();
      break;
    case 0x16a35615:
      sldrThis.ballCameraFreeLookMinDistance = input.ReadFloat();
      break;
    case 0x475aed48:
      sldrThis.ballCameraFreeLookMaxDistance = input.ReadFloat();
      break;
    case 0x14de6d39:
      sldrThis.ballCameraFreeLookMaxVertAngle = input.ReadFloat();
      break;
    case 0x144db504:
      sldrThis.unknown_0x144db504 = input.ReadFloat();
      break;
    case 0xee5bea64:
      sldrThis.unknown_0xee5bea64 = input.ReadFloat();
      break;
    case 0xf20ba524:
      sldrThis.ballCameraChaseDistance = input.ReadFloat();
      break;
    case 0xd5d38cbc:
      sldrThis.ballCameraChaseElevation = input.ReadFloat();
      break;
    case 0x5429d2ae:
      sldrThis.ballCameraChaseYawSpeed = input.ReadFloat();
      break;
    case 0x31bd251e:
      sldrThis.ballCameraChaseDampenAngle = input.ReadFloat();
      break;
    case 0x50fc4acb:
      sldrThis.ballCameraChaseAnglePerSecond = input.ReadFloat();
      break;
    case 0xa8a86de9:
      // sldrThis.ballCameraChaseLookAtOffset = TODO
      break;
    case 0x1f526953:
      sldrThis.ballCameraChaseSpringConstant = input.ReadFloat();
      break;
    case 0xbc5a0377:
      sldrThis.ballCameraChaseSpringMax = input.ReadFloat();
      break;
    case 0x9a39cf14:
      sldrThis.ballCameraChaseSpringTardis = input.ReadFloat();
      break;
    case 0xe6f4443f:
      sldrThis.ballCameraBoostDistance = input.ReadFloat();
      break;
    case 0x5fa2bab1:
      sldrThis.ballCameraBoostElevation = input.ReadFloat();
      break;
    case 0x40d633b5:
      sldrThis.ballCameraBoostYawSpeed = input.ReadFloat();
      break;
    case 0x4ba944bb:
      sldrThis.ballCameraBoostDampenAngle = input.ReadFloat();
      break;
    case 0xe9735ba1:
      sldrThis.ballCameraBoostAnglePerSecond = input.ReadFloat();
      break;
    case 0x416d0e9f:
      // sldrThis.ballCameraBoostLookAtOffset = TODO
      break;
    case 0xa6dd7839:
      sldrThis.ballCameraBoostSpringConstant = input.ReadFloat();
      break;
    case 0x362b357a:
      sldrThis.ballCameraBoostSpringMax = input.ReadFloat();
      break;
    case 0x3cff8c12:
      sldrThis.ballCameraBoostSpringTardis = input.ReadFloat();
      break;
    case 0xee4c1f4b:
      sldrThis.ballCameraControlDistance = input.ReadFloat();
      break;
    case 0x9388fa50:
      sldrThis.ballCameraLookAtMinHeight = input.ReadFloat();
      break;
    case 0x50f77df0:
      sldrThis.unknown_0x50f77df0 = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
