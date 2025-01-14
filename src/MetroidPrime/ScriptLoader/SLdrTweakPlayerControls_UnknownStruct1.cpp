#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct1.hpp"

SLdrTweakPlayerControls_UnknownStruct1::SLdrTweakPlayerControls_UnknownStruct1() {
  forward = 1;
  backward = 2;
  turnLeft = 3;
  turnRight = 4;
  strafeLeft = 3;
  strafeRight = 4;
  lookLeft = 3;
  lookRight = 4;
  lookUp = 2;
  lookDown = 1;
  jump = 16;
  jump2 = 16;
  fireBeam = 15;
  fireBeam2 = 15;
  autoFireBeam = 0;
  chargeBeam = 15;
  chargeBeam2 = 15;
  useItem = 18;
  aimUp = 0;
  aimDown = 0;
  cycleBeamUp = 0;
  cycleBeamDown = 0;
  cycleItem = 0;
  selectPowerBeam = 11;
  selectIceBeam = 12;
  selectWaveBeam = 14;
  selectPlasmaBeam = 13;
  gunToggleHolster = 0;
  orbitClose = 0;
  orbitFar = 9;
  orbitObject = 20;
  orbitSelect = 0;
  orbitConfirm = 0;
  orbitLeft = 3;
  orbitRight = 4;
  orbitUp = 1;
  orbitDown = 2;
  holdLook1 = 10;
  holdLook2 = 0;
  lookZoomIn = 17;
  lookZoomOut = 18;
  holdAim = 0;
  mapCircleUp = 2;
  mapCircleDown = 1;
  mapCircleLeft = 3;
  mapCircleRight = 4;
  mapMoveForward = 5;
  mapMoveBack = 6;
  mapMoveLeft = 7;
  mapMoveRight = 8;
  mapZoomIn = 10;
  mapZoomOut = 9;
  spiderBall = 10;
  chaseCamera = 9;
  xRayVisor = 0;
  thermoVisor = 0;
  enviroVisor = 0;
  noVisor = 0;
  visorMenu = 0;
  cycleVisorUp = 19;
  cycleVisorDown = 0;
  darkVisorToggle = 0;
  crosshairs = 21;
  unknown_0x29293fb1 = 0;
  useShield = 0;
  scanItem = 9;
  inventoryScreen = 13;
  mapScreen = 22;
  optionsScreen = 12;
  logScreen = 14;
  unknown_0xbf218f4f = 9;
  unknown_0x05ef2422 = 10;
  boostBall = 16;
  morphIntoBall = 17;
  morphFromBall = 17;
}

SLdrTweakPlayerControls_UnknownStruct1::~SLdrTweakPlayerControls_UnknownStruct1() {}

