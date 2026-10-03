#ifndef _CGUISLIDERGROUP
#define _CGUISLIDERGROUP

#include "GuiSys/CGuiCompoundWidget.hpp"
#include "Kyoto/TFunctor.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed class and method names correlated with Prime; native type ID is SLGP.
class CGuiSliderGroup : public CGuiCompoundWidget {
public:
  enum EState { kS_None, kS_Decreasing, kS_Increasing };

  CGuiSliderGroup(const CGuiWidgetParms& parms, float min, float max, float cur, float increment);
  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);

  // CGuiObject
  ~CGuiSliderGroup() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  EWidgetUsageFlags GetWidgetUsageFlags() const override;
  bool AddWorkerWidget(CGuiWidget* worker) override;
  void Update(float dt) override;
  void ProcessUserInput(const CFinalInput& input) override;
  CGuiWidget* GetWorkerWidget(int id) override;

  float GetCurVal() const { return mRoundedCurVal; }

  EState GetState() const { return mState; }

private:
  // Guessed names: native operations only select direction and mark held input.
  void Decrement();
  void Increment();

  float mMinVal;
  float mMaxVal;
  float mRoundedCurVal;
  float mCurVal;
  float mIncrement;
  rstl::reserved_vector< CGuiWidget*, 2 > mSliderRangeWidgets;
  TFunctor2< CGuiSliderGroup* const, const float > mChangeCallback;
  EState mState;
  bool mInputPending : 1;
};
CHECK_SIZEOF(CGuiSliderGroup, 0xfc)

#endif // _CGUISLIDERGROUP
