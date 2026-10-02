#ifndef _CBODYSTATECMDMGR
#define _CBODYSTATECMDMGR

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
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

class CBCGetupCmd : public CBodyStateCmd {
public:
  explicit CBCGetupCmd(pas::EGetupType type) : CBodyStateCmd(kBSC_Getup), mType(type) {}

private:
  pas::EGetupType mType;
};
CHECK_SIZEOF(CBCGetupCmd, 0xc)

class CBCStepCmd : public CBodyStateCmd {
public:
  CBCStepCmd(pas::EStepDirection dir, pas::EStepType type)
  : CBodyStateCmd(kBSC_Step)
  , mDir(dir)
  , mType(type)
  , mTargetPos(CVector3f::Zero())
  , mHasTargetPos(false) {}

private:
  pas::EStepDirection mDir;
  pas::EStepType mType;
  CVector3f mTargetPos;
  bool mHasTargetPos;
};
CHECK_SIZEOF(CBCStepCmd, 0x20)

class CBCKnockDownCmd : public CBodyStateCmd {
public:
  CBCKnockDownCmd(const CVector3f& dir, pas::ESeverity severity)
  : CBodyStateCmd(kBSC_KnockDown), mDir(dir), mSeverity(severity), x18_(false) {}

private:
  CVector3f mDir;
  pas::ESeverity mSeverity;
  bool x18_;
};
CHECK_SIZEOF(CBCKnockDownCmd, 0x1c)

class CBCKnockBackCmd : public CBodyStateCmd {
public:
  CBCKnockBackCmd(const CVector3f& dir, pas::ESeverity severity)
  : CBodyStateCmd(kBSC_KnockBack), mDir(dir), mSeverity(severity), x18_(-1), x1c_(false) {}

private:
  CVector3f mDir;
  pas::ESeverity mSeverity;
  int x18_;
  bool x1c_;
};
CHECK_SIZEOF(CBCKnockBackCmd, 0x20)

class CBCMeleeAttackCmd : public CBodyStateCmd {
public:
  explicit CBCMeleeAttackCmd(pas::ESeverity severity)
  : CBodyStateCmd(kBSC_MeleeAttack)
  , mSeverity(severity)
  , mTargetPos(CVector3f::Zero())
  , mHasTargetPos(false) {}

  pas::ESeverity GetAttackSeverity() const { return mSeverity; }
  bool HasAttackTargetPos() const { return mHasTargetPos; }
  const CVector3f& GetAttackTargetPos() const { return mTargetPos; }

private:
  pas::ESeverity mSeverity;
  CVector3f mTargetPos;
  bool mHasTargetPos;
};
CHECK_SIZEOF(CBCMeleeAttackCmd, 0x1c)

class CBCProjectileAttackCmd : public CBodyStateCmd {
public:
  CBCProjectileAttackCmd(pas::ESeverity severity, const CVector3f& target, bool blendAnims)
  : CBodyStateCmd(kBSC_ProjectileAttack)
  , mSeverity(severity)
  , mTarget(target)
  , mBlendAnims(blendAnims) {}

  pas::ESeverity GetAttackSeverity() const { return mSeverity; }
  const CVector3f& GetTargetPosition() const { return mTarget; }
  bool BlendTwoClosest() const { return mBlendAnims; }

private:
  pas::ESeverity mSeverity;
  CVector3f mTarget;
  bool mBlendAnims;
};
CHECK_SIZEOF(CBCProjectileAttackCmd, 0x1c)

class CBCLoopAttackCmd : public CBodyStateCmd {
public:
  CBCLoopAttackCmd(pas::ELoopAttackType type, bool waitForAnimOver = false)
  : CBodyStateCmd(kBSC_LoopAttack)
  , mType(type)
  , mWaitForAnimOver(waitForAnimOver)
  , mSkipInto(false) {}

  pas::ELoopAttackType GetAttackType() const { return mType; }
  int WaitForAnimOver() const { return mWaitForAnimOver; }
  bool SkipInto() const { return mSkipInto; }

private:
  pas::ELoopAttackType mType;
  int mWaitForAnimOver;
  bool mSkipInto : 1; // Guessed name
};
CHECK_SIZEOF(CBCLoopAttackCmd, 0x14)

class CBCLoopReactionCmd : public CBodyStateCmd {
public:
  explicit CBCLoopReactionCmd(pas::EReactionType type)
  : CBodyStateCmd(kBSC_LoopReaction), mType(type) {}

