#ifndef _CPLAYERBODYSTATECMDMGR
#define _CPLAYERBODYSTATECMDMGR

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed names. The player controller has a separate command domain from the AI controller.
enum EPlayerBodyStateCmd {
  kPBSC_Locomotion,
  kPBSC_MorphToScrewAttack,
  kPBSC_MorphToBall,
  kPBSC_MorphToPlayer,
  kPBSC_Jump,
  kPBSC_Grapple,
  kPBSC_DoubleJump,
  kPBSC_Dash,
  kPBSC_KnockBack,
  kPBSC_DeathReaction,
  kPBSC_GibDeath,
  kPBSC_ContinueLocomotion,
  kPBSC_HardLanding,
  kPBSC_Aim,
  kPBSC_Flinch,
  kPBSC_AdditiveReaction
};

class CPlayerBodyStateCmd {
public:
  explicit CPlayerBodyStateCmd(EPlayerBodyStateCmd command) : mCommand(command) {}

  virtual ~CPlayerBodyStateCmd() {}

  EPlayerBodyStateCmd GetCommandId() const { return mCommand; }

private:
  EPlayerBodyStateCmd mCommand;
};
CHECK_SIZEOF(CPlayerBodyStateCmd, 0x8)

class CPBCLocomotionCmd : public CPlayerBodyStateCmd {
public:
  CPBCLocomotionCmd(const CVector3f& movement, const CVector3f& facing)
  : CPlayerBodyStateCmd(kPBSC_Locomotion), mMovement(movement), mFacing(facing) {}

  const CVector3f& GetMovement() const { return mMovement; }

  const CVector3f& GetFacing() const { return mFacing; }

private:
  CVector3f mMovement;
  CVector3f mFacing;
};
CHECK_SIZEOF(CPBCLocomotionCmd, 0x20)

class CPBCMorphToScrewAttackCmd : public CPlayerBodyStateCmd {
public:
  CPBCMorphToScrewAttackCmd(int transitionType, int animationVariant)
  : CPlayerBodyStateCmd(kPBSC_MorphToScrewAttack)
  , mTransitionType(transitionType)
  , mAnimationVariant(animationVariant) {}

  int GetTransitionType() const { return mTransitionType; }

  int GetAnimationVariant() const { return mAnimationVariant; }

private:
  int mTransitionType;
  int mAnimationVariant;
};
CHECK_SIZEOF(CPBCMorphToScrewAttackCmd, 0x10)

class CPBCMorphToBallCmd : public CPlayerBodyStateCmd {
public:
  CPBCMorphToBallCmd(int transitionType, int animationVariant)
  : CPlayerBodyStateCmd(kPBSC_MorphToBall)
  , mTransitionType(transitionType)
  , mAnimationVariant(animationVariant) {}

  int GetTransitionType() const { return mTransitionType; }

  int GetAnimationVariant() const { return mAnimationVariant; }

private:
  int mTransitionType;
  int mAnimationVariant;
};
CHECK_SIZEOF(CPBCMorphToBallCmd, 0x10)

class CPBCMorphToPlayerCmd : public CPlayerBodyStateCmd {
public:
  CPBCMorphToPlayerCmd(int transitionType, int animationVariant)
  : CPlayerBodyStateCmd(kPBSC_MorphToPlayer)
  , mTransitionType(transitionType)
  , mAnimationVariant(animationVariant) {}

  int GetTransitionType() const { return mTransitionType; }

  int GetAnimationVariant() const { return mAnimationVariant; }

private:
  int mTransitionType;
  int mAnimationVariant;
};
CHECK_SIZEOF(CPBCMorphToPlayerCmd, 0x10)

class CPBCJumpCmd : public CPlayerBodyStateCmd {
public:
  CPBCJumpCmd(int animationVariant, int jumpParameter)
  : CPlayerBodyStateCmd(kPBSC_Jump)
  , mAnimationVariant(animationVariant)
  , mJumpParameter(jumpParameter) {}

  int GetAnimationVariant() const { return mAnimationVariant; }
  int GetJumpParameter() const { return mJumpParameter; }

private:
  int mAnimationVariant;
  // Guessed name; copied auxiliary parameter observed as 0 or 4, use unresolved.
  int mJumpParameter;
};
CHECK_SIZEOF(CPBCJumpCmd, 0x10)

class CPBCGrappleCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCGrappleCmd(int animationVariant)
  : CPlayerBodyStateCmd(kPBSC_Grapple), mAnimationVariant(animationVariant) {}

  int GetAnimationVariant() const { return mAnimationVariant; }

private:
  int mAnimationVariant;
};
CHECK_SIZEOF(CPBCGrappleCmd, 0xC)

class CPBCDashCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCDashCmd(int animationVariant)
  : CPlayerBodyStateCmd(kPBSC_Dash), mAnimationVariant(animationVariant) {}

  int GetAnimationVariant() const { return mAnimationVariant; }

private:
  int mAnimationVariant;
};
CHECK_SIZEOF(CPBCDashCmd, 0xC)

class CPBCKnockBackCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCKnockBackCmd(const CVector3f& direction)
  : CPlayerBodyStateCmd(kPBSC_KnockBack), mDirection(direction) {}

  const CVector3f& GetDirection() const { return mDirection; }

private:
  CVector3f mDirection;
};
CHECK_SIZEOF(CPBCKnockBackCmd, 0x14)

