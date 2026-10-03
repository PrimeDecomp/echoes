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
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xaf03e16c);
  case CControlMapper::kC_Backward:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xcfa71717);
  case CControlMapper::kC_TurnLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x91532a8c);
  case CControlMapper::kC_TurnRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x07acc58d);
  case CControlMapper::kC_StrafeLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xacc575a2);
  case CControlMapper::kC_StrafeRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xdb475e1d);
  case CControlMapper::kC_LookLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xa900887a);
  case CControlMapper::kC_LookRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x534ac106);
  case CControlMapper::kC_LookUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x0d723723);
  case CControlMapper::kC_LookDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5c46b025);
  case CControlMapper::kC_JumpOrBoost:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xf836180a);
  case CControlMapper::kC_JumpOrBoost2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xfe16f98d);
  case CControlMapper::kC_FireOrBomb:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xfd59aa9f);
  case CControlMapper::kC_FireOrBomb2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x7e76f1f4);
  case CControlMapper::kC_Unknown15:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x93dd818b);
  case CControlMapper::kC_ChargeBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x258402ec);
  case CControlMapper::kC_ChargeBeam2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xb7a20cda);
  case CControlMapper::kC_MissileOrPowerBomb:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5b9a9219);
  case CControlMapper::kC_AimUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x82a717cd);
  case CControlMapper::kC_AimDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xa7d5c15a);
  case CControlMapper::kC_CycleBeamUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x33731936);
  case CControlMapper::kC_CycleBeamDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xb72565ff);
  case CControlMapper::kC_CycleItem:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xc592ca02);
  case CControlMapper::kC_PowerBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5228272c);
  case CControlMapper::kC_IceBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x901ac820);
  case CControlMapper::kC_WaveBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x4ecea0c0);
  case CControlMapper::kC_PlasmaBeam:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xa4f35804);
  case CControlMapper::kC_ToggleHolster:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x919d7de0);
  case CControlMapper::kC_OrbitClose:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5200b48b);
  case CControlMapper::kC_OrbitFar:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x49c493a3);
  case CControlMapper::kC_OrbitObject:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xeb38a36b);
  case CControlMapper::kC_OrbitSelect:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xc60f66d2);
  case CControlMapper::kC_OrbitConfirm:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x1d97cc2b);
  case CControlMapper::kC_OrbitLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xc449ae1d);
  case CControlMapper::kC_OrbitRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x80f17cdb);
  case CControlMapper::kC_OrbitUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xabc5a6aa);
  case CControlMapper::kC_OrbitDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x310f9642);
  case CControlMapper::kC_LookHold1:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xc4923775);
  case CControlMapper::kC_LookHold2:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xf57a2de8);
  case CControlMapper::kC_LookZoomIn:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xba4fb516);
  case CControlMapper::kC_LookZoomOut:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x9f45c8db);
  case CControlMapper::kC_AimHold:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5344d2f7);
  case CControlMapper::kC_MapCircleUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x018c157d);
  case CControlMapper::kC_MapCircleDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xad1e8de5);
  case CControlMapper::kC_MapCircleLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5858b5ba);
  case CControlMapper::kC_MapCircleRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xc8df5b8b);
  case CControlMapper::kC_MapMoveForward:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x8d86d7b5);
  case CControlMapper::kC_MapMoveBack:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xab429ebd);
  case CControlMapper::kC_MapMoveLeft:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x31111d41);
  case CControlMapper::kC_MapMoveRight:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xe2d939b7);
  case CControlMapper::kC_MapZoomIn:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xb06d1b60);
  case CControlMapper::kC_MapZoomOut:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x26293e7c);
  case CControlMapper::kC_SpiderBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x649b0835);
  case CControlMapper::kC_ChaseCamera:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5b1e0e7c);
  case CControlMapper::kC_XrayVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xb35d2cca);
  case CControlMapper::kC_ThermoVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5a7e4dfc);
  case CControlMapper::kC_EnviroVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x76faf77e);
  case CControlMapper::kC_NoVisor:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x9ba498f6);
  case CControlMapper::kC_VisorMenu:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x2b9a4a7f);
  case CControlMapper::kC_VisorUp:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xd6fb0bf9);
  case CControlMapper::kC_VisorDown:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x08fe3abe);
  case CControlMapper::kC_DarkVisorToggle:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xc3f4f3ef);
  case CControlMapper::kC_Unknown63:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x53e56da8);
  case CControlMapper::kC_Unknown64:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x29293fb1);
  case CControlMapper::kC_UseShield:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x02c06b91);
  case CControlMapper::kC_ScanItem:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xbaa185cf);
  case CControlMapper::kC_Unknown67:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x6cdd19a4);
  case CControlMapper::kC_ExitMap:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xe08f6c6f);
  case CControlMapper::kC_Unknown69:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x1230759b);
  case CControlMapper::kC_Unknown70:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x5b9b4285);
  case CControlMapper::kC_Unknown71:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xbf218f4f);
  case CControlMapper::kC_Unknown72:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x05ef2422);
  case CControlMapper::kC_Unknown73:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0xced85a1b);
  case CControlMapper::kC_MorphIntoBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x39cf6e72);
  case CControlMapper::kC_MorphFromBall:
    return static_cast< CControlMapper::EFunctionList >(mData->controls.unknown_0x64003596);
  case CControlMapper::kC_None:
  default:
    return CControlMapper::kFL_None;
  }
}
