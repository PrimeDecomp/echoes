#include "MetroidPrime/Enemies/CSplitterCommandModule.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CParticleGenInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CSplitterBeamEffect.hpp"
#include "MetroidPrime/Enemies/CSplitterMainChassis.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplitterCommandModule.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"
#include "REL/REL_Setup.h"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::StateOver)},
    {"InDetectionRange", static_cast< CPatterned::StateMachine::TriggerFunc >(
                             &CSplitterCommandModule::InDetectionRange)},
    {"IsScanning",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::IsScanning)},
    {"IsInitiallyDocked", static_cast< CPatterned::StateMachine::TriggerFunc >(
                              &CSplitterCommandModule::IsInitiallyDocked)},
    {"IsDocked",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::IsDocked)},
    {"HasDockingPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::HasDockingPath)},
    {"HasDockingTarget", static_cast< CPatterned::StateMachine::TriggerFunc >(
                             &CSplitterCommandModule::HasDockingTarget)},
    {"DockingPathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(
                            &CSplitterCommandModule::DockingPathOver)},
    {"HasTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::HasTarget)},
    {"InHoverRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::InHoverRange)},
    {"InLaserPulseRange", static_cast< CPatterned::StateMachine::TriggerFunc >(
                              &CSplitterCommandModule::InLaserPulseRange)},
    {"ShouldDodge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::ShouldDodge)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::ShouldAttack)},
    {"ShouldFireAgain", static_cast< CPatterned::StateMachine::TriggerFunc >(
                            &CSplitterCommandModule::ShouldFireAgain)},
    {"ShouldLaserSweep", static_cast< CPatterned::StateMachine::TriggerFunc >(
                             &CSplitterCommandModule::ShouldLaserSweep)},
    {"PathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::PathOver)},
    {"PathShagged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterCommandModule::PathShagged)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::Start)},
    {"Idle", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::Idle)},
    {"SpawnIdle",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::SpawnIdle)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::Dead)},
    {"Scanning",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::Scanning)},
    {"FaceTarget",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::FaceTarget)},
    {"PathFind",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::PathFind)},
    {"Hover", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::Hover)},
    {"Dodge", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::Dodge)},
    {"LaserPulse",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::LaserPulse)},
    {"LaserSweep",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::LaserSweep)},
    {"LostChassisReaction", static_cast< CPatterned::StateMachine::StateFunc >(
                                &CSplitterCommandModule::LostChassisReaction)},
    {"SeekMainChassis",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterCommandModule::SeekMainChassis)},
    {"FollowDockingPath", static_cast< CPatterned::StateMachine::StateFunc >(
                              &CSplitterCommandModule::FollowDockingPath)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"NotifyDocking",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterCommandModule::NotifyDocking)},
    {"SelectTarget",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterCommandModule::SelectTarget)},
    {"SetTargetDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterCommandModule::SetTargetDest)},
    {"SetDockingDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterCommandModule::SetDockingDest)},
    {"FindBestDodgeDirection", static_cast< CPatterned::StateMachine::CodeFunc >(
                                   &CSplitterCommandModule::FindBestDodgeDirection)},
    {"RaiseShields",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterCommandModule::RaiseShields)},
    {"ResetAttackTimes",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterCommandModule::ResetAttackTimes)},
};

// Guessed struct: the sphere that wraps the shield collision actor.
struct SShieldSphere {
  const char* mName;
  float mRadius;
};

static const SShieldSphere skShieldSphere = {"Skeleton_Root", 3.f}; // Guessed name
static const char* const skShieldActorName = "Skeleton_Root";       // Guessed name
static const char* const skBeamLocatorName = "Beam_LCTR";           // Guessed name
static const char* const skLightShieldEffect = "LightShield";       // Guessed name
static const char* const skDarkShieldEffect = "DarkShield";         // Guessed name
static const char* const skLaserMuzzleEffect = "LaserMuzzle";       // Guessed name
static const char* const skAlertEyeEffect = "AlertEye";             // Guessed name

static EMaterialTypes skSolidMaterial = kMT_Solid;                       // Guessed name
static EMaterialTypes skCollisionActorMaterial = kMT_CollisionActor;     // Guessed name
static EMaterialTypes skCharacterMaterial = kMT_Character;               // Guessed name
static EMaterialTypes skWallMaterial = kMT_Wall;                         // Guessed name
static EMaterialTypes skFloorMaterial = kMT_Floor;                       // Guessed name
static EMaterialTypes skCeilingMaterial = kMT_Ceiling;                   // Guessed name
static EMaterialTypes skPathSolidMaterial = kMT_Solid;                   // Guessed name
static EMaterialTypes skPathCollisionActorMaterial = kMT_CollisionActor; // Guessed name

SSplitterCommandModuleData::SSplitterCommandModuleData(const SLdrSplitterCommandModuleData& data)
: SLdrSplitterCommandModuleData(data)
, laserPulseDamageInfo(LdrToDamageInfo(data.laserPulseDamage))
, laserSweepDamageInfo(LdrToDamageInfo(data.laserSweepDamage))
, lightShieldVulnerabilityData(LdrToDamageVulnerability(data.lightShieldVulnerability))
, darkShieldVulnerabilityData(LdrToDamageVulnerability(data.darkShieldVulnerability)) {}

CSplitterCommandModule::CSplitterCommandModule(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const CModelData& modelData,
                                               const CActorParameters& actorParams,
                                               const CPatternedInfo& patternedInfo,
                                               const SSplitterCommandModuleData& data)
