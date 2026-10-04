#include "MetroidPrime/Player/CFidget.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

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
  if (mState != kS_NoFidget) {
    return;
  }

  if (fireButtonStates != 0) {
    mTimeSinceFire = 0.f;
    mHolsterTimeSinceFire = 0.f;
  } else {
    if (mTimeSinceFire < 6.f) {
      mTimeSinceFire += dt;
    }
    if (mHolsterTimeSinceFire < mTimeUntilHolster + 1.f) {
      mHolsterTimeSinceFire += dt;
    }
  }

  if (inStrikeCooldown) {
    mTimeSinceStrikeCooldown = 0.f;
  } else if (mTimeSinceStrikeCooldown < 11.f) {
    mTimeSinceStrikeCooldown += dt;
  }

  if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    if (mTimeSinceUnmorph < 21.f) {
      mTimeSinceUnmorph += dt;
    }
  } else {
    mTimeSinceUnmorph = 0.f;
  }

  if (bobbing) {
    mTimeSinceBobbing = 0.f;
  } else if (mTimeSinceBobbing < 21.f) {
    mTimeSinceBobbing += dt;
  }

  mFidgetDelayTimer += dt;
  if (mFidgetDelayTimer > mTimeUntilFidget) {
    mState = mgr.Random()->Next() % 100 > 50 ? kS_MajorFidget : kS_MinorFidget;
    mFidgetDelayTimer = 0.f;
  }
  if (mHolsterTimeSinceFire > mTimeUntilHolster) {
    mState = kS_HolsterBeam;
  }

  switch (mState) {
  case kS_MajorFidget:
    mTimeUntilFidget = mgr.Random()->Range(20.f, 30.f);
    mType = SamusGun::kFT_Major;
    mAnimSet = mgr.Random()->Range(0, 5);
    break;
  case kS_MinorFidget:
    mTimeUntilFidget = mgr.Random()->Range(20.f, 30.f);
    mType = SamusGun::kFT_Minor;
    mAnimSet = mgr.Random()->Range(0, 4);
    break;
  case kS_HolsterBeam:
    mType = SamusGun::kFT_Minor;
    mAnimSet = 0;
    break;
  default:
    break;
  }
}
