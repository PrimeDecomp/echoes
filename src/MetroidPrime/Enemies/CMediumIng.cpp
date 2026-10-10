#include "MetroidPrime/Enemies/CMediumIng.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMediumIng.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"PathShagged", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::PathShagged)},
    {"IsOffPath", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::IsOffPath)},
    {"Leash", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::Leash)},
    {"IsMisting", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::IsMisting)},
    {"IsAggressive",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::IsAggressive)},
    {"ShouldGenerate",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldGenerate)},
    {"ShouldArmAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldArmAttack)},
    {"ShouldMeleeAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldMeleeAttack)},
    {"ShouldMistAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldMistAttack)},
    {"ShouldTaunt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldTaunt)},
    {"ShouldDoubleDash",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldDoubleDash)},
    {"ShouldBackUp",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldBackUp)},
    {"HasLineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::HasLineOfSight)},
    {"HasJumpPoint",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::HasJumpPoint)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::HasAttackPattern)},
    {"HasApproachPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::HasApproachPattern)},
    {"PatternOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::PatternOver)},
    {"IsDestInsideCurrentRegion",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::IsDestInsideCurrentRegion)},
    {"IsFrustated", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::IsFrustated)},
    {"ShouldDoSafezoneReaction",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldDoSafezoneReaction)},
    {"ShouldEvaporate",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::ShouldEvaporate)},
    {"IsLuredBySafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::IsLuredBySafeZone)},
    {"StillLuredBySafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMediumIng::StillLuredBySafeZone)},
    {"InPosition", static_cast< CPatterned::StateMachine::TriggerFunc >(&CPatterned::InPosition)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"SubStart", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::SubStart)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::PathFind)},
    {"FollowAttackPattern",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::FollowAttackPattern)},
    {"FollowApproachPattern",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::FollowApproachPattern)},
    {"MistIn", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::MistIn)},
    {"MistOut", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::MistOut)},
    {"ArmAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::ArmAttack)},
    {"MeleeAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::MeleeAttack)},
    {"MistAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::MistAttack)},
    {"Jump", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::Jump)},
    {"Dash", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::Dash)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::Taunt)},
    {"WaitForLocomotion",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::WaitForLocomotion)},
    {"Idle", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::Idle)},
    {"SafezoneReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::SafezoneReaction)},
    {"MoveToSafeZone",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::MoveToSafeZone)},
    {"BackUp", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::BackUp)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::Dead)},
    {"SelectTargetState",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMediumIng::SelectTargetState)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SelectTarget", static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::SelectTarget)},
    {"SetLeashTarget",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::SetLeashTarget)},
    {"SetRetreatDestination",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::SetRetreatDestination)},
    {"SetMeshPathDestination",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::SetMeshPathDestination)},
    {"SetDashDestination",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::SetDashDestination)},
    {"ResetFrustatedCounter",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::ResetFrustatedCounter)},
    {"IncrementFrustatedCounter",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMediumIng::IncrementFrustatedCounter)},
};

CMediumIngData::CMediumIngData(
    const CDamageVulnerability& mistingVulnerability, CAssetId tentacleModel,
    int tentacleCharacterIndex, const CActorParameters& tentacleActorParameters, int spawnMode,
    float aggressiveness, const CColor& lightColor, float lightAttenuation,
    const CDamageInfo& mistDamage, float maxMistAttackRange, const CDamageInfo& meleeDamage,
    float maxMeleeAttackRange, float minArmAttackRange, float maxArmAttackRange,
    const CMayaSpline& attackMotion, const CCameraShakerData& attackTentacleImpact,
    const CDamageInfo& attackTentacleDamage, float noMistDamageThreshold, float tauntChance,
    const CMayaSpline& unknownSpline, const CMayaSpline& dashSpeed, CAssetId ingSpotBlobFx,
    float doubleDashChance, float minMistAttackInterval, float minArmAttackInterval,
    float minTentacleLength, float maxTentacleLength, float armAttackTime, float unknown8f1d597c,
    float minMeleeAttackInterval, float unknown0e3d3708, ushort ingSpotSound)
