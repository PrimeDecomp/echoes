#include "MetroidPrime/Enemies/CWispTentacle.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CSpatialPrimitive.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWispTentacle.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "REL/REL_Setup.h"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldSleep",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::ShouldSleep)},
    {"ShouldPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::ShouldPatrol)},
    {"ShouldSearch",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::ShouldSearch)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::ShouldAttack)},
    {"ShouldGrab",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::ShouldGrab)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::AnimOver)},
    {"Attacked", static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::Attacked)},
    {"Delay", static_cast< CPatterned::StateMachine::TriggerFunc >(&CWispTentacle::Delay)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Sleep", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Sleep)},
    {"Spawn", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Spawn)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Patrol)},
    {"Search", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Search)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Attack)},
    {"Withdraw", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Withdraw)},
    {"Flinch", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Flinch)},
    {"Grab", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Grab)},
    {"Pull", static_cast< CPatterned::StateMachine::StateFunc >(&CWispTentacle::Pull)},
};

static const char* const skArmEndLocator = "M_arm_end";
static const char* const skLockOnLocator = "lockon_target_LCTR";
static const char* const skClawAttachLocator = "claw_attach_LCTR";

CWispTentacle::CWispTentacle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& modelData,
                             const CPatternedInfo& patternedInfo, const CDamageInfo& attackDamage,
                             const CActorParameters& actorParams, bool spawnFromPortal,
                             float wakeUpDistance, float searchDistance, float attackDistance,
                             float detectionHeight, float hurtSleepDelay, float grabBlendTime)
: CPatterned(kPAI_WispTentacle, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_Floater, actorParams)
, mSpawnFromPortal(spawnFromPortal)
, mWakeUpDistance(wakeUpDistance)
, mSearchDistance(searchDistance)
, mAttackDistance(attackDistance)
, mAttackDamage(attackDamage)
, mArmEndPosition(CVector3f::Zero())
, mDetectionHeight(detectionHeight)
, mLocatorsCached(false)
, mState(kS_Invalid)
, mPreviousState(kS_Invalid)
, mHurtSleepDelay(hurtSleepDelay)
, mCollisionManager(nullptr)
, mLockOnOffset(CVector3f::Zero())
, mInitialTransform(GetTransform())
, x84c_(CVector3f::Zero())
, mPortalPlane(0.f, CUnitVector3f(CVector3f::Forward()))
, mPirateId(kInvalidUniqueId)
, mShouldGrab(false)
, mPirateCaptured(false)
, mGrabOffset(CVector3f::Zero())
, mWaypointId(kInvalidUniqueId)
, mGrabTimer(0.f)
, mPirateStartRotation(CQuaternion::NoRotation())
, mPirateStartPosition(CVector3f::Zero())
, mGrabBlendTime(grabBlendTime)
, mAttacked(false) {
  mKnockBackController.EnableKnockBackPhysics(false);
  SetPendingDeath(false);
  SetDrawShadow(false);

  if (mSpawnFromPortal) {
    const CMatrix3f rot = GetTransform().BuildMatrix3f();
    const CAABox bounds = GetModelData()->GetAnimationData()->GetBoundingBox();
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f offset =
        rot * CVector3f(0.f,
                        scale.GetY() * (bounds.GetMaxPoint().GetY() - bounds.GetMinPoint().GetY()),
                        0.f);
    const CVector3f point = GetTranslation() + offset;
    mPortalPlane = CPlane(point, CUnitVector3f(rot * CVector3f::Forward()));
  }

  const CPASAnimParmData parms(pas::kAS_Taunt, CPASAnimParm::FromEnum(1));
  CCharAnimTime time = GetTimeOfUserEventForAnimation(parms, kUE_ObjectPickUp);
  if (time != CCharAnimTime::Infinity()) {
    CAnimData* animData = ModelData()->AnimationData();
    const rstl::pair< float, int > best = animData->GetPASDatabase().FindBestAnimation(parms, -1);
    if (best.first > FLT_EPSILON) {
      const int previousAnim = animData->GetCurrentAnimation();
      animData->SetAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), true);
      animData->SetPoseBuilt(false);
      const CVector3f scale = GetModelData()->GetScale();
      const CTransform4f locatorXf = GetModelData()->GetScaledLocatorTransformDynamic(
          rstl::string_l(skClawAttachLocator), &time);
      const SAdvancementResults results = animData->GetAnimationTree()->VGetAdvancementResults(
          time, CCharAnimTime(CCharAnimTime::kT_ZeroSteady, 0.f));
      const CVector3f grabPoint =
          locatorXf.GetTranslation() + CVector3f(scale.GetX() * results.mDeltas.mPosDelta.GetX(),
                                                 scale.GetY() * results.mDeltas.mPosDelta.GetY(),
                                                 scale.GetZ() * results.mDeltas.mPosDelta.GetZ());
      mGrabOffset = results.mDeltas.mRotDelta.BuildTransform() * grabPoint;
      animData->SetAnimation(CAnimPlaybackParms(previousAnim, -1, 1.f, true), true);
    }
  }
}

