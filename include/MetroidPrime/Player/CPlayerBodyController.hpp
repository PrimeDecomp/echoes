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
class CPASAnimParmData;
class CRandom16;

// Guessed names throughout this dependency scaffold. Native AFSM labels establish
// the component roles; these are not the AI controller's command or PAS domains.
class CPlayerBodyController : public CEntity {
public:
  // Guessed player PAS domain: the same numeric values denote different AI states.
  enum EPlayerAnimationState {
    kPAS_Locomotion = 0,
    kPAS_MorphTransition = 1,
    kPAS_Dash = 2,
    kPAS_Jump = 3,
    kPAS_AdditiveAim = 4,
    kPAS_DeathReaction = 5,
    kPAS_MorphDuration = 7,
    kPAS_GunReaction = 9,
    kPAS_Grapple = 11
  };

  CPlayerBodyController(CPlayer& player, CAssetId stateMachine);

  // CEntity
  ~CPlayerBodyController() override;

  void Update(float dt, CStateManager& mgr);
  void ResetStates(CStateManager& mgr);
  void SetAnimationChangeDisabled(bool disabled);
  void PlayGunReaction(CStateManager& mgr);

  CPlayerBodyStateCmdMgr& CommandMgr() { return mCommandMgr; }

  const CPlayerBodyStateCmdMgr& CommandMgr() const { return mCommandMgr; }

  CPlayer& GetPlayer() const { return *mPlayer; }

  int GetCurrentAnimationId() const { return mAnimationId; }

  int GetLocomotionMode() const { return mLocomotion.mLocomotionMode; }

  bool IsAnimationOver() const { return (mAnimationFlags & kAF_AnimationOver) != 0; }

  const CPASDatabase& GetPASDatabase() const;
  void MultiplyPlaybackRate(float rate);
  void RequestAnimation(const CAnimPlaybackParms& parameters, bool looping, bool noTransition);
  bool IsAnimationLooping() const;

  bool IsDeathReactionActive() const { return (mReactionFlags & kRF_DeathReactionActive) != 0; }

private:
  enum EAnimationFlags {
    kAF_AnimationOver = 0x80,
    kAF_StateMachinesInitialized = 0x40,
    kAF_Moving = 0x20,
    kAF_FastLocomotion = 0x10,
    kAF_LocomotionActive = 0x8,
    kAF_Aiming = 0x4,
    kAF_Unfreezing = 0x2,
    kAF_MorphTransitionActive = 0x1
  };

  enum EReactionFlags { kRF_GibDeath = 0x80, kRF_DeathReactionActive = 0x40 };

  enum EStatePhase { kSP_Invalid = -1, kSP_Active = 2, kSP_Over = 3 };

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
    // Guessed player-specific state and PAS phase names.
    enum EState { kS_Invalid = -1, kS_Firing, kS_Pull, kS_Swinging };
    enum EAnimationPhase { kAP_Firing, kAP_Pull, kAP_Swinging };

    SGrappleState();
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    bool Update(CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);
    bool TryPlayFiring(CStateManager& mgr, CPlayerBodyController& controller);
    bool TryPlayPull(CStateManager& mgr, CPlayerBodyController& controller);
    void PlaySwing(CStateManager& mgr, CPlayerBodyController& controller);

