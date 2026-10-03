#ifndef _CGUITABLEGROUP
#define _CGUITABLEGROUP

#include "GuiSys/CGuiCompoundWidget.hpp"
#include "GuiSys/CRepeatState.hpp"
#include "Kyoto/TFunctor.hpp"

// Guessed class and method names correlated with Prime; native type ID is TBGP.
class CGuiTableGroup : public CGuiCompoundWidget {
public:
  enum ETableSelectReturn { kTSR_Changed, kTSR_Unchanged, kTSR_WrappedAround };

  CGuiTableGroup(const CGuiWidgetParms& parms, bool selectWrapAround);
  static CGuiTableGroup* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                                uint version);

  // CGuiObject
  ~CGuiTableGroup() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  EWidgetUsageFlags GetWidgetUsageFlags() const override;
  bool AddWorkerWidget(CGuiWidget* worker) override;
  void ProcessUserInput(const CFinalInput& input) override;
  void OnActivate() override;
  void Initialize() override;

  void SetMenuAdvanceCallback(const TFunctor1< CGuiTableGroup* const >& callback);
  void
  SetMenuSelectionChangeCallback(const TFunctor2< CGuiTableGroup* const, const int >& callback);

  void SetColors(const CColor& selected, const CColor& unselected);
  void SelectWorker(int worker);
  bool IsWorkerSelectable(int worker);

  int GetUserSelection() const { return mUserSelection; }

  int GetElementCount() const { return mElementCount; }

  void SetUserSelection(int selection) {
    mPrevUserSelection = mUserSelection;
    mUserSelection = selection;
  }

  void SetVertical(bool vertical) { mVertical = vertical; }

  bool HasMenuAdvanceCallback() const { return mDoMenuAdvance; }

private:
  bool DoAdvance();
  bool DoCancel();
  bool DoDecrement();
  bool DoIncrement();
  bool PreDecrement();
  bool PreIncrement();
  void ActivateWorker(CGuiWidget* worker);
  void DeactivateWorker(CGuiWidget* worker);
  ETableSelectReturn IncrementSelectedRow();
  ETableSelectReturn DecrementSelectedRow();
  void DoSelectNextRow();
  void DoSelectPrevRow();

  CRepeatState mDecRepeat;
  CRepeatState mIncRepeat;
  int mElementCount;
  int mUserSelection;
  int mPrevUserSelection;
  bool mSelectWrapAround;
  bool mVertical;
  TFunctor1< CGuiTableGroup* const > mDoMenuAdvance;
  TFunctor1< CGuiTableGroup* const > mDoMenuCancel;
  TFunctor2< CGuiTableGroup* const, const int > mDoMenuSelChange;
};
CHECK_SIZEOF(CGuiTableGroup, 0x11c)

#endif // _CGUITABLEGROUP
