#ifndef _CGSCOMBOFIRE
#define _CGSCOMBOFIRE

#include "types.h"

class CGSComboFire {
public:
  CGSComboFire();
  bool IsComboOver() const { return mOver; }

private:
  float mDelay;
  int mLoopState;
  int mCueAnimId;
  int mGunId;
  bool mOver : 1;
  bool mIdle : 1;
};
CHECK_SIZEOF(CGSComboFire, 0x14)

#endif // _CGSCOMBOFIRE