: mMistingVulnerability(mistingVulnerability)
, mTentacleModel(tentacleModel)
, mTentacleCharacterIndex(tentacleCharacterIndex)
, mTentacleActorParameters(tentacleActorParameters)
, mSpawnMode(spawnMode)
, mAggressiveness(aggressiveness)
, mLightColor(lightColor)
, mLightAttenuation(lightAttenuation)
, mIngSpotBlobFx(ingSpotBlobFx)
, mIngSpotSound(ingSpotSound)
, mMaxMistAttackRange(maxMistAttackRange)
, mMistDamage(mistDamage)
, mMaxMeleeAttackRange(maxMeleeAttackRange)
, mMeleeDamage(meleeDamage)
, mMinArmAttackRange(minArmAttackRange)
, mMaxArmAttackRange(maxArmAttackRange)
, mAttackMotion(attackMotion)
, mAttackTentacleImpact(attackTentacleImpact)
, mAttackTentacleDamage(attackTentacleDamage)
, mUnknownSpline(unknownSpline)
, mDashSpeed(dashSpeed)
, mNoMistDamageThreshold(noMistDamageThreshold)
, mTauntChance(tauntChance)
, mDoubleDashChance(doubleDashChance)
, mMinMistAttackInterval(minMistAttackInterval)
, mMinArmAttackInterval(minArmAttackInterval)
, mMinTentacleLength(minTentacleLength)
, mMaxTentacleLength(maxTentacleLength)
, mArmAttackTime(armAttackTime)
, mUnknown8f1d597c(unknown8f1d597c)
, mMinMeleeAttackInterval(minMeleeAttackInterval)
, mSafeZoneRangeSq(unknown0e3d3708 * unknown0e3d3708) {}

CMediumIngData::CMediumIngData(const CMediumIngData& other)
: mMistingVulnerability(other.mMistingVulnerability)
, mTentacleModel(other.mTentacleModel)
, mTentacleCharacterIndex(other.mTentacleCharacterIndex)
, mTentacleActorParameters(other.mTentacleActorParameters)
, mSpawnMode(other.mSpawnMode)
, mAggressiveness(other.mAggressiveness)
, mLightColor(other.mLightColor)
, mLightAttenuation(other.mLightAttenuation)
, mIngSpotBlobFx(other.mIngSpotBlobFx)
, mIngSpotSound(other.mIngSpotSound)
, mMaxMistAttackRange(other.mMaxMistAttackRange)
, mMistDamage(other.mMistDamage)
, mMaxMeleeAttackRange(other.mMaxMeleeAttackRange)
, mMeleeDamage(other.mMeleeDamage)
, mMinArmAttackRange(other.mMinArmAttackRange)
, mMaxArmAttackRange(other.mMaxArmAttackRange)
, mAttackMotion(other.mAttackMotion)
, mAttackTentacleImpact(other.mAttackTentacleImpact)
, mAttackTentacleDamage(other.mAttackTentacleDamage)
, mUnknownSpline(other.mUnknownSpline)
, mDashSpeed(other.mDashSpeed)
, mNoMistDamageThreshold(other.mNoMistDamageThreshold)
, mTauntChance(other.mTauntChance)
, mDoubleDashChance(other.mDoubleDashChance)
, mMinMistAttackInterval(other.mMinMistAttackInterval)
, mMinArmAttackInterval(other.mMinArmAttackInterval)
, mMinTentacleLength(other.mMinTentacleLength)
, mMaxTentacleLength(other.mMaxTentacleLength)
, mArmAttackTime(other.mArmAttackTime)
, mUnknown8f1d597c(other.mUnknown8f1d597c)
, mMinMeleeAttackInterval(other.mMinMeleeAttackInterval)
, mSafeZoneRangeSq(other.mSafeZoneRangeSq) {}

CMediumIngTentacle::CMediumIngTentacle(TUniqueId uid, TAreaId areaId, const rstl::string& name,
                                       const CModelData& modelData, const CMediumIngData& data)
