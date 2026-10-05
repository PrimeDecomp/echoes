#ifndef _CCONTROLMAPPER
#define _CCONTROLMAPPER

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

class CFinalInput;
class CTweakPlayerControls;

typedef float (CFinalInput::*FAnalogInput)() const;
typedef bool (CFinalInput::*FDigitalInput)() const;

class CControlMapper {
public:
  // Names follow retail descriptions and verified control properties.
  enum ECommands {
    kC_None,
    kC_Forward,
    kC_Backward,
    kC_TurnLeft,
    kC_TurnRight,
    kC_StrafeLeft,
    kC_StrafeRight,
    kC_LookLeft,
    kC_LookRight,
    kC_LookUp,
    kC_LookDown,
    kC_JumpOrBoost,
    kC_JumpOrBoost2,
    kC_FireOrBomb,
    kC_FireOrBomb2,
    kC_AutoFireBeam,
    kC_ChargeBeam,
    kC_ChargeBeam2,
    kC_MissileOrPowerBomb,
    kC_AimUp,
    kC_AimDown,
    kC_CycleBeamUp,
    kC_CycleBeamDown,
    kC_CycleItem,
    kC_PowerBeam,
    kC_IceBeam,
    kC_WaveBeam,
    kC_PlasmaBeam,
    kC_ToggleHolster,
    kC_OrbitClose,
    kC_OrbitFar,
    kC_OrbitObject,
    kC_OrbitSelect,
    kC_OrbitConfirm,
    kC_OrbitLeft,
    kC_OrbitRight,
    kC_OrbitUp,
    kC_OrbitDown,
    kC_LookHold1,
    kC_LookHold2,
    kC_LookZoomIn,
    kC_LookZoomOut,
    kC_AimHold,
    kC_MapCircleUp,
    kC_MapCircleDown,
    kC_MapCircleLeft,
    kC_MapCircleRight,
    kC_MapMoveForward,
    kC_MapMoveBack,
    kC_MapMoveLeft,
    kC_MapMoveRight,
    kC_MapZoomIn,
    kC_MapZoomOut,
    kC_SpiderBall,
    kC_ChaseCamera,
    kC_XrayVisor,
    kC_ThermoVisor,
    kC_EnviroVisor,
    kC_NoVisor,
    kC_VisorMenu,
    kC_VisorUp,
    kC_VisorDown,
    kC_DarkVisorToggle,
    kC_Crosshairs,
    kC_Unknown64,
    kC_UseShield,
    kC_ScanItem,
    kC_InventoryScreen,
    kC_MapScreen,
    kC_OptionsScreen,
    kC_LogScreen,
    kC_PauseScreenCycleLeft,
    kC_PauseScreenCycleRight,
    kC_BoostBall,
    kC_MorphIntoBall,
    kC_MorphFromBall,
    kC_Count
  };

  enum EFunctionList {
    kFL_None,
    kFL_LeftStickUp,
    kFL_LeftStickDown,
    kFL_LeftStickLeft,
    kFL_LeftStickRight,
    kFL_RightStickUp,
    kFL_RightStickDown,
    kFL_RightStickLeft,
    kFL_RightStickRight,
    kFL_LeftTrigger,
    kFL_RightTrigger,
    kFL_DPadUp,
    kFL_DPadDown,
    kFL_DPadLeft,
    kFL_DPadRight,
    kFL_AButton,
    kFL_BButton,
    kFL_XButton,
    kFL_YButton,
    kFL_ZButton,
    kFL_LeftTriggerPress,
    kFL_RightTriggerPress,
    kFL_Start,
    kFL_MAX
  };

  enum EFilterType { kFT_Filtered, kFT_Unfiltered };

  explicit CControlMapper(int controlScheme);

  float GetAnalogInput(ECommands command, const CFinalInput& input,
                       EFilterType filter = kFT_Filtered) const;
  const bool GetDigitalInput(ECommands command, const CFinalInput& input,
                             EFilterType filter = kFT_Filtered) const;
  const bool GetPressInput(ECommands command, const CFinalInput& input,
                           EFilterType filter = kFT_Filtered) const;

  // Echoes method names below are inferred from their implementations.
  void Reset();
  void ResetCommandFilters();
  void SetCommandFilters(const rstl::reserved_vector< bool, kC_Count >& enabled) {
    mCommandEnabled = enabled;
  }
  void ResetCommandOverrides();
  void SetControlScheme(int scheme) { mControlScheme = scheme; }
  EFunctionList GetMapping(ECommands command) const;
  const CTweakPlayerControls* GetTweakPlayerControls() const;

  static const char* GetDescriptionForCommand(ECommands command);
  static const char* GetDescriptionForFunction(EFunctionList function);

  static const FAnalogInput gAnalogInputs[kFL_MAX];
  static const FDigitalInput gDigitalInputs[kFL_MAX];
  static const FDigitalInput gPressInputs[kFL_MAX];

private:
  rstl::reserved_vector< bool, kC_Count > mCommandEnabled;
  rstl::reserved_vector< bool, kC_Count > mCommandOverridden;
  rstl::reserved_vector< rstl::pair< ECommands, int >, 8 > mCommandOverrides;
  int mControlScheme;
};
CHECK_SIZEOF(CControlMapper, 0xe8)

#endif // _CCONTROLMAPPER
