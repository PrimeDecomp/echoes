#ifndef _CCREDITS
#define _CCREDITS

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"

class CCredits : public CIOWin {
public:
  CCredits();

  ~CCredits() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;

private:
  char x14_[0x54];
};
CHECK_SIZEOF(CCredits, 0x68)

#endif // _CCREDITS
