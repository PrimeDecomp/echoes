#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/math.hpp"

#include <float.h>

const CPlayerBodyController::SLocomotionState::ECategory
    CPlayerBodyController::SLocomotionState::skDirectionalCategories[4][3] = {
        {kC_ForwardSlow, kC_ForwardMedium, kC_ForwardFast},
        {kC_BackwardSlow, kC_BackwardMedium, kC_BackwardFast},
        {kC_LeftSlow, kC_LeftMedium, kC_LeftFast},
        {kC_RightSlow, kC_RightMedium, kC_RightFast}};

CPlayerBodyController::SLocomotionState::SLocomotionState(CActor& actor)
: mLocomotionMode(1)
, mPreviousLocomotionMode(1)
, mAnims(11, rstl::reserved_vector< rstl::pair< int, float >, 13 >(
                 13, rstl::pair< int, float >(0, 0.f)))
, mCategory(kC_Invalid)
, mPrimeTime(0.f)
, mAnimationChangeDisabled(false) {
  const CVector3f scale = actor.GetModelData()->GetScale();
  const CPASDatabase& database = actor.GetAnimationData()->GetPASDatabase();
  for (int mode = 0; mode < 11; ++mode) {
    for (int category = 0; category < 13; ++category) {
      // The player's PAS state 0 is locomotion, unlike the AI PAS domain.
      const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_Locomotion),
                                   CPASAnimParm::FromEnum(mode), CPASAnimParm::FromEnum(category));
      const rstl::pair< float, int > best = database.FindBestAnimation(parms, -1);
      float speed = 0.f;
      if (best.second != -1) {
        speed = actor.GetAverageAnimVelocity(best.second);
        speed = category != 0 ? scale.GetY() * speed : 0.f;
      }
      mAnims[mode][category] = rstl::pair< int, float >(best.second, speed);
    }
  }
}

void CPlayerBodyController::SLocomotionState::SetLocomotionMode(int mode) {
  mLocomotionMode = mode;
}

void CPlayerBodyController::SLocomotionState::SetAnimationChangeDisabled(bool disabled) {
  mAnimationChangeDisabled = disabled;
}

void CPlayerBodyController::SLocomotionState::Start(CStateManager& mgr,
                                                    CPlayerBodyController& controller) {
  mPrimeTime = 0.f;
  mPreviousLocomotionMode = mLocomotionMode;
  UpdateAnimation(controller, true);
}

void CPlayerBodyController::SLocomotionState::Update(float dt, CStateManager& mgr,
                                                     CPlayerBodyController& controller) {
  if (mPrimeTime < 0.2f) {
    mPrimeTime += dt;
  }

  const int previousMode = mPreviousLocomotionMode;
  mPreviousLocomotionMode = mLocomotionMode;
  UpdateAnimation(controller, previousMode != mLocomotionMode);
}

void CPlayerBodyController::SLocomotionState::Shutdown(CPlayerBodyController& controller) {
  controller.MultiplyPlaybackRate(1.f);
}

float CPlayerBodyController::SLocomotionState::ComputeWeightPercentage(
    float speed, const rstl::pair< int, float >& lower,
    const rstl::pair< int, float >& upper) const {
  const float delta = upper.second - lower.second;
  if (delta > FLT_EPSILON) {
    return rstl::max_val(rstl::min_val((speed - lower.second) / delta, 1.f), 0.f);
  }
  return 0.f;
}

void CPlayerBodyController::SLocomotionState::UpdateAnimation(CPlayerBodyController& controller,
                                                              bool force) {
  if (force || (mPrimeTime >= 0.2f && !mAnimationChangeDisabled)) {
    const ECategory previous = force ? kC_Invalid : mCategory;
    float speed = 0.f;
    const CPBCLocomotionCmd* command =
        static_cast< const CPBCLocomotionCmd* >(controller.CommandMgr().GetCmd(kPBSC_Locomotion));
    if (command) {
      speed = command->GetMovement().Magnitude();
    } else if (controller.CommandMgr().GetCmd(kPBSC_ContinueLocomotion)) {
      if (CPlayer* player = TCastToPtr< CPlayer >(&controller.GetPlayer())) {
        speed = player->GetDampedClampedVelocityWR().ToVec2f().Magnitude();
      }
    }

    if (speed < 0.01f) {
      UpdateIdle(controller, previous);
    } else {
      if (IsStrafing(controller)) {
        UpdateStrafe(speed, controller, previous);
      } else {
        UpdateDirectional(speed, controller, previous, kD_Forward);
      }
    }
  }
}

const rstl::pair< int, float >&
CPlayerBodyController::SLocomotionState::GetLocoAnimation(int mode, ECategory category) const {
  return mAnims[mode][category];
}

bool CPlayerBodyController::SLocomotionState::IsStrafing(
    const CPlayerBodyController& controller) const {
  const CPBCLocomotionCmd* command =
      static_cast< const CPBCLocomotionCmd* >(controller.CommandMgr().GetCmd(kPBSC_Locomotion));
  return command && command->GetFacing().IsNonZero() && command->GetMovement().IsNonZero();
}

void CPlayerBodyController::SLocomotionState::UpdateStrafe(float speed,
                                                           CPlayerBodyController& controller,
                                                           ECategory previous) {
  const CPBCLocomotionCmd* command =
      static_cast< const CPBCLocomotionCmd* >(controller.CommandMgr().GetCmd(kPBSC_Locomotion));
  CVector3f movement = command->GetMovement();
  movement = controller.GetPlayer().GetTransform().TransposeRotate(movement);
  const CVector3f squared = CVector3f::ByElementMultiply(movement, movement);
  EDirection direction;
  if (squared.GetX() <= squared.GetY()) {
    direction = movement.GetY() < 0.f ? kD_Backward : kD_Forward;
  } else {
    direction = movement.GetX() < 0.f ? kD_Left : kD_Right;
  }
  UpdateDirectional(speed, controller, previous, direction);
}

