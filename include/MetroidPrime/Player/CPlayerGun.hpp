#ifndef _CPLAYERGUN
#define _CPLAYERGUN

#include "MetroidPrime//TGameTypes.hpp"

class CPlayerGun {
public:
  CPlayerGun(TUniqueId, int);
  ~CPlayerGun();

private:
  char m_pad[0x814];
};
CHECK_SIZEOF(CPlayerGun, 0x814)

#endif // _CPLAYERGUN