: CPatterned(kPAI_SplitterCommandModule, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_AiMovedFlyer, actorParams)
, mData(data)
, mPathFindSearch(nullptr, 3, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mCollisionActorManager(nullptr)
, mLaserPulseProjectileInfo(data.laserPulseProjectile, data.laserPulseDamageInfo)
, mLaserSweepProjectileInfo(data.laserSweepBeamInfo.weaponSystem, CDamageInfo())
, mDamageVulnerability(*CPatterned::GetDamageVulnerability())
, mVulnerabilityState(kVS_Normal)
, mShieldType(kST_None)
, mLastShieldType(kST_None)
, mStepDistance(0.f)
, mDodgeCount(-1)
, mSelfDestructTimer(0.f)
, mCollisionTime(0.f)
, mBeamLocator(CSegId::Invalid())
, mMainChassisId(kInvalidUniqueId)
, mDockingTargetId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mShieldCollisionActorId(kInvalidUniqueId)
, mLaserSweepProjectileId(kInvalidUniqueId)
, mFlashColor(CColor::White())
, mFlashTimer(0.f)
, mAttackTimer(0.f)
, mShieldTimer(0.f)
, mDockSearchTimer(3.f)
, mTimeInState(0.f)
, mTimeSinceChassisShield(0.f)
, mShotsRemaining(0)
, mFacingDirection(xf.GetForward())
, mDodgeDirection(pas::kSD_Invalid)
, mLaserSweepStep(-1)
, mLaserSweepStart(CVector3f::Zero())
, mLaserSweepEnd(CVector3f::Zero())
, mLaserSweepDirection(CVector3f::Zero())
, mScanBeamEffectId(kInvalidUniqueId)
, mHasDestination(true)
, mPathObstructed(false)
, mLevelingOut(false)
, mCanBreakLockOn(true)
, mShieldsRaised(false)
, mAutoDestructing(false)
, mFirstDocking(true)
, mCollided(false) {
  mLaserPulseProjectileInfo.Token().Lock();
  mLaserSweepProjectileInfo.Token().Lock();
  CAnimData* animData = AnimationData();
  mBeamLocator = animData->GetLocatorSegId(rstl::string_l(skBeamLocatorName));
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skSolidMaterial),
      CMaterialList(skCollisionActorMaterial, skCharacterMaterial)));
  const CPASAnimParmData parms(pas::kAS_Step, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1));
  mStepDistance = GetModelData()->GetScale().GetX() * GetAnimationDistance(parms);
}

CSplitterCommandModule::~CSplitterCommandModule() {}

void CSplitterCommandModule::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    CreateCollisionActors(mgr);
    mLastShieldType = mgr.Random()->Range(0.f, 100.f) < 50.f ? kST_Light : kST_Dark;
    break;
  case kSM_Delete:
    if (mCollisionActorManager.get()) {
      mCollisionActorManager->Destroy(mgr);
    }
    if (mScanBeamEffectId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mScanBeamEffectId);
    }
    StopLaserSweep(mgr);
    break;
  case kSM_Activate:
    if (mCollisionActorManager.get()) {
      mCollisionActorManager->SetActive(mgr, true);
    }
    break;
  case kSM_Deactivate:
    if (mCollisionActorManager.get()) {
      mCollisionActorManager->SetActive(mgr, false);
    }
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionActorManager.get()) {
      mCollisionActorManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Alert:
    SetHitByPlayerProjectile(true);
    break;
  case kSM_AreaLoaded:
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    FindDockingTargetFromConnection(mgr);
    break;
  case kSM_Damage:
    OnShieldHit(msg.GetSenderId());
    break;
  default:
    break;
  }
}

void CSplitterCommandModule::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
  mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  UpdateChassisLink(mgr);
  UpdateTimers(dt, mgr);
  UpdateShield(mgr);
  UpdateLaserSweep(mgr, dt);
  UpdateScanBeam(mgr, dt);
  UpdateCollisionTimer(dt, mgr);
}

void CSplitterCommandModule::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                             EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_BreakLockOn:
    if (mCanBreakLockOn && (mData.unknown_0xbd80fd94 & 4)) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId))) {
        player->SetOrbitRequestForTarget(GetUniqueId(), CPlayer::kOR_ActivateOrbitSource, mgr);
      }
    }
    handled = true;
    break;
  case kUE_Projectile:
    FireLaserPulse(mgr, node.GetLocatorName());
    handled = true;
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CSplitterCommandModule::Render(const CStateManager& mgr) const {
  if (mAlive) {
    CPatterned::Render(mgr);
  }
}

void CSplitterCommandModule::PreRender(CStateManager& mgr) {
  if (mAlive) {
    CPatterned::PreRender(mgr);
  }
}

void CSplitterCommandModule::AddToRenderer(const CStateManager& mgr) const {
  if (mAlive) {
    CPatterned::AddToRenderer(mgr);
  }
}

void CSplitterCommandModule::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                          CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (!mCollided && id == kInvalidUniqueId) {
    static const CMaterialList testList(skWallMaterial, skFloorMaterial, skCeilingMaterial);
    for (int i = 0; i < list.GetCount(); ++i) {
      if (list[i].GetMaterialLeft().SharesMaterials(testList)) {
        mCollided = true;
        break;
      }
    }
  }
}

void CSplitterCommandModule::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {}

const CDamageVulnerability* CSplitterCommandModule::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

CVector3f CSplitterCommandModule::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f aim = CPatterned::GetAimPosition(mgr, dt);
  if (mVulnerabilityState == kVS_ChassisShield) {
    if (const CSplitterMainChassis* chassis =
            TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mMainChassisId))) {
      const CVector3f chassisAim = chassis->GetAimPosition(mgr, dt);
      const float t = CMath::Min(1.f, mTimeInState);
      aim = CVector3f::Lerp(aim, chassisAim, t);
    }
  } else if (mTimeSinceChassisShield < 1.f) {
    if (const CSplitterMainChassis* chassis =
            TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mMainChassisId))) {
      const CVector3f chassisAim = chassis->GetAimPosition(mgr, dt);
      const float t = CMath::Min(1.f, mTimeSinceChassisShield);
      aim = CVector3f::Lerp(chassisAim, aim, t);
    }
  }
  return aim;
}

bool CSplitterCommandModule::Listen(CStateManager& mgr, const CVector3f& position,
                                    EListenNoiseType type) {
  bool handled = false;
  if (mAlive) {
    switch (type) {
    case kLNT_PathObstruction: {
      const CVector3f delta = position - GetTranslation();
      if (delta.MagSquared() < 400.f) {
        mPathObstructed = handled = true;
      }
      break;
    }
    default:
      break;
    }
  }
  return handled;
}

void CSplitterCommandModule::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

bool CSplitterCommandModule::CanBeIngPossessed(CStateManager& mgr) const {
  if (mMainChassisId != kInvalidUniqueId) {
    return false;
  }
  return CPatterned::CanBeIngPossessed(mgr);
}

void CSplitterCommandModule::SetChassisShieldState(CStateManager& mgr) {
  mDamageVulnerability = CDamageVulnerability::ReflectVulnerabilty();
  SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
  mVulnerabilityState = kVS_ChassisShield;
  mTimeInState = 0.f;
  UpdateEffects(mgr);
}

