#ifndef _CGAMESTATE
#define _CGAMESTATE

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

class CGameState {
public:
  CGameState();
  CGameState(CInputStream& in, int saveIdx);

  void ReadSystemOptions(CInputStream& in);
  void PutTo(COutputStream& out) const;
  void WriteSystemOptions(COutputStream& out);

private:
  char pad[0x2f0];
};

CHECK_SIZEOF(CGameState, 0x2f0)

extern CGameState* gpGameState;

#endif // _CGAMESTATE
