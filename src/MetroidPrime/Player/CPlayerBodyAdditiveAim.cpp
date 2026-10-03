#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include <float.h>
#include <string.h>

CPlayerBodyController::SAdditiveAimState::SAdditiveAimState(CActor& actor)
: mCategory(kC_Default)
, mYawWeight(0.f)
, mYawVelocity(0.f)
, mPitchWeight(0.f)
, mPitchVelocity(0.f) {
  memset(mAvailableAnimations, 0, sizeof(mAvailableAnimations));
  const CPASDatabase& database = actor.AnimationData()->GetPASDatabase();
  for (int direction = 0; direction < 4; ++direction) {
    for (int category = 0; category < 3; ++category) {
      const CPASAnimParmData parms(static_cast< pas::EAnimationState >(kPAS_AdditiveAim),
                                  CPASAnimParm::FromEnum(direction),
                                  CPASAnimParm::FromEnum(category));
      const rstl::pair< float, int > best = database.FindBestAnimation(parms, -1);
      mAnimationIds[category][direction] = best.second;
      if (best.first > FLT_EPSILON) {
        ++mAvailableAnimations[category];
      }
    }

    if (direction < 2) {
      mYawLimits[direction] = CRelAngle::FromDegrees(85.f).AsRadians();
    } else {
      mPitchLimits[direction - 2] = CRelAngle::FromDegrees(85.f).AsRadians();
    }
  }
}

void CPlayerBodyController::SAdditiveAimState::Start(CStateManager& mgr,
                                                  CPlayerBodyController& controller) {
  if (mAvailableAnimations[mCategory] == 4) {
    CAnimData& animation = *controller.GetPlayer().AnimationData();
    mYawWeight = -animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_NegativeYaw]);
    mYawWeight += animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_PositiveYaw]);
    mPitchWeight = -animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_NegativePitch]);
    mPitchWeight += animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_PositivePitch]);
  }

  switch (controller.GetLocomotionMode()) {
  case 4:
    mCategory = kC_LocomotionMode4;
    break;
  case 5:
    mCategory = kC_LocomotionMode5;
    break;
  default:
    mCategory = kC_Default;
    break;
  }
}

void CPlayerBodyController::SAdditiveAimState::Update(float dt, CStateManager& mgr,
                                                   CPlayerBodyController& controller) {
  if (mAvailableAnimations[mCategory] != 4) {
    return;
  }

  const CPBCAimCmd* command =
      static_cast< const CPBCAimCmd* >(controller.CommandMgr().GetCmd(kPBSC_Aim));
  CVector3f direction = CVector3f::Zero();
  if (command) {
    direction = command->GetDirection();
    if (mCategory == kC_LocomotionMode5) {
      direction.SetZ(-direction.GetZ());
    }
    direction = controller.GetPlayer().GetTransform().TransposeRotate(direction);
  }

  if (direction.CanBeNormalized()) {
    UpdatePitch(dt, direction, controller);
  }
}

void CPlayerBodyController::SAdditiveAimState::Shutdown(CPlayerBodyController& controller) {
  if (mAvailableAnimations[mCategory] != 4) {
    return;
  }

  CAnimData& animation = *controller.GetPlayer().AnimationData();
  if (mYawWeight != 0.f) {
    animation.DelAdditiveAnimation(
        mAnimationIds[mCategory][mYawWeight < 0.f ? kD_NegativeYaw : kD_PositiveYaw]);
  }
  if (mPitchWeight != 0.f) {
    animation.DelAdditiveAnimation(
        mAnimationIds[mCategory][mPitchWeight > 0.f ? kD_PositivePitch : kD_NegativePitch]);
  }
}

void CPlayerBodyController::SAdditiveAimState::UpdatePitch(float dt, const CVector3f& direction,
                                                        CPlayerBodyController& controller) {
  CAnimData& animation = *controller.GetPlayer().AnimationData();
  const float pitch = CMath::Clamp(
      -mPitchLimits[1],
      atan2f(direction.GetZ(), CMath::SqrtF(direction.GetY() * direction.GetY() +
                                         direction.GetX() * direction.GetX())),
      mPitchLimits[0]);
  const float velocity = CMath::Clamp(-3.f, 0.25f * ((2.f / M_PIF) * pitch - mPitchWeight) / dt, 3.f);
  const float acceleration = CMath::Clamp(-10.f, (velocity - mPitchVelocity) / dt, 10.f);
  mPitchVelocity += dt * acceleration;
  const float weight = mPitchWeight + dt * mPitchVelocity;
  if (weight != mPitchWeight) {
    if (CMath::AbsF(mPitchWeight) > 0.f && mPitchWeight * weight <= 0.f) {
      animation.DelAdditiveAnimation(
          mAnimationIds[mCategory][mPitchWeight > 0.f ? kD_PositivePitch : kD_NegativePitch]);
    }
    if (CMath::AbsF(weight) > 0.f) {
      animation.AddAdditiveAnimation(
          mAnimationIds[mCategory][weight > 0.f ? kD_PositivePitch : kD_NegativePitch],
          CMath::AbsF(weight), false, false);
    }
  }
  mPitchWeight = weight;
}
