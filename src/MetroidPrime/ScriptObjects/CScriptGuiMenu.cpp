#include "MetroidPrime/ScriptObjects/CScriptGuiMenu.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiMenu.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/math.hpp"

CScriptGuiMenu::CScriptGuiMenu(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               bool wrap, bool vertical, int controller,
                               const rstl::string& label, bool locked,
                               ushort selectionChangedSfx)
: CScriptGuiWidget(uid, name, info, controller, label, locked)
, mSelection(0)
, mWrapSelection(wrap)
, mVertical(vertical)
, mSelectionChangedSfx(selectionChangedSfx) {}

void CScriptGuiMenu::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CScriptGuiWidget::AcceptScriptMsg(mgr, msg);

  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    BuildItemList(mgr);
    return;
  case kSM_Activate: {
    TUniqueId id = mItems[mSelection];
    CEntity* item = mgr.ObjectById(id);
    mgr.SendScriptMsg(item, GetUniqueId(), kSM_Activate);
    return;
  }
  case kSM_Deactivate:
    for (rstl::vector< TUniqueId >::iterator it = mItems.begin(); it != mItems.end(); ++it) {
      CEntity* item = mgr.ObjectById(*it);
      mgr.SendScriptMsg(item, GetUniqueId(), kSM_Deactivate);
    }
    return;
  default:
    break;
  }

  switch (msg.GetMessage()) {
  case kSM_Increment: {
    const int prevSelection = mSelection;
    SelectNext(mgr);
    if (prevSelection != mSelection) {
      SendScriptMsgs(kSS_Footstep, mgr);
      SendScriptMsgs(kSS_Modify, mgr);
      CSfxManager::SfxStart(mSelectionChangedSfx, 127, 63);
    }
    break;
  }
  case kSM_Decrement: {
    const int prevSelection = mSelection;
    SelectPrevious(mgr);
    if (prevSelection != mSelection) {
      SendScriptMsgs(kSS_Footstep, mgr);
      SendScriptMsgs(kSS_Modify, mgr);
      CSfxManager::SfxStart(mSelectionChangedSfx, 127, 63);
    }
    break;
  }
  case kSM_SetToZero:
    SelectFirst(mgr);
    break;
  case kSM_SetToMax:
    SelectLast(mgr);
    break;
  case kSM_InternalMessage00:
  case kSM_InternalMessage01:
  case kSM_InternalMessage02:
    UpdateItemStates(mgr, msg.GetMessage());
    break;
  default:
    break;
  }

  if (!GetActive()) {
    return;
  }
}

void CScriptGuiMenu::Think(float dt, CStateManager& mgr) {
  CScriptGuiWidget::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  if (saveScreen != nullptr && saveScreen->GetUIType() != CSaveGameScreen::kUIT_SaveReady) {
    return;
  }

  if (mControllers.size() == 0) {
    ProcessMenuInput(mgr.mFinalInputs[mControllerNumber], mgr);
  } else if (!ProcessMenuInput(mgr.mFinalInputs[mControllers[mActiveController]], mgr)) {
    for (int i = 0; i < mControllers.size(); ++i) {
      if (i != mActiveController && ProcessMenuInput(mgr.mFinalInputs[mControllers[i]], mgr)) {
        mActiveController = i;
        return;
      }
    }
  }
}

void CScriptGuiMenu::BuildItemList(CStateManager& mgr) {
  if (mItems.size() != 0) {
    return;
  }

  mItems.reserve(GetConnectionList().size());
  TUniqueId selectedId = kInvalidUniqueId;
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    const EScriptObjectState state = it->state;
    const TUniqueId id = mgr.GetIdForScript(it->objId);
    if (state == kSS_Connect) {
      mItems.push_back_unsafe(id);
    } else if (state == kSS_Play) {
      selectedId = id;
    }
  }

  for (int i = 0; i < mItems.size(); ++i) {
    if (mItems[i] == selectedId) {
      mSelection = i;
      return;
    }
  }
}

void CScriptGuiMenu::SetSelection(int idx, CStateManager& mgr) {
  if (idx == mSelection) {
    return;
  }

  TUniqueId oldId = mItems[mSelection];
  CEntity* oldItem = mgr.ObjectById(oldId);
  mgr.SendScriptMsg(oldItem, GetUniqueId(), kSM_Close);
  if (GetActive()) {
    mgr.SendScriptMsg(oldItem, GetUniqueId(), kSM_Deactivate);
  }

  mSelection = idx;
  TUniqueId newId = mItems[mSelection];
  CEntity* newItem = mgr.ObjectById(newId);
  if (GetActive()) {
    mgr.SendScriptMsg(newItem, GetUniqueId(), kSM_Activate);
  }
  mgr.SendScriptMsg(newItem, GetUniqueId(), kSM_Open);
}

