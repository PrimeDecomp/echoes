#ifndef _CGSFIDGET
#define _CGSFIDGET

#include "Kyoto/CToken.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CAnimData;
class CStateManager;

class CGSFidget {
public:
  CGSFidget();
  bool Update(CAnimData& data, float dt, CStateManager& mgr);
  int SetAnim(CAnimData& data, int type, int gunId, int animSet, CStateManager& mgr);
  void LoadAnimAsync(CAnimData& data, int type, int gunId, int animSet, CStateManager& mgr);
  void UnLoadAnim();
  bool IsAnimLoaded() const;

private:
  rstl::vector< CToken > mAnims;
  int mUnknown;
  int mGunId;
  int mAnimSet;
};
CHECK_SIZEOF(CGSFidget, 0x1c)

#endif // _CGSFIDGET
