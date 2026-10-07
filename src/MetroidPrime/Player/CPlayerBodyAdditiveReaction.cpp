#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

CPlayerBodyController::SAdditiveReactionState::SAdditiveReactionState()
: mAnimationId(-1), mType(CPBCAdditiveReactionCmd::kART_Invalid), mLooping(false) {}

void CPlayerBodyController::SAdditiveReactionState::Start(CStateManager& mgr,
                                                       CPlayerBodyController& controller) {
  const CPBCAdditiveReactionCmd* command = static_cast< const CPBCAdditiveReactionCmd* >(
      controller.CommandMgr().GetCmd(kPBSC_AdditiveReaction));
  if (command) {
    mType = command->GetType();
    mLooping = command->IsLooping();
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_AdditiveReaction),
                                CPASAnimParm::FromEnum(mType));
    const CPASDatabase& database = controller.GetPASDatabase();
    const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
    mAnimationId = best.second;
    if (mAnimationId != -1) {
      controller.GetPlayer().AnimationData()->AddAdditiveAnimation(mAnimationId, 1.f, mLooping,
                                                                 false);
    }
  } else {
    mAnimationId = -1;
    mType = CPBCAdditiveReactionCmd::kART_Invalid;
    mLooping = false;
  }
}

bool CPlayerBodyController::SAdditiveReactionState::Update(CStateManager& mgr,
                                                        CPlayerBodyController& controller) {
  if (mAnimationId == -1) {
    return false;
  }

  CAnimData& animation = *controller.GetPlayer().AnimationData();
  if (mLooping) {
    if (!controller.CommandMgr().GetCmd(kPBSC_AdditiveReaction)) {
      StopAnimation(controller);
      controller.GetPlayer().StopLoopedSounds();
    }
  } else {
    if (animation.IsAdditiveAnimationActive(mAnimationId)) {
      const rstl::rc_ptr< CAnimTreeNode > tree = animation.GetAdditiveAnimationTree(mAnimationId);
      if (tree && close_enough(tree->VGetTimeRemaining().GetSeconds(), 0.f)) {
        StopAnimation(controller);
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

void CPlayerBodyController::SAdditiveReactionState::Shutdown(CPlayerBodyController& controller) {
  StopAnimation(controller);
}

void CPlayerBodyController::SAdditiveReactionState::StopAnimation(
    CPlayerBodyController& controller) {
  if (mAnimationId != -1) {
    controller.GetPlayer().AnimationData()->DelAdditiveAnimation(mAnimationId);
    mAnimationId = -1;
    mType = CPBCAdditiveReactionCmd::kART_Invalid;
    mLooping = false;
  }
}
