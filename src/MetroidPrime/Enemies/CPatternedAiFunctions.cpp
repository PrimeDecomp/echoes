#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include <float.h>

void CPatterned::Start(CStateManager&, EStateMsg, float) {}

void CPatterned::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
}

void CPatterned::Dead(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Update) {
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Die));
    if (!mFadeToDeath && mBodyController->GetBodyStateInfo().GetCurrentState()->IsDead()) {
      mFadeToDeath = true;
      mAlphaDelta = -1.f / GetFadeOnDeathTime();
      RemoveMaterial(kMT_Character, kMT_Unknown59, kMT_Target, kMT_Orbit, mgr);
      AddMaterial(kMT_NoPlatformCollision, mgr);
    }
  }
}

float CPatterned::GetFadeOnDeathTime() const { return mFadeOnDeathTime; }

void CPatterned::PathFind(CStateManager& mgr, EStateMsg msg, float) {
  if (!GetSearchPath()) {
    return;
  }
  switch (msg) {
  case kStateMsg_Activate:
    fn_801524fc(mgr);
    break;
  case kStateMsg_Update:
    if (!GetSearchPath()->IsOver()) {
      if (mVerticalMovement || mOnGround) {
        ++mPathOverCount;
        mPathOverCount &= 3;
      }
      const CVector3f position = GetTranslation() + 0.3f * CVector3f::Up();
      mReflectedDestPos = position - (mDestPos - position);
      ApproachDest(mgr);
      const float scale = GetModelData()->GetScale().GetY();
      CVector3f point = position + scale * GetTransform().GetForward();
      GetSearchPath()->GetSplinePointWithLookahead(point, position,
                                                   skActorApproachDistance * scale);
      SetDestPos(point);
      if (GetSearchPath()->SegmentOver(position)) {
        GetSearchPath()->Advance();
      }
    }
    break;
  default:
    break;
  }
}

void CPatterned::fn_801524fc(CStateManager& mgr) {
  CPathFindSearch* search = GetSearchPath();
  if (search->Search(GetTranslation(), mDestPos) == CPathFindSearch::kR_Success) {
    mReflectedDestPos = GetTranslation();
    SetDestPos(search->GetPoint());
    mInPosition = false;
    ApproachDest(mgr);
  }
}

bool CPatterned::OffLine(CStateManager&, const CTriggerData& data) const {
  const CVector3f fromStart = GetTranslation() - mReflectedDestPos;
  CVector3f segment = mDestPos - mReflectedDestPos;
  float distanceSquared = fromStart.MagSquared();
  if (CVector3f::Dot(segment, fromStart) > 0.f) {
    segment.Normalize();
    const CVector3f fromEnd = GetTranslation() - mDestPos;
    const float along = CVector3f::Dot(segment, fromStart);
    distanceSquared = (fromStart - along * segment).MagSquared();
    if (CVector3f::Dot(segment, fromEnd) > 0.f) {
      distanceSquared = fromEnd.MagSquared();
    }
  }
  return distanceSquared > data.GetFloat() * data.GetFloat();
}

bool CPatterned::InRange(CStateManager& mgr, const CTriggerData&) const {
  const float range = 0.5f * (mMinAttackRange + mMaxAttackRange);
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() < range * range;
}

bool CPatterned::TooClose(CStateManager& mgr, const CTriggerData&) const {
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() <
         mMinAttackRange * mMinAttackRange;
}

bool CPatterned::InMaxRange(CStateManager& mgr, const CTriggerData&) const {
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() <
         mMaxAttackRange * mMaxAttackRange;
}

bool CPatterned::InDetectionRange(CStateManager& mgr, const CTriggerData&) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CVector3f delta = mgr.GetPlayer(i)->GetTranslation() - GetTranslation();
    if (delta.MagSquared() < mDetectionRange * mDetectionRange &&
        (mDetectionHeightRange <= 0.f ||
         delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange)) {
      return true;
    }
  }
  return false;
}

bool CPatterned::Leash(CStateManager&, const CTriggerData&) const {
  return mCurPlayerLeashTime > mPlayerLeashTime &&
         (mLatestLeashPosition - GetTranslation()).MagSquared() > mLeashRadius * mLeashRadius;
}

