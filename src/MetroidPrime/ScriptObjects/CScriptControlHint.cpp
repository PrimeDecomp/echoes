#include "MetroidPrime/ScriptObjects/CScriptControlHint.hpp"

#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrControlHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptControlHint::CScriptControlHint(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       int priority, float timer, uint disableFlags,
                                       const TCommandStates& commandStates,
                                       EBreakHintType breakType, uint deleteOnRemoval,
                                       uint requiredPresses, float unknown16c, SCallback onExpire,
                                       SCallback onBreak, float breakDelay, int acrossAreas)
: CGameHint(uid, name, info, xf, priority, timer, acrossAreas, breakType, deleteOnRemoval,
            requiredPresses, unknown16c, onExpire, onBreak, breakDelay)
, mDisableFlags(disableFlags)
, mCommandEnabled(true) {
  if (mDisableFlags & kDF_All) {
    for (int i = 0; i < mCommandEnabled.size(); ++i) {
      mCommandEnabled[i] = false;
    }
  }
  if (mDisableFlags & kDF_Weapons) {
    mCommandEnabled[CControlMapper::kC_FireOrBomb] = false;
    mCommandEnabled[CControlMapper::kC_FireOrBomb2] = false;
    mCommandEnabled[CControlMapper::kC_MissileOrPowerBomb] = false;
    mCommandEnabled[CControlMapper::kC_AutoFireBeam] = false;
    mCommandEnabled[CControlMapper::kC_ChargeBeam] = false;
    mCommandEnabled[CControlMapper::kC_ChargeBeam2] = false;
    mCommandEnabled[CControlMapper::kC_ChargeBeam2] = false;
  }
  if (mDisableFlags & kDF_Movement) {
    mCommandEnabled[CControlMapper::kC_Backward] = false;
    mCommandEnabled[CControlMapper::kC_Forward] = false;
    mCommandEnabled[CControlMapper::kC_JumpOrBoost] = false;
    mCommandEnabled[CControlMapper::kC_JumpOrBoost2] = false;
    mCommandEnabled[CControlMapper::kC_StrafeLeft] = false;
    mCommandEnabled[CControlMapper::kC_StrafeRight] = false;
    mCommandEnabled[CControlMapper::kC_TurnLeft] = false;
    mCommandEnabled[CControlMapper::kC_TurnRight] = false;
  }
  if (mDisableFlags & kDF_Orbit) {
    mCommandEnabled[CControlMapper::kC_OrbitClose] = false;
    mCommandEnabled[CControlMapper::kC_OrbitConfirm] = false;
    mCommandEnabled[CControlMapper::kC_OrbitDown] = false;
    mCommandEnabled[CControlMapper::kC_OrbitFar] = false;
    mCommandEnabled[CControlMapper::kC_OrbitLeft] = false;
    mCommandEnabled[CControlMapper::kC_OrbitObject] = false;
    mCommandEnabled[CControlMapper::kC_OrbitRight] = false;
    mCommandEnabled[CControlMapper::kC_OrbitSelect] = false;
    mCommandEnabled[CControlMapper::kC_OrbitUp] = false;
  }
  if (mDisableFlags & kDF_Visors) {
    mCommandEnabled[CControlMapper::kC_VisorDown] = false;
    mCommandEnabled[CControlMapper::kC_VisorUp] = false;
    mCommandEnabled[CControlMapper::kC_DarkVisorToggle] = false;
    mCommandEnabled[CControlMapper::kC_EnviroVisor] = false;
    mCommandEnabled[CControlMapper::kC_NoVisor] = false;
    mCommandEnabled[CControlMapper::kC_ThermoVisor] = false;
    mCommandEnabled[CControlMapper::kC_VisorMenu] = false;
    mCommandEnabled[CControlMapper::kC_XrayVisor] = false;
  }
  if (mDisableFlags & kDF_Beams) {
    mCommandEnabled[CControlMapper::kC_ToggleHolster] = false;
    mCommandEnabled[CControlMapper::kC_IceBeam] = false;
    mCommandEnabled[CControlMapper::kC_PlasmaBeam] = false;
    mCommandEnabled[CControlMapper::kC_PowerBeam] = false;
    mCommandEnabled[CControlMapper::kC_WaveBeam] = false;
  }
  if (mDisableFlags & kDF_Look) {
    mCommandEnabled[CControlMapper::kC_LookDown] = false;
    mCommandEnabled[CControlMapper::kC_LookHold1] = false;
    mCommandEnabled[CControlMapper::kC_LookHold2] = false;
    mCommandEnabled[CControlMapper::kC_LookLeft] = false;
    mCommandEnabled[CControlMapper::kC_LookRight] = false;
    mCommandEnabled[CControlMapper::kC_LookUp] = false;
    mCommandEnabled[CControlMapper::kC_LookZoomIn] = false;
    mCommandEnabled[CControlMapper::kC_LookZoomOut] = false;
  }
  if (mDisableFlags & kDF_Morph) {
    mCommandEnabled[CControlMapper::kC_MorphIntoBall] = false;
  }
  if (mDisableFlags & kDF_Unmorph) {
    mCommandEnabled[CControlMapper::kC_MorphFromBall] = false;
  }
  if (mDisableFlags & kDF_Command73) {
    mCommandEnabled[CControlMapper::kC_BoostBall] = false;
  }
  if (mDisableFlags & kDF_SpiderBall) {
    mCommandEnabled[CControlMapper::kC_SpiderBall] = false;
  }
  for (int i = 0; i < commandStates.size(); ++i) {
    if (commandStates[i].second == 0) {
      mCommandEnabled[commandStates[i].first] = false;
    }
  }
}