class CPBCDeathReactionCmd : public CPlayerBodyStateCmd {
public:
  enum EDeathReactionMode { kDRM_Invalid = -1, kDRM_Fall, kDRM_Burning, kDRM_Hurled };

  CPBCDeathReactionCmd(const CVector3f& direction, EDeathReactionMode mode)
  : CPlayerBodyStateCmd(kPBSC_DeathReaction), mDirection(direction), mMode(mode) {}

private:
  CVector3f mDirection;
  EDeathReactionMode mMode;
};
CHECK_SIZEOF(CPBCDeathReactionCmd, 0x18)

class CPBCAimCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCAimCmd(const CVector3f& direction)
  : CPlayerBodyStateCmd(kPBSC_Aim), mDirection(direction) {}

  const CVector3f& GetDirection() const { return mDirection; }

private:
  CVector3f mDirection;
};
CHECK_SIZEOF(CPBCAimCmd, 0x14)

class CPBCFlinchCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCFlinchCmd(const CVector3f& direction)
  : CPlayerBodyStateCmd(kPBSC_Flinch), mDirection(direction) {}

  const CVector3f& GetDirection() const { return mDirection; }

private:
  CVector3f mDirection;
};
CHECK_SIZEOF(CPBCFlinchCmd, 0x14)

class CPBCAdditiveReactionCmd : public CPlayerBodyStateCmd {
public:
  enum EAdditiveReactionType { kART_Invalid = -1, kART_Shock, kART_UnFreeze };

  CPBCAdditiveReactionCmd(EAdditiveReactionType type, bool looping)
  : CPlayerBodyStateCmd(kPBSC_AdditiveReaction), mType(type), mLooping(looping) {}

  EAdditiveReactionType GetType() const { return mType; }
  bool IsLooping() const { return mLooping; }

private:
  EAdditiveReactionType mType;
  bool mLooping : 1;
};
CHECK_SIZEOF(CPBCAdditiveReactionCmd, 0x10)

class CPlayerBodyStateCmdMgr {
public:
  CPlayerBodyStateCmdMgr();
  ~CPlayerBodyStateCmdMgr();
  void ClearCmds();

  const CPlayerBodyStateCmd* GetCmd(EPlayerBodyStateCmd command) const;

  void DeliverCmd(const CPlayerBodyStateCmd& command);
  void DeliverCmd(const CPBCLocomotionCmd& command) {
    DeliverCmd(command.GetCommandId());
    mLocomotion = command;
  }

  void DeliverCmd(const CPBCMorphToScrewAttackCmd& command) {
    DeliverCmd(command.GetCommandId());
    mMorphToScrewAttack = command;
  }

  void DeliverCmd(const CPBCMorphToBallCmd& command) {
    DeliverCmd(command.GetCommandId());
    mMorphToBall = command;
  }

  void DeliverCmd(const CPBCMorphToPlayerCmd& command) {
    DeliverCmd(command.GetCommandId());
    mMorphToPlayer = command;
  }

  void DeliverCmd(const CPBCJumpCmd& command) {
    DeliverCmd(command.GetCommandId());
    mJump = command;
  }

  void DeliverCmd(const CPBCGrappleCmd& command) {
    DeliverCmd(command.GetCommandId());
    mGrapple = command;
  }

  void DeliverCmd(const CPBCDashCmd& command) {
    DeliverCmd(command.GetCommandId());
    mDash = command;
  }

  void DeliverCmd(const CPBCAimCmd& command) {
    DeliverCmd(command.GetCommandId());
    mAim = command;
  }

  void DeliverCmd(const CPBCKnockBackCmd& command) {
    DeliverCmd(command.GetCommandId());
    mKnockBack = command;
  }

  void DeliverCmd(const CPBCDeathReactionCmd& command) {
    DeliverCmd(command.GetCommandId());
    mDeathReaction = command;
  }

  void DeliverCmd(const CPBCFlinchCmd& command) {
    DeliverCmd(command.GetCommandId());
    mFlinch = command;
  }

  void DeliverCmd(const CPBCAdditiveReactionCmd& command) {
    DeliverCmd(command.GetCommandId());
    mAdditiveReaction = command;
  }

private:
  void DeliverCmd(EPlayerBodyStateCmd command);

  rstl::reserved_vector< CPlayerBodyStateCmd*, 16 > mCommands;
  rstl::reserved_vector< bool, 16 > mDelivered;
  CPBCLocomotionCmd mLocomotion;
  CPBCMorphToScrewAttackCmd mMorphToScrewAttack;
  CPBCMorphToBallCmd mMorphToBall;
  CPBCMorphToPlayerCmd mMorphToPlayer;
  CPBCJumpCmd mJump;
  CPBCGrappleCmd mGrapple;
  CPlayerBodyStateCmd mDoubleJump;
  CPBCDashCmd mDash;
  CPBCKnockBackCmd mKnockBack;
  CPBCDeathReactionCmd mDeathReaction;
  CPlayerBodyStateCmd mGibDeath;
  CPlayerBodyStateCmd mContinueLocomotion;
  CPlayerBodyStateCmd mHardLanding;
  CPBCAimCmd mAim;
  CPBCFlinchCmd mFlinch;
  CPBCAdditiveReactionCmd mAdditiveReaction;
};
CHECK_SIZEOF(CPlayerBodyStateCmdMgr, 0x154)

#endif // _CPLAYERBODYSTATECMDMGR
