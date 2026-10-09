#include "MetroidPrime/Enemies/CShrieker.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShrieker.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"
#include "rstl/algorithm.hpp"

static EMaterialTypes ApplyDamageSolidMaterial = kMT_Solid;
static EMaterialTypes SolidMaterial = kMT_Solid;
static EMaterialTypes PlayerMaterial = kMT_Player;
static EMaterialTypes CollisionActorMaterial = kMT_CollisionActor;

static CMaterialFilter skUnusedFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(SolidMaterial), CMaterialList(PlayerMaterial, CollisionActorMaterial));

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"PathShagged", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::PathShagged)},
    {"PlayerEntersProximity",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::PlayerEntersProximity)},
    {"PlayerLeavesProximity",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::PlayerLeavesProximity)},
    {"EnterUprootShriek",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::EnterUprootShriek)},
    {"HasPatrolPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::HasPatrolPath)},
    {"PlayerLeashReached",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::PlayerLeashReached)},
    {"LeavePatrol", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::LeavePatrol)},
    {"CloseToStart",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::CloseToStart)},
    {"LineOfSight", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::LineOfSight)},
    {"ShouldFire", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::ShouldFire)},
    {"ShouldMelee", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::ShouldMelee)},
    {"ShouldDodge", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::ShouldDodge)},
    {"InMaxRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CShrieker::InMaxRange)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Buried", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Buried)},
    {"BuriedRumbling",
     static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::BuriedRumbling)},
    {"TargetPlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::TargetPlayer)},
    {"UprootShriek", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::UprootShriek)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::PathFind)},
    {"ShriekAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::ShriekAttack)},
    {"Bury", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Bury)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Patrol)},
    {"ReturnToStart",
     static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::ReturnToStart)},
    {"Retreat", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Retreat)},
    {"Melee", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Melee)},
    {"Dodge", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Dodge)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CShrieker::Dead)},
};

static EMaterialTypes DodgeSolidMaterial = kMT_Solid;
static EMaterialTypes TriggerMaterial = kMT_Trigger;
static EMaterialTypes ImmovableMaterial = kMT_Immovable;
static EMaterialTypes DamageSolidMaterial = kMT_Solid;
static EMaterialTypes NonSolidDamageableMaterial = kMT_NonSolidDamageable;
static EMaterialTypes ProjectileMaterial = kMT_Projectile;
static EMaterialTypes PowerBombMaterial = kMT_PowerBomb;
static EMaterialTypes DetectPlayerMaterial = kMT_Player;
static EMaterialTypes DeflectProjectileMaterial = kMT_Projectile;
static EMaterialTypes BuriedFloorMaterial = kMT_Floor;
static EMaterialTypes UnburiedFloorMaterial = kMT_Floor;

static CVector3f skCloseToStartTolerance(1.f, 1.f, 0.1f);

static rstl::string skRootLocatorName = rstl::string_l("Skeleton_Root");
static rstl::string skHoverReferenceLocatorName = rstl::string_l("HoverReference_LCTR");

CShriekerData::CShriekerData(
    float detectionHeight, float rustleDetectionRadius, float popDetectionRadius,
    float morphballDetectionRadius, CAssetId shriekEffect, const CDamageInfo& shriekDamage,
    CAssetId projectile, const CDamageInfo& projectileDamage, uchar combatVisorMaxVolume,
    uchar echoVisorMaxVolume, CAssetId meleeEffect, const CDamageInfo& meleeDamage,
    float meleeRange, float meleeAverageAttackTime, float meleeAttackTimeVariation,
    const CDamageVulnerability& buriedVulnerability, float hostileAccumulatePriority,
    float hoverHeight, const CVector3f& missileDeflectionOffset, float missileDeflectionRadius,
    float missileDeflectRate, ushort missileDeflectionSound, float dodgeTime, float dodgePercentage,
    float visibilityChangeTime)
: mDetectionHeight(detectionHeight)
, mRustleDetectionRadius(rustleDetectionRadius)
, mPopDetectionRadius(popDetectionRadius)
, mMorphballDetectionRadius(morphballDetectionRadius)
, mShriekEffect(shriekEffect)
, mShriekDamage(shriekDamage)
, mProjectile(projectile)
, mProjectileDamage(projectileDamage)
, mCombatVisorMaxVolume(combatVisorMaxVolume)
, mEchoVisorMaxVolume(echoVisorMaxVolume)
, mMeleeEffect(meleeEffect)
, mMeleeDamage(meleeDamage)
, mMeleeRangeSquared(meleeRange * meleeRange)
, mMeleeAverageAttackTime(meleeAverageAttackTime)
, mMeleeAttackTimeVariation(meleeAttackTimeVariation)
, mBuriedVulnerability(buriedVulnerability)
, mHostileAccumulatePriority(hostileAccumulatePriority)
, mHoverHeight(hoverHeight)
, mMissileDeflectionOffset(missileDeflectionOffset)
, mMissileDeflectionRadius(missileDeflectionRadius)
, mMissileDeflectRate(missileDeflectRate)
, mMissileDeflectionSound(missileDeflectionSound)
, mDodgeTime(dodgeTime)
, mDodgePercentage(dodgePercentage)
, mVisibilityChangeTime(visibilityChangeTime) {}

