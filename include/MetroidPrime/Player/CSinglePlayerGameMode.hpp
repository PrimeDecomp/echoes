#ifndef _CSINGLEPLAYERGAMEMODE
#define _CSINGLEPLAYERGAMEMODE

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"

// Name inferred: its game mode type is 'SNGL'.
class CSinglePlayerGameMode : public CGameMode {
public:
  CSinglePlayerGameMode();

private:
  bool x4_;
  bool x5_;
  int x8_;
};
CHECK_SIZEOF(CSinglePlayerGameMode, 0xc)

#endif // _CSINGLEPLAYERGAMEMODE
