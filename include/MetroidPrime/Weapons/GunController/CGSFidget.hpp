#ifndef _CGSFIDGET
#define _CGSFIDGET

#include "Kyoto/CToken.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CGSFidget {
public:
  CGSFidget();
  void UnLoadAnim();
  bool IsAnimLoaded() const;

private:
  rstl::vector< CToken > mAnims;
  int x10_;
  int mGunId;
  int mAnimSet;
};
CHECK_SIZEOF(CGSFidget, 0x1c)

#endif // _CGSFIDGET
