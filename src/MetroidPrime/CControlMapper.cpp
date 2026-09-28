#include "MetroidPrime/CControlMapper.hpp"

#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"

#include "Kyoto/Input/CFinalInput.hpp"

const FAnalogInput CControlMapper::gAnalogInputs[] = {
    nullptr,
    &CFinalInput::ALAUp,
    &CFinalInput::ALADown,
    &CFinalInput::ALALeft,
    &CFinalInput::ALARight,
    &CFinalInput::ARAUp,
    &CFinalInput::ARADown,
    &CFinalInput::ARALeft,
    &CFinalInput::ARARight,
    &CFinalInput::ALTrigger,
    &CFinalInput::ARTrigger,
    &CFinalInput::ADPUp,
    &CFinalInput::ADPDown,
    &CFinalInput::ADPLeft,
    &CFinalInput::ADPRight,
    &CFinalInput::AA,
    &CFinalInput::AB,
    &CFinalInput::AX,
    &CFinalInput::AY,
    &CFinalInput::AZ,
    &CFinalInput::AL,
    &CFinalInput::AR,
    &CFinalInput::AStart,
};

const FDigitalInput CControlMapper::gDigitalInputs[] = {
    nullptr,
    &CFinalInput::DLAUp,
    &CFinalInput::DLADown,
    &CFinalInput::DLALeft,
    &CFinalInput::DLARight,
    &CFinalInput::DRAUp,
    &CFinalInput::DRADown,
    &CFinalInput::DRALeft,
    &CFinalInput::DRARight,
    &CFinalInput::DLTrigger,
    &CFinalInput::DRTrigger,
    &CFinalInput::DDPUp,
    &CFinalInput::DDPDown,
    &CFinalInput::DDPLeft,
    &CFinalInput::DDPRight,
    &CFinalInput::DA,
    &CFinalInput::DB,
    &CFinalInput::DX,
    &CFinalInput::DY,
    &CFinalInput::DZ,
    &CFinalInput::DL,
    &CFinalInput::DR,
    &CFinalInput::DStart,
};

const FDigitalInput CControlMapper::gPressInputs[] = {
    nullptr,
    &CFinalInput::PLAUp,
    &CFinalInput::PLADown,
    &CFinalInput::PLALeft,
    &CFinalInput::PLARight,
    &CFinalInput::PRAUp,
    &CFinalInput::PRADown,
    &CFinalInput::PRALeft,
    &CFinalInput::PRARight,
    &CFinalInput::PLTrigger,
    &CFinalInput::PRTrigger,
    &CFinalInput::PDPUp,
    &CFinalInput::PDPDown,
    &CFinalInput::PDPLeft,
    &CFinalInput::PDPRight,
    &CFinalInput::PA,
    &CFinalInput::PB,
    &CFinalInput::PX,
    &CFinalInput::PY,
    &CFinalInput::PZ,
    &CFinalInput::PL,
    &CFinalInput::PR,
    &CFinalInput::PStart,
};

const char* CControlMapper::GetDescriptionForFunction(EFunctionList function) {
  switch (function) {
  case kFL_None:
    return "None";
  case kFL_LeftStickUp:
    return "Left Stick Up";
  case kFL_LeftStickDown:
    return "Left Stick Down";
  case kFL_LeftStickLeft:
    return "Left Stick Left";
  case kFL_LeftStickRight:
    return "Left Stick Right";
  case kFL_RightStickUp:
    return "Right Stick Up";
  case kFL_RightStickDown:
    return "Right Stick Down";
  case kFL_RightStickLeft:
    return "Right Stick Left";
  case kFL_RightStickRight:
    return "Right Stick Right";
  case kFL_LeftTrigger:
    return "Left Trigger";
  case kFL_RightTrigger:
    return "Right Trigger";
  case kFL_DPadUp:
    return "D-Pad Up   ";
  case kFL_DPadDown:
    return "D-Pad Down ";
  case kFL_DPadLeft:
    return "D-Pad Left ";
  case kFL_DPadRight:
    return "D-Pad Right";
  case kFL_AButton:
    return "A Button";
  case kFL_BButton:
    return "B Button";
  case kFL_XButton:
    return "X Button";
  case kFL_YButton:
    return "Y Button";
  case kFL_ZButton:
    return "Z Button";
  case kFL_LeftTriggerPress:
    return "Left Trigger Press";
  case kFL_RightTriggerPress:
    return "Right Trigger Press";
  case kFL_Start:
    return "Start";
  }

  return "UNKNOWN";
}

