#ifndef _CGUNCONTROLLER
#define _CGUNCONTROLLER

#include "MetroidPrime/Weapons/GunController/CGSComboFire.hpp"
#include "MetroidPrime/Weapons/GunController/CGSFidget.hpp"
#include "MetroidPrime/Weapons/GunController/CGSFreeLook.hpp"
#include "types.h"

class CModelData;
class CStateManager;

class CGunController {
public:
  explicit CGunController(CModelData& modelData);
  ~CGunController() {}

  void Reset();
  void ReturnToDefault(CStateManager& mgr, float delay, bool reset);
  void EnterComboFire(CStateManager& mgr, int gunId);
  void EnterFidget(CStateManager& mgr, int type, int gunId, int animSet);
  void LoadFidgetAnimAsync(CStateManager& mgr, int type, int gunId, int animSet);
  void UnLoadFidget() { mFidget.UnLoadAnim(); }
  bool IsFidgetLoaded() const { return mFidget.IsAnimLoaded(); }

private:
  CModelData& mModelData;
  CGSFreeLook mFreeLook;
  CGSComboFire mComboFire;
  CGSFidget mFidget;
  uchar x4c_[4]; // Unresolved storage between the fidget state and controller state.
  int mGunState;
  int mCurAnimId;
  bool mAnimDone : 1;
  bool mEnteredComboFire : 1;
};
CHECK_SIZEOF(CGunController, 0x5c)

#endif // _CGUNCONTROLLER
