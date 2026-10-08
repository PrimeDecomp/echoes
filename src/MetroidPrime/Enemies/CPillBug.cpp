#include "MetroidPrime/Enemies/CPillBug.hpp"

#include "Collision/CollisionUtil.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPillBug.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"
#include <float.h>

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"Attacked", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::Attacked)},
    {"Bombed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::Bombed)},
    {"PathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::PathOver)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::AnimOver)},
    {"CloseToPath", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::CloseToPath)},
    {"Landed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::Landed)},
    {"HitPlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::HitPlayer)},
    {"HasReturnPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CPillBug::HasReturnPath)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::Patrol)},
    {"FastPatrol", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::FastPatrol)},
    {"Injured", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::Injured)},
    {"Turn", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::Turn)},
    {"Wander", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::Wander)},
    {"Fall", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::Fall)},
    {"Pause", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::Pause)},
    {"ReturnToPath", static_cast< CPatterned::StateMachine::StateFunc >(&CPillBug::ReturnToPath)},
};

CPillBug::CPillBug(TUniqueId uid, const rstl::string& name, EFlavorType flavor, CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                   int planarConstraint, const CDamageVulnerability& damageVulnerability,
                   const CDamageVulnerability& wanderVulnerability, float crawlRadius,
                   float rollRadius, float floorTurnSpeed, float stickRadius,
                   float waypointApproachDistance, float visibleDistance, float unknown_0x519c7197,
                   float collisionLookAheadTime, float forwardPriority, float unknown_0x558c0692,
                   float unknown_0x0f991bf1, float unknown_0x385a1bed, float unknown_0xcf4ea141)
: CWallCrawler(kPAI_PillBug, uid, name, flavor, info, xf, modelData, patternedInfo, kMT_Flyer,
               kCT_Zero, kBT_WallWalker, actorParams, crawlRadius, stickRadius, floorTurnSpeed,
               waypointApproachDistance, visibleDistance, kT_PillBug, false, 1.f,
               unknown_0x519c7197, unknown_0x558c0692, unknown_0x0f991bf1, unknown_0x385a1bed,
               unknown_0xcf4ea141)
, mCurrentWaypointId(kInvalidUniqueId)
, mCrawlRadius(crawlRadius)
, mRollRadius(rollRadius)
, mCollisionLookAheadTime(collisionLookAheadTime)
, mForwardPriority(forwardPriority)
, mPlanarConstraint(planarConstraint)
, mDamageVulnerability(damageVulnerability)
, mWanderVulnerability(wanderVulnerability)
, mAnimSubState(2)
, mMode(kM_Crawl)
, x8f8_(CVector3f::Zero())
, mTurnFaceDirection(CVector3f::Zero())
, mAttacked(false)
, mBombed(false)
, mPathOver(false)
, mPathForward(true)
, mPathLoops(false)
, mLanded(true)
, mTouchingStaticGround(false)
, mHitPlayer(false)
, mReturningToPath(false) {
  SetDrawShadow(false);
}

CPillBug::~CPillBug() {}

void CPillBug::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId uid = msg.GetSenderId();
  bool handled = false;
  switch (msg.GetMessage()) {
  case kSM_Create:
    mBodyController->Activate(mgr, pas::kAS_Invalid);
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kSM_AreaLoaded:
    BuildPath(mgr);
    UpdateConstraintPlane(mgr);
    break;
  case kSM_LandedOnStaticGround:
    mOnStaticGround = true;
    handled = true;
    break;
  case kSM_Landed:
    mOnGround = true;
    handled = true;
    break;
  case kSM_Falling:
    mOnGround = false;
    mOnStaticGround = false;
    handled = true;
    break;
  case kSM_ResistedDamage:
  case kSM_XXDG:
    if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(uid))) {
      if (const CPlayer* player =
              TCastToConstPtr< CPlayer >(mgr.GetObjectById(weapon->GetOwnerId()))) {
        const CDamageVulnerability* vulnerability = GetDamageVulnerability();
        if (mMode == kM_Crawl) {
          if (vulnerability->GetVulnerability(weapon->GetCurrentDamageInfo().GetWeaponMode())
                  .WeaponHurts()) {
            mBombed = true;
          } else {
            mAttacked = true;
          }
        }
      }
    }
    break;
  }

  if (!handled) {
    CWallCrawler::AcceptScriptMsg(mgr, msg);
  }
}