CShrieker::CShrieker(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& modelData,
                     const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                     const CShriekerData& data)
: CPatterned(kPAI_Shrieker, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_Flyer, actorParams)
, mData(data)
, mProjectileInfo(data.mProjectile, data.mProjectileDamage)
, mPathFindSearch(nullptr, 3, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mBuriedPosition(xf.GetTranslation())
, mStartPosition(xf.GetTranslation())
, mLineOfSightTracker(GetUniqueId(), CSegId(0xff), 0.1f, 0.05f)
, mScriptedProximity(false)
, mScriptedAlert(false)
, mHasPatrolPath(false)
, mBuried(true)
, mChasing(false)
, mPlayerInProximity(false)
, mShriekTriggered(false)
, mDodgeRight(false)
, mDying(false)
, mLaunched(false)
, mSeparating(false)
, mDeathKnockBackStarted(false)
, mReferenceHealth(patternedInfo.GetHealthInfo().GetHP())
, mPreviousHealth(patternedInfo.GetHealthInfo().GetHP())
, mFireTimer(0.f)
, mMeleeTimer(0.f)
, mDodgeTimer(0.f)
, mChaseTimer(0.f)
, mCheckRadius(4.f)
, mDetectionRadius(0.f)
, mTargetId(kInvalidUniqueId)
, mTargetPos(CVector3f::Zero())
, mTeamAiMgrId(kInvalidUniqueId)
, mDeflectSfx()
, mRootSegId(0xff)
, mHoverSegId(0xff)
, mEffectDescs(2, rstl::optional_object< TLockedToken< CGenDescription > >())
, mEffectGens(rstl::auto_ptr< CElementGen >()) {
  if (mData.mShriekEffect != kInvalidAssetId) {
    mEffectDescs[0] = TLockedToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mData.mShriekEffect)));
  }
  if (mData.mMeleeEffect != kInvalidAssetId) {
    mEffectDescs[1] = TLockedToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mData.mMeleeEffect)));
  }
  mProjectileInfo.Token().Lock();
  const CPASAnimParmData parms(pas::kAS_Step, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1));
  mCheckRadius = GetModelData()->GetScale().GetX() * GetAnimationDistance(parms);
  KnockBackController().SetHurlVelocityEnabled(false);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  mRootSegId = animData->GetLocatorSegId(skRootLocatorName);
  mHoverSegId = animData->GetLocatorSegId(skHoverReferenceLocatorName);
  mDetectionRadius =
      rstl::max_val(rstl::max_val(mData.mRustleDetectionRadius, mData.mPopDetectionRadius),
                    mData.mMorphballDetectionRadius);
  mLineOfSightTracker.SetSegment(mRootSegId);
}

CShrieker::~CShrieker() {}

void CShrieker::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CShrieker::Think(float dt, CStateManager& mgr) {
  for (int i = 0; i < 2; ++i) {
    if (CElementGen* gen = mEffectGens[i].get()) {
      gen->Update(dt);
      if (gen->IsSystemDeletable()) {
        mEffectGens[i] = rstl::auto_ptr< CElementGen >();
      }
    }
  }
  UpdateValidTarget(mgr);
  if (!mBuried) {
    DeflectMissiles(mgr);
  } else if (!mShriekTriggered) {
    if (!mScriptedProximity || !mScriptedAlert) {
      DetectPlayer(mgr);
    }
  }
  ApplyMeleeDamage(mgr);
  if (mDying && IsInCollision()) {
    MassiveDeath(mgr);
  }
  if (CSfxManager::IsPlaying(mDeflectSfx)) {
    mDeflectSfxTimer += dt;
    CSfxManager::UpdateEmitter(mDeflectSfx, GetTranslation(), CVector3f::Zero(), 127);
  }
  CPatterned::Think(dt, mgr);
}

void CShrieker::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_AreaLoaded: {
    for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
      if (it->state == kSS_Patrol && it->msg == kSM_Follow) {
        mHasPatrolPath = true;
        mBuried = false;
      } else if ((it->state == kSS_Entered && it->msg == kSM_Increment) ||
                 (it->state == kSS_Exited && it->msg == kSM_Decrement)) {
        mScriptedProximity = true;
      } else if (it->state == kSS_Entered && it->msg == kSM_Alert) {
        mScriptedAlert = true;
      }
    }
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    SetBuriedCollision(mgr, mBuried);
    JoinTeam(mgr);
    break;
  }
  case kSM_Create:
    RemoveMaterial(kMT_GroundCollider, mgr);
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Internal9);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }
    break;
  case kSM_Damage:
    ReactToDamage(mgr, senderId);
    break;
  case kSM_OffGround:
    if (!mAlive && mDying) {
      SetMomentumWR(CVector3f::Zero());
    }
    break;
  case kSM_Launching:
    if (!mAlive && mDying && !mLaunched) {
      mLaunched = true;
      Stop();
      const float angle = mgr.Random()->Range(0.f, 360.f) * 0.5f / 3.1415927f;
      SetVelocityWR(CVector3f(15.f * CMath::FastSinR(angle), 15.f * CMath::FastCosR(angle), 0.f));
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass() * 10.5f));
    }
    break;
  case kSM_Alert:
    mShriekTriggered = true;
    break;
  case kSM_Increment:
    mPlayerInProximity = true;
    break;
  case kSM_Decrement:
    mPlayerInProximity = false;
    break;
  case kSM_Activate:
    JoinTeam(mgr);
    break;
  case kSM_Delete:
  case kSM_Deactivate:
    QuitTeam(mgr);
    break;
  }
}

