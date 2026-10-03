#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include <float.h>

CPlayerBodyController::SDashState::SDashState() : mState(kS_Invalid), mAnimationVariant(-1) {}

void CPlayerBodyController::SDashState::Start(CStateManager& mgr,
                                           CPlayerBodyController& controller) {
  const CPBCDashCmd* command =
      static_cast< const CPBCDashCmd* >(controller.CommandMgr().GetCmd(kPBSC_Dash));
  if (command) {
    mAnimationVariant = command->GetAnimationVariant();
    const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Dash),
                                CPASAnimParm::FromEnum(mAnimationVariant),
                                CPASAnimParm::FromEnum(kAP_IntoDash));
    const rstl::pair< float, int > best =
        controller.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > FLT_EPSILON) {
      controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
      mState = kS_IntoDash;
    } else {
      PlayLoop(mgr, controller);
    }
  } else {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  }
}

bool CPlayerBodyController::SDashState::Update(CStateManager& mgr,
                                            CPlayerBodyController& controller) {
  switch (mState) {
  case kS_IntoDash:
    if (!controller.CommandMgr().GetCmd(kPBSC_Jump)) {
      PlayExit(mgr, controller);
    } else if (controller.IsAnimationOver()) {
      PlayLoop(mgr, controller);
    }
    break;
  case kS_Loop:
    if (!controller.CommandMgr().GetCmd(kPBSC_Jump)) {
      PlayExit(mgr, controller);
    }
    break;
  case kS_Exit:
    if (controller.IsAnimationOver()) {
      mState = kS_Invalid;
    } else {
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

void CPlayerBodyController::SDashState::Shutdown(CPlayerBodyController& controller) {}

void CPlayerBodyController::SDashState::PlayLoop(CStateManager& mgr,
                                              CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Dash),
                              CPASAnimParm::FromEnum(mAnimationVariant),
                              CPASAnimParm::FromEnum(kAP_Loop));
  const rstl::pair< float, int > best =
      controller.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true, false);
    mState = kS_Loop;
  } else {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  }
}

void CPlayerBodyController::SDashState::PlayExit(CStateManager& mgr,
                                              CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Dash),
                              CPASAnimParm::FromEnum(mAnimationVariant),
                              CPASAnimParm::FromEnum(kAP_Exit));
  const rstl::pair< float, int > best =
      controller.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    mState = kS_Exit;
  } else {
    mState = kS_Invalid;
  }
}