void CPillBug::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  ++mThinkCounter;
  if (mPlayerObstructed) {
    SetMovable(false);
    return;
  }

  SetMovable(!mAlignToFloor);
  CWallCrawler::Think(dt, mgr);
  if (!mDisableMove && close_enough(mBodyController->GetPercentageFrozen(), 0.f) && mAlignToFloor) {
    AlignToFloor(mgr, mColSphere.GetSphere().GetRadius(),
                 GetTranslation() + mCollisionLookAheadTime * GetVelocityWR(), dt);
  }
}

void CPillBug::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

const CDamageVulnerability* CPillBug::GetDamageVulnerability() const {
  switch (mMode) {
  case kM_Wander:
    return &mWanderVulnerability;
  case kM_Injured:
    return &mDamageVulnerability;
  }
  return CPatterned::GetDamageVulnerability();
}

rstl::optional_object< CAABox > CPillBug::GetTouchBounds() const {
  const float radius = mColSphere.GetSphere().GetRadius();
  const CVector3f position = GetTranslation();
  return rstl::optional_object< CAABox >(CAABox(position - CVector3f(radius, radius, radius),
                                                position + CVector3f(radius, radius, radius)));
}

bool CPillBug::IsOnGround() const { return mLanded; }

bool CPillBug::IsOnStaticGround() const { return mTouchingStaticGround; }

CVector3f CPillBug::GetAimPosition(const CStateManager& mgr, float dt) const {
  return GetTranslation();
}

void CPillBug::ThinkAboutMove(float dt) {
  if (mAlignToFloor || mOnGround || mOnStaticGround) {
    CPatterned::ThinkAboutMove(dt);
  }
}

void CPillBug::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                            CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(id))) {
    const CVector3f toPlayer = player->GetTranslation() - GetTranslation();
    if (CVector3f::Dot(GetTransform().GetForward(), toPlayer) > 0.f) {
      mHitPlayer = true;
    }
  }
}

TUniqueId CPillBug::GetNextWaypoint(CStateManager& mgr, const CScriptWaypoint* waypoint,
                                    bool reverse) {
  if (mReturningToPath) {
    const TUniqueId next = waypoint->NextWaypoint(mgr);
    if (next != kInvalidUniqueId) {
      return next;
    }
  }

  if (mPath.empty()) {
    return kInvalidUniqueId;
  }

  int index = 0;
  for (rstl::vector< TUniqueId >::const_iterator it = mPath.begin(); it != mPath.end();
       ++it, ++index) {
    if (waypoint->GetUniqueId() == *it) {
      break;
    }
  }
  if (mPathForward) {
    ++index;
  } else {
    --index;
  }

  if (mPath.size() > 1) {
    if (!mPathLoops) {
      if (index < 0) {
        index = 1;
        if (reverse) {
          mPathOver = true;
          mPathForward = true;
        }
      }
      if (index >= mPath.size()) {
        index = mPath.size() - 2;
        if (reverse) {
          mPathOver = true;
          mPathForward = false;
        }
      }
    } else if (index >= mPath.size()) {
      index = 0;
    }
  } else {
    index = 0;
  }
  return mPath[index];
}

void CPillBug::BuildPath(CStateManager& mgr) {
  if (HasInnerPathLoop(mgr)) {
    return;
  }

  const TUniqueId start = CheckConnectedObject(mgr, kSS_Patrol, kSM_Follow);
  int count = 0;
  TUniqueId current = start;
  while (current != kInvalidUniqueId) {
    const CScriptWaypoint* waypoint =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(current));
    if (!waypoint) {
      break;
    }
    ++count;
    current = waypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
    if (current == start) {
      break;
    }
  }

  mPath.reserve(count);
  mPathLoops = false;
  current = start;
  while (current != kInvalidUniqueId) {
    const CScriptWaypoint* waypoint =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(current));
    if (!waypoint) {
      break;
    }
    mPath.push_back_unsafe(current);
    current = waypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
    if (current == start) {
      mPathLoops = true;
      return;
    }
  }
}

