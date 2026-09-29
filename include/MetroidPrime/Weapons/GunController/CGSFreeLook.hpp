#ifndef _CGSFREELOOK
#define _CGSFREELOOK

#include "types.h"

class CAnimData;
class CStateManager;

class CGSFreeLook {
public:
  CGSFreeLook();
  bool Update(CAnimData& data, float dt, CStateManager& mgr);
  int SetAnim(CAnimData& data, int gunId, int setId, int loopState, CStateManager& mgr,
              float delay);

  int GetGunId() const { return mGunId; }
  int GetSetId() const { return mSetId; }
  int GetLoopState() const { return mLoopState; }
  void SetLoopState(int state) { mLoopState = state; }
  void SetIdle(bool idle) { mIdle = idle; }

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