CWispTentacle::~CWispTentacle() {}

void CWispTentacle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    for (const SConnection* it = GetConnectionList().data();
         it != GetConnectionList().data() + GetConnectionList().size(); ++it) {
      if (it->state == kSS_Approach) {
        const TUniqueId id = mgr.GetIdForScript(it->objId);
        if (TCastToPtr< CSpacePirate >(mgr.ObjectById(id))) {
          mPirateId = id;
        } else if (TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
          mWaypointId = id;
        }
      }
    }
    break;
  case kSM_Create:
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Internal7);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
      const CAABox box = GetBoundingBox();
      const float halfX = 0.5f * (box.GetMaxPoint().GetX() - box.GetMinPoint().GetX());
      const float halfY = 0.5f * (box.GetMaxPoint().GetY() - box.GetMinPoint().GetY());
      const float halfZ = 0.5f * (box.GetMaxPoint().GetZ() - box.GetMinPoint().GetZ());
      SetBoundingBox(CAABox(-halfX, -halfY, -halfZ, halfX, halfY, halfZ));
    }
    {
      rstl::vector< CJointCollisionDescription > descs;
      if (HasAnimation() && GetAnimationData()->GetSpatialPrimitive()) {
        const rstl::vector< CSpatialPrimitive::SSphere >& spheres =
            (*GetAnimationData()->GetSpatialPrimitive())->GetSpheres();
        const uint sphereCount = spheres.size();
        descs.reserve(sphereCount);
        for (uint i = 0; i < sphereCount; ++i) {
          const CSpatialPrimitive::SSphere& sphere = spheres[i];
          descs.push_back_unsafe(CJointCollisionDescription::SphereCollision(
              sphere.mFirstSegment, sphere.mSphere.GetCenter(), sphere.mSphere.GetRadius(),
              rstl::string_l("sphere") + CStringExtras::CreateFromInteger(i), 0.001f));
        }
        mCollisionManager = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(),
                                                          descs, GetActive());
        mCollisionManager->AddMaterialList(mgr,
                                           CMaterialList(kMT_CameraPassthrough, kMT_Immovable));
        for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
          const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
          if (CActor* colAct = TCastToPtr< CActor >(mgr.ObjectById(desc.GetCollisionActorId()))) {
            CMaterialList& materials = colAct->MaterialList();
            materials.Add(spheres[i].x8_);
            materials.Add(kMT_NoStepLogic);
            materials.Add(kMT_NoPlatformCollision);
            materials.Remove(kMT_Lava);
            materials.Remove(kMT_Dirt);
            colAct->SetMaterialFilter(
                CMaterialFilter(GetMaterialFilter().GetIncludeList().Union(
                                    colAct->GetMaterialFilter().GetIncludeList()),
                                GetMaterialFilter().GetExcludeList().Union(
                                    colAct->GetMaterialFilter().GetExcludeList()),
                                CMaterialFilter::kFT_IncludeExclude));
          }
        }
      }
    }
    if (ActorLights()) {
      ActorLights()->SetNeedsRelight(true);
    }
    {
      const CVector3f lightingOffset =
          GetTransform().BuildMatrix3f() *
          GetScaledLocatorTransform(rstl::string_l(skArmEndLocator)).GetTranslation();
      ActorLights()->SetLightingPositionOffset(lightingOffset);
    }
    break;
  case kSM_XHIT:
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      const TUniqueId touched = colAct->GetLastTouchedObject();
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(touched))) {
        if (mCurDamageRemTime <= 0.f && mState != kS_Flinch && mState != kS_Withdraw) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
      }
    }
    break;
  case kSM_Delete:
    mCollisionManager->Destroy(mgr);
    break;
  case kSM_Alert:
    mShouldGrab = true;
    break;
  case kSM_Activate:
    if (ActorLights()) {
      ActorLights()->SetNeedsRelight(true);
    }
    break;
  case kSM_Decrement:
    if (mState != kS_Sleep && mState != kS_Spawn && mState != kS_Withdraw && mState != kS_Flinch) {
      mAttacked = true;
      mDamageCooldownTimer = mDamageWaitTime;
    }
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionManager.get()) {
      mCollisionManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Damage:
  case kSM_ResistedDamage:
    if (mState != kS_Sleep && mState != kS_Spawn && mState != kS_Withdraw && mState != kS_Flinch) {
      const CCollisionActor* colAct =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(senderId));
      if (colAct) {
        const TUniqueId touched = colAct->GetLastTouchedObject();
        const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(touched));
        const CPowerBomb* powerBomb = TCastToConstPtr< CPowerBomb >(mgr.GetObjectById(touched));
        if (bomb || powerBomb) {
          mAttacked = true;
          mDamageCooldownTimer = mDamageWaitTime;
        }
      }
    }
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