void CSplitterCommandModule::SetNormalState(CStateManager& mgr) {
  mDamageVulnerability = *CPatterned::GetDamageVulnerability();
  SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
  if (mVulnerabilityState == kVS_ChassisShield) {
    mTimeSinceChassisShield = 0.f;
  }
  mVulnerabilityState = kVS_Normal;
  mTimeInState = 0.f;
  UpdateEffects(mgr);
}

void CSplitterCommandModule::SetReflectiveState(CStateManager& mgr) {
  mDamageVulnerability = CDamageVulnerability::ReflectVulnerabilty();
  SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
  if (mVulnerabilityState == kVS_ChassisShield) {
    mTimeSinceChassisShield = 0.f;
  }
  mVulnerabilityState = kVS_Reflective;
  mTimeInState = 0.f;
  UpdateEffects(mgr);
}

void CSplitterCommandModule::SetInvulnerableState(CStateManager& mgr) {
  mDamageVulnerability = CDamageVulnerability::ReflectVulnerabilty();
  SetVisorOrbitableFlags(CVisorParameters::kVOF_All, false);
  mVulnerabilityState = kVS_Invulnerable;
  mTimeInState = 0.f;
  UpdateEffects(mgr);
}

void CSplitterCommandModule::RequestLaserSweep(const CVector3f& start, const CVector3f& end) {
  if (mMainChassisId != kInvalidUniqueId) {
    mLaserSweepStep = 0;
    mLaserSweepStart = start;
    mLaserSweepEnd = end;
  }
}

CVector3f CSplitterCommandModule::GetBeamPosition() const {
  const CTransform4f locator = GetLctrTransform(mBeamLocator);
  return locator.GetTranslation();
}

void CSplitterCommandModule::AutoDestruct(float time) {
  if (mAlive && !mAutoDestructing) {
    mAutoDestructing = true;
    mSelfDestructTimer = time;
  }
}

bool CSplitterCommandModule::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

bool CSplitterCommandModule::InDetectionRange(CStateManager& mgr, const CTriggerData& data) const {
  if (IsScanning(mgr, data) && CPatterned::InDetectionRange(mgr, data)) {
    CVector3f delta = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
    delta.SetZ(0.f);
    const float magnitude = delta.Magnitude();
    return CVector3f::Dot(delta, GetTransform().GetForward()) > magnitude * mDetectionAngle;
  }
  return false;
}

bool CSplitterCommandModule::IsScanning(CStateManager& mgr, const CTriggerData& data) const {
  const CSplitterMainChassis* chassis =
      TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mMainChassisId));
  return chassis ? chassis->GetScanTimer() > 0.f : false;
}

bool CSplitterCommandModule::IsInitiallyDocked(CStateManager& mgr, const CTriggerData& data) const {
  return mData.unknown_0xbd80fd94 & 1;
}

bool CSplitterCommandModule::IsDocked(CStateManager& mgr, const CTriggerData& data) const {
  return mMainChassisId != kInvalidUniqueId;
}

bool CSplitterCommandModule::HasDockingPath(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Connect, kSM_Follow) != kInvalidUniqueId;
}

bool CSplitterCommandModule::HasDockingTarget(CStateManager& mgr, const CTriggerData& data) const {
  const CSplitterMainChassis* chassis =
      TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mDockingTargetId));
  bool hasTarget = false;
  if (chassis && chassis->GetAlive() && chassis->GetActive() &&
      chassis->GetCommandModuleId() == kInvalidUniqueId) {
    hasTarget = true;
  }
  return hasTarget;
}

bool CSplitterCommandModule::DockingPathOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CSplitterCommandModule::HasTarget(CStateManager& mgr, const CTriggerData& data) const {
  return mTargetId != kInvalidUniqueId;
}

bool CSplitterCommandModule::InHoverRange(CStateManager& mgr, const CTriggerData& data) const {
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const float hoverRange = 0.5f * (mData.minLaserPulseRange + mData.maxLaserPulseRange);
    const CVector3f delta = target->GetTranslation() - GetTranslation();
    return delta.MagSquared() <= hoverRange * hoverRange;
  }
  return false;
}

bool CSplitterCommandModule::InLaserPulseRange(CStateManager& mgr, const CTriggerData& data) const {
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const CVector3f delta = target->GetTranslation() - GetTranslation();
    const float distanceSquared = delta.MagSquared();
    const float minRangeSquared = mData.minLaserPulseRange * mData.minLaserPulseRange;
    const float maxRangeSquared = mData.maxLaserPulseRange * mData.maxLaserPulseRange;
    return distanceSquared >= minRangeSquared && distanceSquared <= maxRangeSquared;
  }
  return false;
}

bool CSplitterCommandModule::ShouldDodge(CStateManager& mgr, const CTriggerData& data) const {
  return mDodgeDirection != pas::kSD_Invalid;
}

bool CSplitterCommandModule::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackTimer <= 0.f;
}

bool CSplitterCommandModule::ShouldFireAgain(CStateManager& mgr, const CTriggerData& data) const {
  return mShotsRemaining > 0;
}

bool CSplitterCommandModule::ShouldLaserSweep(CStateManager& mgr, const CTriggerData& data) const {
  return mLaserSweepStep != -1;
}

bool CSplitterCommandModule::PathOver(CStateManager& mgr, const CTriggerData& data) const {
  bool over = false;
  if (!mHasDestination || CPatterned::PathOver(mgr, data)) {
    over = true;
  }
  return over;
}

bool CSplitterCommandModule::PathShagged(CStateManager& mgr, const CTriggerData& data) const {
  bool shagged = false;
  if (mPathObstructed || CPatterned::PathShagged(mgr, data)) {
    shagged = true;
  }
  return shagged;
}

void CSplitterCommandModule::Start(CStateManager& mgr, EStateMsg msg, float dt) {}

void CSplitterCommandModule::Idle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
  case kStateMsg_Update:
    mFacingDirection = GetTransform().GetForward();
    break;
  default:
    break;
  }
}

void CSplitterCommandModule::SpawnIdle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mFacingDirection = CVector3f::Zero();
    break;
  case kStateMsg_Update:
    if (mDockingTargetId == kInvalidUniqueId) {
      mDockingTargetId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    }
    break;
  default:
    break;
  }
}