void LoadTypedefSLdrTweakPlayerControls_UnknownStruct1(SLdrTweakPlayerControls_UnknownStruct1& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xaf03e16c:
      sldrThis.forward = input.ReadInt32();
      break;
    case 0xcfa71717:
      sldrThis.backward = input.ReadInt32();
      break;
    case 0x91532a8c:
      sldrThis.turnLeft = input.ReadInt32();
      break;
    case 0x07acc58d:
      sldrThis.turnRight = input.ReadInt32();
      break;
    case 0xacc575a2:
      sldrThis.strafeLeft = input.ReadInt32();
      break;
    case 0xdb475e1d:
      sldrThis.strafeRight = input.ReadInt32();
      break;
    case 0xa900887a:
      sldrThis.lookLeft = input.ReadInt32();
      break;
    case 0x534ac106:
      sldrThis.lookRight = input.ReadInt32();
      break;
    case 0x0d723723:
      sldrThis.lookUp = input.ReadInt32();
      break;
    case 0x5c46b025:
      sldrThis.lookDown = input.ReadInt32();
      break;
    case 0xf836180a:
      sldrThis.jump = input.ReadInt32();
      break;
    case 0xfe16f98d:
      sldrThis.jump2 = input.ReadInt32();
      break;
    case 0xfd59aa9f:
      sldrThis.fireBeam = input.ReadInt32();
      break;
    case 0x7e76f1f4:
      sldrThis.fireBeam2 = input.ReadInt32();
      break;
    case 0x93dd818b:
      sldrThis.autoFireBeam = input.ReadInt32();
      break;
    case 0x258402ec:
      sldrThis.chargeBeam = input.ReadInt32();
      break;
    case 0xb7a20cda:
      sldrThis.chargeBeam2 = input.ReadInt32();
      break;
    case 0x5b9a9219:
      sldrThis.useItem = input.ReadInt32();
      break;
    case 0x82a717cd:
      sldrThis.aimUp = input.ReadInt32();
      break;
    case 0xa7d5c15a:
      sldrThis.aimDown = input.ReadInt32();
      break;
    case 0x33731936:
      sldrThis.cycleBeamUp = input.ReadInt32();
      break;
    case 0xb72565ff:
      sldrThis.cycleBeamDown = input.ReadInt32();
      break;
    case 0xc592ca02:
      sldrThis.cycleItem = input.ReadInt32();
      break;
    case 0x5228272c:
      sldrThis.selectPowerBeam = input.ReadInt32();
      break;
    case 0x901ac820:
      sldrThis.selectIceBeam = input.ReadInt32();
      break;
    case 0x4ecea0c0:
      sldrThis.selectWaveBeam = input.ReadInt32();
      break;
    case 0xa4f35804:
      sldrThis.selectPlasmaBeam = input.ReadInt32();
      break;
    case 0x919d7de0:
      sldrThis.gunToggleHolster = input.ReadInt32();
      break;
    case 0x5200b48b:
      sldrThis.orbitClose = input.ReadInt32();
      break;
    case 0x49c493a3:
      sldrThis.orbitFar = input.ReadInt32();
      break;
    case 0xeb38a36b:
      sldrThis.orbitObject = input.ReadInt32();
      break;
    case 0xc60f66d2:
      sldrThis.orbitSelect = input.ReadInt32();
      break;
    case 0x1d97cc2b:
      sldrThis.orbitConfirm = input.ReadInt32();
      break;
    case 0xc449ae1d:
      sldrThis.orbitLeft = input.ReadInt32();
      break;
    case 0x80f17cdb:
      sldrThis.orbitRight = input.ReadInt32();
      break;
    case 0xabc5a6aa:
      sldrThis.orbitUp = input.ReadInt32();
      break;
    case 0x310f9642:
      sldrThis.orbitDown = input.ReadInt32();
      break;
    case 0xc4923775:
      sldrThis.holdLook1 = input.ReadInt32();
      break;
    case 0xf57a2de8:
      sldrThis.holdLook2 = input.ReadInt32();
      break;
    case 0xba4fb516:
      sldrThis.lookZoomIn = input.ReadInt32();
      break;
    case 0x9f45c8db:
      sldrThis.lookZoomOut = input.ReadInt32();
      break;
    case 0x5344d2f7:
      sldrThis.holdAim = input.ReadInt32();
      break;
    case 0x018c157d:
      sldrThis.mapCircleUp = input.ReadInt32();
      break;
    case 0xad1e8de5:
      sldrThis.mapCircleDown = input.ReadInt32();
      break;
    case 0x5858b5ba:
      sldrThis.mapCircleLeft = input.ReadInt32();
      break;
    case 0xc8df5b8b:
      sldrThis.mapCircleRight = input.ReadInt32();
      break;
    case 0x8d86d7b5:
      sldrThis.mapMoveForward = input.ReadInt32();
      break;
    case 0xab429ebd:
      sldrThis.mapMoveBack = input.ReadInt32();
      break;
    case 0x31111d41:
      sldrThis.mapMoveLeft = input.ReadInt32();
      break;
    case 0xe2d939b7:
      sldrThis.mapMoveRight = input.ReadInt32();
      break;
    case 0xb06d1b60:
      sldrThis.mapZoomIn = input.ReadInt32();
      break;
    case 0x26293e7c:
      sldrThis.mapZoomOut = input.ReadInt32();
      break;
    case 0x649b0835:
      sldrThis.spiderBall = input.ReadInt32();
      break;
    case 0x5b1e0e7c:
      sldrThis.chaseCamera = input.ReadInt32();
      break;
    case 0xb35d2cca:
      sldrThis.xRayVisor = input.ReadInt32();
      break;
    case 0x5a7e4dfc:
      sldrThis.thermoVisor = input.ReadInt32();
      break;
    case 0x76faf77e:
      sldrThis.enviroVisor = input.ReadInt32();
      break;
    case 0x9ba498f6:
      sldrThis.noVisor = input.ReadInt32();
      break;
    case 0x2b9a4a7f:
      sldrThis.visorMenu = input.ReadInt32();
      break;
    case 0xd6fb0bf9:
      sldrThis.cycleVisorUp = input.ReadInt32();
      break;
    case 0x08fe3abe:
      sldrThis.cycleVisorDown = input.ReadInt32();
      break;
    case 0xc3f4f3ef:
      sldrThis.darkVisorToggle = input.ReadInt32();
      break;
    case 0x53e56da8:
      sldrThis.crosshairs = input.ReadInt32();
      break;
    case 0x29293fb1:
      sldrThis.unknown_0x29293fb1 = input.ReadInt32();
      break;
    case 0x02c06b91:
      sldrThis.useShield = input.ReadInt32();
      break;
    case 0xbaa185cf:
      sldrThis.scanItem = input.ReadInt32();
      break;
    case 0x6cdd19a4:
      sldrThis.inventoryScreen = input.ReadInt32();
      break;
    case 0xe08f6c6f:
      sldrThis.mapScreen = input.ReadInt32();
      break;
    case 0x1230759b:
      sldrThis.optionsScreen = input.ReadInt32();
      break;
    case 0x5b9b4285:
      sldrThis.logScreen = input.ReadInt32();
      break;
    case 0xbf218f4f:
      sldrThis.unknown_0xbf218f4f = input.ReadInt32();
      break;
    case 0x05ef2422:
      sldrThis.unknown_0x05ef2422 = input.ReadInt32();
      break;
    case 0xced85a1b:
      sldrThis.boostBall = input.ReadInt32();
      break;
    case 0x39cf6e72:
      sldrThis.morphIntoBall = input.ReadInt32();
      break;
    case 0x64003596:
      sldrThis.morphFromBall = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
