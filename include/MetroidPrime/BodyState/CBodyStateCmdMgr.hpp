#ifndef _CBODYSTATECMDMGR
#define _CBODYSTATECMDMGR

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

enum ESteeringBlendMode { kSBM_Normal, kSBM_FullSpeed, kSBM_Clamped };

class CBodyStateCmd {
public:
  explicit CBodyStateCmd(EBodyStateCmd cmd) : mCmd(cmd) {}

  virtual ~CBodyStateCmd() {}

  EBodyStateCmd GetCommandId() const { return mCmd; }

private:
  EBodyStateCmd mCmd;
};
CHECK_SIZEOF(CBodyStateCmd, 0x8)

// Command storage recovered from CBodyStateCmdMgr. Constructors and additional
// delivery overloads remain with the command-manager TU.

class CBCGetupCmd : public CBodyStateCmd {
private:
  pas::EGetupType mType;
};
CHECK_SIZEOF(CBCGetupCmd, 0xc)

class CBCStepCmd : public CBodyStateCmd {
private:
  pas::EStepDirection mDir;
  pas::EStepType mType;
  CVector3f mTargetPos;
  bool mHasTargetPos;
};
CHECK_SIZEOF(CBCStepCmd, 0x20)

class CBCKnockDownCmd : public CBodyStateCmd {
private:
  CVector3f mDir;
  pas::ESeverity mSeverity;
  bool x18_;
};
CHECK_SIZEOF(CBCKnockDownCmd, 0x1c)

class CBCKnockBackCmd : public CBodyStateCmd {
private:
  CVector3f mDir;
  pas::ESeverity mSeverity;
  int x18_;
  bool x1c_;
};
CHECK_SIZEOF(CBCKnockBackCmd, 0x20)

class CBCMeleeAttackCmd : public CBodyStateCmd {
private:
  pas::ESeverity mSeverity;
  CVector3f mTargetPos;
  bool mHasTargetPos;
};
CHECK_SIZEOF(CBCMeleeAttackCmd, 0x1c)

class CBCProjectileAttackCmd : public CBodyStateCmd {
private:
  pas::ESeverity mSeverity;
  CVector3f mTarget;
  bool mBlendAnims;
};
CHECK_SIZEOF(CBCProjectileAttackCmd, 0x1c)

class CBCLoopAttackCmd : public CBodyStateCmd {
private:
  pas::ELoopAttackType mType;
  int mWaitForAnimOver;
  bool mSkipInto : 1; // Guessed name
};
CHECK_SIZEOF(CBCLoopAttackCmd, 0x14)

class CBCLoopReactionCmd : public CBodyStateCmd {
private:
  pas::EReactionType mType;
};
CHECK_SIZEOF(CBCLoopReactionCmd, 0xc)

class CBCLoopHitReactionCmd : public CBodyStateCmd {
private:
  pas::EReactionType mType;
};
CHECK_SIZEOF(CBCLoopHitReactionCmd, 0xc)

class CBCGenerateCmd : public CBodyStateCmd {
private:
  pas::EGenerateType mType;
  CVector3f mTargetPos;
  int mAnimId;
  uint mTargetTransform : 1;
  uint mOverrideAnim : 1;
  uint x1c_2_ : 1;
};
CHECK_SIZEOF(CBCGenerateCmd, 0x20)

class CBCHurledCmd : public CBodyStateCmd {
private:
  CVector3f mDirection;
  CVector3f mLaunchVel;
  bool mStartInKnockLoop;
};
CHECK_SIZEOF(CBCHurledCmd, 0x24)

class CBCJumpCmd : public CBodyStateCmd {
private:
  pas::EJumpType mType;
  int xc_;
  CVector3f mWaypoint1;
  CVector3f mWaypoint2;
  pas::EJumpState mInitialState; // Guessed name
  int x2c_;
  bool mWallJump : 1;
};
CHECK_SIZEOF(CBCJumpCmd, 0x34)

// Guessed name
class CBCUnknown18Cmd : public CBodyStateCmd {
private:
  int x8_;
};
CHECK_SIZEOF(CBCUnknown18Cmd, 0xc)

// Guessed name
class CBCUnknown19Cmd : public CBodyStateCmd {};
CHECK_SIZEOF(CBCUnknown19Cmd, 0x8)

class CBCSlideCmd : public CBodyStateCmd {
public:
  CBCSlideCmd(pas::ESlideType type, CVector3f dir)
  : CBodyStateCmd(kBSC_Slide), mType(type), mDir(dir) {}

  // CBodyStateCmd
  ~CBCSlideCmd() override {}

private:
  pas::ESlideType mType;
  CVector3f mDir;
};
CHECK_SIZEOF(CBCSlideCmd, 0x18)

class CBCTauntCmd : public CBodyStateCmd {
private:
  pas::ETauntType mType;
};
CHECK_SIZEOF(CBCTauntCmd, 0xc)

class CBCScriptedCmd : public CBodyStateCmd {
private:
  int mAnimId;
  bool mIsLooped : 1;
  bool mUseLoopDuration : 1;
  float mLoopDuration;
};
CHECK_SIZEOF(CBCScriptedCmd, 0x14)

