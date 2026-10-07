#ifndef _CSCRIPTGUIFRONTENDSCREEN
#define _CSCRIPTGUIFRONTENDSCREEN

#include "MetroidPrime/ScriptObjects/CScriptGuiScreen.hpp"

#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CScriptGuiMenu;
class CScriptGuiSlider;
class CScriptRelay;
class CScriptSound;
class CScriptSwitch;
class CScriptTextPane;
class CScriptWorldTeleporter;

// Guessed name. The ScriptGui REL's front-end screen: save slots, options and multiplayer setup.
class CScriptGuiFrontEndScreen : public CScriptGuiScreen {
public:
  // Guessed names.
  struct SPlayerSetup {
    CScriptGuiMenu* mRumbleMenu;
    CScriptGuiMenu* mInvertMenu;
    CScriptSwitch* mJoinedSwitch;
    CScriptSwitch* mReadySwitch;
  };

  struct SSaveSlot {
    SSaveSlot()
    : mTitle(nullptr)
    , mWorldName(nullptr)
    , mPlayTime(nullptr)
    , mSlotEntity(nullptr)
    , mUsedSwitch(nullptr)
    , mNewGameSwitch(nullptr)
    , mDifficultyMenu(nullptr) {}
    SSaveSlot(const SSaveSlot& other)
    : mTitle(other.mTitle)
    , mWorldName(other.mWorldName)
    , mPlayTime(other.mPlayTime)
    , mSlotEntity(other.mSlotEntity)
    , mUsedSwitch(other.mUsedSwitch)
    , mNewGameSwitch(other.mNewGameSwitch)
    , mDifficultyMenu(other.mDifficultyMenu) {}

    CScriptTextPane* mTitle;
    CScriptTextPane* mWorldName;
    CScriptTextPane* mPlayTime;
    CEntity* mSlotEntity;
    CScriptSwitch* mUsedSwitch;
    CScriptSwitch* mNewGameSwitch;
    CScriptGuiMenu* mDifficultyMenu;
  };

  enum EOption {
    kO_Brightness,
    kO_Stretch,
    kO_PositionX,
    kO_PositionY,
    kO_HudAlpha,
    kO_HelmetAlpha,
    kO_HintSystem,
    kO_HudLag,
    kO_InvertY,
    kO_Rumble,
    kO_SfxVolume,
    kO_MusicVolume,
    kO_SurroundMode,
    kO_Count,
  };

  CScriptGuiFrontEndScreen(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           CAssetId stringTable);
  ~CScriptGuiFrontEndScreen() override;

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  virtual void CollectWidgets(CStateManager& mgr);

  // Guessed names.
  void ShowSlideShow(CStateManager& mgr);
  void SetPercentText(CScriptTextPane* pane, float value);
  void UpdateSoundVolumes();
  void HighlightSelectedSlot(CStateManager& mgr);
  void SaveOptions(CStateManager& mgr);
  void RecordOptions(CStateManager& mgr);
  void LoadOptions(CStateManager& mgr, CEntity* page);
  void EraseSelectedGame(CStateManager& mgr);
  void CopySelectedGame(CStateManager& mgr);
  void ResetOptionPage(CStateManager& mgr, CEntity* widget);
  void ApplyOptionWidget(CStateManager& mgr, CEntity* widget);
  void ResetOptionWidget(CStateManager& mgr, CEntity* widget);
  void RestoreOptionWidget(CStateManager& mgr, CEntity* widget);
  void CompareOptionWidget(CStateManager& mgr, CEntity* widget);
  void StoreOptionWidget(CStateManager& mgr, CEntity* widget);
  void CloseSaveGameScreen(CStateManager& mgr);
  void OpenSaveGameScreen(CStateManager& mgr);
  void UpdateUnlocks(CStateManager& mgr);
  void UpdateSaveSlots(CStateManager& mgr);
  void RefreshSaveSlots(CStateManager& mgr);
  void StartSelectedGame(CStateManager& mgr);
  void SelectSaveSlot(CStateManager& mgr, TUniqueId slotId);
  void StartMultiplayerGame(CStateManager& mgr);
  void UpdateControllerCount(CStateManager& mgr);

private:
  TLockedToken< CStringTable > mStringTable;
  rstl::reserved_vector< SPlayerSetup, 4 > mPlayerSetups;
  CScriptWorldTeleporter* mDefaultTeleporter;
  rstl::vector< CScriptWorldTeleporter* > mTeleporters;
  CScriptSwitch* mDeathMatchSwitch;
  CEntity* mDeathMatchEntity;
  CScriptSwitch* mCoinSwitch;
  CEntity* mCoinEntity;
  rstl::reserved_vector< SSaveSlot, 3 > mSaveSlots;
  CScriptGuiSlider* mBrightnessSlider;
  CScriptGuiSlider* mStretchSlider;
  CScriptGuiSlider* mPositionXSlider;
  CScriptGuiSlider* mPositionYSlider;
  CScriptGuiSlider* mHudAlphaSlider;
  CScriptGuiSlider* mHelmetAlphaSlider;
  CScriptGuiMenu* mHintSystemMenu;
  CScriptGuiMenu* mHudLagMenu;
  CScriptGuiMenu* mInvertYMenu;
  CScriptGuiMenu* mRumbleMenu;
  CScriptGuiSlider* mSfxVolumeSlider;
  CScriptGuiSlider* mMusicVolumeSlider;
  CScriptGuiMenu* mSurroundMenu;
  CScriptGuiMenu* mFragLimitMenu;
  CScriptGuiMenu* mDeathMatchTimeMenu;
  CScriptGuiMenu* mCoinTimeMenu;
  CScriptGuiMenu* mCoinLimitMenu;
  CScriptGuiMenu* mDeathMatchMusicMenu;
  CScriptGuiMenu* mCoinMusicMenu;
  CScriptSwitch* mCopySwitch;
  CScriptSwitch* mEraseSwitch;
  CScriptSwitch* mLoadSwitch;
  CScriptSwitch* mStartSwitch;
  rstl::reserved_vector< CScriptRelay*, 2 > mSlotCountRelays;
  rstl::reserved_vector< CEntity*, 5 > mOptionsPages;
  rstl::reserved_vector< CEntity*, 4 > mResetPages;
  rstl::reserved_vector< CScriptTextPane*, 5 > mSlotNamePanes;
  rstl::vector< CScriptSwitch* > mUnlockSwitches;
  rstl::vector< CScriptSound* > mSounds;
  rstl::vector< int > mSoundVolumes;
  rstl::reserved_vector< CScriptTextPane*, 8 > mValuePanes;
  CEntity* mGalleryEntity;
  int mSelectedSlot;
  int x32c_;
  int mOptionsPage;
  int mSavedBrightness;
  int mSavedStretch;
  int mSavedPositionX;
  int mSavedPositionY;
  int mSavedHudAlpha;
  int mSavedHelmetAlpha;
  int mSavedHintSystem;
  int mSavedHudLag;
  int mSavedInvertY;
  int mSavedRumble;
  int mSavedSfxVolume;
  int mSavedMusicVolume;
  int mSavedSurroundMode;
  bool mCardDriverReset : 1;
  bool mSaveScreenBusy : 1;
  bool mSaveScreenFailed : 1;
  bool mGameStarted : 1;
  bool mOptionsDirty : 1;
  bool mMultipleControllers : 1;
};
CHECK_SIZEOF(CScriptGuiFrontEndScreen, 0x370)

#endif // _CSCRIPTGUIFRONTENDSCREEN