void CScriptControlHint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
  case kSM_Decrement: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
    if (!player || !mgr.IsMultiplayer()) {
      player = mgr.GetPlayer(0);
    }
    player->GetControlHintManager()->RemoveHint(GetUniqueId(), sender, mgr);
    break;
  }
  case kSM_Increment:
    if (GetActive()) {
      CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
      if (!player || !mgr.IsMultiplayer()) {
        player = mgr.GetPlayer(0);
      }
      player->GetControlHintManager()->AddHint(GetUniqueId(), sender, mgr);
    }
    break;
  case kSM_AreaLoaded:
  case kSM_Delete:
    break;
  default:
    break;
  }
  CGameHint::AcceptScriptMsg(mgr, msg);
}

// Guessed helper name, based on the loader's nonzero-command conversion.
static void AppendCommand(CControlMapper::ECommands command, int state,
                          CScriptControlHint::TCommandStates& commands) {
  if (static_cast< uint >(command) != CControlMapper::kC_None) {
    commands.push_back(rstl::pair< CControlMapper::ECommands, int >(command, state));
  }
}

CEntity* LoadControlHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrControlHint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrControlHint.inc"

  CScriptControlHint::TCommandStates commands;
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command1.command.command),
                sldrThis.command1.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command2.command.command),
                sldrThis.command2.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command3.command.command),
                sldrThis.command3.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command4.command.command),
                sldrThis.command4.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command5.command.command),
                sldrThis.command5.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command6.command.command),
                sldrThis.command6.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command7.command.command),
                sldrThis.command7.state, commands);
  AppendCommand(static_cast< CControlMapper::ECommands >(sldrThis.command8.command.command),
                sldrThis.command8.state, commands);
  return rs_new CScriptControlHint(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.priority, sldrThis.timer, sldrThis.disableControlFlagsControlHint, commands,
      static_cast< CGameHint::EBreakHintType >(sldrThis.cancelMethod), 0, sldrThis.cancelPressCount,
      sldrThis.cancelPressTime, CGameHint::SCallback(), CGameHint::SCallback(),
      sldrThis.cancelTimer, 0);
}

CScriptControlHint::~CScriptControlHint() {}