void CWispTentacle::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

bool CWispTentacle::ShouldSleep(CStateManager& mgr, const CTriggerData& data) const {
  return !ShouldPatrol(mgr, data);
}

bool CWispTentacle::ShouldPatrol(CStateManager& mgr, const CTriggerData&) const {
  return IsPlayerNearby(mWakeUpDistance, mgr);
}

bool CWispTentacle::ShouldSearch(CStateManager& mgr, const CTriggerData&) const {
  return IsPlayerNearby(mSearchDistance, mgr);
}

bool CWispTentacle::ShouldAttack(CStateManager& mgr, const CTriggerData&) const {
  return IsPlayerNearby(mAttackDistance, mgr);
}

bool CWispTentacle::ShouldGrab(CStateManager&, const CTriggerData&) const { return mShouldGrab; }

bool CWispTentacle::Attacked(CStateManager&, const CTriggerData&) const { return mAttacked; }

bool CWispTentacle::Delay(CStateManager&, const CTriggerData&) const {
  if (mAttacked) {
    if (mStateMachine->GetTime() > mHurtSleepDelay) {
      mAttacked = false;
      return true;
    }
    return false;
  }
  return true;
}

bool CWispTentacle::IsPlayerNearby(float distance, CStateManager& mgr) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CVector3f playerPos = mgr.GetPlayer(i)->GetTranslation();
    if (mDetectionHeight > 0.f &&
        CMath::AbsF(playerPos.GetZ() - GetTranslation().GetZ()) >= mDetectionHeight) {
      continue;
    }
    if ((playerPos - mArmEndPosition).MagSquared() < distance * distance) {
      return true;
    }
  }
  return false;
}

bool CWispTentacle::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

void CWispTentacle::Sleep(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_Sleep;
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    SetTransform(mInitialTransform);
    SnapWaypointToFloor(mgr);
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Sleep;
    break;
  }
}

void CWispTentacle::Spawn(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Solid, mgr);
    mState = kS_Spawn;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(
        CBCStepCmd(pas::kSD_Forward, mSpawnFromPortal ? pas::kStep_BreakDodge : pas::kStep_Normal));
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(
          pas::kSD_Forward, mSpawnFromPortal ? pas::kStep_BreakDodge : pas::kStep_Normal));
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Spawn;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    SnapWaypointToFloor(mgr);
    break;
  }
}

