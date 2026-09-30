#ifndef _CAUIMETER
#define _CAUIMETER

#include "GuiSys/CGuiCompoundWidget.hpp"
#include "rstl/vector.hpp"

class CAuiMeter : public CGuiCompoundWidget {
public:
  CAuiMeter(const CGuiWidgetParms& parms, const bool noRoundUp, const int maxCapacity,
            const int workerCount);

  // CGuiWidget
  FourCC GetWidgetTypeID() const override { return 'METR'; }
  bool AddWorkerWidget(CGuiWidget* worker) override;
  CGuiWidget* GetWorkerWidget(int idx) override;
  void OnVisible() override;

  void UpdateMeterWorkers();
  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);

private:
  bool mNoRoundUp;
  int mMaxCapacity;
  int mCapacity;
  int mValue;
  rstl::vector< CGuiWidget* > mWorkers;
};
CHECK_SIZEOF(CAuiMeter, 0xdc)

#endif // _CAUIMETER
