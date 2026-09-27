#ifndef _CGSFREELOOK
#define _CGSFREELOOK

#include "types.h"

class CGSFreeLook {
public:
  CGSFreeLook();

private:
  float mDelay;
  int mCueAnimId;
  int mLoopState;
  int mGunId;
  int mSetId;
  bool mIdle : 1;
};
CHECK_SIZEOF(CGSFreeLook, 0x18)

#endif // _CGSFREELOOK