void CShrieker::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CShrieker::AddToRenderer(const CStateManager& mgr) const {
  for (int i = 0; i < 2; ++i) {
    if (mEffectGens[i].get() != nullptr) {
      gpRender->AddParticleGen(*mEffectGens[i]);
    }
  }
  CPatterned::AddToRenderer(mgr);
}

void CShrieker::Render(const CStateManager& mgr) const {
  if (mColor.GetAlpha() != 0.f ||
      mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
    CPatterned::Render(mgr);
  }
  if (mDeflectedIds.size() != 0) {
    const CVector3f center = GetAimPosition(mgr, 0.f) + mData.mMissileDeflectionOffset;
    for (rstl::reserved_vector< TUniqueId, 6 >::const_iterator it = mDeflectedIds.begin();
         it != mDeflectedIds.end(); ++it) {
      const CGameProjectile* projectile =
          TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(*it));
      if (projectile) {
        const float ratio =
            (center - projectile->GetTranslation()).Magnitude() / mData.mMissileDeflectionRadius;
        mgr.DrawSpaceWarp(projectile->GetTranslation(), 1.f - rstl::min_val(ratio, 1.f));
      }
    }
  }
}

const CDamageVulnerability* CShrieker::GetDamageVulnerability() const {
  if (mBuried) {
    return &mData.mBuriedVulnerability;
  }
  return CPatterned::GetDamageVulnerability();
}

const CDamageVulnerability* CShrieker::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                              const CDamageInfo&) const {
  return GetDamageVulnerability();
}

void CShrieker::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  float scale = 1.f;
  bool handled = false;
  int effectIndex = -1;
  switch (type) {
  case kUE_Activate:
    scale = mData.mShriekDamage.GetRadius();
    effectIndex = 0;
    handled = true;
    mChaseTimer = 0.f;
    break;
  case kUE_EffectOn:
    scale = mData.mMeleeDamage.GetRadius();
    effectIndex = 1;
    handled = true;
    mChaseTimer = 0.f;
    break;
  case kUE_Projectile: {
    UpdateTargetPosition(mgr);
    const CTransform4f lctr = GetLctrTransform(node.GetLocatorName());
    const CTransform4f xf =
        CTransform4f::LookAt(lctr.GetTranslation(), mTargetPos, CVector3f::Up());
    CEnergyProjectile* projectile =
        LaunchProjectile(xf, mgr, 1, 0, false, CImpactVisorEffect::None(), CVector3f::One());
    if (projectile != nullptr) {
      projectile->SetCombatVisorMaxVolume(mData.mCombatVisorMaxVolume);
      projectile->SetEchoVisorMaxVolume(mData.mEchoVisorMaxVolume);
    }
    handled = true;
    mChaseTimer = 0.f;
    break;
  }
  }

  if (effectIndex != -1 && mEffectDescs[effectIndex].valid()) {
    const CVector3f position = GetLctrTransform(node.GetLocatorName()).GetTranslation();
    CElementGen* gen = rs_new CElementGen(*mEffectDescs[effectIndex]);
    gen->SetGlobalTranslation(position);
    gen->SetGlobalScale(CVector3f(scale, scale, scale));
    mEffectGens[effectIndex] = rstl::auto_ptr< CElementGen >(gen);
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CShrieker::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  bool callBase = !mBuried;
  mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Hurled, !mAlive);
  if (!mAlive) {
    bool hurled = false;
    if (info.GetDamageInfo().GetWeaponMode().IsComboed()) {
      const EWeaponType type = info.GetDamageInfo().GetWeaponMode().GetType();
      const bool darkWeapon = type == kWT_Dark || type == kWT_Annihilator;
      if (darkWeapon) {
        hurled = true;
      }
    }
    if (mBuried && !hurled) {
      MassiveDeath(mgr);
    } else if (!mDeathKnockBackStarted) {
      FadeAlpha(mgr, true, true);
      if (hurled) {
        mKnockBackController.SetAnimReactionRange(CKnockBackMgr::kAR_None, CKnockBackMgr::kAR_Fall);
        mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Hurled, false);
      } else {
        mDying = true;
        mKnockBackController.SetAnimReactionRange(CKnockBackMgr::kAR_Hurled,
                                                  CKnockBackMgr::kAR_Hurled);
      }
      RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
      mDeathKnockBackStarted = true;
      callBase = true;
    }
  }
  if (callBase) {
    CPatterned::KnockBack(mgr, info);
  }
}

CProjectileInfo* CShrieker::ProjectileInfo() { return &mProjectileInfo; }