void CSplitterCommandModule::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    StopLaserSweep(mgr);
    DeathDelete(mgr);
    break;
  default:
    break;
  }
}

void CSplitterCommandModule::Scanning(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SetHitByPlayerProjectile(false);
    mFacingDirection = CVector3f::Zero();
    UpdateEffects(mgr);
    break;
  case kStateMsg_Update: {
    if (CSplitterMainChassis* chassis =
            TCastToPtr< CSplitterMainChassis >(mgr.ObjectById(mMainChassisId))) {
      if (GetHitByPlayerProjectile() || InDetectionRange(mgr, CTriggerData(0.f))) {
        chassis->SetHitByPlayerProjectile(true);
      }
    }
    const float turn = dt * mData.scanningTurnSpeed;
    const CVector3f forward(GetTransform().GetForward().ToVec2f(), 0.f);
    const CVector3f right(GetTransform().GetRight().ToVec2f(), 0.f);
    if (forward.IsMagnitudeSafe() && right.IsMagnitudeSafe()) {
      mFacingDirection = CVector3f::Slerp(forward.AsNormalized(), right.AsNormalized(),
                                          CRelAngle::FromDegrees(turn));
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateEffects(mgr);
    break;
  }
}

void CSplitterCommandModule::FaceTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mFacingDirection = GetTransform().GetForward();
    break;
  case kStateMsg_Update:
    if (dt > 0.f) {
      if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
        CVector3f direction = target->GetTranslation() - GetTranslation();
        direction.SetZ(0.f);
        if (direction.IsMagnitudeSafe()) {
          const CVector3f forward = GetTransform().GetForward();
          const float turnSpeed = dt * mData.maxTurnSpeed;
          if (CVector3f::GetAngleDiff(forward, direction) < turnSpeed * (M_PIF / 180.f)) {
            mFacingDirection = direction.AsNormalized();
          } else {
            mFacingDirection = CVector3f::Slerp(forward, direction.AsNormalized(),
                                                CRelAngle::FromDegrees(turnSpeed));
          }
        }
      }
    }
    break;
  default:
    break;
  }
}

void CSplitterCommandModule::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (mHasDestination) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
  }
  switch (msg) {
  case kStateMsg_Activate:
    mPathObstructed = false;
    break;
  case kStateMsg_Update: {
    CVector3f velocity = BodyController()->CommandMgr().GetMoveVector();
    velocity += GetSeparationVector(mgr);
    if (velocity.IsMagnitudeSafe()) {
      const float step = dt * mData.maxLinearVelocity;
      const CVector3f movement = step * velocity;
      MoveTowards(GetTranslation() + movement, dt);
      CVector3f direction = movement;
      if (const CActor* target = static_cast< const CActor* >(
              mgr.GetObjectById(mPathFindNavigation.GetFaceTarget()))) {
        direction = target->GetTranslation() - GetTranslation();
        direction.SetZ(0.f);
      }
      BodyController()->FaceDirection(direction, dt);
    }
    break;
  }
  case kStateMsg_Deactivate:
    mHasDestination = true;
    break;
  }
}

void CSplitterCommandModule::Hover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
    if (dt > 0.f) {
      if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
        const CVector3f aim = target->GetAimPosition(mgr, 0.f);
        const CVector3f flat(aim.GetX() - GetTranslation().GetX(),
                             aim.GetY() - GetTranslation().GetY(), 0.f);
        const float dz = aim.GetZ() - GetTranslation().GetZ();
        BodyController()->FaceDirection(flat, dt);
        const float step = dt * mData.maxLinearVelocity;
        if (CMath::AbsF(dz) > step) {
          MoveTowards(GetTranslation() + CVector3f(0.f, 0.f, dz > 0.f ? step : -step), dt);
        }
      }
    }
    break;
  default:
    break;
  }
}

void CSplitterCommandModule::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mCanBreakLockOn = true;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDirection, pas::kStep_Dodge));
    } else if (dt > 0.f) {
      if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
        CVector3f direction = target->GetTranslation() - GetTranslation();
        direction.SetZ(0.f);
        BodyController()->FaceDirection(direction, dt);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mCanBreakLockOn = false;
    break;
  }
}

void CSplitterCommandModule::LaserPulse(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mDodgeCount = 0;
    --mShotsRemaining;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(
          pas::kS_One, GetTranslation() + GetTransform().GetForward(), false));
    } else if (dt > 0.f) {
      if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
        const CVector3f aim = target->GetAimPosition(mgr, 0.f);
        const CVector3f direction(aim.GetX() - GetTranslation().GetX(),
                                  aim.GetY() - GetTranslation().GetY(), 0.f);
        BodyController()->FaceDirection(direction, dt);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterCommandModule::LaserSweep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mLaserSweepDirection = GetTransform().GetForward();
    if (mgr.GetObjectById(mTargetId)) {
      const CVector3f toStart = mLaserSweepStart - GetTranslation();
      const CVector3f toEnd = mLaserSweepEnd - GetTranslation();
      const float startAngle = CVector3f::GetAngleDiff(mLaserSweepDirection, toStart);
      if (CVector3f::GetAngleDiff(mLaserSweepDirection, toEnd) < startAngle) {
        const CVector3f start = mLaserSweepStart;
        mLaserSweepStart = mLaserSweepEnd;
        mLaserSweepEnd = start;
      }
    }
    CSfxManager::AddEmitter(mData.sound_LaserChargeUp, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    break;
  case kStateMsg_Update:
    if (dt > 0.f) {
      const CVector3f& source = mLaserSweepStep == 0 ? mLaserSweepStart : mLaserSweepEnd;
      const CVector3f sourceCopy = source;
      const CTransform4f beamXf = GetLctrTransform(mBeamLocator);
      const CVector3f toSource = sourceCopy - beamXf.GetTranslation();
      const CVector3f flat(toSource.GetX(), toSource.GetY(), 0.f);
      if (flat.IsMagnitudeSafe()) {
        const CVector3f forward = GetTransform().GetForward();
        const float turnSpeed = dt * mData.laserSweepTurnSpeed;
        if (CVector3f::GetAngleDiff(forward, flat) < turnSpeed * (M_PIF / 180.f)) {
          mFacingDirection = flat.AsNormalized();
          mLaserSweepDirection = toSource.AsNormalized();
          if (mLaserSweepStep == 0) {
            mLaserSweepStep = 1;
            StartLaserSweep(mgr, sourceCopy);
          } else {
            mLaserSweepStep = -1;
          }
        } else {
          mFacingDirection =
              CVector3f::Slerp(forward, flat.AsNormalized(), CRelAngle::FromDegrees(turnSpeed));
          mLaserSweepDirection =
              CVector3f::Slerp(mLaserSweepDirection.AsNormalized(), toSource.AsNormalized(),
                               CRelAngle::FromDegrees(turnSpeed));
        }
      } else {
        mLaserSweepStep = -1;
      }
    }
    break;
  case kStateMsg_Deactivate:
    StopLaserSweep(mgr);
    mLaserSweepStep = -1;
    break;
  }
}

