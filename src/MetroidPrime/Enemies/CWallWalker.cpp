#include "MetroidPrime/Enemies/CWallWalker.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWallWalker.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CTargetableProjectile.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

CWallWalker::CWallWalker(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& mData,
                         const CActorParameters& actParms, const CPatternedInfo& pInfo,
                         const CWallWalkerData& data)
: CWallCrawler(kPAI_WallWalker, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
               kBT_WallWalker, actParms, pInfo.GetHalfExtent(), data.mStickyReach,
               data.mFloorTurnSpeed, data.mWaypointApproachDistance, data.mVisibleDistance,
               kT_WallWalker, false, 1.f, 0.167f, 0.6f, 1.5f, 0.6f, 1.5f)
, mCollisionActorManager(nullptr)
, mPatrolState(kPS_Patrol)
, mLegVulnerability(data.mLegVulnerability)
, mGrenadeData(data.mGrenadeData)
, mLeftLegHit(false)
, mRightLegHit(false)
, mExploded(false)
, mLegHitByMissile(false)
, mProjectile(data.mProjectile)
, mProjectileVisorParticle(data.mProjectileVisorParticle)
, mProjectileDamage(data.mProjectileDamage)
, mProjectileShakeData(data.mProjectileShakeData)
, mNumShots(0)
, mProjectileTimer(data.mProjectileInterval)
, mProjectileInterval(data.mProjectileInterval)
, mProjectileStopHomingRange(data.mProjectileStopHomingRange)
, mDeathTime(0.f)
, mGrenadeId(kInvalidUniqueId) {
  SetDrawShadow(false);
  mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, false);
  mKnockBackController.EnableKnockBackPhysics(false);
  mWaypointNavigation.SetHorizontalMovement(true);
}

CWallWalker::~CWallWalker() {}

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CWallWalker::Patrol)},
    {"LeftLegHitReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CWallWalker::LeftLegHitReaction)},
    {"RightLegHitReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CWallWalker::RightLegHitReaction)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CWallWalker::Dead)},
    {"ShootProjectile",
     static_cast< CPatterned::StateMachine::StateFunc >(&CWallWalker::ShootProjectile)},
};

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"IsLeftLegHit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWallWalker::IsLeftLegHit)},
    {"IsRightLegHit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWallWalker::IsRightLegHit)},
    {"AreBothLegsHit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWallWalker::AreBothLegsHit)},
    {"ShouldShootProjectile",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CWallWalker::ShouldShootProjectile)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SetNumberShots",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CWallWalker::SetNumberShots)},
};

void CWallWalker::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

void CWallWalker::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CWallCrawler::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    SetupCollisionManager(mgr);
    break;
  case kSM_Delete:
    DestroyCollisionManager(mgr);
    break;
  case kSM_ReflectedDamage:
  case kSM_ResistedDamage:
    if (!TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(msg.GetSenderId()))) {
      BodyController()->CommandMgr().DeliverCmd(CBCAdditiveFlinchCmd(1.f));
    }
    break;
  case kSM_Damage:
    if (const CCollisionActor* colAct =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(msg.GetSenderId()))) {
      if (const CWeapon* weapon =
              TCastToConstPtr< CWeapon >(mgr.GetObjectById(colAct->GetLastTouchedObject()))) {
        if (weapon->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Missile &&
            !weapon->HasAttrib(CWeapon::kPA_Unknown22)) {
          mLegHitByMissile = true;
        }
      }
    }
    break;
  }
}

void CWallWalker::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (mPatrolState) {
  case kPS_Patrol:
    if (msg == kStateMsg_Update && mWaypointNavigation.IsInPosition()) {
      if (const CScriptAIWaypoint* waypoint = TCastToConstPtr< CScriptAIWaypoint >(
              mgr.GetObjectById(mWaypointNavigation.GetDestination()))) {
        if (waypoint->GetFlags() & 0x200) {
          mPatrolState = kPS_StartGenerate;
        }
      }
    }
    CPatterned::Patrol(mgr, msg, dt);
    break;
  case kPS_StartGenerate:
    mPatrolState = kPS_Generate;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kPS_Generate:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    } else if (mAnimationState.IsOver()) {
      mPatrolState = kPS_Patrol;
    }
    break;
  }
}

void CWallWalker::LeftLegHitReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Zero(), pas::kS_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mLeftLegHit = false;
    mLegHitByMissile = false;
    break;
  }
}

void CWallWalker::RightLegHitReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Zero(), pas::kS_One));
    }
    break;
  case kStateMsg_Deactivate:
    mRightLegHit = false;
    mLegHitByMissile = false;
    break;
  }
}

void CWallWalker::ShootProjectile(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mProjectileTimer = mProjectileInterval;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCProjectileAttackCmd(pas::kS_Zero, CVector3f::Zero(), false));
    }
    break;
  case kStateMsg_Deactivate:
    mProjectileTimer = mProjectileInterval;
    break;
  }
}

bool CWallWalker::IsLeftLegHit(CStateManager& mgr, const CTriggerData& data) const {
  return mLeftLegHit;
}

