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

CPlayerBodyController::SAdditiveAimState::SAdditiveAimState(CActor& actor) : mCategory(CPlayerBodyController::SAdditiveAimState::kC_Default), mYawWeight(0.0f), mYawVelocity(0.0f), mPitchWeight(0.0f), mPitchVelocity(0.0f) {
    void* ptr;
    CPlayerBodyController::SAdditiveAimState* additiveAimState;
    CPlayerBodyController::SAdditiveAimState* additiveAimState2;
    int i;
    float f;
    memset(this, 0, 12);
    CPlayerBodyController::SAdditiveAimState* additiveAimState3 = this;
    i = 0;
    ptr = (char*)*(CAnimData**)((char*)*(CModelData**)((char*)&actor + 0x60) + 0x10) + 0x3c;
    do {
        additiveAimState = additiveAimState3;
        additiveAimState2 = this;
        int i2 = 0;
        do {
            CPASAnimParmData cpasAnimParmData(pas::kAS_Death, CPASAnimParm::FromEnum(i), CPASAnimParm::FromEnum(i2), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter());
            rstl::pair<float, int> bestAnimation = ((CPASDatabase*)ptr)->FindBestAnimation(cpasAnimParmData, -1);
            f = bestAnimation.first;
            *(int*)((char*)additiveAimState + 0xc) = bestAnimation.second;
            if (f > 1.1920929e-7f) {
                additiveAimState2->mAvailableAnimations[0] += 1;
            }
            i2++;
            additiveAimState = (CPlayerBodyController::SAdditiveAimState*)((char*)additiveAimState + 0x10);
            additiveAimState2 = (CPlayerBodyController::SAdditiveAimState*)(&additiveAimState2->mAvailableAnimations[1]);
        } while (i2 <= 2);
        i++;
        additiveAimState3->mYawLimits[0] = 1.4835298f;
        additiveAimState3 = (CPlayerBodyController::SAdditiveAimState*)(&additiveAimState3->mAvailableAnimations[1]);
    } while (i <= 3);
}

void CPlayerBodyController::SAdditiveAimState::Start(CStateManager& mgr,
                                                     CPlayerBodyController& controller) {
  if (mAvailableAnimations[mCategory] == 4) {
    CAnimData& animation = *controller.GetPlayer().AnimationData();
    mYawWeight = -animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_NegativeYaw]);
    mYawWeight += animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_PositiveYaw]);
    mPitchWeight =
        -animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_NegativePitch]);
    mPitchWeight +=
        animation.GetAdditiveAnimationWeight(mAnimationIds[mCategory][kD_PositivePitch]);
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
    CVector3f aim = command->GetDirection();
    if (mCategory == kC_LocomotionMode5) {
      aim.SetZ(-aim.GetZ());
    }
    direction = controller.GetPlayer().GetTransform().TransposeRotate(aim);
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
  const float pitch =
      CMath::Clamp(-mPitchLimits[1],
                   atan2f(direction.GetZ(), CMath::SqrtF(direction.GetY() * direction.GetY() +
                                                         direction.GetX() * direction.GetX())),
                   mPitchLimits[0]);
  const float velocity =
      CMath::Clamp(-3.f, 0.25f * ((2.f / M_PIF) * pitch - mPitchWeight) / dt, 3.f);
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