void CShrieker::ResetAttackTimer(CStateManager& mgr, int type) {
  float variation = 0.f;
  float* timer = nullptr;
  switch (type) {
  case 1:
    mMeleeTimer = mData.mMeleeAverageAttackTime;
    timer = &mMeleeTimer;
    variation = mData.mMeleeAttackTimeVariation;
    break;
  case 0:
    mFireTimer = GetAverageAttackTime();
    timer = &mFireTimer;
    variation = mAttackTimeVariation;
    break;
  }
  if (timer != nullptr && mgr.IsRandomAvailable()) {
    *timer += variation * mgr.Random()->Float();
  }
}

void CShrieker::ApplySeparation(CStateManager& mgr) {
  CVector3f total = CVector3f::Zero();
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CPatterned* other = TCastToConstPtr< CPatterned >(list[i]);
    if (other != nullptr && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CVector2f separation = mSteeringBehaviors.Separation2D(
          *this, CVector2f(other->GetTranslation().GetX(), other->GetTranslation().GetY()),
          10.f * GetModelData()->GetScale().GetX());
      const CVector3f separation3(separation.GetX(), separation.GetY(), 0.f);
      if (separation3.IsMagnitudeSafe()) {
        total += separation3;
      }
    }
  }
  if (total != CVector3f::Zero()) {
    mSeparating = true;
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(total, CVector3f::Zero(), 0.5f));
  }
}

void CShrieker::SetTeamMemberTarget(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->SetMemberTargetId(GetUniqueId(), mTargetId);
    }
  }
}

void CShrieker::EndTeamAttack(CStateManager& mgr, bool projectile) {
  CScriptTeamAiMgr::EndAttack(projectile ? CScriptTeamAiMgr::kAT_Projectile
                                         : CScriptTeamAiMgr::kAT_Melee,
                              mgr, mTeamAiMgrId, GetUniqueId(), true);
}

void CShrieker::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CShrieker::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
  }
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->JoinTeam(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Melee,
                     CTeamAiRole::kTAR_Invalid);
    }
  }
}

CVector3f CShrieker::GetTranslationCopy() const { return GetTranslation(); }

void CShrieker::SetBuriedCollision(CStateManager& mgr, bool buried) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  if (buried) {
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, kMT_Scannable, mgr);
    exclude.Add(BuriedFloorMaterial);
  } else {
    AddMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, kMT_Scannable, mgr);
    exclude.Remove(UnburiedFloorMaterial);
  }
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
}

bool CShrieker::GetHeightAboveMesh(float& height) const {
  const CTransform4f lctr = GetLctrTransform(mHoverSegId);
  if (mPathFindSearch.GetHeightOfPointAboveMesh(lctr.GetTranslation(), height, 0.f) == 0) {
    height -= mData.mHoverHeight;
    return true;
  }
  return false;
}

void CShrieker::FaceTarget(CStateManager& mgr, float dt) {
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
    CVector3f direction = target->GetTranslation() - GetTranslation();
    if (direction.CanBeNormalized()) {
      direction.Normalize();
      BodyController()->FaceDirection(direction, dt);
      if (!mBuried) {
        float height = 0.f;
        if (GetHeightAboveMesh(height) && CMath::AbsF(height) > 0.5f) {
          BodyController()->CommandMgr().ClearLocomotionCmds();
          BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
              height > 0.f ? CVector3f::Down() : CVector3f::Up(), CVector3f::Zero(), 0.5f));
        }
      }
    }
  }
}

void CShrieker::DeflectMissiles(CStateManager& mgr) {
  const CVector3f center = GetAimPosition(mgr, 0.f) + mData.mMissileDeflectionOffset;
  const CMaterialFilter filter =
      CMaterialFilter::MakeInclude(CMaterialList(DeflectProjectileMaterial));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList,
                    CAABox(center - mData.mMissileDeflectionRadius * CVector3f::One(),
                           center + mData.mMissileDeflectionRadius * CVector3f::One()),
                    filter, this);

  const rstl::reserved_vector< TUniqueId, 6 > previousIds = mDeflectedIds;
  mDeflectedIds.clear();
  if (nearList.size() == 0) {
    return;
  }

  const float radiusSq = mData.mMissileDeflectionRadius * mData.mMissileDeflectionRadius;
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CGameProjectile* projectile = TCastToPtr< CGameProjectile >(mgr.ObjectById(*it));
    if (!projectile ||
        !(projectile->GetType() == kWT_Missile ||
          (projectile->GetType() == kWT_Power && projectile->HasAttrib(CWeapon::kPA_ComboShot)))) {
      continue;
    }

    const CVector3f delta = projectile->GetTranslation() - center;
    if (delta.MagSquared() < radiusSq) {
      mDeflectedIds.push_back(*it);
      projectile->SetMinHomingDistance(mData.mMissileDeflectionRadius);

      CProjectileWeapon& weapon = projectile->Projectile();
      const CVector3f dir = delta + (projectile->GetTranslation() - projectile->GetPreviousPos());
      const CVector3f axis = CVector3f::Cross(dir, delta);
      if (axis.CanBeNormalized()) {
        const CQuaternion rotation = CQuaternion::AxisAngle(
            CUnitVector3f(axis), CRelAngle::FromDegrees(mData.mMissileDeflectRate));
        weapon.SetWorldSpaceOrientation(rotation.BuildTransform4f() *
                                        weapon.GetTransform().GetRotation());
      }
    }
  }

  for (rstl::reserved_vector< TUniqueId, 6 >::const_iterator it = mDeflectedIds.begin();
       it != mDeflectedIds.end(); ++it) {
    if (rstl::find(previousIds.begin(), previousIds.end(), *it) != previousIds.end()) {
      continue;
    }

    bool playSound = true;
    if (CSfxManager::IsPlaying(mDeflectSfx)) {
      if (mDeflectSfxTimer > 0.5f) {
        CSfxManager::SfxStop(mDeflectSfx);
      } else {
        playSound = false;
      }
    }
    if (playSound) {
      mDeflectSfx = CSfxManager::AddEmitter(mData.mMissileDeflectionSound, GetTranslation(), 127,
                                            GetCurrentAreaId().Value(), true, false,
                                            CSfxManager::kMedPriority);
      mDeflectSfxTimer = 0.f;
    }
    break;
  }
}