bool CPillBug::HasInnerPathLoop(CStateManager& mgr) const {
  const TUniqueId start = CheckConnectedObject(mgr, kSS_Patrol, kSM_Follow);
  TUniqueId current = start;
  TUniqueId anchor = start;
  while (current != kInvalidUniqueId && anchor != kInvalidUniqueId) {
    const TUniqueId loopStart = anchor;
    for (int i = 0; i < 2; ++i) {
      const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(current));
      if (!waypoint) {
        return false;
      }
      current = waypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
      if (current == start) {
        return false;
      }
      if (current == loopStart) {
        return true;
      }
    }

    const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(anchor));
    if (!waypoint) {
      return false;
    }
    anchor = waypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
  }
  return false;
}

int CPillBug::GetClosestPathIndex(CStateManager& mgr) const {
  int closest = 0;
  float closestDistSq = FLT_MAX;
  const CVector3f position = GetTranslation();
  int index = 0;
  for (rstl::vector< TUniqueId >::const_iterator it = mPath.begin(); it != mPath.end();
       ++it, ++index) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (actor) {
      const float distSq = (actor->GetTranslation() - position).MagSquared();
      if (distSq < closestDistSq) {
        closestDistSq = distSq;
        closest = index;
      }
    }
  }
  return closest;
}

TUniqueId CPillBug::FindReturnWaypoint(CStateManager& mgr) const {
  TUniqueId best = kInvalidUniqueId;
  float bestDistSq = FLT_MAX;
  const CVector3f position = GetTranslation();
  const rstl::vector< TUniqueId > connected =
      FindConnectedObjects(mgr, kSS_InternalState00, kSM_None);
  for (rstl::vector< TUniqueId >::const_iterator it = connected.begin(); it != connected.end();
       ++it) {
    TUniqueId id = *it;
    int depth = 0;
    while (id != kInvalidUniqueId && depth++ < 5) {
      const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
      if (!actor) {
        break;
      }
      const float distSq = (actor->GetTranslation() - position).MagSquared();
      if (distSq < bestDistSq) {
        bestDistSq = distSq;
        best = *it;
      }
      actor->FindConnectedObject(mgr, kSS_Arrived, kSM_Next);
    }
  }

  for (rstl::vector< TUniqueId >::const_iterator it = mPath.begin(); it != mPath.end(); ++it) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (actor) {
      const float distSq = (actor->GetTranslation() - position).MagSquared();
      if (distSq < bestDistSq) {
        return kInvalidUniqueId;
      }
    }
  }
  return best;
}

int CPillBug::GetCurrentConstraint() const {
  if (mPathLoops && !mPathForward) {
    if (mPlanarConstraint == 1) {
      return 2;
    }
    if (mPlanarConstraint == 2) {
      return 1;
    }
  }
  return mPlanarConstraint;
}

CDamageInfo CPillBug::GetContactDamage() const {
  if (mMode == kM_Injured) {
    return CDamageInfo();
  }
  return CPatterned::GetContactDamage();
}

bool CPillBug::Attacked(CStateManager& mgr, const CTriggerData& data) const { return mAttacked; }

bool CPillBug::Bombed(CStateManager& mgr, const CTriggerData& data) const { return mBombed; }

bool CPillBug::HasReturnPath(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  const TUniqueId connected = FindConnectedObject(mgr, kSS_InternalState00, kSM_None);
  if (connected != kInvalidUniqueId) {
    if (FindReturnWaypoint(mgr) != kInvalidUniqueId) {
      result = true;
    }
  }
  return result;
}

bool CPillBug::PathOver(CStateManager& mgr, const CTriggerData& data) const { return mPathOver; }

bool CPillBug::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimSubState == 2;
}

bool CPillBug::Landed(CStateManager& mgr, const CTriggerData& data) const { return mLanded; }

