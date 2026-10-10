#include "MetroidPrime/Enemies/CPuddleSpore.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPuddleSpore.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "REL/REL_Setup.h"
#include <math.h>

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"InAttackPosition",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CPuddleSpore::InAttackPosition)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CPuddleSpore::ShouldAttack)},
    {"ShouldTurn", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPuddleSpore::ShouldTurn)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPuddleSpore::AnimOver)},
    {"StateOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPuddleSpore::StateOver)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"InActive", static_cast< CPatterned::StateMachine::StateFunc >(&CPuddleSpore::InActive)},
    {"Active", static_cast< CPatterned::StateMachine::StateFunc >(&CPuddleSpore::Active)},
    {"Run", static_cast< CPatterned::StateMachine::StateFunc >(&CPuddleSpore::Run)},
    {"TurnAround", static_cast< CPatterned::StateMachine::StateFunc >(&CPuddleSpore::TurnAround)},
    {"GetUp", static_cast< CPatterned::StateMachine::StateFunc >(&CPuddleSpore::GetUp)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CPuddleSpore::Attack)},
};

CPuddleSpore::CPuddleSpore(TUniqueId uid, const rstl::string& name, EFlavorType flavor,
                           const CEntityInfo& info, const CTransform4f& xf,
                           const CModelData& modelData, const CPatternedInfo& patternedInfo,
                           EColliderType collider, const CActorParameters& actorParams,
                           const SPuddleSporeData& data)
: CPatterned(kPAI_PuddleSpore, uid, name, flavor, info, xf, modelData, patternedInfo, kMT_Flyer,
             collider, kBT_Restricted, actorParams)
, mData(data)
, mStateTimer(0.f)
, mSecondaryStateTimer(0.f)
, mTouchBounds(CAABox::MakeNullBox())
, mCollisionActorManager(nullptr)
, mAimPosition(CVector3f::Zero())
, mState(kPS_Closed)
, mAnimPhase(0)
, mShockWaveId(kInvalidUniqueId)
, mShockWaveSfx(0)
, mStateTimerRunning(false)
, mSecondaryTimerRunning(false)
, mOpen(true) {
  KnockBackController().SetHurlVelocityEnabled(false);
  KnockBackController().EnableKnockBackPhysics(false);
  KnockBackController().EnableBurn(false);
}

void CPuddleSpore::PreThink(float dt, CStateManager& mgr) {
  UpdatePlayerContact(dt, mgr);
  CPatterned::PreThink(dt, mgr);
}

void CPuddleSpore::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CPuddleSpore::Think(float dt, CStateManager& mgr) {
  mAimPosition = GetAimPosition(mgr, 0.f);
  if (mSecondaryTimerRunning) {
    mSecondaryStateTimer += dt;
  }
  if (mStateTimerRunning) {
    mStateTimer += dt;
  }
  if (!mCollisionActorManager.get()) {
    SetupCollisionManager(mgr);
  } else {
    mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  }
  UpdateEffects(mgr);
  HealthInfo()->SetKnockbackResistance(1000000.f);

  if (mShockWaveSfx && mShockWaveId != kInvalidUniqueId && CSfxManager::IsPlaying(mShockWaveSfx)) {
    if (const CShockWave* shockWave =
            static_cast< const CShockWave* >(mgr.GetObjectById(mShockWaveId))) {
      CVector3f direction = mgr.GetPlayer(0)->GetTranslation() - shockWave->GetTranslation();
      direction.SetZ(0.f);
      if (direction.CanBeNormalized()) {
        direction.Normalize();
        CSfxManager::UpdateEmitter(mShockWaveSfx,
                                   shockWave->GetTranslation() + direction * shockWave->GetRadius(),
                                   direction, 0xff);
      }
    }
  }
  CPatterned::Think(dt, mgr);
}

void CPuddleSpore::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CPuddleSpore::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {}

void CPuddleSpore::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  bool handled = false;
  switch (msg.GetMessage()) {
  case kSM_Create:
    SetState(mgr, kPS_Closed);
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    break;
  case kSM_Activate:
    SetCollisionActive(mgr, true);
    break;
  case kSM_Deactivate:
    SetCollisionActive(mgr, false);
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionActorManager.get()) {
      mCollisionActorManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Delete:
    if (mCollisionActorManager.get()) {
      mCollisionActorManager->Destroy(mgr);
    }
    break;
  case kSM_Damage:
    if (IsOpen()) {
      mHitByPlayerProjectile = true;
      TakeDamage(CVector3f::Zero(), 0.f);
      handled = true;
    }
    break;
  case kSM_ResistedDamage:
  case kSM_ReflectedDamage:
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::AcceptScriptMsg(mgr, msg);
  }
}

