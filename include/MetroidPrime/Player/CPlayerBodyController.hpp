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
class CActor;
class CPASDatabase;

// Guessed names throughout this dependency scaffold. Native AFSM labels establish
// the component roles; these are not the AI controller's command or PAS domains.
class CPlayerBodyController : public CEntity {
public:
  // Guessed player PAS domain: the same numeric values denote different AI states.
  enum EPlayerAnimationState { kPAS_Locomotion = 0, kPAS_Jump = 3 };

  // CEntity
  ~CPlayerBodyController() override;

  CPlayerBodyStateCmdMgr& CommandMgr() { return mCommandMgr; }

  const CPlayerBodyStateCmdMgr& CommandMgr() const { return mCommandMgr; }

  CPlayer& GetPlayer() const { return *mPlayer; }

  int GetCurrentAnimationId() const { return mAnimationId; }

  bool IsAnimationOver() const { return (mAnimationFlags & kAF_AnimationOver) != 0; }

  const CPASDatabase& GetPASDatabase() const;
  void MultiplyPlaybackRate(float rate);
  void RequestAnimation(const CAnimPlaybackParms& parameters, bool looping, bool noTransition);
  bool IsAnimationLooping() const;

  bool IsDeathReactionActive() const { return (mReactionFlags & kRF_DeathReactionActive) != 0; }

private:
  enum EAnimationFlags { kAF_AnimationOver = 0x80 };

  enum EReactionFlags { kRF_GibDeath = 0x80, kRF_DeathReactionActive = 0x40 };

  struct SLocomotionState {
    // Guessed names: direction and speed categories recovered from the native table.
    enum EDirection { kD_Forward, kD_Backward, kD_Left, kD_Right };
    enum ECategory {
      kC_Invalid = -1,
      kC_Idle,
      kC_ForwardSlow,
      kC_ForwardMedium,
      kC_ForwardFast,
      kC_BackwardSlow,
      kC_BackwardMedium,
      kC_BackwardFast,
      kC_LeftSlow,
      kC_LeftMedium,
      kC_LeftFast,
      kC_RightSlow,
      kC_RightMedium,
      kC_RightFast
    };

    explicit SLocomotionState(CActor& actor);
    void SetLocomotionMode(int mode);
    void SetAnimationChangeDisabled(bool disabled);
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    void Update(float dt, CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);
    float ComputeWeightPercentage(float speed, const rstl::pair< int, float >& lower,
                                  const rstl::pair< int, float >& upper) const;
    void UpdateAnimation(CPlayerBodyController& controller, bool force);
    const rstl::pair< int, float >& GetLocoAnimation(int mode, ECategory category) const;
    bool IsStrafing(const CPlayerBodyController& controller) const;
    void UpdateStrafe(float speed, CPlayerBodyController& controller, ECategory previous);
    void UpdateIdle(CPlayerBodyController& controller, ECategory previous);
    void UpdateDirectional(float speed, CPlayerBodyController& controller, ECategory previous,
                           EDirection direction);
    void UpdateSlow(float speed, CPlayerBodyController& controller, ECategory previous,
                    EDirection direction);
    void UpdateMedium(float speed, CPlayerBodyController& controller, ECategory previous,
                      EDirection direction);
    void UpdateFast(float speed, CPlayerBodyController& controller, ECategory previous,
                    EDirection direction);

    static const ECategory skDirectionalCategories[4][3];

    int mLocomotionMode;
    int mPreviousLocomotionMode;
    rstl::reserved_vector< rstl::reserved_vector< rstl::pair< int, float >, 13 >, 11 > mAnims;
    ECategory mCategory;
    float mPrimeTime;
    bool mAnimationChangeDisabled : 1;
  };

  struct SJumpState {
    // Guessed player-specific phases; these do not share the AI PAS enum domain.
    enum EState { kS_Invalid = -1, kS_IntoJump, kS_Jump, kS_Loop, kS_Landing };
    enum EAnimationPhase { kAP_IntoJump, kAP_Jump, kAP_Loop, kAP_Landing, kAP_HardLanding };
    enum EDoubleJumpVariant { kDJV_Forward = 2, kDJV_Backward = 3 };

    SJumpState();
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    bool Update(CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);
    bool PlayDoubleJump(CStateManager& mgr, CPlayerBodyController& controller);
    bool PlayJump(CStateManager& mgr, CPlayerBodyController& controller);
    void PlayJumpLoop(CStateManager& mgr, CPlayerBodyController& controller);
    void PlayLanding(CStateManager& mgr, CPlayerBodyController& controller);
    bool PlayHardLanding(CStateManager& mgr, CPlayerBodyController& controller);

    EState mState;
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
    bool mNoTransition;
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
  // Packed native flag bytes. Only investigated animation/reaction masks are exposed.
  uchar mAnimationFlags;
  uchar mReactionFlags;
};
CHECK_SIZEOF(CPlayerBodyController, 0x794)

#endif // _CPLAYERBODYCONTROLLER
