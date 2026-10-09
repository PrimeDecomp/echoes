#include "MetroidPrime/Enemies/CEmperorIngStage2Tentacle.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage2Tentacle.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

// Guessed name: a collision box spanning two joints of the tentacle.
struct SJointPair {
  const char* from;
  const char* to;
  float scale;
};

static const SJointPair skJoints[] = {
    {"joint2", "joint4", 0.25f},  {"joint4", "joint6", 0.25f},   {"joint6", "joint8", 0.25f},
    {"joint8", "joint10", 0.25f}, {"joint10", "joint11", 0.25f},
};

static EMaterialTypes skTouchMaterial = kMT_Solid; // Guessed name

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Attack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CEmperorIngStage2Tentacle::Attack)},
    {"Retracted",
     static_cast< CPatterned::StateMachine::StateFunc >(&CEmperorIngStage2Tentacle::Retracted)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CEmperorIngStage2Tentacle::Dead)},
};

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldAttack", static_cast< CPatterned::StateMachine::TriggerFunc >(
                         &CEmperorIngStage2Tentacle::ShouldAttack)},
    {"ShouldRetract", static_cast< CPatterned::StateMachine::TriggerFunc >(
                          &CEmperorIngStage2Tentacle::ShouldRetract)},
};

static EMaterialTypes skCollisionMaterial = kMT_Solid;              // Guessed name
static EMaterialTypes skExcludeCollisionActor = kMT_CollisionActor; // Guessed name
static EMaterialTypes skExcludeCharacter = kMT_Character;           // Guessed name
static EMaterialTypes skExcludeAIPassthrough = kMT_AIPassthrough;   // Guessed name
static EMaterialTypes skExcludePlayer = kMT_Player;                 // Guessed name
static EMaterialTypes skExcludeProjectile = kMT_Projectile;         // Guessed name

CEmperorIngStage2Tentacle::CEmperorIngStage2Tentacle(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& modelData, const CActorParameters& actorParams,
    const CPatternedInfo& patternedInfo, const SLdrEmperorIngStage2TentacleData& data)
: CPatterned(kPAI_EmperorIngStage2Tentacle, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_Restricted, actorParams)
, mSpotTime(0.f)
, mLostTime(0.f)
, mActiveTime(0.f)
, mAttackTime(0.f)
, mAttacking(false)
, mCollisionActorManager(nullptr)
, mDetectionTime(data.detectionTime)
, mForgetTime(data.forgetTime) {
  SetDrawShadow(false);
}

CEmperorIngStage2Tentacle::~CEmperorIngStage2Tentacle() {}

void CEmperorIngStage2Tentacle::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CEmperorIngStage2Tentacle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    RemoveMaterial(kMT_Solid, kMT_Orbit, mgr);
    SetupCollisionActors(mgr);
    break;
  case kSM_Damage:
    if (mAttacking && HealthInfo()->GetHP() <= 0.f && mAlive) {
      SetEnableRender(false);
      mPendingDeath = true;
    }
    break;
  case kSM_Delete:
    DestroyCollisionActors(mgr);
    break;
  default:
    break;
  }
}

void CEmperorIngStage2Tentacle::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  UpdateCollisionActors(dt, mgr);
  if (mAttacking) {
    mActiveTime += dt;
    mAttackTime += dt;
  } else {
    mActiveTime = 0.f;
    mAttackTime = 0.f;
  }

  const CVector3f& playerPos = mgr.GetPlayer(0)->GetTranslation();
  const CVector3f& pos = GetTranslation();
  const CVector3f flatDelta(pos.GetX() - playerPos.GetX(), pos.GetY() - playerPos.GetY(), 0.f);
  if (flatDelta.MagSquared() < mDetectionRange * mDetectionRange) {
    mSpotTime += dt;
    mLostTime = 0.f;
  } else {
    mLostTime += dt;
    if (mLostTime > mForgetTime) {
      mSpotTime = 0.f;
    }
  }
}

void CEmperorIngStage2Tentacle::Touch(CActor& actor, CStateManager& mgr) {
  if (TCastToPtr< CPlayer >(actor)) {
    mgr.ApplyDamage(
        GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), GetContactDamage(),
        CMaterialFilter::MakeIncludeExclude(CMaterialList(skTouchMaterial), CMaterialList()),
        CVector3f::Zero());
  }
}