void CPuddleSpore::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (IsOpen()) {
    CPatterned::KnockBack(mgr, info);
  }
}

void CPuddleSpore::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case 0: {
    mSecondaryStateTimer = 0.f;
    if (mData.mShockWaveInfo.GetParticleDescId() != kInvalidAssetId) {
      CTransform4f xf = GetTransform();
      xf.SetTranslation(xf.GetTranslation() + CVector3f(0.f, 0.f, mData.mShockWaveHeight));
      CShockWave* shockWave =
          rs_new CShockWave(mgr.AllocateUniqueId(), rstl::string_l("Shock Wave"),
                            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), xf,
                            GetUniqueId(), mData.mShockWaveInfo, 2.f, 0.4f);
      if (shockWave) {
        mgr.AddObject(shockWave);
        mShockWaveId = shockWave->GetUniqueId();
        mShockWaveSfx = CSfxManager::AddEmitter(mData.mShockWaveSound, GetTranslation(),
                                                GetCurrentAreaId().Value(), true, false,
                                                CSfxManager::kMedPriority);
        SetState(mgr, kPS_Closed);
        handled = true;
      }
    }
    break;
  }
  case 0x1d:
    SendScriptMsgs(kSS_Closed, mgr);
    mOpen = false;
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CPuddleSpore::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  if (mState != kPS_Platform) {
    CPatterned::CollidedWith(id, list, mgr);
  }
}

void CPuddleSpore::Touch(CActor& actor, CStateManager& mgr) {
  if (mAlive) {
    if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
      if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(projectile->GetOwnerId()))) {
        mHitByPlayerProjectile = !WeaponHitsWeakSpot(
            projectile->GetTranslation(), projectile->GetCurrentDamageInfo().GetWeaponMode());
      }
    }
  }
}

rstl::optional_object< CAABox > CPuddleSpore::GetTouchBounds() const { return mTouchBounds; }

bool CPuddleSpore::WeaponHitsWeakSpot(const CVector3f& position, const CWeaponMode& mode) const {
  if (IsOpen() && GetDamageVulnerability()->WeaponHits(mode, false)) {
    const CUnitVector3f direction(position - mTouchBounds.GetCenterPoint());
    const float dot = CVector3f::Dot(GetTransform().GetUp(), direction);
    if (dot > -mData.mHitDetectionSine && dot < mData.mHitDetectionSine) {
      return false;
    }
  }
  return true;
}

bool CPuddleSpore::IsOpen() const { return mState == kPS_Open; }

EWeaponCollisionResponseTypes CPuddleSpore::GetCollisionResponseType(const CVector3f& position,
                                                                     const CVector3f& direction,
                                                                     const CWeaponMode& mode,
                                                                     int attributes) const {
  return WeaponHitsWeakSpot(position, mode) ? kWCR_PuddleSporeWeakSpot : kWCR_Unknown34;
}

const CDamageVulnerability* CPuddleSpore::GetDamageVulnerability() const {
  if (IsOpen()) {
    return CPatterned::GetDamageVulnerability();
  }
  return &CDamageVulnerability::ReflectVulnerabilty();
}

static CVector3f skKnockOffDirection(-1.f, 0.f, 0.3f); // Guessed name

void CPuddleSpore::KnockOffPlayers(float force, CStateManager& mgr) {
  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    CPlayer* player = mgr.GetPlayer(i);
    const CAABox playerBounds = player->GetBoundingBox();
    if (mTouchBounds.GetMaxPoint().GetZ() >= playerBounds.GetMinPoint().GetZ() &&
        mTouchBounds.GetMaxPoint().GetX() >= playerBounds.GetMinPoint().GetX() &&
        mTouchBounds.GetMaxPoint().GetY() >= playerBounds.GetMinPoint().GetY() &&
        playerBounds.GetMaxPoint().GetX() >= mTouchBounds.GetMinPoint().GetX() &&
        playerBounds.GetMaxPoint().GetY() >= mTouchBounds.GetMinPoint().GetY() &&
        playerBounds.GetMinPoint().GetZ() - mTouchBounds.GetMaxPoint().GetZ() < 0.2f) {
      const float scale =
          player->GetMorphballTransitionState() == CPlayer::kMS_Morphed ? 1.5f : 1.f;
      const CVector3f impulse =
          GetTransform().Rotate(skKnockOffDirection) * (scale * (force * player->GetMass()));
      player->ApplyImpulseWR(impulse, CAxisAngle::Identity());
      player->SetMoveState(NPlayer::kMS_Falling, mgr);
    }
  }
}

