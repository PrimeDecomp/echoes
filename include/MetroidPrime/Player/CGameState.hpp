#ifndef _CGAMESTATE
#define _CGAMESTATE

#include "types.h"

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"
#include "MetroidPrime/Player/CPersistentOptions.hpp"

class CGameMode;
class CWorldState;

class CGameState {
public:
  CGameState();
  CGameState(CInputStream& in, int saveIdx);
  ~CGameState();

  void ReadSystemOptions(CInputStream& in);
  void PutTo(COutputStream& out) const;
  void WriteSystemOptions(COutputStream& out);

  void SetIsDarkWorld(bool);
  CGameMode& GetGameMode();
  CWorldState& StateForWorld(CAssetId worldId);
  CAssetId CurrentWorldAssetId() const;

  CGameOptions& GameOptions() { return gameOptions; }

  CHintOptions& HintOptions() { return hintOptions; }

  CControlMapper& ControlMapper() { return mControlMapper; }

  CPersistentOptions& SystemOptions() { return mSystemOptions; }

  CPersistentOptions& PersistentOptions() { return persistentOptions; }

  u32 GetCardSerialA() const { return cardSerialA; }
  u32 GetCardSerialB() const { return cardSerialB; }
  float GetHardModeDamageMultiplier() const;
  bool GetHardModeEnabled() const;

private:
  char pad1[0x54];
  CPersistentOptions mSystemOptions;
  CGameOptions gameOptions;
  CHintOptions hintOptions;
  CPersistentOptions persistentOptions;
  u32 cardSerialA;
  u32 cardSerialB;

  char x110_[0xf4];
  CControlMapper mControlMapper;
  char x2ec_[4];
};

CHECK_SIZEOF(CGameState, 0x2f0)

extern CGameState* gpGameState;

#endif // _CGAMESTATE