bool CWallWalker::IsRightLegHit(CStateManager& mgr, const CTriggerData& data) const {
  return mRightLegHit;
}

bool CWallWalker::AreBothLegsHit(CStateManager& mgr, const CTriggerData& data) const {
  return mLeftLegHit & mRightLegHit;
}

bool CWallWalker::ShouldShootProjectile(CStateManager& mgr, const CTriggerData& data) const {
  if (!mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 4)) {
    return false;
  }
  if (mNumShots > 0) {
    return true;
  }
  if (CVector3f(mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() <
          mDetectionRange * mDetectionRange &&
      mProjectileTimer < 0.f) {
    return true;
  }
  return false;
}

void CWallWalker::SetNumberShots(CStateManager& mgr, float arg) {
  mNumShots = mgr.Random()->Range(1, 4);
}

struct SSphereJointInfo {
  const char* name;
  float radius;
};

static const SSphereJointInfo skSphereJointList[] = {
    {"LeftLeg_3", 2.f},
    {"RightLeg_3", 2.f},
};

void CWallWalker::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(2);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (int i = 0; i < ARRAY_SIZE(skSphereJointList); ++i) {
    const SSphereJointInfo& joint = skSphereJointList[i];
    const CSegId seg = animData->GetLocatorSegId(rstl::string_l(joint.name));
    const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
        seg, CVector3f::Zero(), joint.radius, rstl::string_l(joint.name), 0.001f);
    joints.push_back_unsafe(desc);
  }

  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, GetActive());
  RemoveMaterial(kMT_SeekerTarget, kMT_Orbit, mgr);
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      colAct->AddMaterial(kMT_SeekerTarget, kMT_Orbit, mgr);
      colAct->SetDamageVulnerability(mLegVulnerability);
      colAct->HealthInfo()->SetHP(5.f);
    }
  }
}

void CWallWalker::DestroyCollisionManager(CStateManager& mgr) {
  if (mCollisionActorManager.get() != nullptr) {
    mCollisionActorManager->Destroy(mgr);
    mCollisionActorManager = nullptr;
  }
}

void CWallWalker::UpdateCollisionManager(float dt, CStateManager& mgr) {
  if (mCollisionActorManager.get() != nullptr) {
    mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
      const TUniqueId id =
          mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
      if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
        if (colAct->GetHealthInfo()->GetHP() < 5.f) {
          colAct->HealthInfo()->SetHP(5.f);
          if (!mLegHitByMissile || (!mLeftLegHit && !mRightLegHit)) {
            switch (i) {
            case 0:
              mLeftLegHit = true;
              break;
            case 1:
              mRightLegHit = true;
              break;
            }
          }
        }
      }
    }
  }
}

void CWallWalker::Think(float dt, CStateManager& mgr) {
  if (!mExploded) {
    mProjectileTimer -= dt;
    CWallCrawler::Think(dt, mgr);
    SetMovable(!mAlignToFloor);
    UpdateCollisionManager(dt, mgr);
    return;
  }

  CActor::Think(dt, mgr);
  if (const CActor* grenade = TCastToConstPtr< CActor >(mgr.GetObjectById(mGrenadeId))) {
    SetTranslation(grenade->GetTranslation());
    return;
  }

  if (!mBurning) {
    const CWeaponMode deathWeapon = GetHealthInfo()->GetCauseOfDeathWeapon();
    if (close_enough(mDeathTime, 0.f)) {
      if (deathWeapon.GetType() == kWT_Dark && deathWeapon.IsCharged() == true) {
        SendScriptMsgs(kSS_IceXDamage, mgr);
      } else if (IsIngPossessed() == true) {
        SendScriptMsgs(kSS_DarkXDamage, mgr);
      } else {
        SendScriptMsgs(kSS_XDamage, mgr);
      }
    }
  }
  mDeathTime += dt;
  if (mDeathTime > 1.f) {
    DeathDelete(mgr);
  }
}

void CWallWalker::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
}

static CVector3f RandomVectorInCone(CStateManager& mgr, float coneAngle, float minMagnitude,
                                    float maxMagnitude) {
  const float magnitude = (maxMagnitude - minMagnitude) * mgr.Random()->Float() + minMagnitude;
  const float cosAngle = CMath::FastCosR((M_PIF / 360.f) * coneAngle);
  const float z = 1.f - (1.f - cosAngle) * mgr.Random()->Float();
  const float zSq = z * z;
  const float radius = magnitude * CMath::FastSqrtF(rstl::max_val(1.f - zSq, 0.f));
  const float theta = (2.f * M_PIF) * mgr.Random()->Float();
  return CVector3f(radius * CMath::FastCosR(theta), radius * CMath::FastSinR(theta), magnitude * z);
}

