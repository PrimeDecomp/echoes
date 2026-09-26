#ifndef _CBODYSTATEINFO
#define _CBODYSTATEINFO

#include "MetroidPrime/BodyState/CABSAim.hpp"
#include "MetroidPrime/BodyState/CABSFlinch.hpp"
#include "MetroidPrime/BodyState/CABSIdle.hpp"
#include "MetroidPrime/BodyState/CABSLoopReaction.hpp"
#include "MetroidPrime/BodyState/CABSReaction.hpp"
#include "MetroidPrime/BodyState/CBSAttack.hpp"
#include "MetroidPrime/BodyState/CBSCover.hpp"
#include "MetroidPrime/BodyState/CBSDie.hpp"
#include "MetroidPrime/BodyState/CBSFall.hpp"
#include "MetroidPrime/BodyState/CBSGenerate.hpp"
#include "MetroidPrime/BodyState/CBSGetup.hpp"
#include "MetroidPrime/BodyState/CBSGroundHit.hpp"
#include "MetroidPrime/BodyState/CBSHurled.hpp"
#include "MetroidPrime/BodyState/CBSJump.hpp"
#include "MetroidPrime/BodyState/CBSKnockBack.hpp"
#include "MetroidPrime/BodyState/CBSLieOnGround.hpp"
#include "MetroidPrime/BodyState/CBSLoopAttack.hpp"
#include "MetroidPrime/BodyState/CBSLoopReaction.hpp"
#include "MetroidPrime/BodyState/CBSProjectileAttack.hpp"
#include "MetroidPrime/BodyState/CBSScripted.hpp"
#include "MetroidPrime/BodyState/CBSSlide.hpp"
#include "MetroidPrime/BodyState/CBSStep.hpp"
#include "MetroidPrime/BodyState/CBSTaunt.hpp"
#include "MetroidPrime/BodyState/CBSWallHang.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CActor;

class CBodyStateInfo {
public:
  CBodyStateInfo(CActor& actor, EBodyType type);
  ~CBodyStateInfo();

  void SetBodyController(CBodyController* controller) { mBodyController = controller; }

  pas::EAnimationState GetCurrentStateId() const { return mState; }

  pas::EAnimationState GetCurrentAdditiveStateId() const { return mAdditiveState; }

  void SetState(pas::EAnimationState state);
  const CBodyState* GetCurrentState() const;
  CBodyState* GetCurrentState();
  void SetAdditiveState(pas::EAnimationState state);
  CAdditiveBodyState* GetCurrentAdditiveState();

private:
  rstl::vector< CBodyState* > mStates;
  pas::EAnimationState mState;
  pas::EAnimationState mAdditiveState;
  rstl::single_ptr< CBodyState > mLocomotion;
  rstl::single_ptr< CBodyState > mTurn;
  CBSFall mFall;
  CBSGetup mGetup;
  CBSLieOnGround mLieOnGround;
  CBSStep mStep;
  CBSDie mDie;
  CBSKnockBack mKnockBack;
  CBSAttack mAttack;
  CBSProjectileAttack mProjectileAttack;
  CBSLoopAttack mLoopAttack;
  CBSLoopReaction mLoopReaction;
  CBSGroundHit mGroundHit;
  CBSGenerate mGenerate;
  CBSJump mJump;
  CBSHurled mHurled;
  CBSSlide mSlide;
  CBSTaunt mTaunt;
  CBSScripted mScripted;
  CBSCover mCover;
  CBSWallHang mWallHang;
  CABSIdle mAdditiveIdle;
  CABSAim mAdditiveAim;
  CABSFlinch mAdditiveFlinch;
  CABSReaction mAdditiveReaction;
  CABSLoopReaction mAdditiveLoopReaction;
  CBodyController* mBodyController;
  float mMaxPitch;
  bool mChangeLocoAtEndOfAnimOnly : 1;
};
CHECK_SIZEOF(CBodyStateInfo, 0x204)

#endif // _CBODYSTATEINFO