rstl::optional_object< CAABox > CEmperorIngStage2Tentacle::GetTouchBounds() const {
  if (mAttacking && mActiveTime > 0.5f) {
    const CAnimData* animData = GetModelData()->GetAnimationData();
    const CAABox box(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));
    const CTransform4f xf =
        GetTransform() * animData->GetLocatorTransform(rstl::string_l("joint10"), nullptr);
    return box.GetTransformedAABox(xf);
  }
  return rstl::optional_object< CAABox >();
}

void CEmperorIngStage2Tentacle::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mAttacking = true;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mAttacking = false;
    break;
  }
}

void CEmperorIngStage2Tentacle::Retracted(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mSpotTime = 0.f;
    mLostTime = 0.f;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CEmperorIngStage2Tentacle::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Update:
    if (BodyController()->GetCurrentStateId() == pas::kAS_LieOnGround) {
      DeathDelete(mgr);
    }
    break;
  default:
    break;
  }
}

bool CEmperorIngStage2Tentacle::ShouldAttack(CStateManager&, const CTriggerData&) const {
  return mSpotTime > mDetectionTime;
}

bool CEmperorIngStage2Tentacle::ShouldRetract(CStateManager&, const CTriggerData&) const {
  return mAttackTime > 3.f;
}

void CEmperorIngStage2Tentacle::SetupCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descs;
  descs.reserve(5);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (uint i = 0; i < ARRAY_SIZE(skJoints); ++i) {
    const SJointPair& joint = skJoints[i];
    const float scale = joint.scale;
    const CSegId from = animData->GetLocatorSegId(rstl::string_l(joint.from));
    const CSegId to = animData->GetLocatorSegId(rstl::string_l(joint.to));
    const CJointCollisionDescription desc = CJointCollisionDescription::OBBAutoSizeCollision(
        from, to, CVector3f(scale, scale, scale), CJointCollisionDescription::kOT_BetweenJoints,
        rstl::string_l(joint.from) + rstl::string("->") + rstl::string_l(joint.to), 10.f);
    descs.push_back_unsafe(desc);
  }

  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), descs, GetActive());
  for (int i = 0; i < static_cast< int >(mCollisionActorManager->GetNumCollisionActors()); ++i) {
    CCollisionActor* actor = static_cast< CCollisionActor* >(
        mgr.ObjectById(mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    const float hp = HealthInfo()->GetHP();
    actor->HealthInfo()->SetHP(hp);
  }

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skCollisionMaterial),
      CMaterialList(skExcludeCollisionActor, skExcludeCharacter, skExcludeAIPassthrough,
                    skExcludePlayer, skExcludeProjectile)));
  RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
}

void CEmperorIngStage2Tentacle::DestroyCollisionActors(CStateManager& mgr) {
  mCollisionActorManager->Destroy(mgr);
}

void CEmperorIngStage2Tentacle::UpdateCollisionActors(float dt, CStateManager& mgr) {
  float minHP = FLT_MAX;
  mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  const bool vulnerable = mAttacking && mActiveTime > 0.5f;
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(vulnerable ? *GetDamageVulnerability()
                                               : CDamageVulnerability::ImmuneVulnerabilty());
      const float hp = actor->HealthInfo()->GetHP();
      if (hp < minHP) {
        minHP = hp;
      }
    }
  }
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->HealthInfo()->SetHP(minHP);
    }
  }
  if (minHP < HealthInfo()->GetHP()) {
    mDamageCooldownTimer = skDamageHitTime;
  }
  HealthInfo()->SetHP(minHP);
}

CEntity* LoadEmperorIngStage2Tentacle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrEmperorIngStage2Tentacle sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage2Tentacle.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CEmperorIngStage2Tentacle(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.data);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SEmperorIngStage2Tentacle_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadEmperorIngStage2Tentacle;
  SetSEmperorIngStage2Tentacle_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSEmperorIngStage2Tentacle_FuncPtrs(nullptr); }
#endif