void CShrieker::DetectPlayer(CStateManager& mgr) {
  const CVector3f position = GetTranslation();
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(DetectPlayerMaterial));
  mgr.BuildNearList(nearList,
                    CAABox(position - mDetectionRadius * CVector3f::One(),
                           position + mDetectionRadius * CVector3f::One()),
                    filter, this);
  if (!mScriptedProximity) {
    mPlayerInProximity = false;
  }
  if (nearList.size() == 0) {
    return;
  }

  const float popRadiusSq = mData.mPopDetectionRadius * mData.mPopDetectionRadius;
  const float rustleRadiusSq = mData.mRustleDetectionRadius * mData.mRustleDetectionRadius;
  const float morphballRadiusSq = mData.mMorphballDetectionRadius * mData.mMorphballDetectionRadius;
  const float detectionHeight = mData.mDetectionHeight;
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    const CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(*it));
    if (!player) {
      continue;
    }
    const CVector3f delta = player->GetTranslation() - position;
    const bool morphed = player->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
    const float distanceSq = delta.MagSquared();
    if (detectionHeight > 0.f && CMath::AbsF(delta.GetZ()) > detectionHeight) {
      continue;
    }
    if (morphed && !mScriptedAlert && distanceSq < morphballRadiusSq) {
      mShriekTriggered = true;
      return;
    }
    if (!morphed && !mScriptedAlert && distanceSq < popRadiusSq) {
      mShriekTriggered = true;
      return;
    }
    if (!morphed && !mScriptedProximity && distanceSq < rustleRadiusSq) {
      mPlayerInProximity = true;
    }
  }
}

void CShrieker::ApplyMeleeDamage(CStateManager& mgr) {
  CElementGen* gen = nullptr;
  const CDamageInfo* damage = nullptr;
  if (mEffectGens[0].get() != nullptr) {
    gen = mEffectGens[0].get();
    damage = &mData.mShriekDamage;
  } else if (mEffectGens[1].get() != nullptr) {
    gen = mEffectGens[1].get();
    damage = &mData.mMeleeDamage;
  }
  if (gen == nullptr || damage == nullptr) {
    return;
  }
  const CElementGen::CAdvancedValues* values = gen->ParticleAdditionalData(0);
  if (values == nullptr) {
    return;
  }

  const float radius = rstl::max_val(0.f, values->mValues[0] * gen->GetGlobalScale().GetX());
  const CVector3f center = gen->GetGlobalTranslation();
  const CSphere sphere(center, radius);
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(TriggerMaterial, ImmovableMaterial, DamageSolidMaterial,
                    NonSolidDamageableMaterial),
      CMaterialList(ProjectileMaterial, PowerBombMaterial));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList,
                    CAABox(center - radius * CVector3f::One(), center + radius * CVector3f::One()),
                    filter, this);
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    if (rstl::find(mDamagedIds.begin(), mDamagedIds.end(), *it) == mDamagedIds.end()) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(*it))) {
        const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
        if (bounds && CollisionUtil::AABoxSphereIntersection(*bounds, sphere) &&
            mgr.TestRayDamage(center, *actor, nearList)) {
          mgr.ApplyDamage(GetUniqueId(), *it, GetUniqueId(), *damage,
                          CMaterialFilter::MakeIncludeExclude(
                              CMaterialList(ApplyDamageSolidMaterial), CMaterialList()),
                          CVector3f::Zero());
          mDamagedIds.push_back(*it);
        }
      }
    }
  }
}

bool CShrieker::IsFacingTarget(CStateManager& mgr) const {
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
    const CTransform4f lctr = GetLctrTransform(mRootSegId);
    const CVector3f direction =
        (target->GetAimPosition(mgr, 0.f) - lctr.GetTranslation()).AsNormalized();
    return CVector3f::Dot(GetTransform().GetForward(), direction) > 0.f;
  }
  return false;
}

bool CShrieker::UpdateTargetPosition(CStateManager& mgr) {
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
    mTargetPos = target->GetAimPosition(mgr, 0.2f);
    return true;
  }
  return false;
}

