#include "MetroidPrime/BodyState/CBSJump.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

CBSJump::CBSJump()
: mState(pas::kJS_Invalid)
, mJumpType(pas::kJT_Invalid)
, mAnimationVariant(0)
, mFacingFlags(0)
, mWaypoint1(CVector3f::Zero())
, mVelocity(CVector3f::Zero())
, mWaypoint2(CVector3f::Zero())
, mApplyLaunchVel(false)
, mWallJump(false)
, mWallBounceRight(false)
, mHasWallBounced(false)
, mExitJumpRequested(false) {}

bool CBSJump::IsInAir(const CBodyController& bc) const {
  return mState == pas::kJS_AmbushJump || mState == pas::kJS_Loop;
}

bool CBSJump::ApplyAnimationDeltas() const {
  return mState != pas::kJS_AmbushJump && mState != pas::kJS_Loop && mState != pas::kJS_ExitJump;
}

bool CBSJump::CanShoot() const { return mState == pas::kJS_AmbushJump || mState == pas::kJS_Loop; }

void CBSJump::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCJumpCmd* cmd = static_cast< const CBCJumpCmd* >(bc.CommandMgr().GetCmd(kBSC_Jump));
  mJumpType = cmd->GetJumpType();
  mWaypoint1 = cmd->GetJumpTarget();
  mWaypoint2 = cmd->GetSecondJumpTarget();
  mState = cmd->GetInitialState();
  mWallJump = cmd->IsWallJump();
  mAnimationVariant = cmd->GetAnimationVariant();
  mApplyLaunchVel = false;
  mHasWallBounced = false;
  mFacingFlags = cmd->GetFacingFlags();
  mExitJumpRequested = false;

  if (mWallJump) {
    const CVector3f toWall = mWaypoint1 - bc.GetOwner().GetTranslation();
    const CVector3f cross = CVector3f::Cross(toWall, CVector3f::Up());
    const CVector3f between = mWaypoint2 - mWaypoint1;
    mWallBounceRight = CVector3f::Dot(cross, between) < 0.f;
  }

  if (mState == pas::kJS_AmbushJump || mState == pas::kJS_Loop) {
    PlayJumpLoop(mgr, bc);
  } else {
    mState = pas::kJS_IntoJump;
    bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                          CPASAnimParm::FromEnum(mJumpType),
                                          CPASAnimParm::FromEnum(mAnimationVariant)),
                         *mgr.Random());
  }
}

void CBSJump::PlayJumpLoop(CStateManager& mgr, CBodyController& bc) {
  if (mState == pas::kJS_AmbushJump) {
    const CPASAnimParmData parms(pas::kAS_Jump, CPASAnimParm::FromEnum(pas::kJS_AmbushJump),
                                 CPASAnimParm::FromEnum(mJumpType),
                                 CPASAnimParm::FromEnum(mAnimationVariant));
    const rstl::pair< float, int > best =
        bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > 99.f) {
      bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    } else {
      mState = pas::kJS_Loop;
    }
  }

  if (mState == pas::kJS_Loop) {
    bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                          CPASAnimParm::FromEnum(mJumpType),
                                          CPASAnimParm::FromEnum(mAnimationVariant)),
                         *mgr.Random());
  }

  if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, actor->GetUniqueId(), kSM_OffGround));
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, actor->GetUniqueId(), kSM_Launching));
    const CVector3f velocity = actor->GetVelocityWR();
    mApplyLaunchVel = false;
    mVelocity = velocity;
  }
}

// Guessed name
void CBSJump::UpdateAnimationVariant(CBodyController& bc) {
  if (const CBCUnknown18Cmd* cmd =
          static_cast< const CBCUnknown18Cmd* >(bc.CommandMgr().GetCmd(kBSC_Unknown18))) {
    mAnimationVariant = cmd->GetAnimationVariant();
  }
}

// Guessed name
pas::EAnimationState CBSJump::UpdateExitJump(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState ret = pas::kAS_Invalid;
  if (mState != pas::kJS_ExitJump) {
    const CPASDatabase& db = bc.GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_Jump, CPASAnimParm::FromEnum(pas::kJS_ExitJump),
                                 CPASAnimParm::FromEnum(mJumpType),
                                 CPASAnimParm::FromEnum(mAnimationVariant));
    const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
    bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
    mState = pas::kJS_ExitJump;
  } else if (bc.IsAnimationOver()) {
    mState = pas::kJS_Invalid;
    ret = pas::kAS_Locomotion;
  }
  return ret;
}

