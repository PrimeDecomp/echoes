#include "GuiSys/CGuiTableGroup.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"

void CGuiTableGroup::ProcessUserInput(const CFinalInput& input) {
  if (input.PA()) {
    DoAdvance();
  } else if (input.PB()) {
    DoCancel();
  } else {
    bool decrement =
        mVertical ? input.DLAUp() || input.DDPUp() : input.DLALeft() || input.DDPLeft();
    bool increment =
        mVertical ? input.DLADown() || input.DDPDown() : input.DLARight() || input.DDPRight();

    if (mDecRepeat.Update(input.DeltaTime(), decrement) && decrement) {
      DoDecrement();
      return;
    }
    if (!decrement && mIncRepeat.Update(input.DeltaTime(), increment) && increment) {
      DoIncrement();
    }
  }
}

CGuiTableGroup* CGuiTableGroup::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                                       uint version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  bool selectWrapAround;
  if (version < 2u) {
    // Legacy selection/range properties are consumed but not retained.
    in.ReadInt16();
    in.ReadUint16();
    in.ReadInt32();
    in.ReadInt16();
    in.ReadUint16();
    const bool wrap = in.ReadBool();
    selectWrapAround = wrap;
    in.ReadBool();
    in.ReadFloat();
    in.ReadFloat();
    in.ReadBool();
    in.ReadFloat();
    in.ReadUint16();
    in.ReadUint16();
    in.ReadUint16();
    in.ReadUint16();
  } else {
    const bool wrap = in.ReadBool();
    selectWrapAround = wrap;
  }

  CGuiTableGroup* group = rs_new CGuiTableGroup(parms, selectWrapAround);
  group->ParseBaseInfo(frame, in, parms, version);
  return group;
}

CGuiTableGroup::CGuiTableGroup(const CGuiWidgetParms& parms, bool selectWrapAround)
: CGuiCompoundWidget(parms)
, mElementCount(0)
, mUserSelection(0)
, mPrevUserSelection(0)
, mSelectWrapAround(selectWrapAround)
, mVertical(true) {}

CGuiTableGroup::~CGuiTableGroup() {}

void CGuiTableGroup::DoSelectNextRow() {
  IncrementSelectedRow();
  DeactivateWorker(GetWorkerWidget(mPrevUserSelection));
  ActivateWorker(GetWorkerWidget(mUserSelection));
}

void CGuiTableGroup::DoSelectPrevRow() {
  DecrementSelectedRow();
  DeactivateWorker(GetWorkerWidget(mPrevUserSelection));
  ActivateWorker(GetWorkerWidget(mUserSelection));
}

bool CGuiTableGroup::DoIncrement() {
  int userSelect = mUserSelection;
  if (PreIncrement()) {
    if (mDoMenuSelChange) {
      mDoMenuSelChange(this, userSelect);
    }
    return true;
  }
  return false;
}

bool CGuiTableGroup::IsWorkerSelectable(int worker) {
  if (CGuiWidget* widget = GetWorkerWidget(worker)) {
    return widget->GetIsSelectable();
  }
  return false;
}

bool CGuiTableGroup::PreIncrement() {
  if (mSelectWrapAround) {
    for (int sel = (mUserSelection + 1) % mElementCount; sel != mUserSelection;
         sel = (sel + 1) % mElementCount) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }
  } else {
    for (int sel = rstl::min_val(mUserSelection + 1, mElementCount); sel < mElementCount; ++sel) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }
  }
  return false;
}

bool CGuiTableGroup::DoDecrement() {
  int userSelect = mUserSelection;
  if (PreDecrement()) {
    if (mDoMenuSelChange) {
      mDoMenuSelChange(this, userSelect);
    }
    return true;
  }
  return false;
}

bool CGuiTableGroup::PreDecrement() {
  if (mSelectWrapAround) {
    for (int sel = (mUserSelection + mElementCount - 1) % mElementCount; sel != mUserSelection;
         sel = (sel + mElementCount - 1) % mElementCount) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }
  } else {
    for (int sel = rstl::max_val(-1, mUserSelection - 1); sel >= 0; --sel) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }
  }
  return false;
}

void CGuiTableGroup::SelectWorker(int worker) {
  const int selection = CMath::Clamp(0, worker, mElementCount - 1);
  if (selection < mUserSelection) {
    while (selection != mUserSelection) {
      DoSelectPrevRow();
    }
  } else {
    while (selection != mUserSelection) {
      DoSelectNextRow();
    }
  }
}

bool CGuiTableGroup::DoAdvance() {
  if (HasMenuAdvanceCallback()) {
    mDoMenuAdvance(this);
  }
  return true;
}

bool CGuiTableGroup::DoCancel() {
  if (mDoMenuCancel) {
    mDoMenuCancel(this);
  }
  return true;
}

void CGuiTableGroup::SetMenuAdvanceCallback(const TFunctor1< CGuiTableGroup* const >& callback) {
  mDoMenuAdvance = callback;
}

void CGuiTableGroup::SetMenuSelectionChangeCallback(
    const TFunctor2< CGuiTableGroup* const, const int >& callback) {
  mDoMenuSelChange = callback;
}

void CGuiTableGroup::OnActivate() {
  CGuiWidget::OnActivate();
  CGuiWidget* worker = GetWorkerWidget(mUserSelection);
  worker->SetIsActive(GetIsActive());
}

void CGuiTableGroup::Initialize() {
  for (;;) {
    if (CGuiWidget::GetWorkerWidget(mElementCount) == nullptr) {
      break;
    }
    ++mElementCount;
  }
}

void CGuiTableGroup::ActivateWorker(CGuiWidget* worker) { worker->SetIsActive(true); }

void CGuiTableGroup::DeactivateWorker(CGuiWidget* worker) { worker->SetIsActive(false); }

CGuiTableGroup::ETableSelectReturn CGuiTableGroup::IncrementSelectedRow() {
  mPrevUserSelection = mUserSelection;
  if (++mUserSelection >= mElementCount) {
    mUserSelection = mSelectWrapAround ? 0 : mElementCount - 1;
    return mSelectWrapAround ? kTSR_WrappedAround : kTSR_Unchanged;
  }
  return kTSR_Changed;
}

CGuiTableGroup::ETableSelectReturn CGuiTableGroup::DecrementSelectedRow() {
  mPrevUserSelection = mUserSelection;
  if (--mUserSelection < 0) {
    mUserSelection = mSelectWrapAround ? mElementCount - 1 : 0;
    return mSelectWrapAround ? kTSR_WrappedAround : kTSR_Unchanged;
  }
  return kTSR_Changed;
}

void CGuiTableGroup::SetColors(const CColor& selected, const CColor& unselected) {
  const int selection = mUserSelection;
  for (int id = 0;; ++id) {
    CGuiWidget* worker = GetWorkerWidget(id);
    if (!worker) {
      break;
    }
    worker->SetColor(id == selection ? selected : unselected);
  }
}

bool CGuiTableGroup::AddWorkerWidget(CGuiWidget* worker) { return true; }

CGuiWidget::EWidgetUsageFlags CGuiTableGroup::GetWidgetUsageFlags() const {
  return EWidgetUsageFlags(kWUF_Update | kWUF_Input);
}

FourCC CGuiTableGroup::GetWidgetTypeID() const { return 'TBGP'; }
