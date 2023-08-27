#ifndef _CGAMESTATE
#define _CGAMESTATE

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"
#include "MetroidPrime/Player/CPersistentOptions.hpp"

class CGameMode;

class CGameState {
public:
  CGameState();
  CGameState(CInputStream& in, int saveIdx);

  void ReadSystemOptions(CInputStream& in);
  void PutTo(COutputStream& out) const;
  void WriteSystemOptions(COutputStream& out);

  void SetIsDarkWorld(bool);
  CGameMode& GetGameMode();

  CGameOptions& GameOptions() { return gameOptions; }

  u32 GetCardSerialA() const { return cardSerialA; }
  u32 GetCardSerialB() const { return cardSerialB; }

private:
  char pad1[0x80];
  CGameOptions gameOptions;
  CHintOptions hintOptions;
  CPersistentOptions persistentOptions;
  u32 cardSerialA;
  u32 cardSerialB;

  char pad2[0x1E0];
};

CHECK_SIZEOF(CGameState, 0x2f0)

extern CGameState* gpGameState;

#endif // _CGAMESTATE
