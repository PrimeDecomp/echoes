#include "MetroidPrime/BodyState/CBSWallHang.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

CBSWallHang::CBSWallHang()
: mState(pas::kWHS_Invalid)
, mWpId(kInvalidUniqueId)
, mLaunchVel(CVector3f::Zero())
, mLaunched(false)
, mNeedsExit(false) {}

void CBSWallHang::SetLaunchVelocity(CBodyController& bc) {
  if (!mLaunched) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
      actor->SetVelocityWR(mLaunchVel);
      actor->SetConstantForceWR(actor->GetMass() * mLaunchVel);
    }
    mLaunched = true;
  }
}

bool CBSWallHang::IsInAir(const CBodyController& bc) const {
  return mState == pas::kWHS_JumpArc || mState == pas::kWHS_JumpAirLoop ||
         mState == pas::kWHS_OutOfWallHangTurn || mState == pas::kWHS_DetachJumpLoop;
}

bool CBSWallHang::ApplyAnimationDeltas() const {
  return mState == pas::kWHS_IntoJump || mState == pas::kWHS_IntoWallHang ||
         mState == pas::kWHS_WallHang || mState == pas::kWHS_Five ||
         mState == pas::kWHS_OutOfWallHang || mState == pas::kWHS_DetachOutOfJump;
}

bool CBSWallHang::ApplyHeadTracking() const {
  return mState == pas::kWHS_WallHang || mState == pas::kWHS_Five;
}

bool CBSWallHang::CanShoot() const { return mState == pas::kWHS_WallHang; }

bool CBSWallHang::ApplyGravity() const {
  return mState != pas::kWHS_WallHang && mState != pas::kWHS_IntoWallHang &&
         mState != pas::kWHS_OutOfWallHang;
}

void CBSWallHang::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCWallHangCmd* cmd =
      static_cast< const CBCWallHangCmd* >(bc.CommandMgr().GetCmd(kBSC_WallHang));
  mState = pas::kWHS_IntoJump;
  mWpId = cmd->GetTarget();
  mNeedsExit = false;
  bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                       *mgr.Random());
}