bool CPillBug::HitPlayer(CStateManager& mgr, const CTriggerData& data) const { return mHitPlayer; }

bool CPillBug::CloseToPath(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.Random()->Next() & 3) {
    return false;
  }

  const CSphere sphere(GetTranslation(), 1.25f * mColSphere.GetSphere().GetRadius());
  const int size = mPath.size();
  for (int i = 0; i < size; ++i) {
    int next = i + 1;
    if (next == size) {
      if (!mPathLoops) {
        continue;
      }
      next = 0;
    }

    const CActor* from = TCastToConstPtr< CActor >(mgr.GetObjectById(mPath[i]));
    const CActor* to = TCastToConstPtr< CActor >(mgr.GetObjectById(mPath[next]));
    if (!to || !from) {
      continue;
    }

    const CVector3f start = from->GetTranslation();
    const CVector3f diff = to->GetTranslation() - start;
    const float mag = diff.Magnitude();
    if (close_enough(mag, 0.f)) {
      continue;
    }

    const CVector3f dir = diff * (1.f / mag);
    float t;
    CVector3f point = CVector3f::Zero();
    if (CollisionUtil::RaySphereIntersection(sphere, start, dir, mag, t, point)) {
      return true;
    }
  }
  return false;
}

void CPillBug::PatrolImpl(CStateManager& mgr, EStateMsg msg, float dt, bool fast) {
  switch (msg) {
  case kStateMsg_Activate:
    mBombed = false;
    mMode = kM_Crawl;
    if (fast) {
      mBodyController->SetLocomotionType(pas::kLT_Combat);
    } else {
      mBodyController->SetLocomotionType(pas::kLT_Lurk);
    }
    mAlignToFloor = true;
    SetMomentumWR(CVector3f::Zero());
    mHasAlignSurface = false;
    SetMovable(false);
    if (mDestObj == kInvalidUniqueId) {
      mDestObj = mPath[GetClosestPathIndex(mgr)];
    }
    SetConstraint(mgr, mPlanarConstraint);
    mPositionHistory.clear();
    break;
  case kStateMsg_Update: {
    if (UpdateWPDestination(mgr)) {
      SetConstraint(mgr, mPlanarConstraint);
    }
    const CVector3f up = GetTransform().GetUp();
    const CVector3f seek = ProjectVectorToPlane(mSteeringBehaviors.Seek(*this, mDestPos), up);
    mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(1.f * seek, up).AsNormalized(), CVector3f::Zero(), 0.9f));
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(1.f * GetTransform().GetForward(), CVector3f::Zero(), mForwardPriority));
    if (mPlanarConstraint != 0) {
      const CVector3f closest = GetClosestPointOnPlane(
          GetTranslation() + GetTransform().GetForward(), GetConstraintPlane());
      const CVector3f arrival =
          ProjectVectorToPlane(mSteeringBehaviors.Arrival(*this, closest, 0.3f), up);
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(ProjectVectorToPlane(1.f * arrival, up), CVector3f::Zero(), 0.5f));
    }

    if (mPositionHistory.size() == 24) {
      CVector3f sum = CVector3f::Zero();
      for (rstl::reserved_vector< CVector3f, 24 >::iterator it = mPositionHistory.begin();
           it != mPositionHistory.end(); ++it) {
        sum += *it;
      }
      const CVector3f average = (1.f / mPositionHistory.size()) * sum;
      if ((average - GetTranslation()).MagSquared() < 0.09f) {
        if (const CScriptWaypoint* waypoint =
                TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj))) {
          mDestObj = GetNextWaypoint(mgr, waypoint, false);
        }
      }
      mPositionHistory.clear();
    }
    mPositionHistory.push_back(GetTranslation());
    break;
  }
  case kStateMsg_Deactivate:
    mCurrentWaypointId = mDestObj;
    break;
  }
}

void CPillBug::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    x861_24_ = true;
  }
  PatrolImpl(mgr, msg, dt, false);
}