: CActor(uid, name, CEntityInfo(areaId, rstl::vector< SConnection >(), false, kInvalidEditorId), 0,
         CTransform4f::Identity(), modelData, CMaterialList(), data.mTentacleActorParameters,
         kInvalidUniqueId)
, mSpline(false, 1.f, CMotionSpline::kST_Linear)
, mAttackMotion(data.mAttackMotion)
, mImpactShake(data.mAttackTentacleImpact)
, mDamage(data.mAttackTentacleDamage)
, mTipBounds()
, mSplinePoints()
, mExtending(false)
, mRetracting(false)
, mElapsedTime(0.f)
, mRetractTime(0.f)
, mLengthScale(1.f)
, mAttackTime(data.mArmAttackTime) {
  SetDrawShadow(false);
}

CMediumIng::CMediumIng(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& modelData,
                       const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                       const CMediumIngData& data)
: CPatterned(kPAI_MediumIng, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_Zero, kBT_AiMovedFlyer, actorParams)
, mPathFindSearch(nullptr, 0x30b, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mLineOfSightTracker(GetUniqueId(), CSegId(0x63), 0.1f, 0.05f)
, mSteering()
, mSurfaceAlignment()
, mData(data)
, mJumpPointId(kInvalidUniqueId)
, mTeamAiMgrId(kInvalidUniqueId)
, mSafeZoneId(kInvalidUniqueId)
, mMistState(0)
, mMistAmount(0.f)
, mF60(0.f)
, mF64(0.f)
, mAggressionTimer(0.f)
, mI6C(0)
, mI74(0)
, mV78(CVector3f::Zero())
, mV84(CVector3f::Zero())
, mV90(CVector3f::Zero())
, mDamageSinceMist(0.f)
, mTentacleIndex(0)
, mFA4(0.f)
, mFA8(1.f)
, mFAC(0.f)
, mFB0(0.f)
, mFB4(1.f)
, mFB8(mData.mMinArmAttackInterval)
, mFBC(mData.mMinMeleeAttackInterval)
, mFC0(mData.mMinMistAttackInterval)
, mFC4(1.f)
, mFC8(0.f)
, mFCC(0.f)
, mFD0(1.f)
, mTentacleIds()
, mLightId(kInvalidUniqueId)
, mMistEffectId(kInvalidUniqueId)
, mSfx0(0)
, mSfx1(0)
, mSfx2(0)
, mTauntAnim(4)
, mSafeZoneReactionCount(0)
, mC0(false)
, mC1(true)
, mC2(false)
, mC3(true)
, mC4(false)
, mC5(false)
, mC6(false)
, mC7(false)
, mD0(false)
, mD1(false)
, mD2(false)
, mD3(false)
, mD4(false)
, mD5(false)
, mD6(false)
, mD7(false)
, mE1(false)
, mE2(false) {
  mPathFindSearch.SetCharacterRadius(2.f);
  mPathFindSearch.SetCharacterHeight(5.f);
  mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
  SetDrawShadow(false);
}

void CMediumIng::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

CEntity* LoadMediumIng(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrMediumIng sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrMediumIng.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CMediumIngData data(
      LdrToDamageVulnerability(sldrThis.mistingVulnerability), sldrThis.attackTentacle.ancs,
      sldrThis.attackTentacle.character_index,
      LdrToActorParameters(sldrThis.attackTentacleActorInformation), sldrThis.spawnMode,
      sldrThis.aggressiveness, sldrThis.lightColor, sldrThis.lightAttenuation,
      LdrToDamageInfo(sldrThis.mistDamage), sldrThis.maxMistAttackRange,
      LdrToDamageInfo(sldrThis.meleeDamage), sldrThis.maxMeleeAttackRange,
      sldrThis.minArmAttackRange, sldrThis.maxArmAttackRange, sldrThis.attackMotion,
      LdrToCameraShakerData(sldrThis.attackTentacleImpact, CVector3f::Zero()),
      LdrToDamageInfo(sldrThis.attackTentacleDamage), sldrThis.noMistDamageThreshold,
      sldrThis.tauntChance, sldrThis.unknown_0xb459c3e9, sldrThis.dashSpeed, sldrThis.ingSpotBlobFx,
      sldrThis.doubleDashChance, sldrThis.minMistAttackInterval, sldrThis.minArmAttackInterval,
      sldrThis.minTentacleLength, sldrThis.maxTentacleLength, sldrThis.armAttackTime,
      sldrThis.unknown_0x8f1d597c, sldrThis.minMeleeAttackInterval, sldrThis.unknown_0x0e3d3708,
      static_cast< ushort >(sldrThis.ingSpotSound));

  return rs_new CMediumIng(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                           LdrToEntityInfo(info, sldrThis.editorProperties),
                           LdrToTransform4f(sldrThis.editorProperties), *modelData,
                           LdrToActorParameters(sldrThis.actorInformation),
                           LdrToPatternedInfo(sldrThis.patterned, nullptr), data);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SMediumIng_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadMediumIng;
  SetSMediumIng_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSMediumIng_FuncPtrs(nullptr); }
#endif

void CMediumIng::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CMediumIng::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
    if (mTeamAiMgrId != kInvalidUniqueId) {
      if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
        team->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                       CTeamAiRole::kTAR_Unknown);
      }
    }
  }
}