const char* CControlMapper::GetDescriptionForCommand(ECommands command) {
  switch (command) {
  case kC_None:
    return "None";
  case kC_Forward:
    return "Forward";
  case kC_Backward:
    return "Backward";
  case kC_TurnLeft:
    return "Turn Left";
  case kC_TurnRight:
    return "Turn Right";
  case kC_StrafeLeft:
    return "Strafe Left";
  case kC_StrafeRight:
    return "Strafe Right";
  case kC_LookLeft:
    return "Look Left";
  case kC_LookRight:
    return "Look Right";
  case kC_LookUp:
    return "Look Up";
  case kC_LookDown:
    return "Look Down";
  case kC_JumpOrBoost:
    return "Jump/Boost";
  case kC_JumpOrBoost2:
    return "Jump2/Boost2";
  case kC_FireOrBomb:
    return "Fire/Bomb";
  case kC_FireOrBomb2:
    return "Fire2/Bomb2";
  case kC_ChargeBeam:
    return "Charge Beam";
  case kC_ChargeBeam2:
    return "Charge Beam2";
  case kC_MissileOrPowerBomb:
    return "Missile/PowerBomb";
  case kC_AimUp:
    return "Aim Up";
  case kC_AimDown:
    return "Aim Down";
  case kC_CycleBeamUp:
    return "Cycle Beam Up";
  case kC_CycleBeamDown:
    return "Cycle Beam Down";
  case kC_CycleItem:
    return "Cycle Item";
  case kC_PowerBeam:
    return "Power Beam";
  case kC_IceBeam:
    return "Ice Beam";
  case kC_WaveBeam:
    return "Wave Beam";
  case kC_PlasmaBeam:
    return "Plasma Beam";
  case kC_ToggleHolster:
    return "Toggle Holster";
  case kC_OrbitClose:
    return "Orbit Close";
  case kC_OrbitFar:
    return "Orbit Far";
  case kC_OrbitObject:
    return "Orbit Object";
  case kC_OrbitSelect:
    return "Orbit Select";
  case kC_OrbitConfirm:
    return "Orbit Confirm";
  case kC_OrbitLeft:
    return "Orbit Left";
  case kC_OrbitRight:
    return "Orbit Right";
  case kC_OrbitUp:
    return "Orbit Up";
  case kC_OrbitDown:
    return "Orbit Down";
  case kC_LookHold1:
    return "Look Hold1";
  case kC_LookHold2:
    return "Look Hold2";
  case kC_LookZoomIn:
    return "Look Zoom In";
  case kC_LookZoomOut:
    return "Look Zoom Out";
  case kC_AimHold:
    return "Aim Hold";
  case kC_MapCircleUp:
    return "Map Circle Up";
  case kC_MapCircleDown:
    return "Map Circle Down";
  case kC_MapCircleLeft:
    return "Map Circle Left";
  case kC_MapCircleRight:
    return "Map Circle Right";
  case kC_MapMoveForward:
    return "Map Move Forward";
  case kC_MapMoveBack:
    return "Map Move Back";
  case kC_MapMoveLeft:
    return "Map Move Left";
  case kC_MapMoveRight:
    return "Map Move Right";
  case kC_MapZoomIn:
    return "Map Zoom In";
  case kC_MapZoomOut:
    return "Map Zoom Out";
  case kC_SpiderBall:
    return "SpiderBall";
  case kC_ChaseCamera:
    return "Chase Camera";
  case kC_XrayVisor:
    return "XRay Visor";
  case kC_ThermoVisor:
    return "Thermo Visor";
  case kC_EnviroVisor:
    return "Enviro Visor";
  case kC_NoVisor:
    return "No Visor";
  case kC_VisorMenu:
    return "Visor Menu";
  case kC_VisorUp:
    return "Visor Up";
  case kC_VisorDown:
    return "Visor Down";
  case kC_DarkVisorToggle:
    return "Dark Visor Toggle";
  case kC_UseShield:
    return "Use Shield";
  case kC_ScanItem:
    return "Scan Item";
  case kC_MorphIntoBall:
    return "MorphIntoBall";
  case kC_MorphFromBall:
    return "MorphFromBall";
  default:
    break;
  }

  return "UNKNOWN";
}

CControlMapper::CControlMapper(int controlScheme)
: mCommandEnabled(kC_Count, true)
, mCommandOverridden(kC_Count, false)
, mControlScheme(controlScheme) {
  Reset();
}

float CControlMapper::GetAnalogInput(ECommands command, const CFinalInput& input,
                                     EFilterType filter) const {
  float result = 0.f;
  EFunctionList mapping = GetMapping(command);
  if (gAnalogInputs[mapping] != nullptr) {
    if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
      result = (input.*gAnalogInputs[mapping])();
    }
  }
  return result;
}

const bool CControlMapper::GetDigitalInput(ECommands command, const CFinalInput& input,
                                           EFilterType filter) const {
  bool result = false;
  EFunctionList mapping = GetMapping(command);
  if (gDigitalInputs[mapping] != nullptr) {
    if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
      result = (input.*gDigitalInputs[mapping])();
    }
  }
  return result;
}

const bool CControlMapper::GetPressInput(ECommands command, const CFinalInput& input,
                                         EFilterType filter) const {
  bool result = false;
  EFunctionList mapping = GetMapping(command);
  if (gPressInputs[mapping] != nullptr) {
    if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
      result = (input.*gPressInputs[mapping])();
    }
  }
  return result;
}

void CControlMapper::ResetCommandFilters() {
  mCommandEnabled.clear();
  for (int i = 0; i < kC_Count; ++i) {
    mCommandEnabled.push_back(true);
  }
}

CControlMapper::EFunctionList CControlMapper::GetMapping(ECommands command) const {
  if (!mCommandOverridden.empty() && command < mCommandOverridden.size() &&
      mCommandOverridden[command]) {
    for (const rstl::pair< ECommands, int >* it = mCommandOverrides.begin();
         it != mCommandOverrides.end(); ++it) {
      if (it->first == command) {
        return static_cast< EFunctionList >(it->second);
      }
    }
  }
  return GetTweakPlayerControls()->GetMapping(command);
}

void CControlMapper::ResetCommandOverrides() {
  mCommandOverridden.clear();
  for (int i = 0; i < kC_Count; ++i) {
    mCommandOverridden.push_back(false);
  }
  mCommandOverrides.clear();
}

void CControlMapper::Reset() {
  ResetCommandFilters();
  ResetCommandOverrides();
}

const CTweakPlayerControls* CControlMapper::GetTweakPlayerControls() const {
  const CTweakPlayerControls* tweak = gpTweakPlayerControlsA.get();
  if (mControlScheme == 1) {
    tweak = gpTweakPlayerControlsB.get();
  }
  return tweak;
}