pas::EAnimationState CBSJump::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  UpdateAnimationVariant(bc);
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    switch (mState) {
    case pas::kJS_IntoJump:
      if (bc.IsAnimationOver()) {
        mState = pas::kJS_AmbushJump;
        PlayJumpLoop(mgr, bc);
      }
      if ((mFacingFlags & CBCJumpCmd::kFF_IntoJump) &&
          bc.CommandMgr().GetTargetVector().IsNonZero()) {
        bc.FaceDirection(bc.CommandMgr().GetTargetVector(), dt);
      }
      break;
    case pas::kJS_AmbushJump:
      if (bc.CommandMgr().GetCmd(kBSC_Unknown19)) {
        mExitJumpRequested = true;
      }
      if (mExitJumpRequested == true) {
        return UpdateExitJump(dt, bc, mgr);
      }

      if (!mApplyLaunchVel) {
        if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
          actor->SetConstantForceWR(actor->GetMass() * mVelocity);
        }
        mApplyLaunchVel = true;
      }
      if ((mFacingFlags & CBCJumpCmd::kFF_AmbushJump) &&
          bc.CommandMgr().GetTargetVector().IsNonZero()) {
        bc.FaceDirection(bc.CommandMgr().GetTargetVector(), dt);
      }

      if (bc.IsAnimationOver()) {
        mState = pas::kJS_Loop;
        bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                              CPASAnimParm::FromEnum(mJumpType),
                                              CPASAnimParm::FromEnum(mAnimationVariant)),
                             *mgr.Random());
      } else if (!CheckForWallJump(bc, mgr)) {
        CheckForLand(bc, mgr);
      }
      break;
    case pas::kJS_Loop:
      if (bc.CommandMgr().GetCmd(kBSC_Unknown19)) {
        mExitJumpRequested = true;
      }
      if (mExitJumpRequested == true) {
        return UpdateExitJump(dt, bc, mgr);
      }

      if (!mApplyLaunchVel) {
        if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
          actor->SetConstantForceWR(actor->GetMass() * mVelocity);
        }
        mApplyLaunchVel = true;
      }
      if (bc.CommandMgr().GetTargetVector().IsNonZero()) {
        bc.FaceDirection(bc.CommandMgr().GetTargetVector(), dt);
      }
      if (!CheckForWallJump(bc, mgr)) {
        CheckForLand(bc, mgr);
      }
      if (bc.CommandMgr().GetCmd(kBSC_ExitState)) {
        ForceLand(bc, mgr);
      }
      break;
    case pas::kJS_ExitJump:
      return UpdateExitJump(dt, bc, mgr);
    case pas::kJS_WallBounceLeft:
    case pas::kJS_WallBounceRight:
      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
        actor->Stop();
        actor->SetMomentumWR(CVector3f::Zero());
      }
      if (bc.IsAnimationOver()) {
        mgr.SendScriptMsg(&bc.GetOwner(), kInvalidUniqueId, kSM_OffGround);
        mState = pas::kJS_Loop;
        bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                              CPASAnimParm::FromEnum(mJumpType),
                                              CPASAnimParm::FromEnum(mAnimationVariant)),
                             *mgr.Random());
        mHasWallBounced = true;

        if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
          const CVector3f delta = mWaypoint2 - actor->GetTranslation();
          const float factor = CMath::SqrtF(actor->GetGravityConstant() / (-2.f * delta.GetZ()));
          actor->SetVelocityWR(CVector3f(factor * delta.GetX(), factor * delta.GetY(), 0.f));
        }
      }
      break;
    case pas::kJS_OutOfJump:
      if (bc.IsAnimationOver()) {
        mState = pas::kJS_Invalid;
        state = pas::kAS_Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return state;
}

void CBSJump::Shutdown(CBodyController& bc) {}

void CBSJump::CheckForLand(CBodyController& bc, CStateManager& mgr) {
  if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
    if (actor->IsInCollision() || actor->IsOnGround()) {
      mState = pas::kJS_OutOfJump;
      UpdateAnimationVariant(bc);
      bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                            CPASAnimParm::FromEnum(mJumpType),
                                            CPASAnimParm::FromEnum(mAnimationVariant)),
                           *mgr.Random());
      mgr.SendScriptMsg(actor, kInvalidUniqueId, kSM_Landed);
    }
  }
}

// Guessed name
void CBSJump::ForceLand(CBodyController& bc, CStateManager& mgr) {
  if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
    mState = pas::kJS_OutOfJump;
    UpdateAnimationVariant(bc);
    bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                          CPASAnimParm::FromEnum(mJumpType),
                                          CPASAnimParm::FromEnum(mAnimationVariant)),
                         *mgr.Random());
    mgr.SendScriptMsg(actor, kInvalidUniqueId, kSM_Landed);
  }
}

uchar CBSJump::CheckForWallJump(CBodyController& bc, CStateManager& mgr) {
  bool ret = false;
  if (mWallJump && !mHasWallBounced) {
    if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
      const float distToWall = (mWaypoint1 - actor->GetTranslation()).Magnitude();
      const float xExtent = 0.5f * actor->GetBoundingBox().GetWidth();
      if (distToWall < 1.414f * xExtent || (actor->IsInCollision() && distToWall < 3.f * xExtent)) {
        mState = mWallBounceRight ? pas::kJS_WallBounceRight : pas::kJS_WallBounceLeft;
        bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                              CPASAnimParm::FromEnum(mJumpType),
                                              CPASAnimParm::FromEnum(mAnimationVariant)),
                             *mgr.Random());
        mgr.SendScriptMsg(actor, kInvalidUniqueId, kSM_Landed);
        ret = true;
      }
    }
  }
  return ret;
}

pas::EAnimationState CBSJump::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
  if (const CBCHurledCmd* cmd =
          static_cast< const CBCHurledCmd* >(bc.GetCommandMgr().GetCmd(kBSC_Hurled))) {
    const_cast< CBCHurledCmd* >(cmd)->SetSkipLaunchState(true);
    return pas::kAS_Hurled;
  }
  if (cmdMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (cmdMgr.GetCmd(kBSC_Jump) && bc.GetBodyType() == kBT_WallWalker) {
    return pas::kAS_Jump;
  }
  if (cmdMgr.GetCmd(kBSC_NextState)) {
    return pas::kAS_Locomotion;
  }
  return pas::kAS_Invalid;
}

bool CBSJump::ApplyHeadTracking() const { return false; }

bool CBSJump::IsMoving() const { return true; }
