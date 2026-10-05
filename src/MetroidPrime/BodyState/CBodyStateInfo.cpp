#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBSLocomotion.hpp"
#include "MetroidPrime/BodyState/CBSTurn.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"

CBodyStateInfo::CBodyStateInfo(CActor& actor, EBodyType type)
: mStates(29, static_cast< CBodyState* >(nullptr))
, mState(pas::kAS_Invalid)
, mAdditiveState(pas::kAS_AdditiveIdle)
, mLieOnGround(actor)
, mBodyController(nullptr)
, mMaxPitch(0.f)
, mChangeLocoAtEndOfAnimOnly(false) {
  SetupBodyStates(actor, type);
}

CBodyStateInfo::~CBodyStateInfo() {}

void CBodyStateInfo::SetState(pas::EAnimationState state) { mState = state; }

const CBodyState* CBodyStateInfo::GetCurrentState() const { return mStates[mState]; }

CBodyState* CBodyStateInfo::GetCurrentState() { return mStates[mState]; }

bool CBodyStateInfo::ApplyHeadTracking() const {
  if (mState != pas::kAS_Invalid) {
    return GetCurrentState()->ApplyHeadTracking();
  }
  return false;
}

void CBodyStateInfo::SetAdditiveState(pas::EAnimationState state) { mAdditiveState = state; }

CAdditiveBodyState* CBodyStateInfo::GetCurrentAdditiveState() {
  return static_cast< CAdditiveBodyState* >(mStates[mAdditiveState]);
}

float CBodyStateInfo::GetMaxSpeed() const {
  float speed = GetLocomotionSpeed(pas::kLA_Run);
  if (close_enough(speed, 0.f)) {
    for (int i = 0; i <= pas::kLA_StrafeDown; ++i) {
      const float candidate = GetLocomotionSpeed(pas::ELocomotionAnim(i));
      if (candidate > speed) {
        speed = candidate;
      }
    }
  }
  return speed;
}

float CBodyStateInfo::GetLocomotionSpeed(pas::ELocomotionAnim anim) const {
  const CBSLocomotion* locomotion =
      static_cast< const CBSLocomotion* >(mStates[pas::kAS_Locomotion]);
  if (locomotion && mBodyController) {
    return locomotion->GetLocomotionSpeed(mBodyController->GetLocomotionType(), anim);
  }
  return 0.f;
}

void CBodyStateInfo::SetupBodyStates(CActor& actor, EBodyType type) {
  SetupLocomotionStates(actor, type);
  mStates[pas::kAS_Locomotion] = mLocomotion.get();
  mStates[pas::kAS_Turn] = mTurn.get();
  mStates[pas::kAS_Fall] = &mFall;
  mStates[pas::kAS_Getup] = &mGetup;
  mStates[pas::kAS_LieOnGround] = &mLieOnGround;
  mStates[pas::kAS_Step] = &mStep;
  mStates[pas::kAS_Death] = &mDie;
  mStates[pas::kAS_KnockBack] = &mKnockBack;
  mStates[pas::kAS_MeleeAttack] = &mAttack;
  mStates[pas::kAS_ProjectileAttack] = &mProjectileAttack;
  mStates[pas::kAS_LoopAttack] = &mLoopAttack;
  mStates[pas::kAS_LoopReaction] = &mLoopReaction;
  mStates[pas::kAS_GroundHit] = &mGroundHit;
  mStates[pas::kAS_Generate] = &mGenerate;
  mStates[pas::kAS_Jump] = &mJump;
  mStates[pas::kAS_Hurled] = &mHurled;
  mStates[pas::kAS_Slide] = &mSlide;
  mStates[pas::kAS_Taunt] = &mTaunt;
  mStates[pas::kAS_Scripted] = &mScripted;
  mStates[pas::kAS_Cover] = &mCover;
  mStates[pas::kAS_WallHang] = &mWallHang;
  mStates[pas::kAS_AdditiveIdle] = &mAdditiveIdle;
  mStates[pas::kAS_AdditiveAim] = &mAdditiveAim;
  mStates[pas::kAS_AdditiveFlinch] = &mAdditiveFlinch;
  mStates[pas::kAS_AdditiveReaction] = &mAdditiveReaction;
  mStates[pas::kAS_AdditiveLoopReaction] = &mAdditiveLoopReaction;
}

void CBodyStateInfo::SetupLocomotionStates(CActor& actor, EBodyType type) {
  switch (type) {
  case kBT_BiPedal:
    mLocomotion = rs_new CBSBiPedLocomotion(actor);
    mTurn = rs_new CBSTurn();
    break;
  case kBT_Flyer:
    mLocomotion = rs_new CBSFlyerLocomotion(actor, false);
    mTurn = rs_new CBSFlyerTurn();
    break;
  case kBT_PitchableFlyer:
    mLocomotion = rs_new CBSFlyerLocomotion(actor, true);
    mTurn = rs_new CBSPitchableFlyerTurn();
    break;
  case kBT_Floater:
    mLocomotion = rs_new CBSFloaterLocomotion(actor);
    mTurn = rs_new CBSTurn();
    break;
  case kBT_WallWalker:
    mLocomotion = rs_new CBSWallWalkerLocomotion(actor);
    mTurn = rs_new CBSFlyerTurn();
    break;
  case kBT_AiMovedFlyer:
    mLocomotion = rs_new CBSAiMovedFlyerLocomotion(actor);
    mTurn = rs_new CBSTurn();
    break;
  case kBT_4WayBlended:
    mLocomotion = rs_new CBSBlendedLocomotion(actor, 300.f);
    mTurn = rs_new CBSTurn();
    break;
  default:
    mLocomotion = rs_new CBSRestrictedLocomotion(actor);
    mTurn = rs_new CBSTurn();
    break;
  }
}
