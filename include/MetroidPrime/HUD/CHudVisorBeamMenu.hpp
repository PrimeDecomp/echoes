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
  void Update(float dt, bool init);
  void UpdateHudAlpha(float alpha);
  void SetIsVisibleGame(bool visible);
  void SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables);
  void SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables, int selection);
  void SetSelection(int selection, int pending, float interpolation);
  void RefreshText(); // Guessed name

private:
  struct SMenuItem {
    SMenuItem()
    : mLozenge(nullptr), mIcon(nullptr), mPosition(0.f), mOpacity(0.f), mEnabled(false) {}
    CGuiWidget* mLozenge;
    CGuiWidget* mIcon;
    float mPosition;
    float mOpacity;
    bool mEnabled : 1;
  };
  enum EAnimationPhase { kAP_None = 0, kAP_Steady = 1, kAP_SelectFlash = 4, kAP_Animate = 5 };
  EVisorBeamMenu GetSwappedType() const {
    return mSwapBeamControls ? static_cast< EVisorBeamMenu >(1 - mType) : mType;
  }
  bool GetIsVisible() const { return mVisibleDebug && mVisibleGame; }
  void UpdateMenuWidgetTransform(int index, CGuiWidget& widget, float factor);
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
  EAnimationPhase mAnimationPhase;
  float x84;
  float x88;
  float mTextFade;
  float mAnimationDuration;
  bool mSwapBeamControls : 1;
  bool mMultiplayer : 1;
};
CHECK_SIZEOF(CHudVisorBeamMenu, 0x98)

#endif // _CHUDVISORBEAMMENU
