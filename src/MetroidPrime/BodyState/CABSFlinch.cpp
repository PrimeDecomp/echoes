#include "MetroidPrime/BodyState/CABSFlinch.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

CABSFlinch::CABSFlinch() : mWeight(1.f), mAnim(0) {}

void CABSFlinch::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveFlinchCmd* cmd =
      static_cast< const CBCAdditiveFlinchCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveFlinch));
  mWeight = cmd->GetWeight();
  mAnim = cmd->GetAnim();
  if (mAnim == -1) {
    const CPASDatabase& db = bc.GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
    mAnim = db.FindBestAnimation(parms, *mgr.Random(), -1).second;
  }

  bc.GetOwner().ModelData()->AnimationData()->AddAdditiveAnimation(mAnim, mWeight, false, true);
}

void CABSFlinch::UpdateWeight(CBodyController& bc) {
  if (mAnim == -1)
    return;

  const CBCAdditiveWeightCmd* cmd =
      static_cast< const CBCAdditiveWeightCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveWeight));
  if (cmd) {
    mWeight = cmd->GetWeight();
    bc.GetOwner().ModelData()->AnimationData()->AddAdditiveAnimation(mAnim, mWeight, false, true);
  }
}

pas::EAnimationState CABSFlinch::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  UpdateWeight(bc);
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    const rstl::rc_ptr< CAnimTreeNode > tree =
        bc.GetOwner().GetModelData()->GetAnimationData()->GetAdditiveAnimationTree(mAnim);
    if (!tree || close_enough(tree->VGetTimeRemaining().GetSeconds(), 0.f))
      state = pas::kAS_AdditiveIdle;
  }
  return state;
}

void CABSFlinch::Shutdown(CBodyController& bc) {}

pas::EAnimationState CABSFlinch::GetBodyStateTransition(float dt, CBodyController& bc) const {
  if (bc.CommandMgr().GetCmd(kBSC_AdditiveReaction))
    return pas::kAS_AdditiveReaction;
  return pas::kAS_Invalid;
}