bool CPatterned::SpotPlayer(CStateManager& mgr, const CTriggerData&) const {
  const CVector3f eye = GetGunEyePos();
  const CVector3f forward = GetTransform().GetForward();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CVector3f delta = mgr.GetPlayer(i)->GetAimPosition(mgr, 0.f) - eye;
    const float forwardDistance = CVector3f::Dot(delta, forward);
    if (forwardDistance > 0.f &&
        delta.MagSquared() * mDetectionAngle < forwardDistance * forwardDistance) {
      return true;
    }
  }
  return false;
}

bool CPatterned::IsOnScreen(const CStateManager& mgr) const {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  const CVector3f screen =
      mgr.GetCameraManager(0)->GetFirstPersonCamera()->ConvertToScreenSpace(center);
  return screen.GetZ() > 0.f && screen.GetX() * screen.GetX() < 1.f &&
         screen.GetY() * screen.GetY() < 1.f;
}

bool CPatterned::PlayerSpot(CStateManager& mgr, const CTriggerData&) const {
  if (mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
      IsOnScreen(mgr)) {
    const CVector3f aim = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
    const CVector3f center = GetBoundingBox().GetCenterPoint();
    CVector3f direction = center - aim;
    const float distance = direction.Magnitude();
    direction *= 1.f / distance;
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    return CGameCollision::RayStaticLineOfSightTest(mgr, aim, direction, distance, filter);
  }
  return false;
}

bool CPatterned::Landed(CStateManager&, const CTriggerData&) const {
  const bool landed = mOnGround && !mPrevOnGround;
  mPrevOnGround = mOnGround;
  return landed;
}

bool CPatterned::PathOver(CStateManager&, const CTriggerData&) const {
  if (GetSearchPath() && (mVerticalMovement || mOnGround)) {
    return !GetSearchPath()->IsShagged() && GetSearchPath()->IsOver();
  }
  return false;
}

bool CPatterned::PathFound(CStateManager&, const CTriggerData&) const {
  return GetSearchPath() && !GetSearchPath()->IsShagged();
}

bool CPatterned::PathShagged(CStateManager&, const CTriggerData&) const {
  if (GetSearchPath()) {
    if (GetSearchPath()->IsShagged()) {
      return true;
    }
    if (GetSearchPath()->GetCurrentWaypoint() > 0 && mPathOverCount == 0) {
      const CVector3f position = GetTranslation() + 0.3f * CVector3f::Up();
      CVector3f point = position;
      GetSearchPath()->GetSplinePoint(point, GetTranslation());
      if ((point - position).MagSquared() >
          4.f * skActorApproachDistance * skActorApproachDistance) {
        return true;
      }
    }
  }
  return false;
}

bool CPatterned::NoPathNodes(CStateManager&, const CTriggerData&) const {
  if (GetSearchPath()) {
    return GetSearchPath()->OnPath(GetTranslation()) != CPathFindSearch::kR_Success;
  }
  return true;
}

bool CPatterned::Attacked(CStateManager&, const CTriggerData&) const {
  return mHitByPlayerProjectile;
}

bool CPatterned::HasPatrolPath(CStateManager& mgr, const CTriggerData&) const {
  return GetConnectedObject(mgr, kSS_Patrol, kSM_Follow) != kInvalidUniqueId;
}

bool CPatterned::InPosition(CStateManager&, const CTriggerData&) const { return mInPosition; }

bool CPatterned::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return GetAnimOver(mgr, data);
}

bool CPatterned::Stuck(CStateManager&, const CTriggerData&) const {
  return mPredictedLeashTime > 0.2f;
}

bool CPatterned::Delay(CStateManager&, const CTriggerData& data) const {
  return mStateMachine->GetTime() > data.GetFloat();
}

bool CPatterned::RandomDelay(CStateManager&, const CTriggerData& data) const {
  const TStateMachineState< CPatterned >& state =
      static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine);
  return state.GetTime() > data.GetFloat() * state.GetRandom();
}

bool CPatterned::FixedDelay(CStateManager&, const CTriggerData&) const {
  return mStateMachine->GetTime() > mStateMachine->GetDelay();
}

bool CPatterned::CodeTrigger(CStateManager&, const CTriggerData&) const {
  return static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine).GetCodeTrigger();
}

bool CPatterned::Random(CStateManager&, const CTriggerData& data) const {
  return static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine).GetRandom() <
         data.GetFloat();
}

