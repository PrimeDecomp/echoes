#ifndef _CSAVEGAMESCREEN
#define _CSAVEGAMESCREEN

#include "types.h"

class CSaveGameScreen {
public:
  CSaveGameScreen(int, uint, uint);
  ~CSaveGameScreen();

private:
  char pad[0x98];
};
CHECK_SIZEOF(CSaveGameScreen, 0x98)

#endif // _CSAVEGAMESCREEN