void CShrieker::FindTarget(CStateManager& mgr) {
  if (mTargetId == kInvalidUniqueId) {
    float bestDistance = FLT_MAX;
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      CPlayer* player = mgr.GetPlayer(i);
      const CVector3f aimPosition = player->GetAimPosition(mgr, 0.f);
      const CVector3f delta = aimPosition - GetTranslation();
      const float distance = delta.MagSquared();
      if (distance < bestDistance) {
        mTargetId = player->GetUniqueId();
        mLineOfSightTracker.SetTarget(mTargetId);
        mTargetPos = aimPosition;
        bestDistance = distance;
      }
    }
  }
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
    mTargetPos = target->GetTranslation();
    CVector3f destination = mTargetPos;
    destination.SetZ(destination.GetZ() + mData.mHoverHeight);
    mPathFindNavigation.SetDestination(destination);
    mPathFindNavigation.SetFaceTarget(mSeparating ? kInvalidUniqueId : mTargetId);
  }
  mSeparating = false;
}

void CShrieker::UpdateValidTarget(CStateManager& mgr) {
  bool valid = !mBuried;
  if (valid) {
    valid = mColor.GetAlpha() > 0.5f;
    if (!valid) {
      valid = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo;
    }
  }
  if (static_cast< uint >(valid) != (GetValidTargetPlayers() & 1)) {
    SetValidTarget(0, valid);
    if (valid) {
      AddMaterial(kMT_SeekerTarget, kMT_Target, mgr);
    } else {
      RemoveMaterial(kMT_SeekerTarget, kMT_Target, mgr);
    }
  }
}

void CShrieker::FadeAlpha(CStateManager& mgr, bool fadeIn, bool immediate) {
  if (immediate) {
    mColor.SetAlpha(1.f);
    mAlphaDelta = 4.f;
  } else {
    mAlphaDelta = (fadeIn ? 1.f : -1.f) / rstl::max_val(mData.mVisibilityChangeTime, FLT_MIN);
  }
}

void CShrieker::ReactToDamage(CStateManager& mgr, TUniqueId senderId) {
  if (mBuried) {
    const float health = GetHealthInfo()->GetHP();
    const float damage = mPreviousHealth - health;
    if (damage > 0.f) {
      if (const CWeapon* weapon =
              TCastToPtr< CWeapon >(const_cast< CEntity* >(mgr.GetObjectById(senderId)))) {
        float scale = 1.f;
        const bool comboShot = weapon->HasAttrib(CWeapon::kPA_ComboShot);
        switch (weapon->GetType()) {
        case kWT_Power:
        case kWT_Dark:
        case kWT_Light:
        case kWT_Annihilator:
          scale = comboShot ? mData.mHostileAccumulatePriority : 0.f;
          break;
        case kWT_Bomb:
        case kWT_Missile:
          scale = mData.mHostileAccumulatePriority;
          break;
        }
        if (scale != 1.f) {
          HealthInfo()->SetHP(damage * (1.f - scale) + health);
        }
      }
      mPreviousHealth = GetHealthInfo()->GetHP();
      mShriekTriggered = true;
    }
  }
  if (rstl::string_l(mStateMachine->GetName()) == "Patrol" ||
      rstl::string_l(mStateMachine->GetName()) == "ReturnToStart") {
    if (const CWeapon* weapon =
            TCastToPtr< CWeapon >(const_cast< CEntity* >(mgr.GetObjectById(senderId)))) {
      mTargetId = weapon->GetOwnerId();
      mLineOfSightTracker.SetTarget(mTargetId);
      UpdateTargetPosition(mgr);
    }
  }
}

void CShrieker::Buried(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mDeflectedIds.clear();
    BodyController()->SetLocomotionType(pas::kLT_Internal9);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CShrieker::BuriedRumbling(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CShrieker::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    FindTarget(mgr);
    break;
  case kStateMsg_Update:
    FaceTarget(mgr, dt);
    mFireTimer -= dt;
    mMeleeTimer -= dt;
    mDodgeTimer -= dt;
    mLineOfSightTracker.Update(dt, mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CShrieker::UprootShriek(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mPathFindSearch.SetCharacterRadius(3.f);
    mPathFindSearch.SetCharacterHeight(mData.mHoverHeight);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
      mBodyController->SetLocomotionType(pas::kLT_Combat);
    }
    break;
  case kStateMsg_Deactivate:
    mBuried = false;
    SetBuriedCollision(mgr, false);
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mBodyController->SetLocomotionType(pas::kLT_Combat);
    mStartPosition = GetTranslation();
    FadeAlpha(mgr, false, false);
    break;
  }
}

void CShrieker::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  const bool hasTarget = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId)) != nullptr;
  switch (msg) {
  case kStateMsg_Activate:
    mChasing = true;
    if (BodyController()->GetLocomotionType() != pas::kLT_Lurk) {
      BodyController()->SetLocomotionType(pas::kLT_Lurk);
    }
    mLineOfSightTracker.ClearLineOfSight();
    break;
  case kStateMsg_Update:
    if (hasTarget) {
      if (CPatterned::PathShagged(mgr, CTriggerData(0.f))) {
        mChaseTimer += dt;
        FaceTarget(mgr, dt);
      } else {
        mChaseTimer = 0.f;
      }
      if (IsFacingTarget(mgr)) {
        mLineOfSightTracker.Update(dt, mgr);
      } else {
        mLineOfSightTracker.ClearLineOfSight();
      }
    } else {
      mTargetId = kInvalidUniqueId;
      mLineOfSightTracker.SetTarget(mTargetId);
    }
    break;
  case kStateMsg_Deactivate:
    mChasing = false;
    break;
  }
  if (mChasing) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    ApplySeparation(mgr);
  }
}

