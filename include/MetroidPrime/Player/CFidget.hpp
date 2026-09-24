#ifndef _CFIDGET
#define _CFIDGET

#include "MetroidPrime/Weapons/GunController/CGunMotion.hpp"

class CPlayer;

class CFidget {
public:
  enum EState { kS_NoFidget, kS_MinorFidget, kS_MajorFidget, kS_HolsterBeam };

  CFidget();
  void Update(int fireButtonStates, bool bobbing, bool inStrikeCooldown, float dt,
              CStateManager& mgr, const CPlayer& player);
  void ResetAll();
  void ResetState(); // Guessed name
  EState GetState() const { return mState; }
  SamusGun::EFidgetType GetType() const { return mType; }
  int GetAnimSet() const { return mAnimSet; }
  void DoneLoading() { mLoading = false; }

private:
  float mTimeSinceFire;
  float mTimeSinceStrikeCooldown;
  float mTimeSinceUnmorph;
  float mTimeSinceBobbing;
  float mFidgetDelayTimer;
  float mHolsterTimeSinceFire;
  float mTimeUntilHolster;
  float mTimeUntilFidget;
  EState mState;
  SamusGun::EFidgetType mType;
  int mAnimSet;
  bool mLoading : 1;
};
CHECK_SIZEOF(CFidget, 0x30)

#endif // _CFIDGET
