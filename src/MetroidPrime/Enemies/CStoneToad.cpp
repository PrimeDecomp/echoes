#include "MetroidPrime/Enemies/CStoneToad.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CSpatialPrimitive.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrStoneToad.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "REL/REL_Setup.h"
#include "rstl/StringExtras.hpp"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::ShouldPatrol)},
    {"ShouldMoveAway",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::ShouldMoveAway)},
    {"MoveAwayFinished",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::MoveAwayFinished)},
    {"ShouldMoveBack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::ShouldMoveBack)},
    {"MoveBackFinished",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::MoveBackFinished)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::AnimOver)},
    {"HitBySafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::HitBySafeZone)},
    {"HitByWeapon", static_cast< CPatterned::StateMachine::TriggerFunc >(&CStoneToad::HitByWeapon)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Sleep", static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::Sleep)},
    {"WakeUp", static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::WakeUp)},
    {"MoveAway", static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::MoveAway)},
    {"Wait", static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::Wait)},
    {"MoveBack", static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::MoveBack)},
    {"GoToSleep", static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::GoToSleep)},
    {"ReactToSafeZone",
     static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::ReactToSafeZone)},
    {"ReactToWeapon",
     static_cast< CPatterned::StateMachine::StateFunc >(&CStoneToad::ReactToWeapon)},
};
CStoneToad::CStoneToad(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& modelData,
                       const CPatternedInfo& patternedInfo, const CActorParameters& actorParams)