bool CMediumIng::ShouldBackUp(CStateManager& mgr, const CTriggerData& data) const {
  return (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared() < 25.f;
}

bool CMediumIng::ShouldDoubleDash(CStateManager& mgr, const CTriggerData& data) const {
  return mData.mDoubleDashChance >= mgr.Random()->Range(0.f, 100.f);
}

bool CMediumIng::ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const {
  return mData.mTauntChance >= mgr.Random()->Range(0.f, 100.f);
}

bool CMediumIng::PatternOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CMediumIng::HasApproachPattern(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (!mD0) {
    if (GetConnectedObject(mgr, kSS_Approach, kSM_Follow) != kInvalidUniqueId) {
      result = true;
    }
  }
  return result;
}

bool CMediumIng::HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (!mD0) {
    if (GetConnectedObject(mgr, kSS_GeneratorConnection, kSM_Follow) != kInvalidUniqueId) {
      result = true;
    }
  }
  return result;
}

bool CMediumIng::CanBeShot(const CStateManager& mgr, int type) { return !mC1; }

void CMediumIng::IncrementFrustatedCounter(CStateManager& mgr, float dt) { ++mFrustratedCounter; }

void CMediumIng::ResetFrustatedCounter(CStateManager& mgr, float dt) { mFrustratedCounter = 0; }

bool CMediumIng::StillLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const {
  return mSafeZoneId != kInvalidUniqueId;
}

bool CMediumIng::IsLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (!mC1 && !mC0 && mSafeZoneId != kInvalidUniqueId) {
    result = true;
  }
  return result;
}

bool CMediumIng::ShouldEvaporate(CStateManager& mgr, const CTriggerData& data) const { return mE1; }

bool CMediumIng::ShouldDoSafezoneReaction(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (!HasMist() && mSafeZoneReactionCount > 0) {
    result = true;
  }
  return result;
}

bool CMediumIng::IsFrustated(CStateManager& mgr, const CTriggerData& data) const {
  return static_cast< float >(mFrustratedCounter) > data.GetFloat();
}

bool CMediumIng::IsOffPath(CStateManager& mgr, const CTriggerData& data) const {
  return GetSearchPath()->OnPath(GetTranslation()) != CPathFindSearch::kR_Success;
}

bool CMediumIng::HasJumpPoint(CStateManager& mgr, const CTriggerData& data) const {
  return mJumpPointId != kInvalidUniqueId;
}

bool CMediumIng::HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return mLineOfSightTracker.HasLineOfSight();
}

void CMediumIng::TakeDamage(const CVector3f& direction, float magnitude) {
  mDamageCooldownTimer = skDamageHitTime;
  mDamageSinceMist += magnitude;
}

bool CMediumIng::ShouldGenerate(CStateManager& mgr, const CTriggerData& data) const {
  if (CPatterned::InDetectionRange(mgr, data) || mD4) {
    return true;
  }
  return false;
}

