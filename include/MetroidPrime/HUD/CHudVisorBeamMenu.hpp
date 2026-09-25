#ifndef _CHUDVISORBEAMMENU
#define _CHUDVISORBEAMMENU
#include "Kyoto/TToken.hpp"
#include "rstl/reserved_vector.hpp"
class CGuiFrame;
class CGuiWidget;
class CGuiTextPane;
class CStringTable;
class CHudVisorBeamMenu {
public:
  enum EVisorBeamMenu { kVBM_Visor, kVBM_Beam };
  CHudVisorBeamMenu(CGuiFrame& frame, const TLockedToken< CStringTable >& strings,
                    EVisorBeamMenu type, const rstl::reserved_vector< bool, 4 >& enables,
                    int selection, bool multiplayer);
  ~CHudVisorBeamMenu();
  void SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables, int selection);

private:
  struct SMenuItem {
    CGuiWidget* mLozenge;
    CGuiWidget* mIcon;
    float mPosition;
    float mOpacity;
    bool mEnabled : 1;
  };
  CGuiFrame& mFrame;
  const TLockedToken< CStringTable >& mStrings;
  EVisorBeamMenu mType;
  int mSelectedItem;
  int mPendingSelection;
  float mInterpolation;
  bool mVisibleDebug : 1;
  bool mVisibleGame : 1;
  bool mDirty : 1;
  CGuiWidget* mMenuRoot;
  CGuiWidget* mTitleRoot;
  CGuiTextPane* mTitle;
  CGuiWidget* mGhost;
  rstl::reserved_vector< SMenuItem, 4 > mItems;
  int mAnimationPhase;
  float x84;
  float x88;
  float mTextFade;
  float mAnimationDuration;
  bool mSwapBeamControls : 1;
  bool mMultiplayer : 1;
};
CHECK_SIZEOF(CHudVisorBeamMenu, 0x98)
#endif // _CHUDVISORBEAMMENU
