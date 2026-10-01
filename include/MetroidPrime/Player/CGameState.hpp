#ifndef _CGAMESTATE
#define _CGAMESTATE

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"
#include "MetroidPrime/Player/CPersistentOptions.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CBitStreamReader;
class CBitStreamWriter;
class CGameMode;
class CWorldTransManager;
class CPlayerState;
class CAudioGrpSetLoc;

class CGameState {
public:
  // Guessed name
  struct SPlayerResult {
    SPlayerResult()
    : mPlayerSelection(0), mScore(0), mDeaths(0), xc_(false), mRumbleEnabled(false) {}
    explicit SPlayerResult(CBitStreamReader& in);
    void PutTo(CBitStreamWriter& out) const;

    uint mPlayerSelection;
    int mScore;
    int mDeaths;
    bool xc_; // The second per-player controller option; meaning unresolved.
    bool mRumbleEnabled;
  };

  // Guessed name
  struct SPreviousGameResults {
    SPreviousGameResults()
    : mGameMode(0), mShowResults(false), x8_(0), mPlayerCount(0), mPlayers(4, SPlayerResult()) {}
    explicit SPreviousGameResults(CBitStreamReader& in);
    void PutTo(CBitStreamWriter& out) const;

    uint mGameMode;
    bool mShowResults;
    int x8_; // Result of the game mode's unresolved v14 query.
    int mPlayerCount;
    rstl::reserved_vector< SPlayerResult, 4 > mPlayers;
  };

  struct GameFileStateInfo {
    double mPlayTime;
    CAssetId mMlvlId;
    float mHealth;
    uint mEnergyTanks;
    uint mTimestamp;
    uint mItemPercent;
    float mScanPercent;
    bool mHardMode;
    bool x21_;
  };

  static GameFileStateInfo LoadGameFileState(const void* data);
  static void SerializeNewForCleanSlot(CBitStreamWriter& out, bool hardMode); // Guessed name

  CGameState();
  explicit CGameState(CBitStreamReader& in);
  ~CGameState();

  void ReadSystemOptions(CInputStream& in);
  void PutTo(CBitStreamWriter& out);
  void WriteSystemOptions(COutputStream& out);
  void SetSystemOptions(const CPersistentOptions& options);
  void ExportPersistentOptions(CPersistentOptions& options);
  void WriteBackupBuf();
  void InitializeMemoryStates();
  void SetCurrentWorldId(CAssetId worldId);
  void SetDesiredWorldId(CAssetId worldId);
  void SetTotalPlayTime(double time);
  void SetEscapeTime(float time);
  void SetHardMode(bool hardMode);
  void SetDeferPowerupInit(bool defer);

  // Guessed names for the compressed-buffer copy and reset operations.
  void CopyCompressedGameState(int slot, const void* data);
  void ClearCompressedGameState(int slot);
  void RecordCompressedGameState(int slot, CGameState& state);
  void CopyCompressedGameOptions(int slot, const void* data);
  void RecordCompressedGameOptions(int slot);
  void CopyCompressedMultiplayerOptions(const void* data);
  void RecordCompressedMultiplayerOptions();
  void LoadCompressedGameOptions(int slot);
  void LoadCompressedMultiplayerOptions();
  void SetCompressedGameStates(const rstl::reserved_vector< rstl::vector< uchar >, 3 >& states);
  void SetCompressedGameOptions(const rstl::reserved_vector< rstl::vector< uchar >, 3 >& options);
  void SetCompressedMultiplayerOptions(const rstl::vector< uchar >& options);
  void RecordCheckpoint();
  void ClearCheckpoint();
  const rstl::reserved_vector< rstl::vector< uchar >, 3 >& GetCompressedGameStates() const {
    return mCompressedGameStates;
  }
  const rstl::reserved_vector< rstl::vector< uchar >, 3 >& GetCompressedGameOptions() const {
    return mCompressedGameOptions;
  }
  const rstl::vector< uchar >& GetCompressedMultiplayerOptions() const {
    return mCompressedMultiplayerOptions;
  }
  const rstl::vector< uchar >& GetCheckpointGameState() const { return mCheckpointGameState; }
  void ClearAudioGroups() { mAudioGroups.clear(); }