: CPatterned(kPAI_StoneToad, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mContactDamageCopy(patternedInfo.GetContactDamage())
, mWaypoints()
, mInitialTransform(CTransform4f::Identity())
, mAwayPosition(CVector3f::Zero())
, mMoveDirection(CVector3f::Forward())
, mAwayDelay(0.f)
, mLookUpAnim(0)
, mLookLeftAnim(0)
, mLookRightAnim(0)
, mLookDownAnim(0)
, mLookUpWeight(0.f)
, mLookDownWeight(0.f)
, mLookLeftWeight(0.f)
, mLookRightWeight(0.f)
, mCollisionActorManager(nullptr)
, mProvoked(false)
, mReturnRequested(false) {
  const CPASDatabase& database = GetAnimationData()->GetPASDatabase();

  const CPASAnimParmData leftParms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(0),
                                   CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > left = database.FindBestAnimation(leftParms, -1);
  if (left.first > FLT_EPSILON) {
    mLookLeftAnim = left.second;
  }

  const CPASAnimParmData rightParms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(1),
                                    CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > right = database.FindBestAnimation(rightParms, -1);
  if (right.first > FLT_EPSILON) {
    mLookRightAnim = right.second;
  }

  const CPASAnimParmData upParms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(2),
                                 CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > up = database.FindBestAnimation(upParms, -1);
  if (up.first > FLT_EPSILON) {
    mLookUpAnim = up.second;
  }

  const CPASAnimParmData downParms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(3),
                                   CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > down = database.FindBestAnimation(downParms, -1);
  if (down.first > FLT_EPSILON) {
    mLookDownAnim = down.second;
  }
}

void CStoneToad::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CStoneToad::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool wasActive = GetActive();
  const TUniqueId senderId = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    mWaypoints.reserve(8);
    rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
    for (; it != GetConnectionList().end(); ++it) {
      const EScriptObjectState state = it->state;
      const TUniqueId id = mgr.GetIdForScript(it->objId);
      if (state == kSS_Approach) {
        if (TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id)) != nullptr) {
          mWaypoints.push_back_unsafe(id);
        }
      }
    }
    break;
  }
  case kSM_Create: {
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Crouch);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }

    {
      rstl::vector< CJointCollisionDescription > joints;
      if (HasAnimation() && GetAnimationData()->GetSpatialPrimitive()) {
        const CSpatialPrimitive* primitive = **GetAnimationData()->GetSpatialPrimitive();
        const rstl::vector< CSpatialPrimitive::SSphere >& spheres = primitive->GetSpheres();
        const uint sphereCount = spheres.size();
        joints.reserve(sphereCount);
        for (uint i = 0; i < sphereCount; ++i) {
          const CSpatialPrimitive::SSphere& sphere = spheres[i];
          const CSegId segId = sphere.mFirstSegment;
          const CSphere& bounds = sphere.mSphere;
          const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
              segId, bounds.GetCenter(), bounds.GetRadius(),
              rstl::string_l("sphere") + CStringExtras::CreateFromInteger(i), 0.001f);
          joints.push_back_unsafe(desc);
        }

        mCollisionActorManager = rs_new CCollisionActorManager(
            mgr, GetUniqueId(), GetCurrentAreaId(), joints, GetActive());
        mCollisionActorManager->AddMaterialList(
            mgr, CMaterialList(kMT_CameraPassthrough, kMT_Immovable));
        for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
          const TUniqueId id =
              mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
          if (CCollisionActor* colAct = static_cast< CCollisionActor* >(mgr.ObjectById(id))) {
            colAct->AddMaterial(spheres[i].x8_);
            colAct->MaterialList().Add(kMT_AIPassthrough);
            colAct->MaterialList().Add(kMT_SolidCharacter);
            colAct->MaterialList().Remove(kMT_Orbit);
            colAct->MaterialList().Remove(kMT_Target);
            const u64 ownInclude = GetMaterialFilter().GetIncludeList().GetValue();
            const u64 ownExclude = GetMaterialFilter().GetExcludeList().GetValue();
            const u64 actorInclude = colAct->GetMaterialFilter().GetIncludeList().GetValue();
            const u64 actorExclude = colAct->GetMaterialFilter().GetExcludeList().GetValue();
            colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
                CMaterialList(ownInclude | actorInclude),
                CMaterialList(ownExclude | (u64(1) << kMT_Character) | actorExclude)));
            colAct->SetResponseType(
                GetCollisionResponseType(CVector3f::Zero(), CVector3f::Zero(),
                                         CWeaponMode(kWT_None, false, false, false), 0));
            const CHealthInfo health = *GetHealthInfo();
            colAct->SetDamageVulnerability(*GetDamageVulnerability());
            *colAct->HealthInfo() = health;
          }
        }
      }
    }
    mInitialTransform = GetTransform();
    mLockOnTarget = CSegId(1);
    break;
  }
  case kSM_AIUpdateDisabled:
    if (mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Delete:
    mCollisionActorManager->Destroy(mgr);
    // Fallthrough
  case kSM_Deactivate:
    mCollisionActorManager->SetActive(mgr, false);
    break;
  case kSM_Increment:
    mProvoked = true;
    SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
    mProvoker = kP_SafeZone;
    break;
  case kSM_Decrement:
    mReturnRequested = true;
    break;
  case kSM_Damage:
  case kSM_ResistedDamage: {
    TUniqueId hitterId = senderId;
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      hitterId = colAct->GetLastTouchedObject();
    }
    const CGameProjectile* projectile =
        TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(hitterId));
    const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(hitterId));
    const CPowerBomb* powerBomb = TCastToConstPtr< CPowerBomb >(mgr.GetObjectById(hitterId));
    if ((projectile != nullptr && GetDamageVulnerability()->WeaponHurts(
                                      projectile->GetCurrentDamageInfo().GetWeaponMode())) ||
        (bomb != nullptr &&
         GetDamageVulnerability()->WeaponHurts(bomb->GetCurrentDamageInfo().GetWeaponMode())) ||
        (powerBomb != nullptr && GetDamageVulnerability()->WeaponHurts(
                                     powerBomb->GetCurrentDamageInfo().GetWeaponMode()))) {
      mProvoked = true;
      mReturnRequested = false;
      mProvoker = kP_Weapon;
      SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  }
  case kSM_Alert:
  case kSM_XHIT:
    break;
  }

  CPatterned::AcceptScriptMsg(mgr, msg);
  if (wasActive != GetActive() && mCollisionActorManager.get() != nullptr) {
    mCollisionActorManager->SetActive(mgr, GetActive());
  }
}

