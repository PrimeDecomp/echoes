#ifndef _CMFGAMELOADER
#define _CMFGAMELOADER

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"

class CMFGameLoader : public CIOWin {
public:
  CMFGameLoader();

  ~CMFGameLoader() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;

private:
  char x14_[0x18];
};
CHECK_SIZEOF(CMFGameLoader, 0x2c)

#endif // _CMFGAMELOADER