  void SetIsDarkWorld(bool);
  CGameMode& GetGameMode();
  const CGameMode& GetGameMode() const;
  void SetGameMode(CGameMode* mode);                                           // name inferred
  SPreviousGameResults& PreviousGameResults() { return mPreviousGameResults; } // Guessed name
  int GetGameModeType() const { return mPreviousGameResults.mGameMode; }       // name inferred
  CWorldState& StateForWorld(CAssetId worldId);
  CWorldState& CurrentWorldState();
  rstl::rc_ptr< CWorldTransManager >& WorldTransitionManager();
  CAssetId CurrentWorldAssetId() const;

  CGameOptions& GameOptions() { return mGameOptions; }
  CGameStateEnvVarManager& PersistentOptions() { return mPersistentOptions; }

  CHintOptions& HintOptions() { return mHintOptions; }

  CControlMapper& ControlMapper() { return mControlMapper; }

  CPersistentOptions& SystemOptions() { return mSystemOptions; }

  u32 GetCardSerialA() const { return mCardSerial >> 32; }
  u32 GetCardSerialB() const { return mCardSerial; }
  u64 GetCardSerial() const { return mCardSerial; }
  void SetCardSerial(u64 serial) { mCardSerial = serial; }
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;
  bool GetHardModeEnabled() const { return mHardMode; }
  double GetTotalPlayTime() const { return mTotalPlayTime; }
  rstl::rc_ptr< CPlayerState > GetPlayerState() const;
  rstl::rc_ptr< CPlayerState > GetPlayerState(int player) const;
  rstl::rc_ptr< CPlayerState >& PlayerState(int player);

private:
  void InitializeMemoryWorlds();

  CAssetId mWorldId;
  CAssetId mDesiredWorldId;
  rstl::vector< CWorldState > mWorldStates;
  rstl::reserved_vector< rstl::rc_ptr< CPlayerState >, 4 > mPlayerStates;
  rstl::rc_ptr< CWorldTransManager > mTransManager;
  double mTotalPlayTime;
  float mEscapeTime;
  CPersistentOptions mSystemOptions;
  CGameOptions mGameOptions;
  CHintOptions mHintOptions;
  CGameStateEnvVarManager mPersistentOptions;
  // Guessed element type: shares the system cinematic vector's native destructor.
  // Its separate purpose in CGameState remains unresolved.
  rstl::vector< rstl::pair< CAssetId, TEditorId > > xf4_;
  u64 mCardSerial;

  rstl::reserved_vector< rstl::vector< uchar >, 3 > mCompressedGameStates;
  rstl::reserved_vector< rstl::vector< uchar >, 3 > mCompressedGameOptions;
  rstl::vector< uchar > mCompressedMultiplayerOptions;
  rstl::vector< uchar > mCheckpointGameState;
  rstl::auto_ptr< CGameMode > mGameMode;
  SPreviousGameResults mPreviousGameResults;
  rstl::vector< TCachedToken< CAudioGrpSetLoc > > mAudioGroups;
  CControlMapper mControlMapper;
  bool mHardMode : 1;
  bool mInitPowerupsAtFirstSpawn : 1;
  bool mIsDarkWorld : 1;
  uchar x2ed_[3];
};

CHECK_SIZEOF(CGameState, 0x2f0)
NESTED_CHECK_SIZEOF(CGameState, GameFileStateInfo, 0x28)
NESTED_CHECK_SIZEOF(CGameState, SPlayerResult, 0x10)
NESTED_CHECK_SIZEOF(CGameState, SPreviousGameResults, 0x54)

extern CGameState* gpGameState;

// Unidentified game-flow helpers in the CGameState text range.
void StartGameFromFrontEnd();   // Guessed name
void ConfigureGameModeLayers(); // Guessed name
void fn_80143E88();

#endif // _CGAMESTATE