void CShrieker::ShriekAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    FadeAlpha(mgr, true, false);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_ProjectileAttack)) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCProjectileAttackCmd(pas::kS_Zero, mTargetPos, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    ResetAttackTimer(mgr, 1);
    ResetAttackTimer(mgr, 0);
    EndTeamAttack(mgr, true);
    FadeAlpha(mgr, false, false);
    break;
  }
}

void CShrieker::Bury(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    FadeAlpha(mgr, true, false);
    SetBuriedCollision(mgr, true);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
      mBodyController->SetLocomotionType(pas::kLT_Internal9);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    {
      CVector3f position = GetTranslation();
      position.SetZ(mBuriedPosition.GetZ());
      SetTranslation(position);
    }
    mBodyController->SetLocomotionType(pas::kLT_Internal9);
    mBuried = true;
    mShriekTriggered = false;
    mReferenceHealth = GetHealthInfo()->GetHP();
    break;
  }
}

void CShrieker::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mTargetId = kInvalidUniqueId;
    mLineOfSightTracker.SetTarget(mTargetId);
    mChaseTimer = 0.f;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CShrieker::ReturnToStart(CStateManager& mgr, EStateMsg msg, float dt) {
  bool pathFind = true;
  switch (msg) {
  case kStateMsg_Activate:
    mChaseTimer = 0.f;
    mTargetId = kInvalidUniqueId;
    mLineOfSightTracker.SetTarget(mTargetId);
    mPathFindNavigation.SetDestination(mStartPosition);
    mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
    break;
  case kStateMsg_Update:
    if ((mStartPosition - GetTranslation()).MagSquared() < 4.f) {
      pathFind = false;
      BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
          mSteeringBehaviors.Arrival(*this, mStartPosition, 0.f), CVector3f::Zero(), 1.f));
    }
    break;
  case kStateMsg_Deactivate:
    FindTarget(mgr);
    pathFind = false;
    break;
  }
  if (pathFind) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
  }
}

void CShrieker::Retreat(CStateManager&, EStateMsg, float) {}

void CShrieker::Melee(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    FadeAlpha(mgr, true, false);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_MeleeAttack)) {
      mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    ResetAttackTimer(mgr, 1);
    ResetAttackTimer(mgr, 0);
    mDamagedIds.clear();
    EndTeamAttack(mgr, false);
    FadeAlpha(mgr, false, false);
    break;
  }
}

void CShrieker::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  DeliverCommand(msg, pas::kAS_Step,
                 CBCStepCmd(mDodgeRight ? pas::kSD_Right : pas::kSD_Left, pas::kStep_Dodge));
  switch (msg) {
  case kStateMsg_Deactivate:
    mDodgeTimer = mData.mDodgeTime;
    break;
  case kStateMsg_Activate:
  case kStateMsg_Update:
    break;
  }
}

void CShrieker::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    FadeAlpha(mgr, true, true);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
  CPatterned::Dead(mgr, msg, dt);
}

bool CShrieker::PathShagged(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::PathShagged(mgr, data) && PlayerLeashReached(mgr, data);
}

bool CShrieker::PlayerEntersProximity(CStateManager&, const CTriggerData&) const {
  return mPlayerInProximity;
}

bool CShrieker::PlayerLeavesProximity(CStateManager&, const CTriggerData&) const {
  return !mPlayerInProximity;
}

bool CShrieker::EnterUprootShriek(CStateManager&, const CTriggerData&) const {
  return mShriekTriggered;
}

bool CShrieker::HasPatrolPath(CStateManager&, const CTriggerData&) const { return mHasPatrolPath; }

bool CShrieker::PlayerLeashReached(CStateManager&, const CTriggerData&) const {
  return mTargetId == kInvalidUniqueId || mChaseTimer > mPlayerLeashTime;
}

bool CShrieker::LeavePatrol(CStateManager& mgr, const CTriggerData& data) const {
  return mTargetId != kInvalidUniqueId || CPatterned::InDetectionRange(mgr, data);
}

bool CShrieker::CloseToStart(CStateManager&, const CTriggerData&) const {
  const CVector3f delta = mStartPosition - GetTranslation();
  bool result = false;
  bool nearXY = false;
  if (CMath::AbsF(delta.GetX()) < skCloseToStartTolerance.GetX() &&
      CMath::AbsF(delta.GetY()) < skCloseToStartTolerance.GetY()) {
    nearXY = true;
  }
  if (nearXY && CMath::AbsF(delta.GetZ()) < skCloseToStartTolerance.GetZ()) {
    result = true;
  }
  return result;
}

bool CShrieker::LineOfSight(CStateManager&, const CTriggerData&) const {
  return mLineOfSightTracker.HasLineOfSight();
}