void CPuddleSpore::UpdatePlayerContact(float dt, CStateManager& mgr) {
  if (!mCollisionActorManager.get()) {
    return;
  }

  CAABox bounds = CAABox::MakeMaxInvertedBox();
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    if (i == 2) {
      continue;
    }
    if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(
            mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId()))) {
      const rstl::optional_object< CAABox > actorBounds = actor->GetTouchBounds();
      bounds.Include(*actorBounds);
    }
  }
  mTouchBounds = CAABox(bounds.GetMinPoint() - CVector3f::One() * 0.05f,
                        bounds.GetMaxPoint() + CVector3f::One() * 0.05f);
  SetBoundingBox(mTouchBounds);

  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    CPlayer* player = mgr.GetPlayer(i);
    CAABox playerBounds(CVector3f::Zero(), CVector3f::Zero());
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      const float radius = player->GetMorphBall()->GetBallRadius();
      const CVector3f center = player->GetTranslation() + CVector3f(0.f, 0.f, radius);
      playerBounds = CAABox(center - CVector3f(radius, radius, radius),
                            center + CVector3f(radius, radius, radius));
    } else {
      playerBounds = player->GetBoundingBox();
    }

    bool inBounds = false;
    if (mTouchBounds.DoBoundsOverlap(playerBounds) && mState != kPS_Platform) {
      inBounds = true;
    }
    if (!inBounds) {
      if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(
              mCollisionActorManager->GetCollisionDescFromIndex(2).GetCollisionActorId()))) {
        const rstl::optional_object< CAABox > actorBounds = actor->GetTouchBounds();
        if (actorBounds) {
          inBounds = actorBounds->DoBoundsOverlap(playerBounds);
        }
      }
    }
    if (!inBounds) {
      continue;
    }

    const float push = (mState == kPS_Platform ? 0.001f : -0.0001f) +
                       (mTouchBounds.GetMaxPoint().GetZ() - playerBounds.GetMinPoint().GetZ());
    if (push > 0.f && mTouchBounds.GetMaxPoint().GetZ() < playerBounds.GetMaxPoint().GetZ()) {
      const bool hasGroundCollider = player->GetMaterialList().HasMaterial(kMT_GroundCollider);
      if (hasGroundCollider) {
        player->RemoveMaterial(kMT_GroundCollider, mgr);
        player->RemoveMaterial(kMT_Player, mgr);
      }
      CPhysicsState state = player->GetPhysicsState();
      CVector3f direction = player->GetTranslation() - GetTranslation();
      if (direction.CanBeNormalized()) {
        direction.Normalize();
      } else {
        direction = CVector3f::Up();
      }
      player->MoveToOR(direction * push, dt);
      CGameCollision::Move(mgr, *player, dt, nullptr);
      state.SetTranslation(player->GetTranslation());
      player->SetPhysicsState(state);
      if (hasGroundCollider) {
        player->AddMaterial(kMT_GroundCollider, mgr);
        player->AddMaterial(kMT_Player, mgr);
      }
    }

    if (mState != kPS_Platform && mCurDamageRemTime <= 0.f) {
      mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                      CMaterialFilter::GetPassEverything(), -player->GetVelocityWR());
      mCurDamageRemTime = mDamageWaitTime;
    }
  }
}

void CPuddleSpore::SetCollisionActive(CStateManager& mgr, bool active) {
  if (mCollisionActorManager.get()) {
    mCollisionActorManager->SetActive(mgr, active);
  }
}