void CSplitterCommandModule::LostChassisReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mLevelingOut = true;
    break;
  case kStateMsg_Update:
    LevelOut(dt);
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Two, -1, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterCommandModule::SeekMainChassis(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mShieldsRaised = false;
    break;
  case kStateMsg_Update:
    if (const CSplitterMainChassis* chassis =
            TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mDockingTargetId))) {
      const CVector3f dockPosition = chassis->GetAttachPosition();
      const CVector3f delta = dockPosition - GetTranslation();
      const float step = dt * mData.maxLinearVelocity;
      if (delta.MagSquared() > step * step && delta.IsMagnitudeSafe()) {
        const CVector3f movement = step * delta.AsNormalized();
        MoveTowards(GetTranslation() + movement, dt);
      } else {
        SetTranslation(dockPosition);
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterCommandModule::FollowDockingPath(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate:
    const TUniqueId pathId = GetConnectedObject(mgr, kSS_Connect, kSM_Follow);
    mWaypointNavigation.SetDestination(pathId);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update: {
    const CVector3f moveVector = BodyController()->CommandMgr().GetMoveVector();
    const float step = dt * mData.maxLinearVelocity;
    if (moveVector.IsMagnitudeSafe()) {
      const CVector3f movement = step * moveVector;
      MoveTowards(GetTranslation() + movement, dt);
      BodyController()->FaceDirection(moveVector, dt);
    }
    break;
  }
  default:
    break;
  }
}

void CSplitterCommandModule::NotifyDocking(CStateManager& mgr, float dt) {
  if (CSplitterMainChassis* chassis =
          TCastToPtr< CSplitterMainChassis >(mgr.ObjectById(mDockingTargetId))) {
    if (chassis->DockCommandModule(mgr, GetUniqueId())) {
      mMainChassisId = mDockingTargetId;
      mDockingTargetId = kInvalidUniqueId;
      SetNextDrawNode(mMainChassisId);
      if (!mFirstDocking || !(mData.unknown_0xbd80fd94 & 1)) {
        CSfxManager::AddEmitter(mData.sound_Docking, GetTranslation(), 127,
                                GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
      }
      mFirstDocking = false;
    }
  }
  BodyController()->SetLocomotionType(pas::kLT_Relaxed);
}

void CSplitterCommandModule::SelectTarget(CStateManager& mgr, float dt) {
  if (mTargetId == kInvalidUniqueId) {
    mTargetId = mgr.GetPlayer(0)->GetUniqueId();
  }
  UpdateEffects(mgr);
  SendScriptMsgs(kSS_Attack, mgr, GetUniqueId(), kSM_None);
}

void CSplitterCommandModule::SetTargetDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mHasDestination = false;
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    destination = target->GetAimPosition(mgr, 0.f) + 4.f * CVector3f::Up();
    mHasDestination = true;
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.SetFaceTarget(mTargetId);
}

void CSplitterCommandModule::SetDockingDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mHasDestination = false;
  if (CSplitterMainChassis* chassis =
          TCastToPtr< CSplitterMainChassis >(mgr.ObjectById(mDockingTargetId))) {
    destination = chassis->GetAttachPosition() + 4.f * CVector3f::Up();
    mHasDestination = true;
    chassis->SetHitByPlayerProjectile(true);
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
}

void CSplitterCommandModule::FindBestDodgeDirection(CStateManager& mgr, float dt) {
  mDodgeDirection = pas::kSD_Invalid;
  if ((mData.unknown_0xbd80fd94 & 2) && mDodgeCount < mData.maxDodges) {
    if (mDodgeCount < mData.minDodges || mgr.Random()->Range(0.f, 100.f) <= mData.dodgeChance) {
      mDodgeDirection = ChooseDodgeDirection(mgr);
      ++mDodgeCount;
    } else {
      mDodgeCount = mData.maxDodges;
    }
  }
}

void CSplitterCommandModule::RaiseShields(CStateManager& mgr, float dt) { mShieldsRaised = true; }

void CSplitterCommandModule::ResetAttackTimes(CStateManager& mgr, float dt) {
  mAttackTimer = mData.minLaserPulseAttackTime;
  mShotsRemaining = mgr.Random()->Range(1, mData.maxLaserPulseShots);
}

void CSplitterCommandModule::FindDockingTargetFromConnection(CStateManager& mgr) {
  mDockingTargetId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
}

void CSplitterCommandModule::CreateCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(1);
  CAnimData* animData = AnimationData();
  const CSegId segId = animData->GetLocatorSegId(rstl::string_l(skShieldSphere.mName));
  const CJointCollisionDescription description =
      CJointCollisionDescription::SphereCollision(segId, CVector3f::Zero(), skShieldSphere.mRadius,
                                                  rstl::string_l(skShieldSphere.mName), 1000.f);
  joints.push_back_unsafe(description);
  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mCollisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      if (desc.GetName() == rstl::string_l(skShieldActorName)) {
        mShieldCollisionActorId = id;
      }
    }
  }
  UpdateShieldCollision(mgr, false);
}

void CSplitterCommandModule::UpdateTimers(float dt, CStateManager& mgr) {
  mShieldTimer -= dt;
  mAttackTimer -= dt;
  mDockSearchTimer -= dt;
  mTimeInState += dt;
  mTimeSinceChassisShield += dt;
  if (mFlashTimer > 0.f) {
    mFlashTimer = CMath::Max(0.f, mFlashTimer - dt);
    const float t = CMath::Min(1.f, mFlashTimer);
    mFlashColor = CColor::Lerp(CColor::White(), mDamageColor, t);
  }
  if (mAutoDestructing && mAlive) {
    mSelfDestructTimer -= dt;
    if (mSelfDestructTimer <= 0.f) {
      CPatterned::Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
    }
  }
}

