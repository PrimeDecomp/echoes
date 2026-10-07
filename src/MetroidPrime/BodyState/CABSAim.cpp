#include "MetroidPrime/BodyState/CABSAim.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "rstl/math.hpp"

CABSAim::CABSAim()
: mNeedsIdle(false)
, mAimType(-1)
, mHWeight(0.f)
, mHWeightVel(0.f)
, mVWeight(0.f)
, mVWeightVel(0.f) {}

void CABSAim::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveAimCmd* cmd =
      static_cast< const CBCAdditiveAimCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveAim));
  const CPASAnimState* aimState = bc.GetPASDatabase().GetAnimState(pas::kAS_AdditiveAim);
  if (mAimType != -1) {
    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    mHWeight = -animData.GetAdditiveAnimationWeight(mAnims[0]);
    mHWeight += animData.GetAdditiveAnimationWeight(mAnims[1]);
    mVWeight = -animData.GetAdditiveAnimationWeight(mAnims[3]);
    mVWeight += animData.GetAdditiveAnimationWeight(mAnims[2]);
  }

  mAimType = cmd->GetAimType();
  // Left, right, up, down.
  for (int i = 0; i < 4; ++i) {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(i),
                                 CPASAnimParm::FromEnum(mAimType));
    mAnims[i] = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1).second;
    const CPASAnimParm parm = aimState->GetAnimParmData(mAnims[i], 2);
    mAngles[i] = CRelAngle::FromDegrees(parm.GetReal32Value()).AsRadians();
  }

  mNeedsIdle = false;
  if (bc.CommandMgr().GetCmd(kBSC_AdditiveIdle)) {
    mNeedsIdle = true;
  }
}

pas::EAnimationState CABSAim::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    const CVector3f target = bc.CommandMgr().GetAdditiveTargetVector();
    if (target.CanBeNormalized()) {
      const float maximumVelocity = 3.f;
      const float maximumAcceleration = 10.f;
      float hAngle = atan2f(target.GetX(), target.GetY());
      hAngle = CMath::Clamp(-mAngles[0], hAngle, mAngles[1]) * (2.f / M_PIF);
      float velocity =
          CMath::Clamp(-maximumVelocity, (hAngle - mHWeight) * 0.25f / dt, maximumVelocity);
      float acceleration = (velocity - mHWeightVel) / dt;
      mHWeightVel += dt * CMath::Clamp(-maximumAcceleration, acceleration, maximumAcceleration);

      float vAngle = atan2f(target.GetZ(), CMath::SqrtF(target.GetY() * target.GetY() +
                                                       target.GetX() * target.GetX()));
      vAngle = CMath::Clamp(-mAngles[3], vAngle, mAngles[2]) * (2.f / M_PIF);
      velocity = CMath::Clamp(-maximumVelocity, (vAngle - mVWeight) * 0.25f / dt, maximumVelocity);
      acceleration = (velocity - mVWeightVel) / dt;
      mVWeightVel += dt * CMath::Clamp(-maximumAcceleration, acceleration, maximumAcceleration);

      const float newHWeight = mHWeight + dt * mHWeightVel;
      const float newVWeight = mVWeight + dt * mVWeightVel;
      CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
      if (newHWeight != mHWeight) {
        const float weight = CMath::AbsF(newHWeight);
        if (CMath::AbsF(mHWeight) > 0.f && mHWeight * newHWeight <= 0.f)
          animData.DelAdditiveAnimation(mAnims[mHWeight < 0.f ? 0 : 1]);
        if (weight > 0.f)
          animData.AddAdditiveAnimation(mAnims[newHWeight < 0.f ? 0 : 1], weight, false, false);
      }
      if (newVWeight != mVWeight) {
        const float weight = CMath::AbsF(newVWeight);
        if (CMath::AbsF(mVWeight) > 0.f && mVWeight * newVWeight <= 0.f)
          animData.DelAdditiveAnimation(mAnims[mVWeight > 0.f ? 2 : 3]);
        if (weight > 0.f)
          animData.AddAdditiveAnimation(mAnims[newVWeight > 0.f ? 2 : 3], weight, false, false);
      }
      mHWeight = newHWeight;
      mVWeight = newVWeight;
    } else {
      if (mHWeight < 0.f) {
        mHWeight = rstl::min_val(mHWeight + dt, 0.f);
      } else {
        mHWeight = rstl::max_val(mHWeight - dt, 0.f);
      }
      if (mVWeight < 0.f) {
        mVWeight = rstl::min_val(mVWeight + dt, 0.f);
      } else {
        mVWeight = rstl::max_val(mVWeight - dt, 0.f);
      }

      CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
      if (CMath::AbsF(mHWeight) > 0.f)
        animData.AddAdditiveAnimation(mAnims[mHWeight < 0.f ? 0 : 1], CMath::AbsF(mHWeight), false,
                                      true);
      // The native no-target path uses the opposite vertical selection from active aiming.
      if (CMath::AbsF(mVWeight) > 0.f)
        animData.AddAdditiveAnimation(mAnims[mVWeight < 0.f ? 2 : 3], CMath::AbsF(mVWeight), false,
                                      true);
    }
  }
  return state;
}

void CABSAim::Shutdown(CBodyController& bc) {
  CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
  if (mHWeight != 0.f)
    animData.DelAdditiveAnimation(mAnims[mHWeight < 0.f ? 0 : 1]);
  if (mVWeight != 0.f)
    animData.DelAdditiveAnimation(mAnims[mVWeight > 0.f ? 2 : 3]);
}

pas::EAnimationState CABSAim::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
  if (commandMgr.GetCmd(kBSC_AdditiveReaction))
    return pas::kAS_AdditiveReaction;
  if (commandMgr.GetCmd(kBSC_AdditiveFlinch))
    return pas::kAS_AdditiveFlinch;
  if (commandMgr.GetCmd(kBSC_AdditiveIdle) || mNeedsIdle)
    return pas::kAS_AdditiveIdle;

  const CBCAdditiveAimCmd* cmd =
      static_cast< const CBCAdditiveAimCmd* >(commandMgr.GetCmd(kBSC_AdditiveAim));
  if (cmd && cmd->GetAimType() != mAimType)
    return pas::kAS_AdditiveAim;
  return pas::kAS_Invalid;
}
