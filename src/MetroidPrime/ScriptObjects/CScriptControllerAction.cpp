#include "MetroidPrime/ScriptObjects/CScriptControllerAction.hpp"

#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrControllerAction.hpp"

CScriptControllerAction::CScriptControllerAction(TUniqueId uid, const rstl::string& name,
                                                 const CEntityInfo& info,
                                                 const CControlMapper::ECommands command,
                                                 const bool mapScreenResponse,
                                                 const uint mapScreenSubaction,
                                                 const bool deactivateOnClose)
: CEntity(uid, info, name, 0)
, mCommand(command)
, mMapScreenSubaction(mapScreenSubaction)
, mMapScreenResponse(mapScreenResponse)
, mDeactivateOnClose(deactivateOnClose)
, mPressed(false) {}

void CScriptControllerAction::Think(float dt, CStateManager& mgr) {
  const bool oldPressed = mPressed;

  if (mMapScreenResponse) {
    switch (mMapScreenSubaction) {
    case 0:
      if (mgr.GetInMapScreen()) {
        mPressed = true;
      } else {
        mPressed = false;
      }
      break;
    default:
      break;
    }
  } else if (gpMain->IsMaxSpeed()) {
    mPressed = !mPressed;
  } else {
    if (gpGameState->ControlMapper().GetDigitalInput(mCommand, mgr.mFinalInputs[0],
                                                     CControlMapper::kFT_Unfiltered)) {
      mPressed = true;
    } else {
      mPressed = false;
    }
  }

  if (GetActive() && mPressed != oldPressed) {
    if (mPressed) {
      SendScriptMsgs(kSS_Opened, mgr);
      return;
    }

    SendScriptMsgs(kSS_Closed, mgr);
    if (mDeactivateOnClose) {
      SetActive(false);
      SendScriptMsgs(kSS_Inactive, mgr);
    }
  }
}

CEntity* LoadControllerAction(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrControllerAction sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrControllerAction.inc"

  const int command = sldrThis.cmd.command;
  if (command > 0 && command <= 0x4c) {
    return rs_new CScriptControllerAction(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                          LdrToEntityInfo(info, sldrThis.editorProperties),
                                          static_cast< CControlMapper::ECommands >(command), false,
                                          0, sldrThis.oneShot);
  }
  return nullptr;
}

CScriptControllerAction::~CScriptControllerAction() {}