void CStoneToad::PreThink(float dt, CStateManager& mgr) { CPatterned::PreThink(dt, mgr); }

void CStoneToad::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  if (mCollisionActorManager.get() != nullptr) {
    mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_WorldSpace);
  }
  UpdateLookAt(mgr);
  AnimationData()->AddAdditiveAnimation(mLookUpAnim, mLookUpWeight, false, false);
  AnimationData()->AddAdditiveAnimation(mLookDownAnim, mLookDownWeight, false, false);
  AnimationData()->AddAdditiveAnimation(mLookLeftAnim, mLookLeftWeight, false, false);
  AnimationData()->AddAdditiveAnimation(mLookRightAnim, mLookRightWeight, false, false);
}

void CStoneToad::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CStoneToad::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CStoneToad::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

void CStoneToad::Sleep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kTS_Sleep;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CStoneToad::WakeUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    mState = kTS_WakeUp;
    mProvoker = kP_None;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    const uint waypointCount = mWaypoints.size();
    for (uint i = 0; i < waypointCount; ++i) {
      TUniqueId waypointId = mWaypoints[i];
      const CActor* waypoint = static_cast< const CActor* >(mgr.GetObjectById(waypointId));
      if (waypoint != nullptr && waypoint->GetActive()) {
        mAwayPosition = waypoint->GetTranslation();
        break;
      }
    }
    break;
  }
  }
}

void CStoneToad::ReactToSafeZone(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    mState = kTS_ReactToSafeZone;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CStoneToad::ReactToWeapon(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Five, -1));
    mState = kTS_ReactToWeapon;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Five, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CStoneToad::MoveAway(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kTS_MoveAway;
    mMoveDirection = GetMoveDirection(kMV_Away);
    break;
  case kStateMsg_Update:
    if (IsMovingAwayFromStart()) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(mMoveDirection, mInitialTransform.GetForward(), 1.f));
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CStoneToad::Wait(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kTS_Wait;
    break;
  case kStateMsg_Update:
    mProvoked = false;
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CStoneToad::MoveBack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kTS_MoveBack;
    mMoveDirection = GetMoveDirection(kMV_Back);
    break;
  case kStateMsg_Update:
    if (IsMovingBackToStart()) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(mMoveDirection, mInitialTransform.GetForward(), 1.f));
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CStoneToad::GoToSleep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    mState = kTS_GoToSleep;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CStoneToad::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

bool CStoneToad::ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const {
  return mProvoked;
}

bool CStoneToad::MoveAwayFinished(CStateManager& mgr, const CTriggerData& data) const {
  return !IsMovingAwayFromStart();
}

bool CStoneToad::ShouldMoveAway(CStateManager& mgr, const CTriggerData& data) const {
  return mStateMachine->GetTime() > mAwayDelay;
}

bool CStoneToad::ShouldMoveBack(CStateManager& mgr, const CTriggerData& data) const {
  return !mProvoked && mReturnRequested;
}

bool CStoneToad::MoveBackFinished(CStateManager& mgr, const CTriggerData& data) const {
  return !IsMovingBackToStart();
}

bool CStoneToad::HitBySafeZone(CStateManager& mgr, const CTriggerData& data) const {
  return mProvoker == kP_SafeZone;
}

bool CStoneToad::HitByWeapon(CStateManager& mgr, const CTriggerData& data) const {
  return mProvoker == kP_Weapon;
}

void CStoneToad::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);
  if (TCastToPtr< CScriptSafeZone >(&actor) != nullptr && !mProvoked) {
    mProvoked = true;
    mReturnRequested = false;
    mProvoker = kP_SafeZone;
    SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
  }
}

bool CStoneToad::IsMovingAwayFromStart() const {
  const CVector3f direction = GetMoveDirection(kMV_Away);
  CVector3f toAway = mAwayPosition - GetTranslation();
  toAway.SetZ(0.f);
  if (toAway.CanBeNormalized()) {
    toAway.Normalize();
  }
  return CVector3f::Dot(direction, toAway) > 0.f;
}

