#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include <float.h>

CPlayerBodyController::SGrappleState::SGrappleState() : mState(kS_Invalid), mAnimationVariant(-1) {}

void CPlayerBodyController::SGrappleState::Start(CStateManager& mgr,
                                                 CPlayerBodyController& controller) {
  const CPBCGrappleCmd* command =
      static_cast< const CPBCGrappleCmd* >(controller.CommandMgr().GetCmd(kPBSC_Grapple));
  const CPlayer* player = TCastToPtr< CPlayer >(&controller.GetPlayer());
  if (command && player) {
    mAnimationVariant = command->GetAnimationVariant();
    switch (player->GetGrappleState()) {
    case CPlayer::kGS_Firing:
      if (!TryPlayFiring(mgr, controller) && !TryPlayPull(mgr, controller)) {
        PlaySwing(mgr, controller);
      }
      break;
    case CPlayer::kGS_Pull:
      if (!TryPlayPull(mgr, controller)) {
        PlaySwing(mgr, controller);
      }
      break;
    case CPlayer::kGS_Swinging:
    default:
      PlaySwing(mgr, controller);
      break;
    }
  } else {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  }
}

bool CPlayerBodyController::SGrappleState::Update(CStateManager& mgr,
                                                  CPlayerBodyController& controller) {
  const CPlayerBodyStateCmd* command = controller.CommandMgr().GetCmd(kPBSC_Grapple);
  const CPlayer* player = TCastToPtr< CPlayer >(&controller.GetPlayer());
  if (!command || !player) {
    mState = kS_Invalid;
  }

  switch (mState) {
  case kS_Firing:
    if (player->GetPlayerMovementState() == NPlayer::kMS_ApplyJump ||
        player->GetPlayerMovementState() == NPlayer::kMS_Jump) {
      if (!TryPlayPull(mgr, controller)) {
        PlaySwing(mgr, controller);
      }
    }
    break;
  case kS_Pull:
    if (player->GetGrappleState() == CPlayer::kGS_Swinging) {
      PlaySwing(mgr, controller);
    }
    break;
  default:
    break;
  }
  return mState != kS_Invalid;
}

void CPlayerBodyController::SGrappleState::Shutdown(CPlayerBodyController& controller) {}

bool CPlayerBodyController::SGrappleState::TryPlayFiring(CStateManager& mgr,
                                                         CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Grapple),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Firing));
  const CPASDatabase& db = controller.GetPASDatabase();
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true, false);
    mState = kS_Firing;
    return true;
  }
  return false;
}

bool CPlayerBodyController::SGrappleState::TryPlayPull(CStateManager& mgr,
                                                       CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Grapple),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Pull));
  const CPASDatabase& db = controller.GetPASDatabase();
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true, false);
    mState = kS_Pull;
    return true;
  }
  return false;
}

void CPlayerBodyController::SGrappleState::PlaySwing(CStateManager& mgr,
                                                     CPlayerBodyController& controller) {
  const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Grapple),
                               CPASAnimParm::FromEnum(mAnimationVariant),
                               CPASAnimParm::FromEnum(kAP_Swinging));
  const CPASDatabase& db = controller.GetPASDatabase();
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    controller.RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true, false);
    mState = kS_Swinging;
  } else {
    mAnimationVariant = -1;
    mState = kS_Invalid;
  }
}