  pas::EReactionType GetReactionType() const { return mType; }

private:
  pas::EReactionType mType;
};
CHECK_SIZEOF(CBCLoopReactionCmd, 0xc)

class CBCLoopHitReactionCmd : public CBodyStateCmd {
public:
  explicit CBCLoopHitReactionCmd(pas::EReactionType type)
  : CBodyStateCmd(kBSC_LoopHitReaction), mType(type) {}

  pas::EReactionType GetReactionType() const { return mType; }

private:
  pas::EReactionType mType;
};
CHECK_SIZEOF(CBCLoopHitReactionCmd, 0xc)

class CBCGenerateCmd : public CBodyStateCmd {
public:
  CBCGenerateCmd(pas::EGenerateType type, int animId)
  : CBodyStateCmd(kBSC_Generate)
  , mType(type)
  , mTargetPos(CVector3f::Zero())
  , mAnimId(animId)
  , mTargetTransform(false)
  , mOverrideAnim(animId != -1)
  , x1c_2_(false) {}

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
public:
  CBCHurledCmd(const CVector3f& dir, const CVector3f& launchVel, bool startInLoop = false)
  : CBodyStateCmd(kBSC_Hurled)
  , mDirection(dir)
  , mLaunchVel(launchVel)
  , mStartInKnockLoop(startInLoop) {}

  void SetSkipLaunchState(bool skip) { mStartInKnockLoop = skip; }

  bool GetSkipLaunchState() const { return mStartInKnockLoop; }

  const CVector3f& GetHitDirection() const { return mDirection; }

  const CVector3f& GetLaunchVelocity() const { return mLaunchVel; }

private:
  CVector3f mDirection;
  CVector3f mLaunchVel;
  bool mStartInKnockLoop;
};
CHECK_SIZEOF(CBCHurledCmd, 0x24)

class CBCJumpCmd : public CBodyStateCmd {
public:
  // Guessed names
  enum EFacingFlags { kFF_IntoJump = 1, kFF_AmbushJump = 2 };

  CBCJumpCmd(const CVector3f& waypoint, pas::EJumpType type, pas::EJumpState initialState,
             int animationVariant, int facingFlags)
  : CBodyStateCmd(kBSC_Jump)
  , mType(type)
  , mAnimationVariant(animationVariant)
  , mWaypoint1(waypoint)
  , mWaypoint2(CVector3f::Zero())
  , mInitialState(initialState)
  , mFacingFlags(facingFlags)
  , mWallJump(false) {}

  pas::EJumpType GetJumpType() const { return mType; }

  const CVector3f& GetJumpTarget() const { return mWaypoint1; }

  const CVector3f& GetSecondJumpTarget() const { return mWaypoint2; }

  bool IsWallJump() const { return mWallJump; }

  pas::EJumpState GetInitialState() const { return mInitialState; }

  int GetAnimationVariant() const { return mAnimationVariant; } // Guessed name

  int GetFacingFlags() const { return mFacingFlags; } // Guessed name

private:
  pas::EJumpType mType;
  int mAnimationVariant; // Guessed name
  CVector3f mWaypoint1;
  CVector3f mWaypoint2;
  pas::EJumpState mInitialState; // Guessed name
  int mFacingFlags; // Guessed name
  bool mWallJump : 1;
};
CHECK_SIZEOF(CBCJumpCmd, 0x34)

// Guessed name
class CBCUnknown18Cmd : public CBodyStateCmd {
public:
  CBCUnknown18Cmd() : CBodyStateCmd(kBSC_Unknown18), mAnimationVariant(0) {}

  int GetAnimationVariant() const { return mAnimationVariant; } // Guessed name

private:
  int mAnimationVariant; // Guessed name
};
CHECK_SIZEOF(CBCUnknown18Cmd, 0xc)

// Guessed name
class CBCUnknown19Cmd : public CBodyStateCmd {
public:
  CBCUnknown19Cmd() : CBodyStateCmd(kBSC_Unknown19) {}
};
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
public:
  explicit CBCTauntCmd(pas::ETauntType type) : CBodyStateCmd(kBSC_Taunt), mType(type) {}

private:
  pas::ETauntType mType;
};
CHECK_SIZEOF(CBCTauntCmd, 0xc)

class CBCScriptedCmd : public CBodyStateCmd {
public:
  CBCScriptedCmd(int animId, bool isLooped, bool useLoopDuration, float loopDuration)
  : CBodyStateCmd(kBSC_Scripted)
  , mAnimId(animId)
  , mIsLooped(isLooped)
  , mUseLoopDuration(useLoopDuration)
  , mLoopDuration(loopDuration) {}