bool CStoneToad::IsMovingBackToStart() const {
  const CVector3f direction = GetMoveDirection(kMV_Back);
  CVector3f toStart = mInitialTransform.GetTranslation() - GetTranslation();
  toStart.SetZ(0.f);
  if (toStart.CanBeNormalized()) {
    toStart.Normalize();
  }
  return CVector3f::Dot(direction, toStart) > 0.f;
}

CVector3f CStoneToad::GetMoveDirection(EMoveType type) const {
  CVector3f direction = CVector3f::Zero();
  switch (type) {
  case kMV_Away:
    if (!(mAwayPosition == CVector3f::Zero())) {
      direction = mAwayPosition - mInitialTransform.GetTranslation();
    }
    break;
  case kMV_Back:
    direction = mInitialTransform.GetTranslation() - mAwayPosition;
    break;
  }
  direction.SetZ(0.f);
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  }
  return direction;
}

static inline void ApproachWeight(float& weight, float target) {
  if (target > 0.f) {
    if (target > weight) {
      weight = CMath::Min(weight + 0.025f, target);
    } else {
      const float lowered = weight - 0.025f;
      weight = target < lowered ? lowered : target;
    }
  } else {
    const float lowered = weight - 0.025f;
    weight = 0.f < lowered ? lowered : 0.f;
  }
}

void CStoneToad::UpdateLookAt(CStateManager& mgr) {
  const CTransform4f toLocal = GetTransform().GetQuickInverse();
  const CVector3f eyePosition = GetScaledLocatorTransform(rstl::string_l("Eye")).GetTranslation();
  const CVector3f aimPosition = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
  const CVector3f targetPosition = toLocal * aimPosition;
  const CVector3f direction = (targetPosition - eyePosition).AsNormalized();
  const float angle =
      CMath::Min(60.f, static_cast< float >(fabs(
                           57.295776f * CVector3f::GetAngleDiff(CVector3f::Forward(), direction))));
  const float scale = angle / 60.f;
  const CVector2f lookOffset(CVector3f::Dot(CVector3f::Up(), direction),
                             CVector3f::Dot(CVector3f::Left(), direction));
  const CVector2f look = lookOffset.AsNormalized();

  if (look.GetX() >= 0.f) {
    ApproachWeight(mLookUpWeight, scale * CMath::AbsF(look.GetX()));
    ApproachWeight(mLookDownWeight, 0.f);
  } else {
    ApproachWeight(mLookDownWeight, scale * CMath::AbsF(look.GetX()));
    ApproachWeight(mLookUpWeight, 0.f);
  }

  if (look.GetY() >= 0.f) {
    ApproachWeight(mLookLeftWeight, scale * CMath::AbsF(look.GetY()));
    ApproachWeight(mLookRightWeight, 0.f);
  } else {
    ApproachWeight(mLookRightWeight, scale * CMath::AbsF(look.GetY()));
    ApproachWeight(mLookLeftWeight, 0.f);
  }
}

EWeaponCollisionResponseTypes CStoneToad::GetCollisionResponseType(const CVector3f& position,
                                                                   const CVector3f& direction,
                                                                   const CWeaponMode& mode,
                                                                   int attributes) const {
  return kWCR_EnemyShielded;
}

CEntity* REL_LoadStoneToad(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrStoneToad sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrStoneToad.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CStoneToad(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                           LdrToEntityInfo(info, sldrThis.editorProperties),
                           LdrToTransform4f(sldrThis.editorProperties), *modelData,
                           LdrToPatternedInfo(sldrThis.patterned, nullptr),
                           LdrToActorParameters(sldrThis.actorInformation));
}

static void SetFuncPtrs() {
  static SStoneToad_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadStoneToad;
  SetSStoneToad_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSStoneToad_FuncPtrs(nullptr); }
