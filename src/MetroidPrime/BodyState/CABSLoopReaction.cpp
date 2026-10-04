#include "MetroidPrime/BodyState/CABSLoopReaction.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

CABSLoopReaction::CABSLoopReaction()
: mWeight(1.f), mType(-1), mState(pas::kLS_Invalid), mAnimationId(-1) {}

void CABSLoopReaction::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveLoopReactionCmd* cmd = static_cast< const CBCAdditiveLoopReactionCmd* >(
      bc.CommandMgr().GetCmd(kBSC_AdditiveLoopReaction));
  mWeight = cmd->GetWeight();
  mType = cmd->GetType();
  mState = pas::kLS_Begin;
  mAnimationId = -1;

  if (!SelectAnimation(bc, mgr, pas::kLS_Begin) && !SelectAnimation(bc, mgr, pas::kLS_Loop))
    mState = pas::kLS_Invalid;
}

pas::EAnimationState CABSLoopReaction::UpdateBody(float dt, CBodyController& bc,
                                                CStateManager& mgr) {
  UpdateWeight(bc);
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state != pas::kAS_Invalid)
    return state;

  CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
  switch (mState) {
  case pas::kLS_Begin:
    if (bc.CommandMgr().GetCmd(kBSC_AdditiveIdle)) {
      state = pas::kAS_AdditiveIdle;
    } else {
      const rstl::rc_ptr< CAnimTreeNode > tree = animData.GetAdditiveAnimationTree(mAnimationId);
      if (!tree || close_enough(tree->VGetTimeRemaining().GetSeconds(), dt)) {
        if (!SelectAnimation(bc, mgr, pas::kLS_Loop))
          state = pas::kAS_AdditiveIdle;
      }
    }
    break;
  case pas::kLS_Loop:
    if (bc.CommandMgr().GetCmd(kBSC_AdditiveIdle) && !SelectAnimation(bc, mgr, pas::kLS_End))
      state = pas::kAS_AdditiveIdle;
    break;
  case pas::kLS_End: {
    const rstl::rc_ptr< CAnimTreeNode > tree = animData.GetAdditiveAnimationTree(mAnimationId);
    if (!tree || close_enough(tree->VGetTimeRemaining().GetSeconds(), dt))
      state = pas::kAS_AdditiveIdle;
    break;
  }
  default:
    state = pas::kAS_AdditiveIdle;
    break;
  }
  return state;
}

void CABSLoopReaction::Shutdown(CBodyController& bc) {
  if (mAnimationId != -1) {
    bc.GetOwner().ModelData()->AnimationData()->DelAdditiveAnimation(mAnimationId);
    mAnimationId = -1;
  }
  mWeight = 0.f;
  mType = -1;
  mState = pas::kLS_Invalid;
}

pas::EAnimationState CABSLoopReaction::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
  if (commandMgr.GetCmd(kBSC_AdditiveReaction))
    return pas::kAS_AdditiveReaction;
  if (commandMgr.GetCmd(kBSC_AdditiveFlinch))
    return pas::kAS_AdditiveFlinch;
  if (commandMgr.GetCmd(kBSC_Unknown33))
    return pas::kAS_AdditiveIdle;
  return pas::kAS_Invalid;
}

bool CABSLoopReaction::SelectAnimation(CBodyController& bc, CStateManager& mgr,
                                     pas::ELoopState state) {
  if (mAnimationId != -1) {
    bc.GetOwner().ModelData()->AnimationData()->DelAdditiveAnimation(mAnimationId);
    mAnimationId = -1;
  }
  const CPASDatabase& db = bc.GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_AdditiveLoopReaction, CPASAnimParm::FromEnum(mType),
                               CPASAnimParm::FromEnum(state));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > 99.f) {
    mAnimationId = best.second;
    mState = state;
    bc.GetOwner().ModelData()->AnimationData()->AddAdditiveAnimation(
        mAnimationId, mWeight, mState == pas::kLS_Loop, true);
    return true;
  }
  return false;
}

void CABSLoopReaction::UpdateWeight(CBodyController& bc) {
  if (mAnimationId == -1)
    return;

  const CBCAdditiveWeightCmd* cmd =
      static_cast< const CBCAdditiveWeightCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveWeight));
  if (cmd) {
    mWeight = cmd->GetWeight();
    bc.GetOwner().ModelData()->AnimationData()->AddAdditiveAnimation(
        mAnimationId, mWeight, mState == pas::kLS_Loop, true);
  }
}