class CBCCoverCmd : public CBodyStateCmd {
private:
  pas::ECoverDirection mDir;
  CVector3f mTargetPos;
  CVector3f mAlignDir;
};
CHECK_SIZEOF(CBCCoverCmd, 0x24)

class CBCWallHangCmd : public CBodyStateCmd {
private:
  TUniqueId mWpId;
};
CHECK_SIZEOF(CBCWallHangCmd, 0xc)

class CBCAdditiveAimCmd : public CBodyStateCmd {
private:
  int x8_;
};
CHECK_SIZEOF(CBCAdditiveAimCmd, 0xc)

class CBCAdditiveFlinchCmd : public CBodyStateCmd {
private:
  float mWeight;
  int xc_;
};
CHECK_SIZEOF(CBCAdditiveFlinchCmd, 0x10)

class CBCAdditiveReactionCmd : public CBodyStateCmd {
public:
  CBCAdditiveReactionCmd(pas::EAdditiveReactionType type, float weight, bool active)
  : CBodyStateCmd(kBSC_AdditiveReaction), mWeight(weight), mType(type), mActive(active) {}

  pas::EAdditiveReactionType GetType() const { return mType; }

  float GetWeight() const { return mWeight; }

  bool GetIsActive() const { return mActive; }

private:
  float mWeight;
  pas::EAdditiveReactionType mType;
  bool mActive;
};
CHECK_SIZEOF(CBCAdditiveReactionCmd, 0x14)

// Guessed name
class CBCAdditiveLoopReactionCmd : public CBodyStateCmd {
private:
  float mWeight;
  int mType;
};
CHECK_SIZEOF(CBCAdditiveLoopReactionCmd, 0x10)

// Guessed name
class CBCUnknown32Cmd : public CBodyStateCmd {
private:
  float x8_;
};
CHECK_SIZEOF(CBCUnknown32Cmd, 0xc)

class CBodyStateCmdMgr {
public:
  CBodyStateCmdMgr();
  ~CBodyStateCmdMgr();

  void DeliverCmd(const CBodyStateCmd& cmd);
  void DeliverCmd(EBodyStateCmd cmd);
  void DeliverCmd(const CBCAdditiveReactionCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mAdditiveReaction = cmd;
  }

  void BlendSteeringCmds();
  void Reset();
  CBodyStateCmd* GetCmd(EBodyStateCmd cmd);
  const CBodyStateCmd* GetCmd(EBodyStateCmd cmd) const;

  const CVector3f& GetMoveVector() const { return mMove; }

  const CVector3f& GetFaceVector() const { return mFace; }

  const CVector3f& GetTargetVector() const { return mTarget; }

  const CVector3f& GetAdditiveTargetVector() const { return mAdditiveTarget; }

private:
  CVector3f mMove;
  CVector3f mFace;
  CVector3f mPreviousMove;
  CVector3f mPreviousFace;
  CVector3f mTarget;
  CVector3f mAdditiveTarget;
  ESteeringBlendMode mSteeringMode;
  float mSteeringSpeedMin;
  float mSteeringSpeedMax;
  float mSteeringSpeed;
  rstl::reserved_vector< CBodyStateCmd*, 34 > mCommandTable;
  rstl::reserved_vector< bool, 34 > mDeliveredCommands;
  CBCGetupCmd mGetup;
  CBCStepCmd mStep;
  CBodyStateCmd mDie;
  CBCKnockDownCmd mKnockDown;
  CBCKnockBackCmd mKnockBack;
  CBCMeleeAttackCmd mMeleeAttack;
  CBCProjectileAttackCmd mProjectileAttack;
  CBCLoopAttackCmd mLoopAttack;
  CBCLoopReactionCmd mLoopReaction;
  CBCLoopHitReactionCmd mLoopHitReaction;
  CBodyStateCmd mExitState;
  CBodyStateCmd mLeanFromCover;
  CBodyStateCmd mNextState;
  CBodyStateCmd mAbortScripted;
  CBodyStateCmd mMaintainVelocity;
  CBCGenerateCmd mGenerate;
  CBCHurledCmd mHurled;
  CBCJumpCmd mJump;
  CBCUnknown18Cmd x280_;
  CBCUnknown19Cmd x28c_;
  CBCSlideCmd mSlide;
  CBCTauntCmd mTaunt;
  CBCScriptedCmd mScripted;
  CBCCoverCmd mCover;
  CBCWallHangCmd mWallHang;
  CBodyStateCmd mLocomotion;
  CBodyStateCmd mAdditiveIdle;
  CBCAdditiveAimCmd mAdditiveAim;
  CBCAdditiveFlinchCmd mAdditiveFlinch;
  CBCAdditiveReactionCmd mAdditiveReaction;
  CBodyStateCmd mStopReaction;
  CBCAdditiveLoopReactionCmd mAdditiveLoopReaction;
  CBCUnknown32Cmd x354_;
  CBodyStateCmd x360_;
};
CHECK_SIZEOF(CBodyStateCmdMgr, 0x368)

#endif // _CBODYSTATECMDMGR
