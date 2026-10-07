#include "MetroidPrime/BodyState/CABSReaction.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

CABSReaction::CABSReaction() : mWeight(1.f), mAnim(-1), mType(pas::kART_Invalid), mActive(false) {}

void CABSReaction::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveReactionCmd* cmd =
      static_cast< const CBCAdditiveReactionCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveReaction));
  mWeight = cmd->GetWeight();
  mType = cmd->GetType();
  mActive = cmd->GetIsActive();

  const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(mType));
  const CPASDatabase& db = bc.GetPASDatabase();
  mAnim = db.FindBestAnimation(parms, *mgr.Random(), -1).second;
  if (mAnim != -1) {
    const bool active = mActive;
    bc.GetOwner().ModelData()->AnimationData()->AddAdditiveAnimation(mAnim, mWeight, active,
                                                                     false);
  }
}

void CABSReaction::UpdateWeight(CBodyController& bc) {
  const CBCAdditiveWeightCmd* cmd =
      static_cast< const CBCAdditiveWeightCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveWeight));
  if (cmd && mAnim != -1) {
    mWeight = cmd->GetWeight();
    const bool active = mActive;
    bc.GetOwner().ModelData()->AnimationData()->AddAdditiveAnimation(mAnim, mWeight, active,
                                                                     false);
  }
}

pas::EAnimationState CABSReaction::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  UpdateWeight(bc);
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    if (mAnim == -1)
      return pas::kAS_AdditiveIdle;

    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    if (mActive) {
      if (bc.CommandMgr().GetCmd(kBSC_StopReaction)) {
        StopAnimation(bc);
        state = pas::kAS_AdditiveIdle;
        bc.GetOwner().StopLoopedSounds();
      }
    } else if (animData.IsAdditiveAnimationActive(mAnim)) {
      const rstl::rc_ptr< CAnimTreeNode > tree = animData.GetAdditiveAnimationTree(mAnim);
      if (close_enough(tree->VGetTimeRemaining().GetSeconds(), 0.f)) {
        StopAnimation(bc);
        state = pas::kAS_AdditiveIdle;
      }
    } else {
      state = pas::kAS_AdditiveIdle;
    }
  }
  return state;
}

void CABSReaction::Shutdown(CBodyController& bc) { StopAnimation(bc); }

bool CBodyController::HasIceBreakoutState() {
  const CPASAnimParmData parms(pas::kAS_AdditiveReaction,
                               CPASAnimParm::FromEnum(pas::kART_IceBreakout));
  const CPASDatabase& db = GetPASDatabase();
  return db.FindBestAnimation(parms, -1).first > 0.f;
}

pas::EAnimationState CABSReaction::GetBodyStateTransition(float dt, CBodyController& bc) {
  const CBCAdditiveReactionCmd* cmd =
      static_cast< const CBCAdditiveReactionCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveReaction));
  if (cmd && cmd->GetType() == pas::kART_IceBreakout)
    return pas::kAS_AdditiveReaction;
  return pas::kAS_Invalid;
}

void CABSReaction::StopAnimation(CBodyController& bc) {
  if (mAnim != -1) {
    bc.GetOwner().ModelData()->AnimationData()->DelAdditiveAnimation(mAnim);
    mAnim = -1;
  }
}
