#ifndef _CGMMULTIPLAYER
#define _CGMMULTIPLAYER

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed name. Shared layout for the deathmatch and coin game modes.
class CGMMultiplayer : public CGameMode {
public:
  // CGameMode
  ~CGMMultiplayer() override;
  // TODO: recover the remaining virtual signatures before emitting this class's vtable.

  void SetMusicIndex(int musicIndex); // Guessed name

private:
  float mTimeLimit;
  float x8_;
  int mMusicIndex;
  uchar x10_[0x14]; // Tree of game-event listeners; key/value types still unresolved.
  rstl::reserved_vector< TUniqueId, 4 > x24_;
  int x30_;
  bool x34_24_ : 1;
  bool x34_25_ : 1;
};

CHECK_SIZEOF(CGMMultiplayer, 0x38)

#endif // _CGMMULTIPLAYER
