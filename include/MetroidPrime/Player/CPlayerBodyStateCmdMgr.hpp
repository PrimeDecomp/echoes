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
  const CVector3f& GetMovement() const { return mMovement; }

  const CVector3f& GetFacing() const { return mFacing; }

private:
  CVector3f mMovement;
  CVector3f mFacing;
};
CHECK_SIZEOF(CPBCLocomotionCmd, 0x20)

class CPBCMorphToScrewAttackCmd : public CPlayerBodyStateCmd {
private:
  int mTransitionType;
  int mAnimationVariant;
};

class CPBCMorphToBallCmd : public CPlayerBodyStateCmd {
private:
  int mTransitionType;
  int mAnimationVariant;
};

class CPBCMorphToPlayerCmd : public CPlayerBodyStateCmd {
private:
  int mTransitionType;
  int mAnimationVariant;
};

class CPBCJumpCmd : public CPlayerBodyStateCmd {
public:
  int GetAnimationVariant() const { return mAnimationVariant; }

private:
  int mAnimationVariant;
  // Guessed name; copied auxiliary parameter observed as 0 or 4, use unresolved.
  int mJumpParameter;
};

class CPBCGrappleCmd : public CPlayerBodyStateCmd {
private:
  int mAnimationVariant;
};

class CPBCDashCmd : public CPlayerBodyStateCmd {
private:
  int mAnimationVariant;
};

class CPBCKnockBackCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCKnockBackCmd(const CVector3f& direction)
  : CPlayerBodyStateCmd(kPBSC_KnockBack), mDirection(direction) {}

private:
  CVector3f mDirection;
};
CHECK_SIZEOF(CPBCKnockBackCmd, 0x14)

class CPBCDeathReactionCmd : public CPlayerBodyStateCmd {
public:
  enum EDeathReactionMode { kDRM_Fall, kDRM_Burning, kDRM_Hurled };

  CPBCDeathReactionCmd(const CVector3f& direction, EDeathReactionMode mode)
  : CPlayerBodyStateCmd(kPBSC_DeathReaction), mDirection(direction), mMode(mode) {}

private:
  CVector3f mDirection;
  EDeathReactionMode mMode;
};
CHECK_SIZEOF(CPBCDeathReactionCmd, 0x18)

class CPBCAimCmd : public CPlayerBodyStateCmd {
private:
  CVector3f mDirection;
};

class CPBCFlinchCmd : public CPlayerBodyStateCmd {
public:
  explicit CPBCFlinchCmd(const CVector3f& direction)
  : CPlayerBodyStateCmd(kPBSC_Flinch), mDirection(direction) {}

private:
  CVector3f mDirection;
};
CHECK_SIZEOF(CPBCFlinchCmd, 0x14)

class CPBCAdditiveReactionCmd : public CPlayerBodyStateCmd {
public:
  enum EAdditiveReactionType { kART_Shock, kART_UnFreeze };

  CPBCAdditiveReactionCmd(EAdditiveReactionType type, bool looping)
  : CPlayerBodyStateCmd(kPBSC_AdditiveReaction), mType(type), mLooping(looping) {}

private:
  EAdditiveReactionType mType;
  bool mLooping : 1;
};
CHECK_SIZEOF(CPBCAdditiveReactionCmd, 0x10)

class CPlayerBodyStateCmdMgr {
public:
  const CPlayerBodyStateCmd* GetCmd(EPlayerBodyStateCmd command) const;

  void DeliverCmd(const CPlayerBodyStateCmd& command);
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
