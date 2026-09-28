#ifndef _CGMFRONTEND
#define _CGMFRONTEND

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed name. Partial interface used when starting a game from the front end.
class CGMFrontEnd : public CGameMode {
public:
  // Guessed names
  enum ESelectedGameMode {
    kSGM_SinglePlayer = 'SNGL',
    kSGM_DeathMatch = 'DTHM',
    kSGM_Coin = 'COIN',
    kSGM_FrontEnd = 'FRND'
  };

  // Guessed name
  struct SPlayerConfig {
    uint mPlayerSelection;
    bool mRumbleEnabled;
    bool x5_; // Second controller option; meaning unresolved.
  };

  CGMFrontEnd();
  CGMFrontEnd(const CGMFrontEnd& other);

  // CGameMode
  ~CGMFrontEnd() override;
  // TODO: recover the remaining virtual signatures before emitting this class's vtable.

  int GetPlayerCount() const;
  const SPlayerConfig& GetPlayer(int player) const;
  ESelectedGameMode GetSelectedGameMode() const { return mSelectedGameMode; }
  int GetFragLimit() const { return mFragLimit; }
  int GetCoinLimit() const { return mCoinLimit; }
  float GetTimeLimit() const { return mTimeLimit; }
  int GetMusicIndex() const { return mMusicIndex; }

private:
  bool x4_;
  int x8_;
  ESelectedGameMode mSelectedGameMode;
  int mFragLimit;
  int mCoinLimit;
  float mTimeLimit;
  int mMusicIndex;
  rstl::reserved_vector< SPlayerConfig, 4 > mPlayers;
};

CHECK_SIZEOF(CGMFrontEnd, 0x44)
NESTED_CHECK_SIZEOF(CGMFrontEnd, SPlayerConfig, 8)

#endif // _CGMFRONTEND