const CDamageVulnerability* CMediumIng::GetDamageVulnerability() const {
  return HasMist() ? &mData.mMistingVulnerability : CPatterned::GetDamageVulnerability();
}

bool CMediumIng::IsAggressive(CStateManager& mgr, const CTriggerData& data) const { return mC2; }

bool CMediumIng::HasMist() const { return !(CMath::AbsF(mMistAmount - 0.f) < 0.00001f); }

bool CMediumIng::IsMisting(CStateManager& mgr, const CTriggerData& data) const { return HasMist(); }

bool CMediumIng::Leash(CStateManager& mgr, const CTriggerData& data) const {
  if (mE2) {
    return true;
  }
  const CPlayer* player = mgr.GetPlayer(0);
  if (player->GetCurrentAreaId() != GetCurrentAreaId()) {
    return true;
  }
  if ((player->GetTranslation() - GetTranslation()).MagSquared() > mLeashRadius * mLeashRadius &&
      mFC8 > 5.f) {
    return true;
  }
  return false;
}

bool CMediumIng::PathShagged(CStateManager& mgr, const CTriggerData& data) const {
  if (CPatterned::PathShagged(mgr, data) || mC4) {
    return true;
  }
  return false;
}

bool CMediumIng::IsPointInSafeZone(const CStateManager& mgr, const CVector3f& pos) const {
  return mgr.GetSafeZoneManager()->PointIsInSafeZone(mgr, pos);
}

bool CMediumIng::IsInSafeZone(const CStateManager& mgr) const {
  return mgr.GetSafeZoneManager()->IsObjectInSafeZone(*mgr.GetPlayer(0), mgr);
}

void CMediumIng::ValidateSafeZone(const CStateManager& mgr) {
  const CScriptSafeZone* safeZone =
      TCastToConstPtr< CScriptSafeZone >(mgr.GetObjectById(mSafeZoneId));
  if (!safeZone || !safeZone->GetActive() || safeZone->GetZoneType() != CScriptSafeZone::kZT_Echo) {
    mSafeZoneId = kInvalidUniqueId;
  }
}

CPFArea* CMediumIng::GetPathArea(CStateManager& mgr) const {
  return mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea;
}

bool CMediumIng::IsDestInsideCurrentRegion(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f position = GetTranslation();
  const CVector3f dest = mDestPos;
  if ((dest - position).MagSquared() > 0.0625f) {
    return false;
  }
  CPFArea* area = GetPathArea(mgr);
  const CPFRegion* region = area->FindClosestRegion(position, GetSearchPath()->GetRegionFlags(),
                                                    GetSearchPath()->GetCreatureMask(), 10.f);
  const CPFRegion* destRegion = area->FindClosestRegion(dest, GetSearchPath()->GetRegionFlags(),
                                                        GetSearchPath()->GetCreatureMask(), 10.f);
  bool result = false;
  if (region && region == destRegion) {
    result = true;
  }
  return result;
}

bool CMediumIng::ShouldMistAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mLineOfSightTracker.HasLineOfSight()) {
    bool mistReady = false;
    if (HasMist() && mFB4 > 1.f) {
      mistReady = true;
    }
    if (mistReady || mFB4 > 1.f) {
      if (mFC0 > mData.mMinMistAttackInterval && (mistReady || !mC2)) {
        const CPlayer* player = mgr.GetPlayer(0);
        CVector3f toPlayer = player->GetTranslation() - GetTranslation();
        const float distance = toPlayer.Magnitude();
        toPlayer *= 1.f / distance;
        if (CVector3f::GetAngleDiff(player->GetTransform().GetForward(), -toPlayer) <
                (M_PIF / 4.f) &&
            distance * distance < mData.mMaxMistAttackRange * mData.mMaxMistAttackRange) {
          return true;
        }
      }
    }
  }
  return false;
}