  int GetAnimId() const { return mAnimId; }
  bool IsLooped() const { return mIsLooped; }
  bool GetUseLoopDuration() const { return mUseLoopDuration; }
  float GetLoopDuration() const { return mLoopDuration; }

private:
  int mAnimId;
  bool mIsLooped : 1;
  bool mUseLoopDuration : 1;
  float mLoopDuration;
};
CHECK_SIZEOF(CBCScriptedCmd, 0x14)

class CBCCoverCmd : public CBodyStateCmd {
public:
  CBCCoverCmd(pas::ECoverDirection dir, const CVector3f& target, const CVector3f& alignDir)
  : CBodyStateCmd(kBSC_Cover), mDir(dir), mTargetPos(target), mAlignDir(alignDir) {}

  pas::ECoverDirection GetDirection() const { return mDir; }
  const CVector3f& GetTarget() const { return mTargetPos; }
  const CUnitVector3f GetAlignDirection() const {
    return CUnitVector3f(mAlignDir, CUnitVector3f::kN_No);
  }

private:
  pas::ECoverDirection mDir;
  CVector3f mTargetPos;
  CVector3f mAlignDir;
};
CHECK_SIZEOF(CBCCoverCmd, 0x24)

class CBCWallHangCmd : public CBodyStateCmd {
public:
  explicit CBCWallHangCmd(TUniqueId uid) : CBodyStateCmd(kBSC_WallHang), mWpId(uid) {}

  TUniqueId GetTarget() const { return mWpId; }

private:
  TUniqueId mWpId;
};
CHECK_SIZEOF(CBCWallHangCmd, 0xc)

class CBCAdditiveAimCmd : public CBodyStateCmd {
public:
  CBCAdditiveAimCmd() : CBodyStateCmd(kBSC_AdditiveAim), mAimType(0) {}

  int GetAimType() const { return mAimType; } // Guessed name

private:
  int mAimType;
};
CHECK_SIZEOF(CBCAdditiveAimCmd, 0xc)

class CBCAdditiveFlinchCmd : public CBodyStateCmd {
public:
  explicit CBCAdditiveFlinchCmd(float weight)
  : CBodyStateCmd(kBSC_AdditiveFlinch), mWeight(weight), mAnim(-1) {}

  float GetWeight() const { return mWeight; }

  int GetAnim() const { return mAnim; }

private:
  float mWeight;
  int mAnim;
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
public:
  CBCAdditiveLoopReactionCmd(int type, float weight)
  : CBodyStateCmd(kBSC_AdditiveLoopReaction), mWeight(weight), mType(type) {}

private:
  float mWeight;
  int mType;
};
CHECK_SIZEOF(CBCAdditiveLoopReactionCmd, 0x10)

// Guessed name
class CBCAdditiveWeightCmd : public CBodyStateCmd {
public:
  CBCAdditiveWeightCmd() : CBodyStateCmd(kBSC_AdditiveWeight), mWeight(0.f) {}

  float GetWeight() const { return mWeight; }

private:
  float mWeight;
};
CHECK_SIZEOF(CBCAdditiveWeightCmd, 0xc)

class CBCLocomotionCmd {
public:
  CBCLocomotionCmd(const CVector3f& move, const CVector3f& face, float weight)
  : mMove(move), mFace(face), mWeight(weight) {}

  const CVector3f& GetMoveVector() const { return mMove; }

  const CVector3f& GetFaceVector() const { return mFace; }

  float GetWeight() const { return mWeight; }

private:
  CVector3f mMove;
  CVector3f mFace;
  float mWeight;
};
CHECK_SIZEOF(CBCLocomotionCmd, 0x1c)

class CBodyStateCmdMgr {
public:
  CBodyStateCmdMgr();
  ~CBodyStateCmdMgr();

  void DeliverCmd(const CBodyStateCmd& cmd);
  void DeliverCmd(const CBCLocomotionCmd& cmd);
  void DeliverCmd(EBodyStateCmd cmd);
  void DeliverCmd(const CBCSlideCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mSlide = cmd;
  }
  void DeliverCmd(const CBCAdditiveReactionCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mAdditiveReaction = cmd;
  }

  void BlendSteeringCmds();
  void ClearLocomotionCmds();
  void SetSteeringSpeedRange(float minimum, float maximum);
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
  CBCAdditiveWeightCmd mAdditiveWeight;
  CBodyStateCmd x360_;
};
CHECK_SIZEOF(CBodyStateCmdMgr, 0x368)

#endif // _CBODYSTATECMDMGR
