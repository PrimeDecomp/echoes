#ifndef _CGSCOMBOFIRE
#define _CGSCOMBOFIRE

#include "types.h"

class CAnimData;
class CStateManager;

class CGSComboFire {
public:
  CGSComboFire();
  bool IsComboOver() const { return mOver; }
  bool Update(CAnimData& data, float dt, CStateManager& mgr);
  int SetAnim(CAnimData& data, int gunId, int loopState, CStateManager& mgr, float delay);
  int GetGunId() const { return mGunId; }
  int GetLoopState() const { return mLoopState; }
  void SetLoopState(int state) { mLoopState = state; }
  void SetIdle(bool idle) { mIdle = idle; }

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
