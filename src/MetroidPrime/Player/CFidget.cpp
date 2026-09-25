#include "MetroidPrime/Player/CFidget.hpp"

CFidget::CFidget()
: mTimeSinceFire(0.f)
, mTimeSinceStrikeCooldown(0.f)
, mTimeSinceUnmorph(0.f)
, mTimeSinceBobbing(0.f)
, mFidgetDelayTimer(0.f)
, mHolsterTimeSinceFire(0.f)
, mTimeUntilHolster(105.f)
, mTimeUntilFidget(25.f)
, mState(kS_NoFidget)
, mType(SamusGun::kFT_Invalid)
, mAnimSet(-1)
, mLoading(false) {}

void CFidget::ResetAll() {
  mState = kS_NoFidget;
  mType = SamusGun::kFT_Invalid;
  mTimeSinceStrikeCooldown = 0.f;
  mTimeSinceUnmorph = 0.f;
  mTimeSinceFire = 0.f;
  mFidgetDelayTimer = 0.f;
  mHolsterTimeSinceFire = 0.f;
  mAnimSet = -1;
  mLoading = false;
}

void CFidget::ResetState() { mState = kS_NoFidget; }

void CFidget::Update(int fireButtonStates, bool bobbing, bool inStrikeCooldown, float dt,
                     CStateManager& mgr, const CPlayer& player) {
  // TODO: Advance inactivity timers, choose a randomized fidget, or request gun holstering.
}