void CSplitterCommandModule::UpdateChassisLink(CStateManager& mgr) {
  if (mMainChassisId != kInvalidUniqueId) {
    const CSplitterMainChassis* chassis =
        TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mMainChassisId));
    if (chassis && chassis->GetCommandModuleId() == GetUniqueId()) {
      if (mTargetId != chassis->GetTargetId()) {
        mTargetId = chassis->GetTargetId();
        UpdateEffects(mgr);
      }
      mDockSearchTimer = 3.f;
    } else {
      mMainChassisId = kInvalidUniqueId;
      SetNextDrawNode(kInvalidUniqueId);
    }
  } else if ((mData.unknown_0xbd80fd94 & 8) && mDockSearchTimer <= 0.f &&
             mDockingTargetId == kInvalidUniqueId) {
    FindDockingTarget(mgr);
    mDockSearchTimer = 3.f;
  } else if (mDockingTargetId != kInvalidUniqueId) {
    const CSplitterMainChassis* chassis =
        TCastToConstPtr< CSplitterMainChassis >(mgr.GetObjectById(mDockingTargetId));
    if (!chassis || !chassis->GetAlive()) {
      mDockingTargetId = kInvalidUniqueId;
    }
  }
}

void CSplitterCommandModule::UpdateShield(CStateManager& mgr) {
  if (CParticleGenInfo* effect = GetShieldEffect()) {
    effect->SetModulationColor(mFlashColor);
  }
  CSfxManager::UpdateEmitter(mShieldSfx, GetTranslation(), GetTransform().GetForward(), 127);

  CCollisionActor* collisionActor =
      TCastToPtr< CCollisionActor >(mgr.ObjectById(mShieldCollisionActorId));
  if (!collisionActor) {
    return;
  }
  if (mCollisionActorManager.get()) {
    mCollisionActorManager->SetActive(mgr, mMainChassisId == kInvalidUniqueId);
  }
  switch (mShieldType) {
  case kST_Light:
  case kST_Dark:
    if (!mShieldsRaised || collisionActor->HealthInfo()->GetHP() <= 0.f) {
      mShieldType = kST_None;
      mShieldTimer = mData.resetShieldTime;
      UpdateShieldCollision(mgr, false);
    }
    break;
  case kST_None:
  default:
    if (mShieldsRaised && mShieldTimer <= 0.f) {
      mShieldType = mLastShieldType == kST_Light ? kST_Dark : kST_Light;
      mLastShieldType = mShieldType;
      UpdateShieldCollision(mgr, true);
    }
    break;
  }
}

void CSplitterCommandModule::UpdateShieldCollision(CStateManager& mgr, bool playSound) {
  CAnimData* animData = AnimationData();
  if (playSound) {
    CSfxManager::AddEmitter(mData.sound_ShieldOn, GetTranslation(), 127, GetCurrentAreaId().Value(),
                            true, false, CSfxManager::kMedPriority);
  }
  if (mShieldSfx) {
    CSfxManager::RemoveEmitter(mShieldSfx);
    mShieldSfx = CSfxHandle();
  }

  CCollisionActor* collisionActor =
      TCastToPtr< CCollisionActor >(mgr.ObjectById(mShieldCollisionActorId));
  if (!collisionActor) {
    return;
  }
  collisionActor->HealthInfo()->SetHP(mData.shieldHP);
  switch (mShieldType) {
  case kST_Light:
    collisionActor->AddMaterial(kMT_Character, mgr);
    AddMaterial(kMT_Unknown54, mgr);
    collisionActor->SetDamageVulnerability(mData.lightShieldVulnerabilityData);
    animData->SetEffectState(rstl::string_l(skLightShieldEffect), true, mgr);
    animData->SetEffectState(rstl::string_l(skDarkShieldEffect), false, mgr);
    mShieldSfx =
        CSfxManager::AddEmitter(mData.sound_LightShield, GetTranslation(), 127,
                                GetCurrentAreaId().Value(), true, true, CSfxManager::kMedPriority);
    break;
  case kST_Dark:
    collisionActor->RemoveMaterial(kMT_Character, mgr);
    AddMaterial(kMT_Unknown54, mgr);
    collisionActor->SetDamageVulnerability(mData.darkShieldVulnerabilityData);
    animData->SetEffectState(rstl::string_l(skLightShieldEffect), false, mgr);
    animData->SetEffectState(rstl::string_l(skDarkShieldEffect), true, mgr);
    mShieldSfx =
        CSfxManager::AddEmitter(mData.sound_DarkShield, GetTranslation(), 127,
                                GetCurrentAreaId().Value(), true, true, CSfxManager::kMedPriority);
    break;
  default:
    collisionActor->RemoveMaterial(kMT_Character, mgr);
    RemoveMaterial(kMT_Unknown54, mgr);
    collisionActor->SetDamageVulnerability(CDamageVulnerability::PassThroughVulnerabilty());
    animData->SetEffectState(rstl::string_l(skLightShieldEffect), false, mgr);
    animData->SetEffectState(rstl::string_l(skDarkShieldEffect), false, mgr);
    break;
  }
}

void CSplitterCommandModule::OnShieldHit(TUniqueId id) {
  if (id == mShieldCollisionActorId) {
    mFlashTimer = 1.f;
  }
}

CParticleGenInfo* CSplitterCommandModule::GetShieldEffect() {
  if (mShieldType != kST_None) {
    const rstl::string name = mShieldType == kST_Light ? rstl::string_l(skLightShieldEffect)
                                                       : rstl::string_l(skDarkShieldEffect);
    return AnimationData()->GetFirstParticleEffect(name);
  }
  return nullptr;
}

void CSplitterCommandModule::LevelOut(float dt) {
  if (!mLevelingOut) {
    return;
  }

  const CVector3f position = GetTranslation();
  const float turn = 2.f * M_PIF * dt;
  const CVector3f up = GetTransform().GetUp();
  const CVector3f worldUp = CVector3f::Up();
  const CVector3f forward = GetTransform().GetForward();
  if (CVector3f::GetAngleDiff(worldUp, up) <= turn) {
    bool hasHeading = true;
    if (forward.GetX() == 0.f && forward.GetY() == 0.f) {
      hasHeading = false;
    }
    if (hasHeading) {
      SetTransform(CTransform4f::LookAt(
          position, position + CVector3f(forward.GetX(), forward.GetY(), 0.f), CVector3f::Up()));
      mLevelingOut = false;
    }
  } else {
    const CVector3f target =
        CVector3f::Slerp(worldUp.AsNormalized(), up.AsNormalized(), CRelAngle::FromRadians(turn));
    const CQuaternion arc = CQuaternion::ShortestRotationArc(up, target);
    const CQuaternion rotation = CQuaternion::FromMatrix(GetTransform()) * arc;
    SetTransform(rotation.BuildTransform4f(position));
  }
}