void CPlayerBodyController::SLocomotionState::UpdateIdle(CPlayerBodyController& controller,
                                                         ECategory previous) {
  if (previous != kC_Idle) {
    const rstl::pair< int, float >& idle = GetLocoAnimation(mPreviousLocomotionMode, kC_Idle);
    if (idle.first != controller.GetCurrentAnimationId() || !controller.IsAnimationLooping()) {
      controller.RequestAnimation(CAnimPlaybackParms(idle.first, -1, 1.f, true), true, false);
      mPrimeTime = 0.f;
    }
    mCategory = kC_Idle;
  }
}

void CPlayerBodyController::SLocomotionState::UpdateDirectional(float speed,
                                                                CPlayerBodyController& controller,
                                                                ECategory previous,
                                                                EDirection direction) {
  const rstl::pair< int, float >& slow =
      GetLocoAnimation(mPreviousLocomotionMode, skDirectionalCategories[direction][0]);
  const rstl::pair< int, float >& medium =
      GetLocoAnimation(mPreviousLocomotionMode, skDirectionalCategories[direction][1]);
  if (speed < slow.second) {
    UpdateSlow(speed, controller, previous, direction);
  } else if (speed < medium.second) {
    UpdateMedium(speed, controller, previous, direction);
  } else {
    UpdateFast(speed, controller, previous, direction);
  }
}

void CPlayerBodyController::SLocomotionState::UpdateSlow(float speed,
                                                         CPlayerBodyController& controller,
                                                         ECategory previous, EDirection direction) {
  const ECategory category = skDirectionalCategories[direction][0];
  if (previous != category) {
    const rstl::pair< int, float >& slow = GetLocoAnimation(mPreviousLocomotionMode, category);
    if (slow.first != controller.GetCurrentAnimationId()) {
      controller.RequestAnimation(CAnimPlaybackParms(slow.first, -1, 1.f, true), true, false);
      mPrimeTime = 0.f;
    }
    mCategory = category;
  }

  const rstl::pair< int, float >& idle = GetLocoAnimation(mPreviousLocomotionMode, kC_Idle);
  const rstl::pair< int, float >& slow = GetLocoAnimation(mPreviousLocomotionMode, category);
  controller.MultiplyPlaybackRate(rstl::max_val(0.f, ComputeWeightPercentage(speed, idle, slow)));
}

void CPlayerBodyController::SLocomotionState::UpdateMedium(float speed,
                                                           CPlayerBodyController& controller,
                                                           ECategory previous,
                                                           EDirection direction) {
  const ECategory slowCategory = skDirectionalCategories[direction][0];
  const ECategory mediumCategory = skDirectionalCategories[direction][1];
  const rstl::pair< int, float >& slow = GetLocoAnimation(mPreviousLocomotionMode, slowCategory);
  const rstl::pair< int, float >& medium =
      GetLocoAnimation(mPreviousLocomotionMode, mediumCategory);
  const float weight = ComputeWeightPercentage(speed, slow, medium);
  if (weight < 0.3f) {
    const float rate = slow.second > 0.f ? speed / slow.second : 1.f;
    if (previous != slowCategory && slow.first != controller.GetCurrentAnimationId()) {
      controller.RequestAnimation(CAnimPlaybackParms(slow.first, -1, 1.f, true), true, false);
      mPrimeTime = 0.f;
    }
    controller.MultiplyPlaybackRate(rate);
    mCategory = slowCategory;
  } else {
    const float rate = rstl::min_val(speed / medium.second, 1.f);
    if (previous != mediumCategory && medium.first != controller.GetCurrentAnimationId()) {
      controller.RequestAnimation(CAnimPlaybackParms(medium.first, -1, 1.f, true), true, false);
      mPrimeTime = 0.f;
    }
    controller.MultiplyPlaybackRate(rate);
    mCategory = mediumCategory;
  }
}

void CPlayerBodyController::SLocomotionState::UpdateFast(float speed,
                                                         CPlayerBodyController& controller,
                                                         ECategory previous, EDirection direction) {
  const ECategory mediumCategory = skDirectionalCategories[direction][1];
  const ECategory fastCategory = skDirectionalCategories[direction][2];
  const rstl::pair< int, float >& medium =
      GetLocoAnimation(mPreviousLocomotionMode, mediumCategory);
  const rstl::pair< int, float >& fast = GetLocoAnimation(mPreviousLocomotionMode, fastCategory);
  const float weight = ComputeWeightPercentage(speed, medium, fast);
  if (weight < 0.6f) {
    const float rate = medium.second > 0.f ? speed / medium.second : 1.f;
    if (previous != mediumCategory && medium.first != controller.GetCurrentAnimationId()) {
      controller.RequestAnimation(CAnimPlaybackParms(medium.first, -1, 1.f, true), true, false);
      mPrimeTime = 0.f;
    }
    controller.MultiplyPlaybackRate(rate);
    mCategory = mediumCategory;
  } else {
    const float rate = rstl::min_val(speed / fast.second, 1.f);
    if (previous != fastCategory && fast.first != controller.GetCurrentAnimationId()) {
      controller.RequestAnimation(CAnimPlaybackParms(fast.first, -1, 1.f, true), true, false);
      mPrimeTime = 0.f;
    }
    controller.MultiplyPlaybackRate(rate);
    mCategory = fastCategory;
  }
}
