#ifndef _CGUNCONTROLLER
#define _CGUNCONTROLLER

#include "MetroidPrime/Weapons/GunController/CGSComboFire.hpp"
#include "MetroidPrime/Weapons/GunController/CGSFidget.hpp"
#include "MetroidPrime/Weapons/GunController/CGSFreeLook.hpp"
#include "types.h"

class CModelData;
class CStateManager;

enum EGunState {
  kGS_Inactive,
  kGS_Default,
  kGS_FreeLook,
  kGS_ComboFire,
  kGS_Idle,
  kGS_Fidget,
  kGS_Strike,
  kGS_BigStrike,
  kGS_Unknown8
};

class CGunController {
public:
  explicit CGunController(CModelData& modelData);
  ~CGunController() {}

  void Reset();
  int Update(float dt, CStateManager& mgr);
  void EnterIdle(CStateManager& mgr);
  void EnterFreeLook(CStateManager& mgr, int gunId, int setId);
  void EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool notInFreeLook);
  void ReturnToDefault(CStateManager& mgr, float delay, bool reset);
  void EnterComboFire(CStateManager& mgr, int gunId);
  void EnterFidget(CStateManager& mgr, int type, int gunId, int animSet);
  void LoadFidgetAnimAsync(CStateManager& mgr, int type, int gunId, int animSet);
  void ReturnToBasePosition(CStateManager& mgr);
  void UnLoadFidget() { mFidget.UnLoadAnim(); }
  bool IsFidgetLoaded() const { return mFidget.IsAnimLoaded(); }
  bool IsComboOver() const { return mComboFire.IsComboOver(); }
  int GetCurAnimId() const { return mCurAnimId; }
  int GetFreeLookSetId() const { return mFreeLook.GetSetId(); }

private:
  CModelData& mModelData;
  CGSFreeLook mFreeLook;
  CGSComboFire mComboFire;
  CGSFidget mFidget;
  uchar mUnresolvedStorage[4]; // Unaccessed here; ownership and purpose remain unknown.
  EGunState mGunState;
  int mCurAnimId;
  bool mAnimDone : 1;
  bool mEnteredComboFire : 1;
};
CHECK_SIZEOF(CGunController, 0x5c)

#endif // _CGUNCONTROLLER