static EMaterialTypes skPlatformAddMaterial0 = kMT_SolidCharacter;      // Guessed name
static EMaterialTypes skPlatformAddMaterial1 = kMT_Character;           // Guessed name
static EMaterialTypes skPlatformRemoveMaterial = kMT_NoPlayerCollision; // Guessed name
static EMaterialTypes skNormalAddMaterial0 = kMT_NoPlayerCollision;     // Guessed name
static EMaterialTypes skNormalAddMaterial1 = kMT_Character;             // Guessed name
static EMaterialTypes skNormalRemoveMaterial = kMT_SolidCharacter;      // Guessed name

void CPuddleSpore::SetState(CStateManager& mgr, EPuddleState state) {
  mState = state;
  if (mCollisionActorManager.get()) {
    if (mState == kPS_Platform) {
      mCollisionActorManager->AddMaterialList(
          mgr, CMaterialList(skPlatformAddMaterial0, skPlatformAddMaterial1));
      mCollisionActorManager->RemoveMaterialList(mgr, CMaterialList(skPlatformRemoveMaterial));
    } else {
      mCollisionActorManager->AddMaterialList(
          mgr, CMaterialList(skNormalAddMaterial0, skNormalAddMaterial1));
      mCollisionActorManager->RemoveMaterialList(mgr, CMaterialList(skNormalRemoveMaterial));
    }
    if (IsOpen()) {
      AddMaterial(kMT_Target, kMT_SeekerTarget, kMT_Unknown54, mgr);
    } else {
      RemoveMaterial(kMT_Target, kMT_SeekerTarget, kMT_Unknown54, mgr);
    }
  }
}

void CPuddleSpore::UpdateEffects(CStateManager& mgr) {}

static EMaterialTypes skFilterMaterial0 = kMT_Solid;          // Guessed name
static EMaterialTypes skFilterMaterial1 = kMT_CollisionActor; // Guessed name
static EMaterialTypes skFilterMaterial2 = kMT_AIPassthrough;  // Guessed name
static EMaterialTypes skFilterMaterial3 = kMT_Player;         // Guessed name

void CPuddleSpore::SetupCollisionManager(CStateManager& mgr) {
  const CVector3f& scale = GetModelData()->GetScale();
  const CAnimData* animData = GetModelData()->GetAnimationData();
  const CTransform4f scaleXf =
      CTransform4f::Scale(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
  const CAABox box = GetBaseBoundingBox().GetTransformedAABox(scaleXf);
  const CVector3f size(box.GetMaxPoint().GetX() - box.GetMinPoint().GetX(),
                       box.GetMaxPoint().GetY() - box.GetMinPoint().GetY(),
                       (box.GetMaxPoint().GetZ() - box.GetMinPoint().GetZ()) * 0.5f);

  const char* locators[3] = {"Top_LCTR_SDK", "Skeleton_Root", "spike_locator"};
  const char* names[3] = {"PuddleSporeTop", "PuddleSporeMainBody", "PuddleSporeSpike"};

  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(3);
  joints.push_back_unsafe(CJointCollisionDescription::AABoxCollision(
      animData->GetLocatorSegId(rstl::string_l(locators[0])), size, rstl::string_l(names[0]),
      GetMass()));
  joints.push_back_unsafe(CJointCollisionDescription::AABoxCollision(
      animData->GetLocatorSegId(rstl::string_l(locators[1])), size, rstl::string_l(names[1]),
      GetMass()));
  joints.push_back_unsafe(CJointCollisionDescription::SphereCollision(
      animData->GetLocatorSegId(rstl::string_l(locators[2])), CVector3f::Zero(), size.GetX() * 0.5f,
      rstl::string_l(names[2]), GetMass()));

  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, true);
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(
            mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId()))) {
      actor->SetDamageVulnerability(*CPatterned::GetDamageVulnerability());
      actor->HealthInfo()->SetKnockbackResistance(1000000.f);
      CMaterialFilter filter = actor->GetMaterialFilter();
      filter.ExcludeList().Add(kMT_Platform);
      actor->SetMaterialFilter(filter);
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
      actor->SetResponseType(kWCR_PuddleSporeWeakSpot);
    }
  }
  SetMaterialFilter(CMaterialFilter::MakeExclude(
      CMaterialList(skFilterMaterial0, skFilterMaterial1, skFilterMaterial2, skFilterMaterial3)));
}

bool CPuddleSpore::InAttackPosition(CStateManager& mgr, const CTriggerData& data) const {
  return mStateTimer >= mData.mChargeTime;
}

bool CPuddleSpore::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mStateTimer >= mData.mTimeOpen;
}