void CWallWalker::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate: {
    CSfxManager::AddEmitter(mDeathSfx, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                            CSfxManager::kMedPriority);
    if (!mBurning) {
      CreateXDamageParticles(mgr);
    }
    mExploded = true;
    DestroyCollisionManager(mgr);
    SetDrawEnabled(false);
    mGrenadeId = mgr.AllocateUniqueId();
    CBouncyGrenade* const grenade = rs_new CBouncyGrenade(
        mGrenadeId, rstl::string_l("Inglet"),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
        CTransform4f::LookAt(GetTranslation(), GetTranslation() - CVector3f::Up(), CVector3f::Up()),
        CModelData::CModelDataNull(), CActorParameters::None(), GetUniqueId(), 1.f, mGrenadeData,
        0.5f, CAABox::MakeMaxInvertedBox(), kInvalidUniqueId, 0.f, 7, nullptr, nullptr);
    const CAxisAngle angle(RandomVectorInCone(mgr, 360.f, 5.f, 8.f));
    grenade->SetAngularImpulseWR(GetAngularImpulseWR() + angle);
    mgr.AddObject(grenade);
    break;
  }
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CWallWalker::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                  EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f lctrXf = GetLctrTransform(node.GetLocatorName());
    const CVector3f pos = lctrXf.GetTranslation();
    const CTransform4f xf = CTransform4f::LookAt(pos, pos + CVector3f::Down(), CVector3f::Up());
    LaunchProjectiles(xf, mgr);
    handled = true;
    break;
  }
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CWallWalker::LaunchProjectiles(const CTransform4f& xf, CStateManager& mgr) {
  float angle = 0.f;
  float angleStep = 0.f;
  if (mNumShots > 1) {
    angle = -45.f;
    angleStep = 90.f / static_cast< float >(mNumShots - 1);
  }

  for (int i = 0; i < mNumShots; ++i) {
    if (mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 4)) {
      const CTransform4f shotXf = CTransform4f::Translate(xf.GetTranslation()) *
                                  GetTransform().GetRotation() *
                                  CTransform4f::RotateY(CRelAngle::FromDegrees(angle)) *
                                  CTransform4f::RotateX(CRelAngle::FromRadians(M_PIF / 2.f));
      angle += angleStep;
      CTargetableProjectile* projectile = rs_new CTargetableProjectile(
          mProjectile, kWT_AI, shotXf, kMT_Character, mProjectileDamage, CDamageInfo(),
          mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), mProjectile,
          mgr.GetPlayer(0)->GetUniqueId(), 0,
          CImpactVisorEffect::ParticleEffect(
              rstl::optional_object< TLockedToken< CGenDescription > >(mProjectileVisorParticle),
              CSfxManager::kInternalInvalidSfxId, false),
          CVector3f(0.5f, 0.5f, 0.5f));
      projectile->SetMinHomingDistance(mProjectileStopHomingRange);
      projectile->SetDeflectToOwner(false);
      mgr.AddObject(projectile);
      projectile->RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      projectile->SetCameraShakerData(mProjectileShakeData);
    }
  }
  mNumShots = 0;
}

CEntity* LoadWallWalker(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrWallWalker sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrWallWalker.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  TLockedToken< CWeaponDescription > projectile =
      gpSimplePool->GetObj(SObjectTag('WPSC', sldrThis.projectile));
  TLockedToken< CGenDescription > projectileVisorParticle =
      gpSimplePool->GetObj(SObjectTag('PART', sldrThis.pART));
  const CBouncyGrenadeData grenadeData(
      sldrThis.grenadeMass, sldrThis.unknown_0xed086ce0, LdrToDamageInfo(sldrThis.explodeDamage),
      sldrThis.unknown_0x454f16b1, sldrThis.grenadeExplosion, sldrThis.grenadeExplosion,
      sldrThis.grenadeTrail, sldrThis.grenadeEffect, sldrThis.grenadeSoundBounce,
      sldrThis.grenadeSoundExplode, 0.1f, 150.f, 0.1f, 150.f, true);
  const CWallWalkerData data(
      sldrThis.stickyReach, sldrThis.floorTurnSpeed, sldrThis.waypointApproachDistance,
      sldrThis.visibleDistance, sldrThis.projectileInterval, sldrThis.projectileStopHomingRange,
      LdrToDamageVulnerability(sldrThis.legVulnerability), projectile, projectileVisorParticle,
      LdrToDamageInfo(sldrThis.projectileDamage),
      LdrToCameraShakerData(sldrThis.projectileExplosionShaker, CVector3f::Zero()), grenadeData);

  return rs_new CWallWalker(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                            LdrToEntityInfo(info, sldrThis.editorProperties),
                            LdrToTransform4f(sldrThis.editorProperties), *modelData,
                            LdrToActorParameters(sldrThis.actorInformation),
                            LdrToPatternedInfo(sldrThis.patterned, nullptr), data);
}

#ifndef MONOLITHIC
SWallWalker_FuncPtrs REL_loader_WallWalker;

void SetRelLoaderFunctionToLoader() {
  REL_loader_WallWalker.mLoadWallWalker = LoadWallWalker;
  SetSWallWalker_FuncPtrs(&REL_loader_WallWalker);
}

extern "C" void RELMain() { SetRelLoaderFunctionToLoader(); }

extern "C" void RELExit() { SetSWallWalker_FuncPtrs(nullptr); }
#endif