    EState mState;
    int mAnimationVariant;
  };

  struct SDashState {
    // Guessed player-specific state and PAS phase names.
    enum EState { kS_Invalid = -1, kS_IntoDash, kS_Loop, kS_Exit };
    enum EAnimationPhase { kAP_IntoDash, kAP_Loop, kAP_Exit };

    SDashState();
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    bool Update(CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);
    void PlayLoop(CStateManager& mgr, CPlayerBodyController& controller);
    void PlayExit(CStateManager& mgr, CPlayerBodyController& controller);

    EState mState;
    int mAnimationVariant;
  };

  // Its native constructor and exit are empty; all state is in the controller.
  struct SKnockBackState {
    SKnockBackState();
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    bool Update(CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);
  };

  struct SAdditiveAimState {
    // Guessed signed-axis labels. Category meanings beyond their mode selectors are unresolved.
    enum EDirection { kD_NegativeYaw, kD_PositiveYaw, kD_PositivePitch, kD_NegativePitch };
    enum ECategory { kC_Default, kC_LocomotionMode4, kC_LocomotionMode5 };

    explicit SAdditiveAimState(CActor& actor);
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    void Update(float dt, CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);
    void UpdatePitch(float dt, const CVector3f& direction, CPlayerBodyController& controller);

    int mAvailableAnimations[3];
    int mAnimationIds[3][4];
    float mYawLimits[2];
    float mPitchLimits[2];
    ECategory mCategory;
    float mYawWeight;
    float mYawVelocity;
    float mPitchWeight;
    float mPitchVelocity;
  };

  struct SAdditiveFlinchState {
    SAdditiveFlinchState();
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    bool Update(CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);

    int mAnimationId;
  };

  struct SAdditiveReactionState {
    SAdditiveReactionState();
    void Start(CStateManager& mgr, CPlayerBodyController& controller);
    bool Update(CStateManager& mgr, CPlayerBodyController& controller);
    void Shutdown(CPlayerBodyController& controller);

    int mAnimationId;
    CPBCAdditiveReactionCmd::EAdditiveReactionType mType;
    bool mLooping : 1;
  };

  struct SAnimationRequest {
    SAnimationRequest(const CAnimPlaybackParms& parameters, bool looping, bool noTransition)
    : mParameters(parameters), mLooping(looping), mNoTransition(noTransition) {}

    CAnimPlaybackParms mParameters;
    bool mLooping;
    bool mNoTransition;
  };

  // Original AFSM labels; the corresponding C++ scope and spellings are reconstructed.
  bool StateOver(CStateManager& mgr, const float& arg);
  bool Jump(CStateManager& mgr, const float& arg);
  bool Grappling(CStateManager& mgr, const float& arg);
  bool Dash(CStateManager& mgr, const float& arg);
  bool TransitionToBall(CStateManager& mgr, const float& arg);
  bool TransitionToPlayer(CStateManager& mgr, const float& arg);
  bool HeavyHit(CStateManager& mgr, const float& arg);
  bool Delay(CStateManager& mgr, const float& arg);
  bool IsMorphball(CStateManager& mgr, const float& arg);
  bool IsFirstPerson(CStateManager& mgr, const float& arg);
  bool Shocked(CStateManager& mgr, const float& arg);
  bool ShouldUnFreeze(CStateManager& mgr, const float& arg);
  bool ShouldFlinch(CStateManager& mgr, const float& arg);
  bool ShouldAim(CStateManager& mgr, const float& arg);
  bool AdditiveStateOver(CStateManager& mgr, const float& arg);
  bool TransitionToScrewAttack(CStateManager& mgr, const float& arg);
  bool WallSlideJump(CStateManager& mgr, const float& arg);
  bool ScrewAttackExpired(CStateManager& mgr, const float& arg);
  bool ScrewAttackHitGround(CStateManager& mgr, const float& arg);
  bool StartWallJump(CStateManager& mgr, const float& arg);
  bool WallSlideExpired(CStateManager& mgr, const float& arg);
  bool WallJumpScrewAttackExpired(CStateManager& mgr, const float& arg);

  void Start(CStateManager& mgr, int msg, float dt);
  void Locomotion(CStateManager& mgr, int msg, float dt);
  void Morphball(CStateManager& mgr, int msg, float dt);
  void MorphToBall(CStateManager& mgr, int msg, float dt);
  void MorphToPlayer(CStateManager& mgr, int msg, float dt);
  void PlayerJump(CStateManager& mgr, int msg, float dt);
  void PlayerGrapple(CStateManager& mgr, int msg, float dt);
  void PlayerDash(CStateManager& mgr, int msg, float dt);
  void KnockBack(CStateManager& mgr, int msg, float dt);
  void Dead(CStateManager& mgr, int msg, float dt);
  void GibDeath(CStateManager& mgr, int msg, float dt);
  void AdditiveIdle(CStateManager& mgr, int msg, float dt);
  void AdditiveFlinch(CStateManager& mgr, int msg, float dt);
  void AdditiveShock(CStateManager& mgr, int msg, float dt);
  void AdditiveUnFreeze(CStateManager& mgr, int msg, float dt);
  void AdditiveAim(CStateManager& mgr, int msg, float dt);
  void MorphToScrewAttack(CStateManager& mgr, int msg, float dt);
  void ScrewAttack(CStateManager& mgr, int msg, float dt);
  void WallSlide(CStateManager& mgr, int msg, float dt);
  void TransitionOutOfWallSlide(CStateManager& mgr, int msg, float dt);
  void WallSlideLand(CStateManager& mgr, int msg, float dt);
  void TransitionOutOfWallScrewAttack(CStateManager& mgr, int msg, float dt);
  void TransitionToWallScrewAttack(CStateManager& mgr, int msg, float dt);
  void WallJumpScrewAttack(CStateManager& mgr, int msg, float dt);
  void TransitionOutOfScrewAttack(CStateManager& mgr, int msg, float dt);
  void TransitionOutOfScrewAttackFloor(CStateManager& mgr, int msg, float dt);

  int GetAnimationSet(CStateManager& mgr) const;
  void CheckDeathCommands(CStateManager& mgr);
  void SetupStateMachines(CStateManager& mgr);
  void TrySetupStateMachines(CStateManager& mgr);
  const CStateMachine* GetStateMachine();
  void SelectLoopingAnimation(const CPASAnimParmData& parameters, CRandom16& random);
  void SelectAnimation(const CPASAnimParmData& parameters, CRandom16& random);
  void SetPlaybackRate(float rate);

  static const TStateMachineState< CPlayerBodyController >::STriggerFunction skTriggerFunctions[22];
  static const TStateMachineState< CPlayerBodyController >::SStateFunction skStateFunctions[26];

  CPlayer* mPlayer;
  EStatePhase mBodyStatePhase;
  EStatePhase mAdditiveStatePhase;
  CPlayerBodyStateCmdMgr mCommandMgr;
  SLocomotionState mLocomotion;
  SJumpState mJump;
  SGrappleState mGrapple;
  SDashState mDash;
  SKnockBackState mKnockBack;
  SAdditiveAimState mAdditiveAim;
  SAdditiveFlinchState mAdditiveFlinch;
  SAdditiveReactionState mAdditiveReaction;
  TCachedToken< CStateMachine > mStateMachineResource;
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