pas::EStepDirection CSplitterCommandModule::ChooseDodgeDirection(CStateManager& mgr) {
  const float stepDistanceSquared = mStepDistance * mStepDistance;
  const CVector3f position = GetTranslation();
  const CVector3f right = GetTransform().GetRight();
  bool leftFree = true;
  bool rightFree = true;
  pas::EStepDirection direction = pas::kSD_Invalid;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CActor* actor = static_cast< const CActor* >(list[i])) {
      if (actor != this && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f delta = actor->GetTranslation() - position;
        if (delta.MagSquared() < stepDistanceSquared) {
          if (CVector3f::Dot(delta, right) >= 0.f) {
            if (rightFree && CVector3f::GetAngleDiff(right, delta) < (M_PIF / 3.f)) {
              rightFree = false;
            }
          } else if (leftFree && CVector3f::GetAngleDiff(-right, delta) < (M_PIF / 3.f)) {
            leftFree = false;
          }
        }
      }
    }
  }
  if (rightFree) {
    rightFree = IsPathClear(mgr, right, mStepDistance);
  }
  if (leftFree) {
    leftFree = IsPathClear(mgr, -right, mStepDistance);
  }
  if (leftFree && rightFree) {
    if (mgr.Random()->Next() & 0x4000) {
      leftFree = false;
    } else {
      rightFree = false;
    }
  }

  if (leftFree) {
    direction = pas::kSD_Left;
  } else if (rightFree) {
    direction = pas::kSD_Right;
  }
  return direction;
}

bool CSplitterCommandModule::IsPathClear(CStateManager& mgr, const CVector3f& direction,
                                         float distance) {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  const CVector3f end = center + distance * direction;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skPathSolidMaterial), CMaterialList(skPathCollisionActorMaterial));
  if (mgr.RayCollideWorld(center, end, filter, this) &&
      mPathFindSearch.OnPath(end) == CPathFindSearch::kR_Success) {
    return true;
  }
  return false;
}

CVector3f CSplitterCommandModule::GetSeparationVector(CStateManager& mgr) {
  CVector3f separation = CVector3f::Zero();
  float count = 0.f;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* other = TCastToConstPtr< CPatterned >(list[i])) {
      if (other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CSplitterCommandModule* module = TCastToConstPtr< CSplitterCommandModule >(other);
        if (!module || module->mMainChassisId == kInvalidUniqueId) {
          const CVector3f force = mSteeringBehaviors.Separation(
              *this, other->GetTranslation(), 5.f * other->GetModelData()->GetScale().GetX());
          if (force.IsMagnitudeSafe()) {
            separation += force.AsNormalized();
            count += 1.f;
          }
        }
      }
    }
  }
  if (count > 0.f) {
    separation *= 1.f / count;
  }
  return separation;
}

void CSplitterCommandModule::FindDockingTarget(CStateManager& mgr) {
  const CVector3f position = GetTranslation();
  float bestDistance = FLT_MAX;
  CObjectList& list = mgr.ObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CSplitterMainChassis* chassis = TCastToPtr< CSplitterMainChassis >(list[i])) {
      if (chassis->GetCurrentAreaId() == GetCurrentAreaId() &&
          chassis->RequestDocking(GetUniqueId())) {
        const CVector3f delta = chassis->GetTranslation() - position;
        const float distance = delta.MagSquared();
        if (distance < bestDistance) {
          if (CSplitterMainChassis* previous =
                  TCastToPtr< CSplitterMainChassis >(mgr.ObjectById(mDockingTargetId))) {
            previous->CancelDocking(GetUniqueId());
          }
          bestDistance = distance;
          mDockingTargetId = chassis->GetUniqueId();
        }
      }
    }
  }
}

void CSplitterCommandModule::UpdateCollisionTimer(float dt, CStateManager& mgr) {
  if (mCollided) {
    mCollisionTime += dt;
    if (mCollisionTime >= 5.f) {
      AutoDestruct(0.f);
    }
    mCollided = false;
  } else {
    mCollisionTime = 0.f;
  }
}

void CSplitterCommandModule::MoveTowards(const CVector3f& position, float dt) {
  if (mCollisionTime > 0.f) {
    SetTranslation(position);
  } else {
    const CVector3f delta = position - GetTranslation();
    MoveInOneFrameOR(GetTransform().TransposeRotate(delta), dt);
  }
}

void CSplitterCommandModule::FireLaserPulse(CStateManager& mgr, const rstl::string& locatorName) {
  if (!mAlive) {
    return;
  }
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (!target) {
    return;
  }
  const CTransform4f locatorXf = GetLctrTransform(locatorName);
  const CVector3f origin = locatorXf.GetTranslation();
  const CVector3f delta = target->GetAimPosition(mgr, 0.f) - origin;
  const CVector3f forward = GetTransform().GetForward();
  const CVector3f projected = origin + CVector3f::Dot(forward, delta) * forward;
  const CVector3f aimPoint(projected.GetX(), projected.GetY(), origin.GetZ() + delta.GetZ());
  const CTransform4f xf = CTransform4f::LookAt(origin, aimPoint, CVector3f::Up());
  LaunchProjectile(xf, mgr, 6, 0, false, CImpactVisorEffect::None(), CVector3f(1.f, 1.f, 1.f));
}

