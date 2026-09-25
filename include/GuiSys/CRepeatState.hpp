#ifndef _CREPEATSTATE
#define _CREPEATSTATE

#include "types.h"

// Guessed scope. Prime nests this helper in CGuiTableGroup; Echoes also uses it in CPauseScreen.
class CRepeatState {
public:
  CRepeatState();
  bool Update(float dt, bool pressed);

private:
  float mTimer;
};
CHECK_SIZEOF(CRepeatState, 0x4)

#endif // _CREPEATSTATE