bool CPatterned::FixedRandom(CStateManager&, const CTriggerData&) const {
  const TStateMachineState< CPatterned >& state =
      static_cast< const TStateMachineState< CPatterned >& >(*mStateMachine);
  return state.GetRandom() < state.GetFixedRandom();
}

void CPatterned::SetAttackTarget(CStateManager&, TUniqueId) {}

void CPatterned::ApproachDest(CStateManager&) {
  CVector3f move = mDestPos - GetTranslation();
  if (!mVerticalMovement) {
    move.SetZ(0.f);
  }
  const CVector3f segment = mDestPos - mReflectedDestPos;
  if (CVector3f::Dot(segment, move) <= 0.f) {
    mInPosition = true;
  } else if (move.MagSquared() < skActorApproachDistance * skActorApproachDistance) {
    move = segment;
  }

  if (!mInPosition) {
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    const EBodyType bodyType = mBodyController->GetBodyType();
    if (bodyType == kBT_NewFlyer) {
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else if (bodyType == kBT_Blended) {
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else if (!mBodyController->HasBodyState(pas::kAS_Step)) {
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else {
      const pas::EStepDirection step = FindBestStepDirection(move);
      if (step != pas::kSD_Forward) {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(step, pas::kStep_Normal));
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
      mBodyController->CommandMgr().SetTargetVector(CVector3f::Zero());
    }
  } else {
    const float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > FLT_EPSILON) {
      const float speed = GetVelocityWR().Magnitude() / maxSpeed;
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(speed * GetTransform().GetForward(), CVector3f::Zero(), 1.f));
    }
  }
}

TUniqueId CPatterned::GetConnectedObject(CStateManager& mgr, EScriptObjectState state,
                                         EScriptObjectMessage message) const {
  rstl::reserved_vector< TUniqueId, 8 > ids;
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state == state && it->msg == message) {
      const TUniqueId id = mgr.GetIdForScript(it->objId);
      const CEntity* entity = mgr.GetObjectById(id);
      if (entity && entity->GetActive()) {
        ids.push_back(id);
        if (ids.size() == ids.capacity()) {
          break;
        }
      }
    }
  }
  if (!ids.empty()) {
    return ids[mgr.Random()->Next() % ids.size()];
  }
  return kInvalidUniqueId;
}

pas::EStepDirection CPatterned::FindBestStepDirection(const CVector3f& direction) const {
  const CVector3f local = GetTransform().TransposeRotate(direction);
  const float angle = CVector3f::GetAngleDiff(local, CVector3f::Forward());
  if (angle < CMath::Deg2Rad(45.f)) {
    return pas::kSD_Forward;
  }
  if (angle > CMath::Deg2Rad(135.f)) {
    return pas::kSD_Backward;
  }
  return CVector3f::Dot(local, CVector3f::Right()) > 0.f ? pas::kSD_Right : pas::kSD_Left;
}

void CPatterned::RotateToPoint(const CVector3f& point, float dt, float turnSpeed) {
  if (dt <= 0.f || mBodyController->GetTimeScale() == 0.f) {
    return;
  }
  CVector3f forward = GetTransform().GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized()) {
    forward.Normalize();
    CVector3f direction = point - GetTranslation();
    direction.SetZ(0.f);
    if (direction.CanBeNormalized()) {
      direction.Normalize();
      const CRelAngle angle =
          CRelAngle::FromRadians(dt * turnSpeed * mBodyController->GetTimeScale());
      const CQuaternion rotation =
          CQuaternion::ShortestRotationArcClamped(forward, direction, angle);
      RotateInOneFrameOR(rotation, dt);
    }
  }
}

void CPatterned::ApplyScreenShake(CStateManager& mgr, const CVector3f& position, TUniqueId uid) {
  if (uid == kInvalidUniqueId) {
    uid = FindConnectedObject(mgr, kSS_Footstep, kSM_Attach);
  }
  CScriptCameraShaker* shaker = static_cast< CScriptCameraShaker* >(
      TryCast(mgr.ObjectById(uid), kET_ScriptCameraShaker));
  if (shaker) {
    CCameraShakerData data = shaker->GetShakeData();
    data.SetPosition(position);
    mgr.CameraManager(0)->CameraShakerManager()->AddCameraShaker(data, mgr, false, false);
  }
}