bool CPuddleSpore::ShouldTurn(CStateManager& mgr, const CTriggerData& data) const {
  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    const CAABox playerBounds = player->GetBoundingBox();
    if (mTouchBounds.GetMaxPoint().GetZ() <
            (playerBounds.GetMinPoint().GetZ() + playerBounds.GetMaxPoint().GetZ()) * 0.5f &&
        mTouchBounds.GetMaxPoint().GetX() >= playerBounds.GetMinPoint().GetX() &&
        mTouchBounds.GetMaxPoint().GetY() >= playerBounds.GetMinPoint().GetY() &&
        playerBounds.GetMaxPoint().GetX() >= mTouchBounds.GetMinPoint().GetX() &&
        playerBounds.GetMaxPoint().GetY() >= mTouchBounds.GetMinPoint().GetY() &&
        player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      return true;
    }
  }
  return mStateTimer >= mData.mPlatformTime;
}

bool CPuddleSpore::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimPhase == 2;
}

bool CPuddleSpore::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

void CPuddleSpore::InActive(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mSecondaryStateTimer = 0.f;
    break;
  case kStateMsg_Update:
    break;
  }
}

void CPuddleSpore::Active(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    mStateTimer = 0.f;
    mSecondaryStateTimer = 0.f;
    mStateTimerRunning = true;
    mSecondaryTimerRunning = true;
    break;
  case kStateMsg_Deactivate:
    mStateTimerRunning = false;
    mSecondaryTimerRunning = false;
    break;
  }
}

void CPuddleSpore::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SetState(mgr, kPS_Open);
    mAnimPhase = 0;
    mStateTimer = 0.f;
    mStateTimerRunning = false;
    break;
  case kStateMsg_Update:
    switch (mAnimPhase) {
    case 0:
      if (BodyController()->GetCurrentStateId() == pas::kAS_LoopReaction) {
        mAnimPhase = 1;
        mStateTimerRunning = true;
      } else {
        BodyController()->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
      }
      break;
    case 1:
      if (BodyController()->GetCurrentStateId() != pas::kAS_LoopReaction) {
        mAnimPhase = 2;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    mStateTimerRunning = false;
    break;
  }
}

void CPuddleSpore::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateTimer = 0.f;
    mSecondaryStateTimer = 0.f;
    mHitByPlayerProjectile = false;
    SetState(mgr, kPS_Platform);
    mAnimPhase = 0;
    mStateTimerRunning = false;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (BodyController()->GetCurrentStateId() == pas::kAS_LieOnGround) {
      mAnimPhase = 2;
      mStateTimerRunning = true;
    } else if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LieOnGround)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockDownCmd(CVector3f::Right(), static_cast< pas::ESeverity >(mCreatureSize > 1)));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mStateTimerRunning = false;
    break;
  }
}

void CPuddleSpore::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SendScriptMsgs(kSS_Open, mgr);
    mOpen = true;
    KnockOffPlayers(mData.mKnockOffForce, mgr);
    mSecondaryStateTimer = 0.f;
    mAnimPhase = 0;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    KnockOffPlayers(mData.mKnockOffForce * 0.25f, mgr);
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Getup)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Over) {
      mAnimPhase = 2;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    SetState(mgr, kPS_Closed);
    break;
  }
}

void CPuddleSpore::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

CEntity* LoadPuddleSpore(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPuddleSpore sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPuddleSpore.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const SPuddleSporeData data(
      sldrThis.chargeTime, sldrThis.timeOpen, sldrThis.platformTime, sldrThis.unknown_0xf1c2d224,
      sldrThis.knockOffForce,
      static_cast< float >(sin((M_PIF / 180.f) * sldrThis.hitDetectionAngle)),
      sldrThis.shockWaveHeight, static_cast< ushort >(sldrThis.sound_ShockWaveTravelSound),
      CShockWaveInfo(sldrThis.shockWaveInfo));
  return rs_new CPuddleSpore(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      sldrThis.flavor != 0 ? CPatterned::kFT_One : CPatterned::kFT_Zero,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      sldrThis.unknown_0x5cdc877d ? CPatterned::kCT_One : CPatterned::kCT_Zero,
      LdrToActorParameters(sldrThis.actorInformation), data);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SPuddleSpore_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadPuddleSpore;
  SetSPuddleSpore_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPuddleSpore_FuncPtrs(nullptr); }
#endif
