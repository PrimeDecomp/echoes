#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include <float.h>

CPlayerBodyController::SJumpState::SJumpState()
: mState(kS_Invalid)
, mAnimationVariant(-1)
, mDoubleJumpStarted(false)
, mHardLandingPending(false) {}

void CPlayerBodyController::SJumpState::Start(CStateManager& mgr,
                                              CPlayerBodyController& controller) {
  CPlayerBodyStateCmdMgr& commandMgr = controller.CommandMgr();
  const CPBCJumpCmd* command = static_cast< const CPBCJumpCmd* >(commandMgr.GetCmd(kPBSC_Jump));
  if (command) {
    mAnimationVariant = command->GetAnimationVariant();
    mDoubleJumpStarted = false;
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                                 CPASAnimParm::FromEnum(mAnimationVariant),
                                 CPASAnimParm::FromEnum(kAP_IntoJump));
    const CPASDatabase& database = controller.GetPASDatabase();
    const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
    if (CMath::IsEpsilon(best.first, 100.f, 0.00001f)) {
      controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
      mState = kS_IntoJump;
    } else if (!PlayJump(mgr, controller)) {
      PlayJumpLoop(mgr, controller);
    }
  } else {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  }

  mHardLandingPending = commandMgr.GetCmd(kPBSC_HardLanding) != nullptr;
}

bool CPlayerBodyController::SJumpState::Update(CStateManager& mgr,
                                               CPlayerBodyController& controller) {
  CPlayerBodyStateCmdMgr& commandMgr = controller.CommandMgr();
  const CPlayerBodyStateCmd* jump = commandMgr.GetCmd(kPBSC_Jump);
  mHardLandingPending |= commandMgr.GetCmd(kPBSC_HardLanding) != nullptr;

  switch (mState) {
  case kS_IntoJump:
    if (controller.IsAnimationOver() && !PlayJump(mgr, controller)) {
      PlayJumpLoop(mgr, controller);
    }
    break;
  case kS_Jump:
    if (!jump) {
      PlayLanding(mgr, controller);
    } else if (controller.IsAnimationOver()) {
      PlayJumpLoop(mgr, controller);
    } else if (commandMgr.GetCmd(kPBSC_DoubleJump) && !mDoubleJumpStarted) {
      mDoubleJumpStarted = PlayDoubleJump(mgr, controller);
    }
    break;
  case kS_Loop:
    if (!jump) {
      PlayLanding(mgr, controller);
    }
    break;
  case kS_Landing:
    if (controller.IsAnimationOver()) {
      mState = kS_Invalid;
    } else if (!mHardLandingPending) {
      const CPBCLocomotionCmd* locomotion =
          static_cast< const CPBCLocomotionCmd* >(commandMgr.GetCmd(kPBSC_Locomotion));
      if (locomotion && locomotion->GetMovement().IsNonZero()) {
        commandMgr.DeliverCmd(CPlayerBodyStateCmd(kPBSC_ContinueLocomotion));
        mState = kS_Invalid;
      }
    }
    break;
  default:
    break;
  }
  return mState != kS_Invalid;
}

void CPlayerBodyController::SJumpState::Shutdown(CPlayerBodyController& controller) {}

bool CPlayerBodyController::SJumpState::PlayDoubleJump(CStateManager& mgr,
                                                       CPlayerBodyController& controller) {
  const CPlayer& player = controller.GetPlayer();
  const CVector3f forward = player.GetTransform().GetForward();
  const EDoubleJumpVariant variant =
      CVector3f::Dot(forward, player.GetVelocityWR()) >= 0.f ? kDJV_Forward : kDJV_Backward;
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(variant), CPASAnimParm::FromEnum(kAP_Jump));
  const CPASDatabase& database = controller.GetPASDatabase();
  const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    mState = kS_Jump;
    return true;
  }
  return false;
}

bool CPlayerBodyController::SJumpState::PlayJump(CStateManager& mgr,
                                                 CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Jump));
  const CPASDatabase& database = controller.GetPASDatabase();
  const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    mState = kS_Jump;
    return true;
  }
  return false;
}

void CPlayerBodyController::SJumpState::PlayJumpLoop(CStateManager& mgr,
                                                     CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Loop));
  const CPASDatabase& database = controller.GetPASDatabase();
  const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true, false);
    mState = kS_Loop;
  } else {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  }
}

void CPlayerBodyController::SJumpState::PlayLanding(CStateManager& mgr,
                                                    CPlayerBodyController& controller) {
  bool needLanding = true;
  if (mHardLandingPending && PlayHardLanding(mgr, controller)) {
    needLanding = false;
  }
  if (needLanding) {
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                                 CPASAnimParm::FromEnum(mAnimationVariant),
                                 CPASAnimParm::FromEnum(kAP_Landing));
    const CPASDatabase& database = controller.GetPASDatabase();
    const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > FLT_EPSILON) {
      controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
      mState = kS_Landing;
    } else {
      mState = kS_Invalid;
    }
  }
}

bool CPlayerBodyController::SJumpState::PlayHardLanding(CStateManager& mgr,
                                                        CPlayerBodyController& controller) {
  const CPASDatabase& database = controller.GetPASDatabase();
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_HardLanding));
  const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    mState = kS_Landing;
    return true;
  }
  return false;
}
