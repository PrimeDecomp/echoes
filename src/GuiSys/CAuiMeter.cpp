#include "GuiSys/CAuiMeter.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CGuiWidget* CAuiMeter::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  in.ReadBool();
  const bool noRoundUp = in.ReadBool();
  const int maxCapacity = in.ReadInt32();
  const int workerCount = in.ReadInt32();
  CGuiWidget* widget = rs_new CAuiMeter(parms, noRoundUp, maxCapacity, workerCount);
  widget->ParseBaseInfo(frame, in, parms, version);
  return widget;
}

CAuiMeter::CAuiMeter(const CGuiWidgetParms& parms, const bool noRoundUp, const int maxCapacity,
                     const int workerCount)
: CGuiCompoundWidget(parms)
, mNoRoundUp(noRoundUp)
, mMaxCapacity(maxCapacity)
, mCapacity(mMaxCapacity)
, mValue(0) {
  mWorkers.reserve(workerCount);
}

bool CAuiMeter::AddWorkerWidget(CGuiWidget* worker) {
  short id = worker->GetWorkerId();
  if (id >= mWorkers.size()) {
    for (int i = mWorkers.size(); i <= id; ++i) {
      mWorkers.push_back_unsafe(nullptr);
    }
  }
  mWorkers[id] = worker;
  return true;
}

CGuiWidget* CAuiMeter::GetWorkerWidget(int idx) { return mWorkers[idx]; }

void CAuiMeter::OnVisible() {
  if (GetIsVisible()) {
    UpdateMeterWorkers();
  }
}

void CAuiMeter::UpdateMeterWorkers() {
  int workerCount = mWorkers.size() / 2;
  const float scale = workerCount / float(mMaxCapacity);
  int etankCap = mNoRoundUp ? static_cast< int >(scale * mCapacity)
                            : static_cast< int >(0.5f + scale * mCapacity);
  int etankFill =
      mNoRoundUp ? static_cast< int >(scale * mValue) : static_cast< int >(0.5f + scale * mValue);

  for (int i = 0; i < workerCount; ++i) {
    CGuiWidget* const empty = mWorkers[i * 2];
    CGuiWidget* const filled = mWorkers[i * 2 + 1];

    if (i < etankFill) {
      if (filled)
        filled->SetIsVisible(true);
      if (empty)
        empty->SetIsVisible(false);
    } else if (i < etankCap) {
      if (filled)
        filled->SetIsVisible(false);
      if (empty)
        empty->SetIsVisible(true);
    } else {
      if (filled)
        filled->SetIsVisible(false);
      if (empty)
        empty->SetIsVisible(false);
    }
  }
}
