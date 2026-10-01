#ifndef _CPREFRONTEND
#define _CPREFRONTEND

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"

class CPreFrontEnd : public CIOWin {
public:
  CPreFrontEnd();

  ~CPreFrontEnd() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() const override;
};
CHECK_SIZEOF(CPreFrontEnd, 0x14)

#endif // _CPREFRONTEND