void CScriptGuiMenu::SelectPrevious(CStateManager& mgr) {
  const int count = mItems.size();
  if (mWrapSelection) {
    for (int i = (mSelection + count - 1) % count; i != mSelection; i = (i + count - 1) % count) {
      if (!IsItemLocked(i, mgr)) {
        SetSelection(i, mgr);
        return;
      }
    }
  } else {
    for (int i = rstl::max_val(-1, mSelection - 1); i >= 0; --i) {
      if (!IsItemLocked(i, mgr)) {
        SetSelection(i, mgr);
        return;
      }
    }
  }
}

void CScriptGuiMenu::SelectNext(CStateManager& mgr) {
  const int count = mItems.size();
  if (mWrapSelection) {
    for (int i = (mSelection + 1) % count; i != mSelection; i = (i + 1) % count) {
      if (!IsItemLocked(i, mgr)) {
        SetSelection(i, mgr);
        return;
      }
    }
  } else {
    for (int i = rstl::min_val(mSelection + 1, count); i < count; ++i) {
      if (!IsItemLocked(i, mgr)) {
        SetSelection(i, mgr);
        return;
      }
    }
  }
}

void CScriptGuiMenu::SelectFirst(CStateManager& mgr) {
  const int count = mItems.size();
  for (int i = 0; i < count; ++i) {
    if (!IsItemLocked(i, mgr)) {
      SetSelection(i, mgr);
      return;
    }
  }
}

void CScriptGuiMenu::SelectLast(CStateManager& mgr) {
  for (int i = mItems.size() - 1; i >= 0; --i) {
    if (!IsItemLocked(i, mgr)) {
      SetSelection(i, mgr);
      return;
    }
  }
}

void CScriptGuiMenu::UpdateItemStates(CStateManager& mgr, EScriptObjectMessage msg) {
  for (int i = 0; i < mItems.size(); ++i) {
    TUniqueId id = mItems[i];
    CEntity* item = mgr.ObjectById(id);
    switch (msg) {
    case kSM_InternalMessage00:
      item->SendScriptMsgs(i == mSelection ? kSS_InternalState00 : kSS_InternalState10, mgr);
      break;
    case kSM_InternalMessage01:
      item->SendScriptMsgs(i == mSelection ? kSS_InternalState01 : kSS_InternalState11, mgr);
      break;
    case kSM_InternalMessage02:
      item->SendScriptMsgs(i == mSelection ? kSS_InternalState02 : kSS_InternalState12, mgr);
      break;
    default:
      break;
    }
  }
}

bool CScriptGuiMenu::IsItemLocked(int idx, CStateManager& mgr) const {
  if (CEntity* item = mgr.ObjectById(mItems[idx])) {
    if (const CScriptGuiWidget* widget = TCastToConstPtr< CScriptGuiWidget >(item)) {
      return widget->IsLocked();
    }
    return false;
  }
  return true;
}

bool CScriptGuiMenu::ProcessMenuInput(const CFinalInput& input, CStateManager& mgr) {
  bool prev = mVertical ? input.DLAUp() || input.DDPUp() : input.DLALeft() || input.DDPLeft();
  bool next =
      mVertical ? input.DLADown() || input.DDPDown() : input.DLARight() || input.DDPRight();

  if (mPrevRepeat.Update(input.DeltaTime(), prev) && prev) {
    mgr.SendScriptMsg(this, GetUniqueId(), kSM_Decrement);
  } else if (!prev && mNextRepeat.Update(input.DeltaTime(), next) && next) {
    mgr.SendScriptMsg(this, GetUniqueId(), kSM_Increment);
  }

  return prev || next;
}

CEntity* LoadGuiMenu(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGuiMenu sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGuiMenu.inc"

  bool vertical = sldrThis.controlDirection == 0;
  const int controller = sldrThis.widgetProperties.controllerNumber - 1;
  return rs_new CScriptGuiMenu(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                               LdrToEntityInfo(info, sldrThis.editorProperties),
                               sldrThis.wrapSelection, vertical, controller,
                               sldrThis.widgetProperties.guiLabel,
                               sldrThis.widgetProperties.isLocked,
                               sldrThis.selectionChangedSound);
}

CScriptGuiMenu::~CScriptGuiMenu() {}
