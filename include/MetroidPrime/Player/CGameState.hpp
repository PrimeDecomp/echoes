#ifndef _CGAMESTATE
#define _CGAMESTATE

#include "types.h"

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"
#include "MetroidPrime/Player/CPersistentOptions.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"

class CGameMode;
class CWorldState;
class CWorldTransManager;
class CPlayerState;

class CGameState {
public:
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

  CGameState();
  CGameState(CInputStream& in, int saveIdx);
  ~CGameState();

  void ReadSystemOptions(CInputStream& in);
  void PutTo(COutputStream& out) const;
  void WriteSystemOptions(COutputStream& out);

  void SetIsDarkWorld(bool);
  CGameMode& GetGameMode();
  void SetGameMode(CGameMode* mode); // name inferred
  int GetGameModeType() const { return mGameModeType; } // name inferred
  CWorldState& StateForWorld(CAssetId worldId);
  rstl::rc_ptr< CWorldTransManager >& WorldTransitionManager();
  CAssetId CurrentWorldAssetId() const;

  CGameOptions& GameOptions() { return gameOptions; }
  CPersistentOptions& PersistentOptions() { return persistentOptions; }

  CHintOptions& HintOptions() { return hintOptions; }

  CControlMapper& ControlMapper() { return mControlMapper; }

  CPersistentOptions& SystemOptions() { return mSystemOptions; }

  u32 GetCardSerialA() const { return cardSerialA; }
  u32 GetCardSerialB() const { return cardSerialB; }
  u64 GetCardSerial() const { return (u64(cardSerialA) << 32) | cardSerialB; }
  void SetCardSerial(u64 serial) {
    cardSerialA = serial >> 32;
    cardSerialB = serial;
  }
  float GetHardModeDamageMultiplier() const;
  bool GetHardModeEnabled() const { return mHardMode; }
  double GetTotalPlayTime() const { return mTotalPlayTime; }
  rstl::rc_ptr< CPlayerState > GetPlayerState() const;

private:
  char pad1[0x48];
  double mTotalPlayTime;
  float mEscapeTime;
  CPersistentOptions mSystemOptions;
  CGameOptions gameOptions;
  CHintOptions hintOptions;
  CPersistentOptions persistentOptions;
  u32 cardSerialA;
  u32 cardSerialB;

  char x110_[0x88];
  rstl::auto_ptr< CGameMode > mGameMode;
  int mGameModeType;
  char x1a4_[0x60];
  CControlMapper mControlMapper;
  bool mHardMode : 1;
  uchar x2ed_[3];
};

CHECK_SIZEOF(CGameState, 0x2f0)
NESTED_CHECK_SIZEOF(CGameState, GameFileStateInfo, 0x28)

extern CGameState* gpGameState;

// Unidentified game-flow helpers in the CGameState text range.
void fn_80143884();
void fn_80143E88();

#endif // _CGAMESTATE