pas::EAnimationState CBSWallHang::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state != pas::kAS_Invalid) {
    return state;
  }

  switch (mState) {
  case pas::kWHS_IntoJump:
    if (bc.IsAnimationOver()) {
      const CPASAnimParmData parms(pas::kAS_WallHang, CPASAnimParm::FromEnum(pas::kWHS_JumpArc));
      const rstl::pair< float, int > best =
          bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
      if (best.first > 0.f) {
        mState = pas::kWHS_JumpArc;
        bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
      } else {
        mState = pas::kWHS_JumpAirLoop;
        bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                             *mgr.Random());
      }

      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
        mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor->GetUniqueId(),
                                        kSM_Jumped, kSS_InvalidState));
        if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(mWpId))) {
          const CVector3f toWaypoint = waypoint->GetTranslation() - actor->GetTranslation();
          if (!(toWaypoint.GetZ() < 0.f)) {
            const float gravity = -actor->GetMomentumWR().GetZ() / actor->GetMass();
            const float zVel = CMath::SqrtF(2.f * gravity * toWaypoint.GetZ());
            const float invTime = 1.f / (zVel / gravity);
            mLaunched = false;
            mLaunchVel = CVector3f(invTime * toWaypoint.ToVec2f(), zVel);
          }
        }
      }
    }
    break;
  case pas::kWHS_JumpArc:
    if (bc.IsAnimationOver()) {
      mState = pas::kWHS_JumpAirLoop;
      bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                           *mgr.Random());
    } else {
      CheckForWall(bc, mgr);
    }
    break;
  case pas::kWHS_JumpAirLoop:
    SetLaunchVelocity(bc);
    if (!CheckForWall(bc, mgr)) {
      CheckForLand(bc, mgr);
    }
    break;
  case pas::kWHS_IntoWallHang:
    if (bc.IsAnimationOver()) {
      mState = pas::kWHS_WallHang;
      bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                           *mgr.Random());
    } else if (bc.CommandMgr().GetCmd(kBSC_ExitState)) {
      mNeedsExit = true;
    }
    break;
  case pas::kWHS_WallHang:
    if (bc.CommandMgr().GetCmd(kBSC_ExitState) || mNeedsExit) {
      mState = pas::kWHS_OutOfWallHang;
      bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                           *mgr.Random());
    }
    FixInPlace(bc);
    break;
  case pas::kWHS_Five:
    if (bc.IsAnimationOver()) {
      mState = pas::kWHS_WallHang;
      bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                           *mgr.Random());
    }
    FixInPlace(bc);
    break;
  case pas::kWHS_OutOfWallHang:
    if (bc.IsAnimationOver()) {
      const CPASAnimParmData parms(pas::kAS_WallHang,
                                   CPASAnimParm::FromEnum(pas::kWHS_OutOfWallHangTurn));
      const rstl::pair< float, int > best =
          bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
      if (best.first > 0.f) {
        mState = pas::kWHS_OutOfWallHangTurn;
        bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
      } else {
        mState = pas::kWHS_DetachJumpLoop;
        bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                             *mgr.Random());
      }

      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
        mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor->GetUniqueId(),
                                        kSM_Jumped, kSS_InvalidState));
        mLaunched = false;
        if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(mWpId))) {
          mLaunchVel = 15.f * waypoint->GetTransform().GetForward();
          mLaunchVel.SetZ(5.f);
        } else {
          mLaunchVel = -15.f * actor->GetTransform().GetForward();
        }
        actor->SetAngularMomentumWR(CAxisAngle::Identity());
      }
    }
    break;
  case pas::kWHS_OutOfWallHangTurn:
    if (bc.IsAnimationOver()) {
      mState = pas::kWHS_DetachJumpLoop;
      bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                           *mgr.Random());
    }
    break;
  case pas::kWHS_DetachJumpLoop:
    SetLaunchVelocity(bc);
    CheckForLand(bc, mgr);
    break;
  case pas::kWHS_DetachOutOfJump:
    if (bc.IsAnimationOver()) {
      mState = pas::kWHS_Invalid;
      state = pas::kAS_Locomotion;
    }
    break;
  default:
    break;
  }
  return state;
}

void CBSWallHang::Shutdown(CBodyController& bc) {}

bool CBSWallHang::CheckForWall(CBodyController& bc, CStateManager& mgr) {
  if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
    const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(mWpId));
    const float magSq =
        waypoint ? (waypoint->GetTranslation() - actor->GetTranslation()).MagSquared() : 10.f;
    if (magSq < 1.f || actor->HasBlockingCollision()) {
      mState = pas::kWHS_IntoWallHang;
      const CPASAnimParmData parms(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState));
      const rstl::pair< float, int > best =
          bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
      const CVector3f target = waypoint ? waypoint->GetTranslation() : actor->GetTranslation();
      const CVector3f scale = bc.GetOwner().GetModelData()->GetScale();
      bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, nullptr, &target,
                                                &bc.GetOwner().GetTransform(), &scale, false),
                             false, false);
      actor->SetVelocityWR(CVector3f::Zero());
      actor->SetMomentumWR(CVector3f::Zero());
      mgr.SendScriptMsg(actor, kInvalidUniqueId, kSM_OnFloor, kInvalidUniqueId);
      return true;
    }
  }
  return false;
}

bool CBSWallHang::CheckForLand(CBodyController& bc, CStateManager& mgr) {
  if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
    if (actor->HasBlockingCollision() || actor->IsOnGround()) {
      mState = pas::kWHS_DetachOutOfJump;
      bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_WallHang, CPASAnimParm::FromEnum(mState)),
                           *mgr.Random());
      mgr.SendScriptMsg(actor, kInvalidUniqueId, kSM_OnFloor, kInvalidUniqueId);
      return true;
    }
  }
  return false;
}

void CBSWallHang::FixInPlace(CBodyController& bc) {
  if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
    actor->SetConstantForceWR(CVector3f::Zero());
    actor->SetVelocityWR(CVector3f::Zero());
  }
}

pas::EAnimationState CBSWallHang::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
  if (cmdMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (cmdMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  return pas::kAS_Invalid;
}

bool CBSWallHang::IsMoving() const { return true; }
