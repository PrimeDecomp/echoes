#ifndef _CAUTOSAVE
#define _CAUTOSAVE

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"

class CAutoSave : public CIOWin {
public:
  CAutoSave();

  ~CAutoSave() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;

private:
  char x14_[0x50];
};
CHECK_SIZEOF(CAutoSave, 0x64)

#endif // _CAUTOSAVE