void CPillBug::FastPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mColSphere = CCollidableSphere(CSphere(CVector3f::Zero(), mRollRadius), GetMaterialList());
    x861_24_ = false;
  }
  if (msg == kStateMsg_Deactivate) {
    mColSphere = CCollidableSphere(CSphere(CVector3f::Zero(), mCrawlRadius), GetMaterialList());
  }
  PatrolImpl(mgr, msg, dt, true);
  mAttacked = false;
}

void CPillBug::ReturnToPath(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mReturningToPath = true;
    mMode = kM_Wander;
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    mAlignToFloor = true;
    SetMomentumWR(CVector3f::Zero());
    mHasAlignSurface = false;
    if (!mPath.empty()) {
      mDestObj = mPath[0];
    }
    SetConstraint(mgr, mPlanarConstraint);
    SetMovable(false);
    mDestObj = FindReturnWaypoint(mgr);
    break;
  case kStateMsg_Update: {
    if (UpdateWPDestination(mgr)) {
      SetConstraint(mgr, mPlanarConstraint);
    }
    const CVector3f up = GetTransform().GetUp();
    const CVector3f seek = ProjectVectorToPlane(mSteeringBehaviors.Seek(*this, mDestPos), up);
    mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(1.f * seek, up).AsNormalized(), CVector3f::Zero(), 0.6f));
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(1.f * GetTransform().GetForward(), CVector3f::Zero(), mForwardPriority));
    if (mPlanarConstraint != 0) {
      const CVector3f closest = GetClosestPointOnPlane(
          GetTranslation() + GetTransform().GetForward(), GetConstraintPlane());
      const CVector3f arrival =
          ProjectVectorToPlane(mSteeringBehaviors.Arrival(*this, closest, 0.3f), up);
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(ProjectVectorToPlane(1.f * arrival, up), CVector3f::Zero(), 0.5f));
    }

    const CVector3f dir = (mDestPos - GetTranslation()).AsNormalized();
    if (CVector3f::Dot(GetTransform().GetUp(), dir) > 0.5f) {
      if (mPlanarConstraint == 1) {
        mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
            ProjectVectorToPlane(GetTransform().GetForward() - GetConstraintPlane().GetNormal(),
                                 up),
            CVector3f::Zero(), 0.5f));
      }
      if (mPlanarConstraint == 2) {
        mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
            ProjectVectorToPlane(GetTransform().GetForward() + GetConstraintPlane().GetNormal(),
                                 up),
            CVector3f::Zero(), 0.5f));
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    mReturningToPath = false;
    mDestObj = mPath[GetClosestPathIndex(mgr)];
    break;
  }
}

void CPillBug::Fall(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const float radius = mColSphere.GetSphere().GetRadius();
    SetTranslation(GetTranslation() + 0.5f * (radius * GetTransform().GetUp()));
    MoveCollisionPrimitive(CVector3f::Zero());
    SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    RemoveMaterial(kMT_GroundCollider, mgr);
    if (CVector3f::Dot(GetTransform().GetUp(), CVector3f::Up()) < -0.998f) {
      SetTransform((CQuaternion::FromMatrix(GetTransform()) *
                    CQuaternion::AxisAngle(CUnitVector3f(GetTransform().GetForward()),
                                           CRelAngle::FromRadians(0.143117f)))
                       .BuildTransform4f(GetTranslation()));
    }
    mMode = kM_Injured;
    SetConstraint(mgr, 0);
    mAlignToFloor = false;
    break;
  }
  case kStateMsg_Update:
    AlignToPlane(CVector3f::Up(), 5400.f * dt);
    SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    if (mBodyController->GetCurrentStateId() != pas::kAS_LoopReaction) {
      mBodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Three));
    }
    break;
  case kStateMsg_Deactivate:
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

