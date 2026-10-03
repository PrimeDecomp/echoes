#ifndef _CMULTIPLAYERGUI
#define _CMULTIPLAYERGUI

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

class CGuiFrame;
class CGuiFrameLoader;
class CGuiTextPane;
class CGuiWidget;
class CStateManager;

// Guessed names; the native object owns the multiplayer scoreboard and its loader.
class CMultiplayerGui {
public:
  explicit CMultiplayerGui(const CStateManager& mgr);
  ~CMultiplayerGui();
  void Draw() const;
  void Update(float dt, const CStateManager& mgr);

private:
  void BindWidgets(const CStateManager& mgr);

  rstl::auto_ptr< CGuiFrameLoader > mFrameLoader;
  rstl::auto_ptr< CGuiFrame > mFrame;
  CGuiFrame* mReadyFrame;
  rstl::reserved_vector< int, 4 > mPreviousScores;
  rstl::reserved_vector< int, 4 > mPreviousDeathCounts;
  rstl::reserved_vector< float, 4 > mScoreIncreaseFlashTimes;
  rstl::reserved_vector< float, 4 > mScoreDecreaseFlashTimes;
  rstl::reserved_vector< CGuiWidget*, 4 > mScoreWidgets;
  rstl::reserved_vector< CGuiWidget*, 4 > mScoreFlashModels;
  rstl::reserved_vector< CGuiTextPane*, 4 > mScoreTextPanes;
  CGuiTextPane* mTimeTextPane;
  CGuiWidget* mTimeBackground;
  CGuiWidget* mTimeFlash;
  float mTimeWarningSfxCooldown;
};
CHECK_SIZEOF(CMultiplayerGui, 0xb0)

#endif // _CMULTIPLAYERGUI
