#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls.hpp"

bool CTweakPlayerControls::GetFreeLookTurnsPlayer() const {
  return mData->booleans.unknown_0xff1b0413;
}

bool CTweakPlayerControls::GetMoveDuringFreeLook() const {
  return mData->booleans.unknown_0x1f99c6ba;
}

bool CTweakPlayerControls::GetHoldButtonsForFreeLook() const {
  return mData->booleans.unknown_0x18eb3ab5;
}

bool CTweakPlayerControls::GetTwoButtonsForFreeLook() const {
  return mData->booleans.unknown_0xbdc01c71;
}

bool CTweakPlayerControls::GetAimWhenOrbitingPoint() const {
  return mData->booleans.unknown_0xc224d966;
}

bool CTweakPlayerControls::GetStayInFreeLookWhileFiring() const {
  return mData->booleans.addGrenadeAlert;
}

bool CTweakPlayerControls::GetOrbitFixedOffset() const {
  return mData->booleans.unknown_0x07bb06a6;
}

bool CTweakPlayerControls::GetGunButtonTogglesHolster() const {
  return mData->booleans.unknown_0x04d8d57b;
}

bool CTweakPlayerControls::GetGunNotFiringHolstersGun() const {
  return mData->booleans.unknown_0x5282c47e;
}

bool CTweakPlayerControls::GetFallingDoubleJump() const {
  return mData->booleans.fallingDoubleJump;
}

bool CTweakPlayerControls::GetImpulseDoubleJump() const {
  return mData->booleans.impulseDoubleJump;
}

bool CTweakPlayerControls::GetFiringCancelsCameraPitch() const {
  return mData->booleans.unknown_0xa796a8b9;
}

bool CTweakPlayerControls::GetAssistedAimingIgnoreHorizontal() const {
  return mData->booleans.unknown_0x7c0599c8;
}

bool CTweakPlayerControls::GetAssistedAimingIgnoreVertical() const {
  return mData->booleans.unknown_0x522ab1ac;
}

CControlMapper::EFunctionList
CTweakPlayerControls::GetMapping(CControlMapper::ECommands command) const {
  switch (command) {
  case CControlMapper::kC_Forward:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.forward);
  case CControlMapper::kC_Backward:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.backward);
  case CControlMapper::kC_TurnLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.turnLeft);
  case CControlMapper::kC_TurnRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.turnRight);
  case CControlMapper::kC_StrafeLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.strafeLeft);
  case CControlMapper::kC_StrafeRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.strafeRight);
  case CControlMapper::kC_LookLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.lookLeft);
  case CControlMapper::kC_LookRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.lookRight);
  case CControlMapper::kC_LookUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.lookUp);
  case CControlMapper::kC_LookDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.lookDown);
  case CControlMapper::kC_JumpOrBoost:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.jump);
  case CControlMapper::kC_JumpOrBoost2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.jump2);
  case CControlMapper::kC_FireOrBomb:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.fireBeam);
  case CControlMapper::kC_FireOrBomb2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.fireBeam2);
  case CControlMapper::kC_AutoFireBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.autoFireBeam);
  case CControlMapper::kC_ChargeBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.chargeBeam);
  case CControlMapper::kC_ChargeBeam2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.chargeBeam2);
  case CControlMapper::kC_MissileOrPowerBomb:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.useItem);
  case CControlMapper::kC_AimUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.aimUp);
  case CControlMapper::kC_AimDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.aimDown);
  case CControlMapper::kC_CycleBeamUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.cycleBeamUp);
  case CControlMapper::kC_CycleBeamDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.cycleBeamDown);
  case CControlMapper::kC_CycleItem:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.cycleItem);
  case CControlMapper::kC_PowerBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.selectPowerBeam);
  case CControlMapper::kC_IceBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.selectIceBeam);
  case CControlMapper::kC_WaveBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.selectWaveBeam);
  case CControlMapper::kC_PlasmaBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.selectPlasmaBeam);
  case CControlMapper::kC_ToggleHolster:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.gunToggleHolster);
  case CControlMapper::kC_OrbitClose:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitClose);
  case CControlMapper::kC_OrbitFar:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitFar);
  case CControlMapper::kC_OrbitObject:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitObject);
  case CControlMapper::kC_OrbitSelect:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitSelect);
  case CControlMapper::kC_OrbitConfirm:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitConfirm);
  case CControlMapper::kC_OrbitLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitLeft);
  case CControlMapper::kC_OrbitRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitRight);
  case CControlMapper::kC_OrbitUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitUp);
  case CControlMapper::kC_OrbitDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.orbitDown);
  case CControlMapper::kC_LookHold1:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.holdLook1);
  case CControlMapper::kC_LookHold2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.holdLook2);
  case CControlMapper::kC_LookZoomIn:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.lookZoomIn);
  case CControlMapper::kC_LookZoomOut:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.lookZoomOut);
  case CControlMapper::kC_AimHold:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.holdAim);
  case CControlMapper::kC_MapCircleUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapCircleUp);
  case CControlMapper::kC_MapCircleDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapCircleDown);
  case CControlMapper::kC_MapCircleLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapCircleLeft);
  case CControlMapper::kC_MapCircleRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapCircleRight);
  case CControlMapper::kC_MapMoveForward:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapMoveForward);
  case CControlMapper::kC_MapMoveBack:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapMoveBack);
  case CControlMapper::kC_MapMoveLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapMoveLeft);
  case CControlMapper::kC_MapMoveRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapMoveRight);
  case CControlMapper::kC_MapZoomIn:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapZoomIn);
  case CControlMapper::kC_MapZoomOut:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapZoomOut);
  case CControlMapper::kC_SpiderBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.spiderBall);
  case CControlMapper::kC_ChaseCamera:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.chaseCamera);
  case CControlMapper::kC_XrayVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.xRayVisor);
  case CControlMapper::kC_ThermoVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.thermoVisor);
  case CControlMapper::kC_EnviroVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.enviroVisor);
  case CControlMapper::kC_NoVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.noVisor);
  case CControlMapper::kC_VisorMenu:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.visorMenu);
  case CControlMapper::kC_VisorUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.cycleVisorUp);
  case CControlMapper::kC_VisorDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.cycleVisorDown);
  case CControlMapper::kC_DarkVisorToggle:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.darkVisorToggle);
  case CControlMapper::kC_Crosshairs:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.crosshairs);
  case CControlMapper::kC_Unknown64:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x29293fb1);
  case CControlMapper::kC_UseShield:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.useShield);
  case CControlMapper::kC_ScanItem:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.scanItem);
  case CControlMapper::kC_InventoryScreen:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.inventoryScreen);
  case CControlMapper::kC_MapScreen:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.mapScreen);
  case CControlMapper::kC_OptionsScreen:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.optionsScreen);
  case CControlMapper::kC_LogScreen:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.logScreen);
  case CControlMapper::kC_PauseScreenCycleLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.pauseScreenCycleLeft);
  case CControlMapper::kC_PauseScreenCycleRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.pauseScreenCycleRight);
  case CControlMapper::kC_BoostBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.boostBall);
  case CControlMapper::kC_MorphIntoBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.morphIntoBall);
  case CControlMapper::kC_MorphFromBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.morphFromBall);
  case CControlMapper::kC_None:
  default:
    return CControlMapper::kFL_None;
  }
}
