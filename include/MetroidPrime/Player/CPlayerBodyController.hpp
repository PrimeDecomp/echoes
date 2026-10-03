#ifndef _CPLAYERBODYCONTROLLER
#define _CPLAYERBODYCONTROLLER

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Player/CPlayerBodyStateCmdMgr.hpp"
#include "MetroidPrime/TStateMachineState.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

class CPlayer;

// Guessed names throughout this dependency scaffold. Native AFSM labels establish
// the component roles; these are not the AI controller's command or PAS domains.
class CPlayerBodyController : public CEntity {
public:
  // CEntity
  ~CPlayerBodyController() override;

  CPlayerBodyStateCmdMgr& CommandMgr() { return mCommandMgr; }

  bool IsDeathReactionActive() const { return (mReactionFlags & kRF_DeathReactionActive) != 0; }

private:
  enum EReactionFlags { kRF_GibDeath = 0x80, kRF_DeathReactionActive = 0x40 };

  struct SLocomotionState {
    int mLocomotionMode;
    int mPreviousLocomotionMode;
    rstl::reserved_vector< rstl::reserved_vector< rstl::pair< int, float >, 13 >, 11 > mAnims;
    int mLocomotionAnim;
    float mPrimeTime;
    bool mAnimationChangeDisabled : 1;
  };

  struct SJumpState {
    int mState;
    int mAnimationVariant;
    bool mDoubleJumpStarted : 1;
    bool mHardLandingPending : 1;
  };

  struct SGrappleState {
    int mState;
    int mAnimationVariant;
  };

  struct SDashState {
    int mState;
    int mAnimationVariant;
  };

  // Its native constructor and exit are empty; all state is in the controller.
  struct SKnockBackState {};

  struct SAdditiveAimState {
    int mAvailableAnimations[3];
    int mAnimationIds[3][4];
    float mYawLimits[2];
    float mPitchLimits[2];
    int mCategory;
    float mYawWeight;
    float mYawVelocity;
    float mPitchWeight;
    float mPitchVelocity;
  };

  struct SAdditiveFlinchState {
    int mAnimationId;
  };

  struct SAdditiveReactionState {
    int mAnimationId;
    CPBCAdditiveReactionCmd::EAdditiveReactionType mType;
    bool mLooping : 1;
  };

  struct SAnimationRequest {
    CAnimPlaybackParms mParameters;
    bool mLooping;
    bool mRestart;
  };

  CPlayer* mPlayer;
  int mBodyStatePhase;
  int mAdditiveStatePhase;
  CPlayerBodyStateCmdMgr mCommandMgr;
  SLocomotionState mLocomotion;
  SJumpState mJump;
  SGrappleState mGrapple;
  SDashState mDash;
  SKnockBackState mKnockBack;
  SAdditiveAimState mAdditiveAim;
  SAdditiveFlinchState mAdditiveFlinch;
  SAdditiveReactionState mAdditiveReaction;
  TLockedToken< CStateMachine > mStateMachineResource;
  TStateMachineState< CPlayerBodyController > mBodyState;
  TStateMachineState< CPlayerBodyController > mAdditiveState;
  int mAnimationId;
  float mAnimationDuration;
  rstl::optional_object< SAnimationRequest > mPendingAnimation;
  // Packed native flag bytes. Only the reaction mask used by this caller is exposed.
  uchar mAnimationFlags;
  uchar mReactionFlags;
};
CHECK_SIZEOF(CPlayerBodyController, 0x794)

#endif // _CPLAYERBODYCONTROLLER
