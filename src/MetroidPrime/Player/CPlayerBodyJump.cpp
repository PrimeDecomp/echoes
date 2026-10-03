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
  const CPBCJumpCmd* command =
      static_cast< const CPBCJumpCmd* >(controller.CommandMgr().GetCmd(kPBSC_Jump));
  if (command) {
    mAnimationVariant = command->GetAnimationVariant();
    mDoubleJumpStarted = false;
    const CPASDatabase& database = controller.GetPASDatabase();
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                                 CPASAnimParm::FromEnum(mAnimationVariant),
                                 CPASAnimParm::FromEnum(kAP_IntoJump));
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

  mHardLandingPending = controller.CommandMgr().GetCmd(kPBSC_HardLanding) != nullptr;
}

bool CPlayerBodyController::SJumpState::Update(CStateManager& mgr,
                                               CPlayerBodyController& controller) {
  const CPlayerBodyStateCmd* jump = controller.CommandMgr().GetCmd(kPBSC_Jump);
  mHardLandingPending |= controller.CommandMgr().GetCmd(kPBSC_HardLanding) != nullptr;

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
    } else if (controller.CommandMgr().GetCmd(kPBSC_DoubleJump) && !mDoubleJumpStarted) {
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
          static_cast< const CPBCLocomotionCmd* >(controller.CommandMgr().GetCmd(kPBSC_Locomotion));
      if (locomotion && locomotion->GetMovement().IsNonZero()) {
        controller.CommandMgr().DeliverCmd(CPlayerBodyStateCmd(kPBSC_ContinueLocomotion));
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
  const EDoubleJumpVariant variant =
      CVector3f::Dot(player.GetTransform().GetForward(), player.GetVelocityWR()) < 0.f
          ? kDJV_Backward
          : kDJV_Forward;
  const CPASDatabase& database = controller.GetPASDatabase();
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(variant), CPASAnimParm::FromEnum(kAP_Jump));
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
  const CPASDatabase& database = controller.GetPASDatabase();
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Jump));
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
  const CPASDatabase& database = controller.GetPASDatabase();
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Loop));
  const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
  if (!(best.first > FLT_EPSILON)) {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  } else {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true, false);
    mState = kS_Loop;
  }
}

void CPlayerBodyController::SJumpState::PlayLanding(CStateManager& mgr,
                                                    CPlayerBodyController& controller) {
  if (mHardLandingPending && PlayHardLanding(mgr, controller)) {
    return;
  }

  const CPASDatabase& database = controller.GetPASDatabase();
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Jump),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Landing));
  const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
  if (!(best.first > FLT_EPSILON)) {
    mState = kS_Invalid;
  } else {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    mState = kS_Landing;
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