bool CShrieker::ShouldFire(CStateManager& mgr, const CTriggerData&) const {
  bool result = false;
  const CTeamAiRole* role = CScriptTeamAiMgr::GetTeamAiRole(mgr, mTeamAiMgrId, GetUniqueId());
  if (mFireTimer <= 0.f &&
      (role == nullptr || role->GetTeamAiRole() != CTeamAiRole::kTAR_Initial)) {
    float height = 0.f;
    if (GetHeightAboveMesh(height) && CMath::AbsF(height) < 0.5f) {
      if (mTeamAiMgrId == kInvalidUniqueId ||
          CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                        GetUniqueId())) {
        result = true;
      }
    }
  }
  return result;
}

bool CShrieker::ShouldMelee(CStateManager& mgr, const CTriggerData&) const {
  bool result = false;
  const CTeamAiRole* role = CScriptTeamAiMgr::GetTeamAiRole(mgr, mTeamAiMgrId, GetUniqueId());
  if (mMeleeTimer <= 0.f &&
      (role == nullptr || role->GetTeamAiRole() != CTeamAiRole::kTAR_Initial)) {
    if (const CActor* target = TCastToPtr< CActor >(mgr.ObjectById(mTargetId))) {
      const CVector3f from = GetAimPosition(mgr, 0.f);
      const CVector3f to = target->GetAimPosition(mgr, 0.f);
      if ((to - from).MagSquared() < mData.mMeleeRangeSquared) {
        if (mTeamAiMgrId == kInvalidUniqueId ||
            CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                          GetUniqueId())) {
          result = true;
        }
      }
    }
  }
  return result;
}

bool CShrieker::ShouldDodge(CStateManager& mgr, const CTriggerData&) const {
  CShrieker* self = const_cast< CShrieker* >(this);
  if (mDodgeTimer <= 0.f) {
    self->mDodgeTimer = mData.mDodgeTime;
    const float dodgePercentage = mData.mDodgePercentage;
    if (mgr.Random()->Range(0.f, 100.f) < dodgePercentage) {
      self->mDodgeRight = mgr.Random()->Range(0, 1) == 0;
      const CVector3f from = GetTranslation();
      for (int i = 0; i < 2; ++i) {
        CVector3f offset = mCheckRadius * GetTransform().GetRight();
        if (!mDodgeRight) {
          offset *= -1.f;
        }
        const CVector3f to = from + offset;
        if (mPathFindSearch.PathExists(from, to) == CPathFindSearch::kR_Success) {
          const CMaterialFilter filter =
              CMaterialFilter::MakeInclude(CMaterialList(DodgeSolidMaterial));
          if (mgr.RayCollideWorld(from, to, filter, this)) {
            rstl::reserved_vector< TUniqueId, 1024 > nearList;
            mgr.BuildNearList(
                nearList,
                CAABox(to - mCheckRadius * CVector3f::One(), to + mCheckRadius * CVector3f::One()),
                filter, this);
            if (nearList.size() == 0) {
              return true;
            }
          }
        } else {
          self->mDodgeRight = !mDodgeRight;
        }
      }
    }
  }
  return false;
}

bool CShrieker::InMaxRange(CStateManager& mgr, const CTriggerData&) const {
  if (const CActor* target = TCastToPtr< CActor >(mgr.ObjectById(mTargetId))) {
    const CVector3f delta = target->GetTranslation() - GetTranslation();
    const float range = mMaxAttackRange - (mBuried ? 0.25f * mMaxAttackRange : 0.f);
    return delta.MagSquared() < range * range;
  }
  return false;
}

template < typename T >
void CShrieker::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBodyController->CommandMgr().DeliverCmd(cmd);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, state)) {
      mBodyController->CommandMgr().DeliverCmd(cmd);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

CEntity* LoadShrieker(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrShrieker sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrShrieker.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CShriekerData data(
      sldrThis.detectionHeight, sldrThis.rustleDetectionRadius, sldrThis.popDetectionRadius,
      sldrThis.morphballDetectionRadius, sldrThis.pART, LdrToDamageInfo(sldrThis.damageInfo),
      sldrThis.projectile, LdrToDamageInfo(sldrThis.projectileDamage),
      sldrThis.combatVisorMaxVolume, sldrThis.echoVisorMaxVolume, sldrThis.meleeEffect,
      LdrToDamageInfo(sldrThis.meleeDamage), sldrThis.meleeRange, sldrThis.meleeAverageAttackTime,
      sldrThis.meleeAttackTimeVariation, LdrToDamageVulnerability(sldrThis.buriedVulnerability),
      sldrThis.hostileAccumulatePriority, sldrThis.hoverHeight, sldrThis.missileDeflectionOffset,
      sldrThis.missileDeflectionRadius, sldrThis.missileDeflectRate,
      sldrThis.sound_MissileDeflection, sldrThis.dodgeTime, sldrThis.dodgePercentage,
      sldrThis.visibilityChangeTime);

  return rs_new CShrieker(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                          LdrToEntityInfo(info, sldrThis.editorProperties),
                          LdrToTransform4f(sldrThis.editorProperties), *modelData,
                          LdrToPatternedInfo(sldrThis.patterned, nullptr),
                          LdrToActorParameters(sldrThis.actorInformation), data);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SShrieker_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadShrieker;
  SetSShrieker_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSShrieker_FuncPtrs(nullptr); }
#endif