void CWispTentacle::Withdraw(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Solid, mgr);
    mState = kS_Withdraw;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(
        pas::kSD_Backward, mSpawnFromPortal ? pas::kStep_BreakDodge : pas::kStep_Normal));
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(
          pas::kSD_Backward, mSpawnFromPortal ? pas::kStep_BreakDodge : pas::kStep_Normal));
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Withdraw;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CWispTentacle::Patrol(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    AddMaterial(kMT_Solid, mgr);
    mState = kS_Patrol;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Patrol;
    break;
  }
}

void CWispTentacle::Search(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_Search;
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Search;
    break;
  }
}

void CWispTentacle::Attack(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_Attack;
    BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Two));
    break;
  case kStateMsg_Update:
    if (BodyController()->GetCurrentStateId() != pas::kAS_LoopAttack) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Two));
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Attack;
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void CWispTentacle::Flinch(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_Flinch;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Flinch;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CWispTentacle::UpdateGrabbedPirate(float dt, CStateManager& mgr) {
  CSpacePirate* pirate = TCastToPtr< CSpacePirate >(mgr.ObjectById(mPirateId));
  if (!pirate) {
    return;
  }
  mGrabTimer += dt;
  const float t = rstl::min_val(1.f, mGrabTimer / mGrabBlendTime);
  const CQuaternion rotation =
      CQuaternion::Slerp(mPirateStartRotation, CQuaternion::FromMatrix(GetTransform()), t);
  const CTransform4f clawXf =
      GetTransform() * GetScaledLocatorTransform(rstl::string_l(skClawAttachLocator));
  const CVector3f spineOffset =
      pirate->GetScaledLocatorTransform(rstl::string_l("Spine_1")).GetTranslation();
  const CVector3f rotatedOffset = rotation.BuildTransform() * spineOffset;
  const CVector3f target = clawXf.GetTranslation() - rotatedOffset;
  const CVector3f position = target * t + mPirateStartPosition * (1.f - t);
  pirate->SetTransform(CTransform4f(rotation.BuildTransform(), position));
  pirate->SetPortalPlane(mPortalPlane);
}

void CWispTentacle::Grab(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kS_Grab;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CWispTentacle::Pull(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Two));
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Two));
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = kS_Grab;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mShouldGrab = false;
    mPirateCaptured = false;
    mGrabTimer = 0.f;
    mAttacked = true;
    break;
  }
}

void CWispTentacle::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  if (mSpawnFromPortal) {
    const CModelFlags flags = GetModelFlags();
    SetModelFlags(CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_Unknown80));
  }
}

void CWispTentacle::Render(const CStateManager& mgr) const {
  if (mState != kS_Sleep) {
    GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), mPortalPlane);
  }
  CPatterned::Render(mgr);
}

void CWispTentacle::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  HealthInfo()->SetHP(1000000.f);
  if (mState != kS_Sleep) {
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f locator = GetLocatorTransform(rstl::string_l(skLockOnLocator)).GetTranslation();
    const CVector3f offset = GetTransform().Rotate(CVector3f(scale.GetX() * locator.GetX(),
                                                             scale.GetY() * locator.GetY(),
                                                             scale.GetZ() * locator.GetZ()));
    MoveCollisionPrimitive(offset);
    SetTransformDirty();
  }
  CPatterned::Think(dt, mgr);
  mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_WorldSpace);
  if (!mLocatorsCached) {
    const CSegId lockOnSeg =
        ModelData()->AnimationData()->GetLocatorSegId(rstl::string_l(skLockOnLocator));
    if (lockOnSeg.val() != 0xff) {
      const CTransform4f locatorXf =
          ModelData()->AnimationData()->GetLocatorTransform(lockOnSeg, nullptr);
      const CTransform4f xf =
          GetTransform() * (CTransform4f::Scale(GetModelData()->GetScale()) * locatorXf);
      mLockOnOffset = xf.GetTranslation() - GetTranslation();
    }
    const CSegId armEndSeg =
        ModelData()->AnimationData()->GetLocatorSegId(rstl::string_l(skArmEndLocator));
    if (armEndSeg.val() != 0xff) {
      mArmEndPosition = (GetTransform() * GetScaledLocatorTransform(armEndSeg)).GetTranslation();
    }
    mLocatorsCached = true;
  }
  if (mPirateCaptured) {
    UpdateGrabbedPirate(dt, mgr);
  }
}

