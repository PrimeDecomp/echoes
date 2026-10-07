#include "MetroidPrime/ScriptObjects/CScriptGuiWidget.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Input/IController.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidget.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptGuiWidget::CScriptGuiWidget(TUniqueId uid, const rstl::string& name,
                                   const CEntityInfo& info, int controller,
                                   const rstl::string& label, bool locked)
: CEntity(uid, info, name, 0)
, mActiveController(0)
, mControllerNumber(controller)
, mLabel(label)
, mControllerPresent(false)
, mLocked(locked) {
  mControllers.reserve(4);
}

void CScriptGuiWidget::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Lock:
    SetLocked(true, mgr);
    break;
  case kSM_Unlock:
    SetLocked(false, mgr);
    break;
  case kSM_Follow:
    if (const CScriptGuiWidget* widget =
            TCastToConstPtr< CScriptGuiWidget >(mgr.GetObjectById(msg.GetSenderId()))) {
      AddController(widget->GetControllerNumber());
    }
    break;
  case kSM_Escape:
    ClearControllers();
    break;
  case kSM_Alert:
    SendScriptMsgs(kSS_Attack, mgr);
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(mgr, msg);

  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Open:
      SendScriptMsgs(kSS_Entered, mgr);
      if (mCallback) {
        mCallback(mgr, this, kWE_Open);
      }
      break;
    case kSM_Close:
      SendScriptMsgs(kSS_Exited, mgr);
      if (mCallback) {
        mCallback(mgr, this, kWE_Close);
      }
      break;
    default:
      break;
    }
  }
}

void CScriptGuiWidget::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  if (!GetActive()) {
    mControllerPresent = false;
    return;
  }

  const bool present = gpController->GetGamepadData(mControllerNumber).DeviceIsPresent();
  if (present != mControllerPresent) {
    mControllerPresent = !mControllerPresent;
    SendScriptMsgs(mControllerPresent ? kSS_UnFrozen : kSS_Frozen, mgr);
  }

  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  if (saveScreen != nullptr && saveScreen->GetUIType() != CSaveGameScreen::kUIT_SaveReady) {
    return;
  }

  if (mControllers.size() == 0) {
    ProcessInput(mgr.mFinalInputs[mControllerNumber], mgr);
  } else if (!ProcessInput(mgr.mFinalInputs[mControllers[mActiveController]], mgr)) {
    for (int i = 0; i < mControllers.size(); ++i) {
      if (i != mActiveController && ProcessInput(mgr.mFinalInputs[mControllers[i]], mgr)) {
        mActiveController = i;
        return;
      }
    }
  }
}

bool CScriptGuiWidget::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  if (input.PA()) {
    SendScriptMsgs(kSS_APRC, mgr);
    SendScriptMsgs(kSS_PressA, mgr);
    if (mCallback) {
      mCallback(mgr, this, kWE_Accept);
    }
    return true;
  }
  if (input.PB()) {
    SendScriptMsgs(kSS_Retreat, mgr);
    SendScriptMsgs(kSS_PressB, mgr);
    if (mCallback) {
      mCallback(mgr, this, kWE_Back);
    }
    return true;
  }
  if (input.PX()) {
    SendScriptMsgs(kSS_PressX, mgr);
    return true;
  }
  if (input.PY()) {
    SendScriptMsgs(kSS_PressY, mgr);
    return true;
  }
  if (input.PZ()) {
    SendScriptMsgs(kSS_PressZ, mgr);
    return true;
  }
  if (input.PStart()) {
    SendScriptMsgs(kSS_PressStart, mgr);
    return true;
  }
  return false;
}

void CScriptGuiWidget::SetLocked(bool locked, CStateManager& mgr) {
  mLocked = locked;
  if (mLocked) {
    SendScriptMsgs(kSS_Locked, mgr);
  } else {
    SendScriptMsgs(kSS_Unlocked, mgr);
  }
}

void CScriptGuiWidget::AddController(int controller) {
  for (int i = 0; i < mControllers.size(); ++i) {
    if (controller == mControllers[i]) {
      return;
    }
  }
  mControllers.push_back_unsafe(controller);
}

void CScriptGuiWidget::ClearControllers() {
  mControllers.clear();
  mActiveController = 0;
}

CEntity* LoadGuiWidget(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGuiWidget sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGuiWidget.inc"

  const int controller = sldrThis.widgetProperties.controllerNumber - 1;
  return rs_new CScriptGuiWidget(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                 LdrToEntityInfo(info, sldrThis.editorProperties),
                                 controller,
                                 sldrThis.widgetProperties.guiLabel,
                                 sldrThis.widgetProperties.isLocked);
}

CScriptGuiWidget::~CScriptGuiWidget() {}
