#include "MetroidPrime/BodyState/CBSKnockBack.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

#include "math.h"
#include "rstl/math.hpp"

CBSKnockBack::CBSKnockBack() : mCurTime(0.f), mRotateSpeed(0.f), mRemTime(0.f) {}

void CBSKnockBack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockBackCmd* cmd =
      static_cast< const CBCKnockBackCmd* >(bc.CommandMgr().GetCmd(kBSC_KnockBack));

  CVector3f localDir = bc.GetOwner().GetTransform().TransposeRotate(cmd->GetHitDirection());
  const float angle = CMath::ClampRadians(atan2(localDir.GetY(), localDir.GetX()));
  const float angleDegrees = CMath::Rad2Deg(angle);

  const CPASDatabase& db = bc.GetPASDatabase();

  int anim = cmd->GetAnimationId();
  if (anim == -1) {
    const CPASAnimParmData parms(pas::kAS_KnockBack, CPASAnimParm::FromReal32(angleDegrees),
                                 CPASAnimParm::FromEnum(cmd->GetHitSeverity()));
    const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
    anim = best.second;
  }

  const CAnimPlaybackParms playParms(anim, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  mRemTime = 0.f;
  mRotateSpeed = 0.f;
  if (cmd->GetAnimationId() == -1) {
    const CPASAnimState* animState = db.GetAnimState(pas::kAS_KnockBack);

    CPASAnimParm parm2(animState->GetAnimParmData(anim, 2));
    if (!parm2.GetBoolValue()) {
      CPASAnimParm parm0(animState->GetAnimParmData(anim, 0));
      float knockdownAngle = parm0.GetReal32Value();
      const float animAngle = CRelAngle::FromDegrees(knockdownAngle).AsRadians();
      const float angleDiff = angle - animAngle;
      const float delta = CMath::ClampRadians(angleDiff);
      const float minAngle = rstl::min_val(delta, CMath::ClampRadians(animAngle - angle));
      const float flippedAngle = CMath::ClampRadians(angleDiff) > M_PIF ? -minAngle : minAngle;
      mRemTime = 0.15f * bc.GetAnimTimeRemaining();
      mRotateSpeed = (mRemTime > FLT_EPSILON) ? flippedAngle / mRemTime : flippedAngle;
    }
  }
  mCurTime = 0.f;
}

pas::EAnimationState CBSKnockBack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    mCurTime += dt;
    if (mRemTime > 0.f) {
      bc.SetDeltaRotation(CQuaternion::ZRotation(CRelAngle::FromRadians(mRotateSpeed * dt)));
      mRemTime -= dt;
    }
  }
  return st;
}

void CBSKnockBack::Shutdown(CBodyController&) {}

pas::EAnimationState CBSKnockBack::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();

  if (commandMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (commandMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (commandMgr.GetCmd(kBSC_LoopHitReaction)) {
    return pas::kAS_LoopReaction;
  }
  const CBCGenerateCmd* generate =
      static_cast< const CBCGenerateCmd* >(commandMgr.GetCmd(kBSC_Generate));
  if (generate && generate->CanInterruptKnockBack()) {
    return pas::kAS_Generate;
  }
  const CBCKnockBackCmd* knockBack =
      static_cast< const CBCKnockBackCmd* >(commandMgr.GetCmd(kBSC_KnockBack));
  if (knockBack) {
    if (mCurTime > 0.2f) {
      return pas::kAS_KnockBack;
    }
    if (knockBack->GetForceRestart() == true) {
      return pas::kAS_KnockBack;
    }
  }
  if (bc.IsAnimationOver()) {
    return pas::kAS_Locomotion;
  }
  return pas::kAS_Invalid;
}

bool CBSKnockBack::IsMoving() const { return true; }