void CPillBug::Injured(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mMode = kM_Injured;
    SetConstraint(mgr, 0);
    mAlignToFloor = false;
    break;
  case kStateMsg_Update:
    AlignToPlane(CVector3f::Up(), 5400.f * dt);
    SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    if (mBodyController->GetCurrentStateId() != pas::kAS_LoopReaction) {
      mBodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Three));
    }
    break;
  case kStateMsg_Deactivate:
    mBombed = false;
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void CPillBug::Turn(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimSubState = 0;
    x861_24_ = false;
    mTurnFaceDirection = -GetTransform().GetForward();
    break;
  case kStateMsg_Update:
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), mTurnFaceDirection, 1.f));
    switch (mAnimSubState) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Turn) {
        mAnimSubState = 1;
      }
      break;
    case 1:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Turn) {
        mAnimSubState = 2;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mPathOver = false;
    break;
  }
}

void CPillBug::Wander(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mMode = kM_Wander;
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    mAlignToFloor = true;
    SetMomentumWR(CVector3f::Zero());
    mHasAlignSurface = false;
    SetMovable(false);
    if (!mPath.empty()) {
      mDestObj = mPath[0];
    }
    SetConstraint(mgr, mPlanarConstraint);
    mDestObj = mPath[GetClosestPathIndex(mgr)];
    x861_24_ = true;
    break;
  case kStateMsg_Update: {
    UpdateWPDestination(mgr);
    const CVector3f up = GetTransform().GetUp();
    const CVector3f seek = ProjectVectorToPlane(mSteeringBehaviors.Seek(*this, mDestPos), up);
    const CVector3f seekDir = 1.f * seek;
    mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(seekDir, CVector3f::Zero(), 0.6f));
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(1.f * GetTransform().GetForward(), CVector3f::Zero(), mForwardPriority));
    const CVector3f dir = (mDestPos - GetTranslation()).AsNormalized();
    if (CVector3f::Dot(GetTransform().GetUp(), dir) > 0.5f) {
      if (mPlanarConstraint == 1) {
        mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
            ProjectVectorToPlane(GetTransform().GetForward() + GetConstraintPlane().GetNormal(),
                                 up),
            CVector3f::Zero(), 0.4f));
      }
      if (mPlanarConstraint == 2) {
        mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
            ProjectVectorToPlane(GetTransform().GetForward() - GetConstraintPlane().GetNormal(),
                                 up),
            CVector3f::Zero(), 0.4f));
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    mDestObj = kInvalidUniqueId;
    mHitPlayer = false;
    break;
  }
}

void CPillBug::Pause(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimSubState = 0;
    mColSphere = CCollidableSphere(CSphere(CVector3f::Zero(), mRollRadius), GetMaterialList());
    x861_24_ = false;
    mMode = kM_Crawl;
    mAlignToFloor = true;
    SetMomentumWR(CVector3f::Zero());
    mHasAlignSurface = false;
    SetMovable(false);
    SetConstraint(mgr, mPlanarConstraint);
    break;
  case kStateMsg_Update:
    switch (mAnimSubState) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mAnimSubState = 1;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 1:
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mAnimSubState = 2;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mPathOver = false;
    mColSphere = CCollidableSphere(CSphere(CVector3f::Zero(), mCrawlRadius), GetMaterialList());
    mHitPlayer = false;
    break;
  }
}

CEntity* REL_LoadPillBug(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPillBug sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPillBug.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CPillBug(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name, CPatterned::kFT_Zero,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.planarConstraint,
      LdrToDamageVulnerability(sldrThis.damageVulnerability),
      LdrToDamageVulnerability(sldrThis.wanderVulnerability), sldrThis.crawlRadius,
      sldrThis.rollRadius, sldrThis.floorTurnSpeed, sldrThis.stickRadius,
      sldrThis.waypointApproachDistance, sldrThis.visibleDistance, sldrThis.unknown_0x519c7197,
      sldrThis.collisionLookAheadTime, sldrThis.forwardPriority, sldrThis.unknown_0x558c0692,
      sldrThis.unknown_0x0f991bf1, sldrThis.unknown_0x385a1bed, sldrThis.unknown_0xcf4ea141);
}

static void SetFuncPtrs() {
  static SPillBug_FuncPtrs funcPtrs;
  funcPtrs.mLoadPillBug = &REL_LoadPillBug;
  SetSPillBug_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPillBug_FuncPtrs(nullptr); }