bool CMediumIng::ShouldMeleeAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (!IsInSafeZone(mgr) && !HasMist() && mFB4 > 1.f && mLineOfSightTracker.HasLineOfSight() &&
      (mTeamAiMgrId == kInvalidUniqueId ||
       CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                        GetUniqueId()))) {
    const float distanceSquared =
        (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
    if (mFBC > mData.mMinMeleeAttackInterval) {
      return distanceSquared < mData.mMaxMeleeAttackRange * mData.mMaxMeleeAttackRange;
    }
  }
  return false;
}

bool CMediumIng::ShouldArmAttack(CStateManager& mgr, const CTriggerData& data) const {
  const bool armReady = mFC4 < 1.f;
  if ((armReady || (mFB4 > 1.f && !HasMist())) &&
      (armReady || mFB8 > mData.mMinArmAttackInterval) &&
      (mTeamAiMgrId == kInvalidUniqueId ||
       CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                        GetUniqueId()))) {
    const float distanceSquared =
        (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
    const bool farEnough = distanceSquared >= mData.mMinArmAttackRange * mData.mMinArmAttackRange;
    bool result = false;
    if (distanceSquared < mData.mMaxArmAttackRange * mData.mMaxArmAttackRange) {
      if (farEnough || IsInSafeZone(mgr)) {
        result = true;
      }
    }
    return result;
  }
  return false;
}