void CSplitterCommandModule::StartLaserSweep(CStateManager& mgr, const CVector3f& target) {
  const CBeamInfo beamInfo = TLdrToBeamInfo(mData.laserSweepBeamInfo, 0x91);
  const TUniqueId uid = mgr.AllocateUniqueId();
  CPlasmaProjectile* laser = rs_new CPlasmaProjectile(
      mLaserSweepProjectileInfo.Token(), rstl::string_l("LaserSweepBeam"), kWT_Light, beamInfo,
      CTransform4f::Identity(), kMT_ProjectilePassthrough, mData.laserSweepDamageInfo, uid,
      GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(), false,
      CWeapon::kPA_KeepInCinematic | CWeapon::kPA_BigStrike);
  if (laser) {
    const CTransform4f beamXf = GetLctrTransform(mBeamLocator);
    const CVector3f beamPosition = beamXf.GetTranslation();
    const CTransform4f xf = CTransform4f::LookAt(beamPosition, target, CVector3f::Up());
    laser->Fire(xf, mgr, false);
    laser->SetNextDrawNode(mMainChassisId);
    mgr.AddObject(*laser);
    const int areaId = GetCurrentAreaId().Value();
    CAudioSys::C3DEmitterParmData emitter(150.f, 0.1f, 1, 127, 35);
    emitter.mPos = laser->GetCurrentPos();
    emitter.mDir = CVector3f::Up();
    emitter.mSfxId = mData.sound_LaserSweep;
    mLaserSweepSfx =
        CSfxManager::AddEmitter(emitter, areaId, true, true, CSfxManager::kMedPriority);
    mLaserSweepProjectileId = uid;
  }
  UpdateEffects(mgr);
}

void CSplitterCommandModule::UpdateLaserSweep(CStateManager& mgr, float dt) {
  if (CPlasmaProjectile* laser =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mLaserSweepProjectileId))) {
    const CTransform4f beamXf = GetLctrTransform(mBeamLocator);
    const CVector3f beamPosition = beamXf.GetTranslation();
    const CTransform4f xf =
        CTransform4f::LookAt(beamPosition, beamPosition + mLaserSweepDirection, CVector3f::Up());
    laser->UpdateFx(xf, dt, mgr);
    CSfxManager::UpdateEmitter(mLaserSweepSfx, laser->GetCurrentPos(), CVector3f::Up(), 127);
  }
}

void CSplitterCommandModule::StopLaserSweep(CStateManager& mgr) {
  if (CPlasmaProjectile* laser =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mLaserSweepProjectileId))) {
    laser->ResetBeam(mgr, false);
    CSfxManager::RemoveEmitter(mLaserSweepSfx);
    mgr.DeleteObjectRequest(mLaserSweepProjectileId);
  }
  mLaserSweepProjectileId = kInvalidUniqueId;
  mLaserSweepSfx = CSfxHandle();
  if (mAlive) {
    UpdateEffects(mgr);
  }
}

void CSplitterCommandModule::UpdateEffects(CStateManager& mgr) {
  CAnimData* animData = AnimationData();
  if (mVulnerabilityState == kVS_ChassisShield || mVulnerabilityState == kVS_Reflective) {
    animData->SetEffectState(rstl::string_l(skAlertEyeEffect), false, mgr);
    animData->SetEffectState(rstl::string_l(skLaserMuzzleEffect), false, mgr);
  } else if (mLaserSweepProjectileId != kInvalidUniqueId) {
    animData->SetEffectState(rstl::string_l(skAlertEyeEffect), false, mgr);
    animData->SetEffectState(rstl::string_l(skLaserMuzzleEffect), true, mgr);
  } else if (mTargetId != kInvalidUniqueId || IsScanning(mgr, CTriggerData(0.f))) {
    animData->SetEffectState(rstl::string_l(skAlertEyeEffect), true, mgr);
    animData->SetEffectState(rstl::string_l(skLaserMuzzleEffect), false, mgr);
  } else {
    animData->SetEffectState(rstl::string_l(skAlertEyeEffect), false, mgr);
    animData->SetEffectState(rstl::string_l(skLaserMuzzleEffect), false, mgr);
  }
}

void CSplitterCommandModule::UpdateScanBeam(CStateManager& mgr, float dt) {
  CSplitterBeamEffect* effect =
      static_cast< CSplitterBeamEffect* >(mgr.ObjectById(mScanBeamEffectId));
  const CTransform4f xf = GetScanBeamTransform();
  if (effect) {
    effect->SetTransform(xf);
  }

  if (IsScanning(mgr, CTriggerData(0.f))) {
    if (!effect && mDetectionRange > 0.f) {
      mScanBeamEffectId = mgr.AllocateUniqueId();
      effect = rs_new CSplitterBeamEffect(
          mScanBeamEffectId,
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("Splitter Beam Effect"), xf, 0x40,
          CAbsAngle::FromDegrees(mData.unknown_0x9ec51fe4.angle), mDetectionRange,
          mData.unknown_0x9ec51fe4.cloudColor1, mData.unknown_0x9ec51fe4.cloudColor2,
          mData.unknown_0x9ec51fe4.addColor1, mData.unknown_0x9ec51fe4.addColor2,
          mData.unknown_0x9ec51fe4.cloudScale, mData.unknown_0x9ec51fe4.fadeOffSize,
          mData.unknown_0x9ec51fe4.openSpeed);
      mgr.AddObject(*effect);
      mScanBeamSfx = CSfxManager::AddEmitter(mData.sound_Scanning, GetTranslation(), 127,
                                             GetCurrentAreaId().Value(), true, true,
                                             CSfxManager::kMedPriority);
    }
    if (effect) {
      effect->SetOpening(true);
    }
    CSfxManager::UpdateEmitter(mScanBeamSfx, GetTranslation(), GetTransform().GetForward(), 127);
  } else if (effect) {
    effect->SetOpening(false);
    if (effect->IsClosed()) {
      mgr.DeleteObjectRequest(mScanBeamEffectId);
      mScanBeamEffectId = kInvalidUniqueId;
    }
  } else {
    mScanBeamEffectId = kInvalidUniqueId;
    CSfxManager::RemoveEmitter(mScanBeamSfx);
    mScanBeamSfx = CSfxHandle();
  }
}

CTransform4f CSplitterCommandModule::GetScanBeamTransform() const {
  CTransform4f xf = GetTransform();
  xf.SetTranslation(xf * GetLocatorTransform(rstl::string_l("Beam_LCTR")).GetTranslation());
  return xf;
}

CEntity* REL_LoadSplitterCommandModule(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSplitterCommandModule sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSplitterCommandModule.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const SSplitterCommandModuleData data(sldrThis.commandModuleProperties);
  return rs_new CSplitterCommandModule(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, &sldrThis.commandModuleProperties.ingPossessionData),
      data);
}
