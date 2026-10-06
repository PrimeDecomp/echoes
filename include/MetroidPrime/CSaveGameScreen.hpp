#ifndef _CSAVEGAMESCREEN
#define _CSAVEGAMESCREEN

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CFinalInput;
class CTexture;
class CStringTable;
class CGuiFrame;
class CGuiTextPane;
class CGuiTableGroup;
class CMemoryCardDriver;
class CWorldSaveGameInfo;

enum ESaveContext { kSC_FrontEnd, kSC_InGame };

class CSaveGameScreen {
public:
  enum EUIType {
    kUIT_Empty = 0,
    kUIT_BusyReading,
    kUIT_BusyWriting,
    kUIT_BusyWritingInitial, // Guessed name
    kUIT_NoCardFound,
    kUIT_NeedsFormatBroken,
    kUIT_NeedsFormatEncoding,
    kUIT_CardDamaged,
    kUIT_WrongDevice,
    kUIT_InsufficientSpaceOKCheck,
    kUIT_IncompatibleCard,
    kUIT_SaveCorrupt,
    kUIT_ProgressWillBeLost,
    kUIT_NotOriginalCard,
    kUIT_AllDataWillBeLost,
    kUIT_SaveReady
  };

  CSaveGameScreen(ESaveContext saveContext, u64 cardSerial);
  ~CSaveGameScreen();

  CIOWin::EMessageReturn Update(float dt);
  bool PumpLoad();

  // Guessed name; the stored completion result is read by CStateManager.
  CIOWin::EMessageReturn GetMessageReturn() const { return mIowRet; }

  EUIType GetUIType() const { return mUiType; } // Guessed name
  void ProcessUserInput(const CFinalInput& input);
  void Draw() const;
  const CGameState::GameFileStateInfo* GetGameData(int idx) const;
  int GetSaveIdx() const; // Guessed name
  void EraseGame(int idx);
  void CopyGame(int from, int to); // Guessed name
  void SaveChanges();              // Guessed name
  void StartGame(int idx);
  void ResetCardDriver();
  static CMemoryCardDriver* ConstructCardDriver(bool importPersistent);
  EUIType SelectUIType() const;
  void SetUIText();
  void SetUIColors();
  void DoAdvance(CGuiTableGroup* caller);
  void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);

private:
  void ContinueWithoutSaving();

  ESaveContext mSaveCtx;
  u64 mSerial;
  EUIType mUiType;
  TCachedToken< CTexture > mTxtrSaveBanner;
  TCachedToken< CTexture > mTxtrSaveIcon0;
  TCachedToken< CTexture > mTxtrSaveIcon1;
  TCachedToken< CStringTable > mStrgMemoryCard;
  TCachedToken< CGuiFrame > mFrmeGenericMenu;
  CGuiFrame* mLoadedFrame;
  CGuiTextPane* mTextpaneMessage;
  CGuiTableGroup* mTablegroupChoices;
  CGuiTextPane* mTextpaneChoice0;
  CGuiTextPane* mTextpaneChoice1;
  CGuiTextPane* mTextpaneChoice2;
  CGuiTextPane* mTextpaneChoice3;
  rstl::single_ptr< CMemoryCardDriver > mCardDriver;
  rstl::vector< TToken< CWorldSaveGameInfo > > mSaveWorlds;
  CIOWin::EMessageReturn mIowRet;
  uint mNavConfirmSfx;
  uint mNavMoveSfx;
  uint mNavBackSfx;
  bool mNeedsDriverReset : 1;
  bool mUiTextDirty : 1;
  bool mSavingDisabled : 1;
  bool mInGame : 1;
  bool mFrontEndSfx : 1; // Guessed name
  bool mHasMessage : 1;  // Guessed name
};
CHECK_SIZEOF(CSaveGameScreen, 0x98)

#endif // _CSAVEGAMESCREEN