void CMediumIng::SubStart(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::FollowApproachPattern(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::MistIn(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::MistOut(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::ArmAttack(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::MistAttack(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::Jump(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::Dash(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::WaitForLocomotion(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::Idle(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::SafezoneReaction(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::MoveToSafeZone(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::BackUp(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::Dead(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::SelectTargetState(CStateManager& mgr, EStateMsg msg, float dt) {}
void CMediumIng::SelectTarget(CStateManager& mgr, float dt) {}
void CMediumIng::SetLeashTarget(CStateManager& mgr, float dt) {}
void CMediumIng::SetRetreatDestination(CStateManager& mgr, float dt) {}
void CMediumIng::SetMeshPathDestination(CStateManager& mgr, float dt) {}
void CMediumIng::SetDashDestination(CStateManager& mgr, float dt) {}
CMediumIng::~CMediumIng() {}
void CMediumIng::Think(float dt, CStateManager& mgr) {}
void CMediumIng::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {}
void CMediumIng::PreRender(CStateManager& mgr) {}
void CMediumIng::Render(const CStateManager& mgr) const {}
void CMediumIng::Touch(CActor& actor, CStateManager& mgr) {}
void CMediumIng::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {}
bool CMediumIng::IsListening() const { return true; }
bool CMediumIng::Listen(CStateManager& mgr, const CVector3f& pos, EListenNoiseType type) {
  return false;
}
CMediumIngTentacle::~CMediumIngTentacle() {}
void CMediumIngTentacle::Think(float dt, CStateManager& mgr) {}
void CMediumIngTentacle::PreRender(CStateManager& mgr) {}
rstl::optional_object< CAABox > CMediumIngTentacle::GetTouchBounds() const {
  return rstl::optional_object< CAABox >();
}
void CMediumIngTentacle::Touch(CActor& actor, CStateManager& mgr) {}
void CMediumIngTentacle::Extend(CStateManager& mgr, const CVector3f& target) {}
void CMediumIngTentacle::Retract(CStateManager& mgr) {}
float CMediumIngTentacle::GetProgress() const { return 0.f; }
void CMediumIngTentacle::UpdateShake(CStateManager& mgr, float dt) {}
bool CMediumIng::FindSafeZoneRetreatPoint(const CStateManager& mgr, CVector3f& outPoint) {
  return false;
}
void CMediumIng::UpdateCurrentSafeZone(const CStateManager& mgr, const CVector3f& point) {}
void CMediumIng::UpdateMistEffect(CStateManager& mgr) {}
void CMediumIng::CreateMistEffect(CStateManager& mgr, const TLockedToken< CGenDescription >& desc) {
}
void CMediumIng::SetFlagE1() {}
TUniqueId CMediumIng::FindClosestRetreatActor(const CStateManager& mgr,
                                              const CVector3f& pos) const {
  return kInvalidUniqueId;
}
TUniqueId CMediumIng::FindClosestDangerActor(const CStateManager& mgr, const CVector3f& pos) const {
  return kInvalidUniqueId;
}
void CMediumIng::CalculateSeparation(CStateManager& mgr) {}

// ---- Raw matching-decompiler output from a local tree (reference only, not cleaned up) ----

extern float lbl_41_rodata_14;
extern int lbl_41_data_770;
extern int lbl_41_data_720;
extern int lbl_41_data_758;
extern int lbl_41_data_764;
extern int lbl_41_data_74C;
extern int lbl_41_data_740;
extern "C" void fn_41_B38();
extern "C" void fn_41_8C08(int, int);

extern "C" int fn_41_3260(int arg0) { return *(int*)(arg0 + 0x254); }

extern "C" float fn_41_6840() { return lbl_41_rodata_14; }

extern "C" void fn_41_9A28(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" void fn_41_F84(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" void fn_41_AF0() {
  void fn_41_B10();
  fn_41_B10();
}

extern "C" int fn_41_4990(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_41_6804(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_41_8918(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_41_9174(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_41_93C0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_41_A7DC(int arg0, int arg1, int arg2) {
  float temp_f0;
  int var_r3 = *(int*)arg0;
  int temp_r0 = *(int*)arg1;
  while (var_r3 != (unsigned int)temp_r0) {
    *(float*)arg2 = *(float*)var_r3;
    *(float*)(arg2 + 0x4) = *(float*)(var_r3 + 0x4);
    temp_f0 = *(float*)(var_r3 + 0x8);
    var_r3 = var_r3 + 12;
    *(float*)(arg2 + 0x8) = temp_f0;
    arg2 = arg2 + 12;
  }
  return arg2;
}

extern "C" int fn_41_3064(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_41_data_770;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_6030(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_41_data_720;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_8FF0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_9C8C(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_A2EC(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_FE8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_8438(int arg0, int arg1) {
  void fn_41_8490(int, int);
  if (arg0) {
    fn_41_8490(arg0 + 80, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_84E8(int arg0, int arg1) {
  void fn_41_8540(int, int);
  if (arg0) {
    fn_41_8540(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_3A88(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_41_data_758;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_41_data_770;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_3008(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_41_data_764;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_41_data_770;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_4DC0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_41_data_74C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_41_data_770;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_4F8C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_41_data_740;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_41_data_770;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_D98(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x4c))) {
      ((CModelData*)arg0)->~CModelData();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_A40(int arg0, int arg1) {
  void fn_41_F90(int, int);
  if (arg0) {
    fn_41_F90(arg0 + 160, -1);
    fn_41_F90(arg0 + 92, -1);
    fn_41_F90(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_41_B10(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_41_B38();
  }
}

extern "C" int fn_41_8490(int arg0, int arg1) {
  void fn_41_84E8(int, int);
  if (arg0) {
    fn_41_84E8(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_A294(int arg0, int arg1) {
  void fn_41_A2EC(int, int);
  if (arg0) {
    fn_41_A2EC(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_F90(int arg0, int arg1) {
  void fn_41_FE8(int, int);
  if (arg0) {
    fn_41_FE8(arg0 + 8, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_41_6760(int arg0, int arg1) {
  *(float*)arg0 = *(float*)arg1;
  *(float*)((char*)arg0 + 0x4) = *(float*)((char*)arg1 + 0x4);
  *(float*)((char*)arg0 + 0x8) = *(float*)((char*)arg1 + 0x8);
  *(float*)((char*)arg0 + 0xc) = *(float*)((char*)arg1 + 0xc);
  *(float*)((char*)arg0 + 0x10) = *(float*)((char*)arg1 + 0x10);
  *(float*)((char*)arg0 + 0x14) = *(float*)((char*)arg1 + 0x14);
  *(int*)((char*)arg0 + 0x18) = *(int*)((char*)arg1 + 0x18);
  *(int*)((char*)arg0 + 0x1c) = *(int*)((char*)arg1 + 0x1c);
  *(float*)((char*)arg0 + 0x20) = *(float*)((char*)arg1 + 0x20);
  *(float*)((char*)arg0 + 0x24) = *(float*)((char*)arg1 + 0x24);
  *(float*)((char*)arg0 + 0x28) = *(float*)((char*)arg1 + 0x28);
  *(float*)((char*)arg0 + 0x2c) = *(float*)((char*)arg1 + 0x2c);
  *(float*)((char*)arg0 + 0x30) = *(float*)((char*)arg1 + 0x30);
  *(float*)((char*)arg0 + 0x34) = *(float*)((char*)arg1 + 0x34);
  *(float*)((char*)arg0 + 0x38) = *(float*)((char*)arg1 + 0x38);
  *(int*)((char*)arg0 + 0x3c) = *(int*)((char*)arg1 + 0x3c);
  *(int*)((char*)arg0 + 0x40) = *(int*)((char*)arg1 + 0x40);
  *(float*)((char*)arg0 + 0x44) = *(float*)((char*)arg1 + 0x44);
  *(float*)((char*)arg0 + 0x48) = *(float*)((char*)arg1 + 0x48);
  *(unsigned char*)((char*)arg0 + 0x4c) = *(unsigned char*)((char*)arg1 + 0x4c);
}

extern "C" int fn_41_8B5C(int arg0, int arg1) {
  *(int*)arg0 = *(int*)arg1;
  *(float*)((char*)arg0 + 0x4) = *(float*)((char*)arg1 + 0x4);
  *(float*)((char*)arg0 + 0x8) = *(float*)((char*)arg1 + 0x8);
  *(float*)((char*)arg0 + 0xc) = *(float*)((char*)arg1 + 0xc);
  *(float*)((char*)arg0 + 0x10) = *(float*)((char*)arg1 + 0x10);
  *(float*)((char*)arg0 + 0x14) = *(float*)((char*)arg1 + 0x14);
  fn_41_8C08(arg0 + 24, arg1 + 24);
  fn_41_8C08(arg0 + 92, arg1 + 92);
  fn_41_8C08(arg0 + 160, arg1 + 160);
  *(int*)((char*)arg0 + 0xe4) = *(int*)((char*)arg1 + 0xe4);
  *(float*)((char*)arg0 + 0xe8) = *(float*)((char*)arg1 + 0xe8);
  *(float*)((char*)arg0 + 0xec) = *(float*)((char*)arg1 + 0xec);
  *(float*)((char*)arg0 + 0xf0) = *(float*)((char*)arg1 + 0xf0);
  return arg0;
}

extern "C" int fn_41_103C(int arg0, int arg1) {
  void fn_41_F90(int, int);
  if (arg0) {
    fn_41_F90(arg0 + 1716, -1);
    fn_41_F90(arg0 + 1648, -1);
    ((SLdrDamageInfo*)(arg0 + 1616))->~SLdrDamageInfo();
    ((SLdrCameraShakerData*)(arg0 + 1396))->~SLdrCameraShakerData();
    fn_41_F90(arg0 + 1328, -1);
    ((SLdrActorParameters*)(arg0 + 1208))->~SLdrActorParameters();
    ((SLdrAnimationSet*)(arg0 + 1196))->~SLdrAnimationSet();
    ((SLdrDamageVulnerability*)(arg0 + 820))->~SLdrDamageVulnerability();
    ((SLdrDamageInfo*)(arg0 + 800))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 780))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 644))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_41_8540(int obj, int val) {
  if (obj) {
    unsigned int ptr = *(int*)((char*)obj + 0xc);
    if (ptr != 0) {
      unsigned char* ptr2 = (unsigned char*)obj;
      if ((*ptr2) >> 5 & 1) {
        CMemory::Free((const void*)ptr);
      } else {
        FreeLockedCache((void*)ptr);
      }
      unsigned char val2 = *ptr2;
      *ptr2 = __rlwimi(val2, (val2 >> 2 & 3) - 1, 2, 28, 29);
      if ((*ptr2) >> 2 & 3) {
        *ptr2 = __rlwimi(*ptr2, *ptr2, 1, 26, 26);
      }
    }
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

// ---- End of raw matching-decompiler output ----
