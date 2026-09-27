#ifndef _CGRAPHICSSYS
#define _CGRAPHICSSYS

#include "types.h"

class COsContext;
class CMemorySys;

class CGraphicsSys {
public:
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, bool progressive);
  ~CGraphicsSys();

private:
  uint x0_;

  static bool mGraphicsInitialized;
};

#endif // _CGRAPHICSSYS