void CWispTentacle::Touch(CActor& actor, CStateManager& mgr) { CPatterned::Touch(actor, mgr); }

CVector3f CWispTentacle::GetAimPosition(const CStateManager&, float dt) const {
  CVector3f offset = CVector3f::Zero();
  if (dt > 0.f) {
    const CMotionState motion = PredictMotion(dt);
    offset = motion.GetTranslation();
  }

  const CAnimData* animData = GetModelData()->GetAnimationData();
  const CSegId lockOnSeg = animData->GetLocatorSegId(rstl::string_l(skLockOnLocator));
  if (lockOnSeg.val() != 0xff) {
    const CTransform4f locatorXf = animData->GetLocatorTransform(lockOnSeg, nullptr);
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f scaledOrigin = CVector3f::ByElementMultiply(scale, locatorXf.GetTranslation());
    offset += GetTransform() * scaledOrigin;
  } else {
    offset += GetBoundingBox().GetCenterPoint();
  }
  return offset;
}

CVector3f CWispTentacle::GetOrbitPosition(const CStateManager& mgr) const {
  const CVector3f base = CPatterned::GetOrbitPosition(mgr);
  float t = rstl::min_val(1.f, mStateMachine->GetTime());
  const CVector3f lockOnPosition = GetTranslation() + mLockOnOffset;
  if (mState == kS_Attack && mPreviousState == kS_Search) {
    return CVector3f::Lerp(base, lockOnPosition, t);
  }
  if (mState == kS_Search && mPreviousState == kS_Attack) {
    return CVector3f::Lerp(lockOnPosition, base, t);
  }
  return base;
}

void CWispTentacle::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType type, float dt) {
  switch (type) {
  case kUE_ObjectPickUp:
    if (CSpacePirate* pirate = TCastToPtr< CSpacePirate >(mgr.ObjectById(mPirateId))) {
      mPirateCaptured = pirate->TryToBeCaptured(mgr);
      if (mPirateCaptured) {
        const CTransform4f& pirateXf = pirate->GetTransform();
        mPirateStartRotation = CQuaternion::FromMatrix(pirateXf);
        mPirateStartPosition = pirateXf.GetTranslation();
      }
    }
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CWispTentacle::SnapWaypointToFloor(CStateManager& mgr) {
  if (mWaypointId != kInvalidUniqueId) {
    const CRayCastResult result =
        mgr.RayStaticIntersection(GetTransform() * mGrabOffset, CVector3f::Down(), 100.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
    if (result.IsValid()) {
      const CVector3f& point = result.GetPoint();
      if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mWaypointId))) {
        waypoint->SetTranslation(point);
      }
    }
  }
}

void CWispTentacle::KnockBack(CStateManager&, const CKnockBackInfo& info) {
  if (info.GetDamageInfo().GetVulnerableDamage(*GetDamageVulnerability()) > 0.f) {
    mAttacked = true;
    mDamageCooldownTimer = mDamageWaitTime;
  }
}

CEntity* REL_LoadWispTentacle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrWispTentacle sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrWispTentacle.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CWispTentacle(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToDamageInfo(sldrThis.attackDamage), LdrToActorParameters(sldrThis.actorInformation),
      sldrThis.spawnFromPortal, sldrThis.wakeUpDistance, sldrThis.searchDistance,
      sldrThis.attackDistance, sldrThis.detectionHeight, sldrThis.hurtSleepDelay,
      sldrThis.grabBlendTime);
}

static void SetFuncPtrs() {
  static SWispTentacle_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadWispTentacle;
  SetSWispTentacle_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSWispTentacle_FuncPtrs(nullptr); }
