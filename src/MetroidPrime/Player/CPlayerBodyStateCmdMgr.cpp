#include "MetroidPrime/Player/CPlayerBodyStateCmdMgr.hpp"

CPlayerBodyStateCmdMgr::CPlayerBodyStateCmdMgr()
: mCommands(nullptr)
, mDelivered(false)
, mLocomotion(CVector3f::Zero(), CVector3f::Zero())
, mMorphToScrewAttack(-1, -1)
, mMorphToBall(-1, -1)
, mMorphToPlayer(-1, -1)
, mJump(-1, 0)
, mGrapple(-1)
, mDoubleJump(kPBSC_DoubleJump)
, mDash(-1)
, mKnockBack(CVector3f::Zero())
, mDeathReaction(CVector3f::Zero(), CPBCDeathReactionCmd::kDRM_Invalid)
, mGibDeath(kPBSC_GibDeath)
, mContinueLocomotion(kPBSC_ContinueLocomotion)
, mHardLanding(kPBSC_HardLanding)
, mAim(CVector3f::Zero())
, mFlinch(CVector3f::Zero())
, mAdditiveReaction(CPBCAdditiveReactionCmd::kART_Invalid, false) {
  mCommands[kPBSC_Locomotion] = &mLocomotion;
  mCommands[kPBSC_MorphToScrewAttack] = &mMorphToScrewAttack;
  mCommands[kPBSC_MorphToBall] = &mMorphToBall;
  mCommands[kPBSC_MorphToPlayer] = &mMorphToPlayer;
  mCommands[kPBSC_Jump] = &mJump;
  mCommands[kPBSC_Grapple] = &mGrapple;
  mCommands[kPBSC_DoubleJump] = &mDoubleJump;
  mCommands[kPBSC_Dash] = &mDash;
  mCommands[kPBSC_KnockBack] = &mKnockBack;
  mCommands[kPBSC_DeathReaction] = &mDeathReaction;
  mCommands[kPBSC_GibDeath] = &mGibDeath;
  mCommands[kPBSC_ContinueLocomotion] = &mContinueLocomotion;
  mCommands[kPBSC_HardLanding] = &mHardLanding;
  mCommands[kPBSC_Aim] = &mAim;
  mCommands[kPBSC_Flinch] = &mFlinch;
  mCommands[kPBSC_AdditiveReaction] = &mAdditiveReaction;
}

CPlayerBodyStateCmdMgr::~CPlayerBodyStateCmdMgr() {}

void CPlayerBodyStateCmdMgr::DeliverCmd(const CPlayerBodyStateCmd& command) {
  DeliverCmd(command.GetCommandId());
  *mCommands[command.GetCommandId()] = command;
}

void CPlayerBodyStateCmdMgr::ClearCmds() {
  for (rstl::reserved_vector< bool, 16 >::iterator it = mDelivered.begin();
       it != mDelivered.end(); ++it) {
    *it = false;
  }
}

const CPlayerBodyStateCmd* CPlayerBodyStateCmdMgr::GetCmd(EPlayerBodyStateCmd command) const {
  if (mDelivered[command]) {
    return mCommands[command];
  }
  return nullptr;
}

void CPlayerBodyStateCmdMgr::DeliverCmd(EPlayerBodyStateCmd command) {
  mDelivered[command] = true;
}
