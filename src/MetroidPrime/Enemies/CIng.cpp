#include "MetroidPrime/Enemies/CIng.hpp"
#include "MetroidPrime/Enemies/CIngExitHostEffect.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CCollisionTracker.hpp"
#include "MetroidPrime/Enemies/CIngMiniPortalAttack.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearchFilter.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CGameStateEnvVarManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIng.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "REL/REL_Setup.h"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include <float.h>

struct SJointSphere {
  const char* mName;
  float mRadius;
};

// Guessed name; the joints that get a sphere collision actor, each with its radius.
static const SJointSphere skJointSpheres[] = {
    {"head", 1.5f},    {"Pelvis_SDK", 1.5f}, {"L_knee", 1.5f},  {"R_knee", 1.5f},
    {"F_elbow", 1.5f}, {"L_elbow", 1.5f},    {"R_elbow", 1.5f},
};

static EMaterialTypes skDamageMaterial = kMT_Solid; // Guessed name

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::StateOver)},
    {"IsAlert", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsAlert)},
    {"IsControllingHost",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsControllingHost)},
    {"IsFacingTarget", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsFacingTarget)},
    {"HasTarget", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasTarget)},
    {"HasNewTarget", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasNewTarget)},
    {"HasLineOfSight", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasLineOfSight)},
    {"UnderFire", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::UnderFire)},
    {"HeardShot", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HeardShot)},
    {"AreaClear", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::AreaClear)},
    {"CoverLeash", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::CoverLeash)},
    {"HasCoverPoint", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasCoverPoint)},
    {"HasWallCoverPoint",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasWallCoverPoint)},
    {"HasMiniPortals", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasMiniPortals)},
    {"IsCorporeal", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsCorporeal)},
    {"IsIngSpot", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsIngSpot)},
    {"IsBodyProjectile",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsBodyProjectile)},
    {"IsFrustrated", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsFrustrated)},
    {"IsAggressive", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsAggressive)},
    {"IsLuredBySafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsLuredBySafeZone)},
    {"StillLuredBySafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::StillLuredBySafeZone)},
    {"UseProjectileFSMEntry",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::UseProjectileFSMEntry)},
    {"TargetInSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::TargetInSafeZone)},
    {"TargetInLightSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::TargetInLightSafeZone)},
    {"EmergePtInSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::EmergePtInSafeZone)},
    {"CanChangeForm", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::CanChangeForm)},
    {"ShouldEvaporate",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldEvaporate)},
    {"ShouldFleeSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldFleeSafeZone)},
    {"ShouldBecomeCorporeal",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldBecomeCorporeal)},
    {"ShouldBecomeIngSpot",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldBecomeIngSpot)},
    {"ShouldArmSwipe", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldArmSwipe)},
    {"ShouldTaunt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldTaunt)},
    {"ShouldBodyProjectile",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldBodyProjectile)},
    {"ShouldOpenMiniPortal",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ShouldOpenMiniPortal)},
    {"ProjectileSplat",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ProjectileSplat)},
    {"ProjectileHitTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::ProjectileHitTarget)},
    {"FoundMovementPos",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::FoundMovementPos)},
    {"PathShagged", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::PathShagged)},
    {"PathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::PathOver)},
    {"PointPathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::PointPathOver)},
    {"FoundPointPath", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::FoundPointPath)},
    {"IsOffPath", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::IsOffPath)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasAttackPattern)},
    {"AttackPatternOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::AttackPatternOver)},
    {"TargetIsBall", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::TargetIsBall)},
    {"InBallPursuitRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::InBallPursuitRange)},
    {"HasPathToTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::HasPathToTarget)},
    {"InGrappleRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::InGrappleRange)},
    {"CanGrappleTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIng::CanGrappleTarget)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Start)},
    {"WaitForFSMTransition",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::WaitForFSMTransition)},
    {"ExitHost", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::ExitHost)},
    {"BecomeCorporeal", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::BecomeCorporeal)},
    {"BecomeIngSpot", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::BecomeIngSpot)},
    {"BecomeBodyProjectile",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::BecomeBodyProjectile)},
    {"BecomeWallProjectile",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::BecomeWallProjectile)},
    {"Alert", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Alert)},
    {"SafeZoneReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::SafeZoneReaction)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Taunt)},
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Lurk)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::PathFind)},
    {"IngSpotPathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::IngSpotPathFind)},
    {"IngSpotPointPathFind",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::IngSpotPointPathFind)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Patrol)},
    {"FollowAttackPattern",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::FollowAttackPattern)},
    {"BodyProjectileFlight",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::BodyProjectileFlight)},
    {"SuckEnergy", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::SuckEnergy)},
    {"SeekWallPoint", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::SeekWallPoint)},
    {"SelectTarget", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::SelectTarget)},
    {"FaceTarget", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::FaceTarget)},
    {"FindCoverPoint", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::FindCoverPoint)},
    {"ArmSwipe", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::ArmSwipe)},
    {"FailSafeMode", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::FailSafeMode)},
    {"GrappleBall", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::GrappleBall)},
    {"FindMiniPortals", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::FindMiniPortals)},
    {"MiniPortalAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIng::MiniPortalAttack)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Dead)},
    {"Evaporate", static_cast< CPatterned::StateMachine::StateFunc >(&CIng::Evaporate)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SetTargetDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetTargetDest)},
    {"SetCoverDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetCoverDest)},
    {"SetRecoverDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetRecoverDest)},
    {"SetPointCoverDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetPointCoverDest)},
    {"SetWallPointCoverDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetWallPointCoverDest)},
    {"SetExitHostDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetExitHostDest)},
    {"SetLuredDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetLuredDest)},
    {"SetLuredPointDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SetLuredPointDest)},
    {"StartRangedAttack",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::StartRangedAttack)},
    {"EndRangedAttack", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::EndRangedAttack)},
    {"SplatOntoMesh", static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::SplatOntoMesh)},
    {"ExpelGrappledBall",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CIng::ExpelGrappledBall)},
};

static EMaterialTypes skSplatMaterial1 = kMT_Ceiling;                  // Guessed name
static EMaterialTypes skSplatMaterial2 = kMT_Wall;                     // Guessed name
static EMaterialTypes skSplatMaterial3 = kMT_Floor;                    // Guessed name
static EMaterialTypes skSplatMaterial4 = kMT_Solid;                    // Guessed name
static EMaterialTypes skSplatMaterial5 = kMT_Immovable;                // Guessed name
static EMaterialTypes skSplatMaterial6 = kMT_Occluder;                 // Guessed name
static EMaterialTypes skAreaClearMaterial1 = kMT_Player;               // Guessed name
static EMaterialTypes skAreaClearMaterial2 = kMT_Character;            // Guessed name
static EMaterialTypes skPortalRayInclude = kMT_Solid;                  // Guessed name
static EMaterialTypes skPortalRayExclude1 = kMT_Character;             // Guessed name
static EMaterialTypes skPortalRayExclude2 = kMT_Player;                // Guessed name
static EMaterialTypes skPortalRayExclude3 = kMT_CollisionActor;        // Guessed name
static EMaterialTypes skPortalRayExclude4 = kMT_ProjectilePassthrough; // Guessed name
static EMaterialTypes skSwipeDamageMaterial = kMT_Solid;               // Guessed name
static EMaterialTypes skColliderInclude = kMT_Solid;                   // Guessed name
static EMaterialTypes skColliderExclude1 = kMT_CollisionActor;         // Guessed name
static EMaterialTypes skColliderExclude2 = kMT_AIPassthrough;          // Guessed name
static EMaterialTypes skColliderExclude3 = kMT_Player;                 // Guessed name
static const char* const skEyesName = "eyes";                          // Guessed name

CIngExitHostEffect::CIngExitHostEffect(TUniqueId uid, const CEntityInfo& info,
                                       const CTransform4f& xf, const CVector3f& exitPosition,
                                       const CVector3f& scale, CAssetId swarmEffect,
                                       CAssetId trailEffect, float trailLength, float speed,
                                       TUniqueId targetId, float homingTime, float homingStrength,
                                       ushort sound)
: CActor(uid, rstl::string_l("IngExitHostSwarmFx"), info, 0, xf, CModelData::None(),
         CMaterialList(), CActorParameters::None(), kInvalidUniqueId)
, mSpawnTransform(xf)
, mPathFindSearch(nullptr, 3, 0, 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mExitPosition(exitPosition)
, mScale(scale)
, mElapsedTime(0.f)
, mSpeed(speed)
, mTargetId(targetId)
, mHomingTime(homingTime)
, mHomingTimeRemaining(homingTime)
, mHomingStrength(homingStrength)
, mSwarmToken(gpSimplePool->GetObj(
      SObjectTag(gpResourceFactory->GetResourceTypeById(swarmEffect), swarmEffect)))
, mSwarmParticle(CreateParticle(scale, GetTranslation(), mSwarmToken))
, mTrailToken(gpSimplePool->GetObj(
      SObjectTag(gpResourceFactory->GetResourceTypeById(trailEffect), trailEffect)))
, mTrailParticle(CreateParticle(scale, GetTranslation(), mTrailToken))
, mTrailLength(trailLength)
, mArcLength(0.f)
, mArcSpeedScale(1.f)
, mHistoryIndex(0)
, mSfxHandle()
, mSound(sound)
, mArrived(false) {
  mSwarmParticle->SetGeneratorRate(1.f);
  const rstl::reserved_vector< CVector3f, 60 > history(60, GetTranslation());
  mPositionHistory = history;
}

void CIngExitHostEffect::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    UpdateMovement(dt, mgr);
    UpdateParticles(dt, mgr);
    UpdateSfxEmitter();
    if (mArrived) {
      mSwarmParticle->SetGeneratorRate(0.f);
      mTrailParticle->SetGeneratorRate(0.f);
      if (mSwarmParticle->GetParticleCount() == 0 && mTrailParticle->GetParticleCount() == 0) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
    }
  }
}

void CIngExitHostEffect::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Delete:
    RemoveSfxEmitter();
    break;
  case kSM_AreaLoaded:
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    break;
  default:
    break;
  }
}

void CIngExitHostEffect::Render(const CStateManager& mgr) const {
  mTrailParticle->Render();
  mSwarmParticle->Render();
}

void CIngExitHostEffect::AddToRenderer(const CStateManager& mgr) const {
  if (mTrailParticle.get() != nullptr) {
    gpRender->AddParticleGen(*mTrailParticle);
  }
  if (mSwarmParticle.get() != nullptr) {
    gpRender->AddParticleGen(*mSwarmParticle);
  }
}

CParticleGen* CIngExitHostEffect::CreateParticle(const CVector3f& scale,
                                                 const CVector3f& translation,
                                                 const CToken& token) {
  return CElementGen::ConstructChildParticleSystem(
      token, token.GetTag().type, 0, CElementGen::kOSF_One, false, true, translation,
      CTransform4f::Identity(), CVector3f::Zero(), CTransform4f::Identity(), scale, CColor::White(),
      CVector3f::One());
}

void CIngExitHostEffect::UpdateParticles(float dt, CStateManager& mgr) {
  CVector3f position = GetTranslation();
  if (mHomingTimeRemaining > 0.f) {
    mHomingTimeRemaining = CMath::Max(mHomingTimeRemaining - dt, 0.f);
    const CActor* target =
        TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(mTargetId)));
    if (target != nullptr) {
      const CVector3f offset = mHomingStrength * target->GetTransform().GetForward();
      const CVector3f aim = target->GetAimPosition(mgr, 0.f);
      const float t = mHomingTimeRemaining / mHomingTime;
      position = CVector3f::Lerp(position, aim + offset, t);
    }
  }
  mPositionHistory[mHistoryIndex] = position;
  ++mHistoryIndex;
  mHistoryIndex %= 60;
  mSwarmParticle->SetTranslation(position);
  mSwarmParticle->Update(dt);
  const int offset = static_cast< int >(mTrailLength * (60.f * (dt / (1.f / 60.f))));
  const int index = (mHistoryIndex - offset + 60) % 60;
  mTrailParticle->SetTranslation(mPositionHistory[index]);
  mTrailParticle->Update(dt);
  rstl::optional_object< CAABox > bounds = mSwarmParticle->GetBounds();
  const rstl::optional_object< CAABox > trailBounds = mTrailParticle->GetBounds();
  if (!bounds.valid()) {
    bounds = trailBounds;
  } else if (trailBounds.valid()) {
    bounds->Include(*trailBounds);
  }
  if (bounds.valid()) {
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
  }
}

void CIngExitHostEffect::UpdateMovement(float dt, CStateManager& mgr) {
  if (mArcPoints.size() == 0) {
    BuildArc(mgr);
  }
  const float speed = mArcSpeedScale * mSpeed;
  const CVector3f origin = GetTranslation();
  CVector3f position = origin;
  if (mArcPoints.size() == 4 && speed > 0.f) {
    const float duration = mArcLength / speed;
    if (duration > 0.f) {
      const float progress = mElapsedTime / duration;
      const float t = CMath::Min(1.f, progress);
      position =
          CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], t);
      if (progress >= 1.f) {
        BuildArc(mgr);
      }
    }
  }
  mElapsedTime += dt;
  const CVector3f remaining = mExitPosition - origin;
  if (remaining.MagSquared() > (dt * mSpeed) * (dt * mSpeed)) {
    SetTranslation(position);
  } else {
    SetTranslation(mExitPosition);
    mArrived = true;
  }
}

CVector3f CIngExitHostEffect::RandomArcOffset(CStateManager& mgr) const {
  const CVector3f right = GetTransform().GetRight();
  const CVector3f up = GetTransform().GetUp();
  const float upAmount = mgr.Random()->Range(0.f, 4.f);
  const float sideAmount = mgr.Random()->Range(-2.f, 2.f);
  return sideAmount * right + upAmount * up;
}

float CIngExitHostEffect::CalculateArcLength() const {
  if (mArcPoints.size() == 4) {
    const CVector3f quarter =
        CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], 0.25f);
    const CVector3f half =
        CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], 0.5f);
    const CVector3f threeQuarters =
        CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], 0.75f);
    return (half - quarter).Magnitude() + (quarter - mArcPoints[0]).Magnitude() +
           (threeQuarters - half).Magnitude() + (mArcPoints[3] - threeQuarters).Magnitude();
  }
  return 0.f;
}

void CIngExitHostEffect::BuildArc(CStateManager& mgr) {
  mArcPoints.clear();
  mElapsedTime = 0.f;
  const CVector3f position = GetTranslation();
  CVector3f next = position;
  const CVector3f direction = mExitPosition - position;
  if (direction.MagSquared() > 25.f) {
    if (mPathFindSearch.Search(position, mExitPosition) == CPathFindSearch::kR_Success) {
      mPathFindSearch.GetSplinePointWithLookahead(next, position, 5.f);
    } else {
      next = position + 5.f * direction.AsNormalized();
    }
  } else {
    next = mExitPosition;
  }
  const CVector3f quarter = 0.25f * (next - position);
  mArcPoints.push_back(position);
  mArcPoints.push_back(position + quarter + RandomArcOffset(mgr));
  mArcPoints.push_back(next - quarter + RandomArcOffset(mgr));
  mArcPoints.push_back(next);
  mArcLength = CalculateArcLength();
  mArcSpeedScale = 1.f - mgr.Random()->Range(0.f, 0.5f);
}

void CIngExitHostEffect::UpdateSfxEmitter() {
  if (!mSfxHandle) {
    mSfxHandle = CSfxManager::AddEmitter(mSound, GetTranslation(), 127, GetCurrentAreaId().Value(),
                                         true, true, CSfxManager::kMedPriority);
  } else {
    CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), GetTransform().GetForward(), 127);
  }
}

void CIngExitHostEffect::RemoveSfxEmitter() {
  if (mSfxHandle) {
    CSfxManager::RemoveEmitter(mSfxHandle);
    mSfxHandle.Clear();
  }
}

CIngExitHostEffect::~CIngExitHostEffect() {}

CIng::CIng(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& modelData, const CActorParameters& actorParams,
           const CPatternedInfo& patternedInfo, const SIngData& data)
: CPatterned(kPAI_Ing, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actorParams)
, mData(data)
, mForm(kF_Corporeal)
, mNextForm(kF_Invalid)
, mPathFindSearch(nullptr, 0x301, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mPointSearch(nullptr)
, mCollisionActorManager(nullptr)
, mSurfaceAlignment()
, mPointNavigation()
, mTouchBounds(CAABox::MakeNullBox())
, mHostId(kInvalidUniqueId)
, mPossessionEffectId(kInvalidUniqueId)
, mPossessionHudEffect(data.swarm.possessionHudEffect != kInvalidAssetId
                           ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                 TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                     SObjectTag('PART', data.swarm.possessionHudEffect))))
                           : rstl::optional_object< TLockedToken< CGenDescription > >())
, mExitHostSmokeEffect(data.swarm.exitHostSmokeEffect != kInvalidAssetId
                           ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                 TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                     SObjectTag('PART', data.swarm.exitHostSmokeEffect))))
                           : rstl::optional_object< TLockedToken< CGenDescription > >())
, mSfxHostInside()
, mSfxGrapple()
, mIngSpotNormalHitEffect(data.ingSpot.GetNormalHitEffect() != kInvalidAssetId
                              ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                    TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                        SObjectTag('PART', data.ingSpot.GetNormalHitEffect()))))
                              : rstl::optional_object< TLockedToken< CGenDescription > >())
, mIngSpotHeavyHitEffect(data.ingSpot.GetHeavyHitEffect() != kInvalidAssetId
                             ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                   TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                       SObjectTag('PART', data.ingSpot.GetHeavyHitEffect()))))
                             : rstl::optional_object< TLockedToken< CGenDescription > >())
, mIngSpotDeathEffect(data.ingSpot.GetDeathEffect() != kInvalidAssetId
                          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                    SObjectTag('PART', data.ingSpot.GetDeathEffect()))))
                          : rstl::optional_object< TLockedToken< CGenDescription > >())
, mMiniPortalEffect(data.miniPortal.effect != kInvalidAssetId
                        ? rstl::optional_object< TLockedToken< CGenDescription > >(
                              TLockedToken< CGenDescription >(
                                  gpSimplePool->GetObj(SObjectTag('PART', data.miniPortal.effect))))
                        : rstl::optional_object< TLockedToken< CGenDescription > >())
, mSplatEffect(data.bodyProjectile.splatEffect != kInvalidAssetId
                   ? rstl::optional_object< TLockedToken< CGenDescription > >(
                         TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                             SObjectTag('PART', data.bodyProjectile.splatEffect))))
                   : rstl::optional_object< TLockedToken< CGenDescription > >())
, mSfxBodyProjectile()
, mBlobEffectId(kInvalidUniqueId)
, mSfxIngSpotIdle()
, mSfxIngSpotMove()
, mLightId(kInvalidUniqueId)
, mTeamManagerId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mLastTargetId(kInvalidUniqueId)
, mPatrolWaypointId(kInvalidUniqueId)
, mCoverHintId(kInvalidUniqueId)
, mLastCoverHintId(kInvalidUniqueId)
, mExitHostEffectId(kInvalidUniqueId)
, mSafeZoneId(kInvalidUniqueId)
, mCollarSegment(CSegId::Invalid())
, mHeadSegment(CSegId::Invalid())
, mRightShoulderSegment(CSegId::Invalid())
, mRightElbowSegment(CSegId::Invalid())
, mRightForearmSegment(CSegId::Invalid())
, mRightWristSegment(CSegId::Invalid())
, mLeftShoulderSegment(CSegId::Invalid())
, mLeftElbowSegment(CSegId::Invalid())
, mLeftForearmSegment(CSegId::Invalid())
, mLeftWristSegment(CSegId::Invalid())
, mLineOfSight(GetUniqueId(), CSegId::Invalid(), 0.1f, 0.05f)
, mDestination(CVector3f::Zero())
, mSplatNormal(CVector3f::Zero())
, mMoveHeading(CVector3f::Zero())
, mSwipeIndex(-1)
, mPortalPlane(GetTranslation(), CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes))
, mMiniPortalCount(0)
, mMiniPortalIndex(-1)
, mSafeZoneCount(0)
, mHeardShotTimer(1.5f)
, mUnderFireTimer(0.75f)
, mTimeSinceArmSwipe(1.f)
, mFormChangeTimer(0.f)
, mLocomotionTime(0.f)
, mFrustrationTimer(0.f)
, mGrappleCooldown(0.f)
, mGrappleHold(0.f)
, mLightIntensity(1.f)
, mDeathDelayTimer(0.f)
, mProjectileFlightTime(0.f)
, mShouldEvaporate(false)
, mShouldTaunt(false)
, mCanBodyProjectile(false)
, mAggressive(false)
, mFoundMovementPos(true)
, mPathObstructed(false)
, mSwipeDamagePending(false)
, mTakeOffReceived(false)
, mDrawModel(true)
, mUsePortalPlane(false)
, mBlobEffectActive(data.startsAsIngSpot)
, mWallProjectileVisible(false)
, mFollowingWaypoint(false)
, mAlert(false)
, mMovingOnSurface(false)
, mGrappling(false)
, mUseProjectileFSMEntry(false)
, mProjectileSplat(false)
, mIngSpotHurt(false)
, mInHurtfulSafeZone(false) {
  KnockBackController().EnableKnockBackPhysics(false);
  const CAnimData* animData = GetAnimationData();
  mHeadSegment = animData->GetLocatorSegId(rstl::string_l("head"));
  mCollarSegment = animData->GetLocatorSegId(rstl::string_l("Collar_SDK"));
  mRightShoulderSegment = animData->GetLocatorSegId(rstl::string_l("R_shoulder"));
  mRightElbowSegment = animData->GetLocatorSegId(rstl::string_l("R_elbow"));
  mRightForearmSegment = animData->GetLocatorSegId(rstl::string_l("R_forearm"));
  mRightWristSegment = animData->GetLocatorSegId(rstl::string_l("R_wrist_end"));
  mLeftShoulderSegment = animData->GetLocatorSegId(rstl::string_l("L_shoulder"));
  mLeftElbowSegment = animData->GetLocatorSegId(rstl::string_l("L_elbow"));
  mLeftForearmSegment = animData->GetLocatorSegId(rstl::string_l("L_forearm"));
  mLeftWristSegment = animData->GetLocatorSegId(rstl::string_l("L_wrist_end"));
  mLineOfSight.SetSegment(mHeadSegment);
  mPathFindSearch.SetCharacterRadius(3.f * GetModelData()->GetScale().GetY());
  mPathFindSearch.SetCharacterHeight(5.f * GetModelData()->GetScale().GetZ());
  SetDrawShadow(false);
}

CIng::~CIng() {}

void CIng::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  const float wasActive = GetActive();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    SetInitialForm(mgr);
    mBodyController->Activate(mgr, pas::kAS_Invalid);
    SetupCollision(mgr);
    SpawnBlobEffect(mgr, TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                             SObjectTag('PART', mData.ingSpot.GetBlobEffect()))));
    CreateLight(mgr);
    break;
  case kSM_Delete:
    mgr.DeleteObjectRequest(mBlobEffectId);
    StopSounds();
    mCollisionActorManager->Destroy(mgr);
    mgr.DeleteObjectRequest(mLightId);
    if (mExitHostEffectId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mExitHostEffectId);
    }
    ReleaseCoverHint(mgr);
    LeaveTeam(mgr);
    break;
  case kSM_Activate:
    if (!wasActive) {
      mCollisionActorManager->SetActive(mgr, mForm == kF_Corporeal);
      if (CEntity* light = mgr.ObjectById(mLightId)) {
        light->SetActive(true);
      }
    }
    break;
  case kSM_Deactivate:
    if (wasActive) {
      mCollisionActorManager->SetActive(mgr, false);
      LeaveTeam(mgr);
      if (CEntity* light = mgr.ObjectById(mLightId)) {
        light->SetActive(false);
      }
      if (CEntity* blob = mgr.ObjectById(mBlobEffectId)) {
        static_cast< CCollisionTracker* >(blob)->SetParticleEmissionRateScalar(0.f);
      }
      StopSounds();
    }
    break;
  case kSM_AIUpdateDisabled:
    if (!mCollisionActorManager.null()) {
      mCollisionActorManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Alert:
    mHitByPlayerProjectile = true;
    break;
  case kSM_AreaLoaded: {
    CPFArea* area =
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea;
    mPathFindSearch.SetArea(area);
    mPointSearch.SetArea(area);
    break;
  }
  case kSM_Damage:
    HandleDamage(mgr, msg.GetSenderId());
    mHitByPlayerProjectile = true;
    break;
  case kSM_ResistedDamage:
    if (mForm == kF_IngSpot && mGrappling) {
      mGrappleHold -= 0.5f;
      SpawnDamageEffect(mgr);
    }
    mHitByPlayerProjectile = true;
    break;
  case kSM_HitObject:
    TouchDamage(mgr, senderId);
    break;
  case kSM_XENZ:
    ++mSafeZoneCount;
    if (mForm == kF_IngSpot) {
      mIngSpotHurt = true;
    }
    mInHurtfulSafeZone = mgr.GetSafeZoneManager()->IsObjectInHurtfulSafeZone(*this, mgr);
    break;
  case kSM_XEXZ:
    --mSafeZoneCount;
    mInHurtfulSafeZone = mgr.GetSafeZoneManager()->IsObjectInHurtfulSafeZone(*this, mgr);
    break;
  default:
    break;
  }
}

void CIng::PreThink(float dt, CStateManager& mgr) {
  if (GetActive() && mForm == kF_IngSpot && !GetMaterialList().HasMaterial(kMT_GroundCollider)) {
    const CPlane plane = mSurfaceAlignment.GetSurface().GetPlane();
    if (CVector3f::Dot(mMoveHeading, plane.GetNormal()) < 0.95f) {
      const float distance = plane.GetHeight(GetTranslation());
      const CVector3f onSurface = GetTranslation() - (distance - 0.1f) * plane.GetNormal();
      SetTranslation(CVector3f::Lerp(GetTranslation(), onSurface, 0.6f * dt));
    }
    mMoveHeading = CVector3f::Zero();
  }
  CPatterned::PreThink(dt, mgr);
}

void CIng::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  switch (mForm) {
  case kF_PossessingHost:
  case kF_ExitingHost:
    CActor::Think(dt, mgr);
    mStateMachine->Update(mgr, *this, dt);
    SetDrawShadow(false);
    return;
  case kF_IngSpot:
  case kF_Evaporating:
    CActor::Think(dt, mgr);
    UpdateStateMachine(dt, mgr);
    SetDrawShadow(false);
    UpdateHitDamageTime(dt);
    break;
  default:
    CPatterned::Think(dt, mgr);
    break;
  }
  mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mLineOfSight.Update(dt, mgr);
  UpdateTimers(dt, mgr);
  mSurfaceAlignment.Update(*this, mgr, dt);
  UpdateBlobEffect(dt, mgr);
  UpdateTargetable(mgr);
  UpdateSounds();
  UpdateTouchBounds();
  UpdateLight(dt, mgr);
  UpdateEchoSafeZone(mgr);
  if (mgr.GetObjectById(mExitHostEffectId) == nullptr) {
    mExitHostEffectId = kInvalidUniqueId;
  }
}

void CIng::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                           float dt) {
  bool handled = false;
  switch (type) {
  case kUE_TakeOff:
    mTakeOffReceived = true;
    handled = true;
    break;
  case kUE_BeginAction:
    if (mAlive) {
      mForm = kF_BodyProjectile;
      mProjectileFlightTime = 0.f;
    }
    handled = true;
    break;
  case kUE_Activate:
    mBlobEffectActive = true;
    handled = true;
    break;
  case kUE_Deactivate:
    mBlobEffectActive = false;
    handled = true;
    break;
  case kUE_EffectOn: {
    CAnimData* animData = AnimationData();
    animData->SetEffectState(rstl::string_l(skEyesName), true, mgr);
    handled = true;
    break;
  }
  case kUE_EffectOff: {
    CAnimData* animData = AnimationData();
    animData->SetEffectState(rstl::string_l(skEyesName), false, mgr);
    handled = true;
    break;
  }
  case kUE_Projectile:
    if (mMiniPortalEffect && mMiniPortalIndex >= 0 && mMiniPortalIndex < mMiniPortalCount) {
      const CVector3f& position = mMiniPortalPositions[mMiniPortalIndex++];
      CTransform4f xf = GetTransform();
      xf.SetTranslation(position);
      CDamageInfo damage(mData.miniPortal.damage);
      damage.SetDamage(dt * damage.GetDamage());
      damage.SetNoImmunity(true);
      const CIngMiniPortalInfo portalInfo(mTargetId, 1.f, 2.f, *mMiniPortalEffect,
                                          mData.miniPortal.sound, 150.f, 1.f, damage,
                                          mData.miniPortal.beamInfo);
      CIngMiniPortalAttack* portal = rs_new CIngMiniPortalAttack(
          mgr.AllocateUniqueId(), rstl::string_l("Ing Mini Portal Attack"),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), xf, GetUniqueId(),
          GetModelData()->GetScale(), portalInfo);
      mgr.AddObject(portal);
    }
    handled = true;
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CIng::Render(const CStateManager& mgr) const {
  if (mUsePortalPlane) {
    GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), mPortalPlane);
  }
  switch (mForm) {
  case kF_Corporeal:
  case kF_BodyProjectile:
  case kF_BecomingCorporeal:
  case kF_BecomingIngSpot:
  case kF_BecomingBodyProjectile:
  case kF_BecomingWallProjectile:
    if (mForm == kF_BecomingWallProjectile && !mWallProjectileVisible) {
      break;
    }
    if (mDrawModel) {
      CPatterned::Render(mgr);
    } else if (mDrawParticles) {
      uint mask = 0;
      uint target = 0;
      mgr.GetCharacterRenderMaskAndTarget(mask, target);
      CPatterned::RenderSystemsToBeDrawnFirst(mgr, mask, target);
      CPatterned::RenderSystemsToBeDrawnLast(mgr, mask, target);
    }
    break;
  case kF_IngSpot:
  case kF_Evaporating:
    if (mLocomotionTime < 2.5f && mDrawParticles) {
      uint mask = 0;
      uint target = 0;
      mgr.GetCharacterRenderMaskAndTarget(mask, target);
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirstPOICheck(mask, target);
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLastPOICheck(mask, target);
    }
    break;
  default:
    break;
  }
}

void CIng::PreRender(CStateManager& mgr) {
  switch (mForm) {
  case kF_Corporeal:
  case kF_BodyProjectile:
  case kF_BecomingCorporeal:
  case kF_BecomingIngSpot:
  case kF_BecomingBodyProjectile:
  case kF_BecomingWallProjectile:
    CPatterned::PreRender(mgr);
    break;
  case kF_IngSpot:
  case kF_Evaporating:
    if (mLocomotionTime < 2.5f) {
      CPatterned::PreRender(mgr);
    }
    break;
  default:
    break;
  }
  if (mUsePortalPlane) {
    SetModelFlags(
        CModelFlags(GetModelFlags(), GetModelFlags().GetOtherFlags() | CModelFlags::kF_Unknown80));
  }
}

void CIng::PreRenderAllViewports(CStateManager& mgr) { CPatterned::PreRenderAllViewports(mgr); }

void CIng::AddToRenderer(const CStateManager& mgr) const {
  switch (mForm) {
  case kF_Corporeal:
  case kF_BodyProjectile:
  case kF_BecomingCorporeal:
  case kF_BecomingIngSpot:
  case kF_BecomingBodyProjectile:
  case kF_BecomingWallProjectile:
    CPatterned::AddToRenderer(mgr);
    break;
  case kF_IngSpot:
  case kF_Evaporating:
    if (mLocomotionTime < 2.5f) {
      CPatterned::AddToRenderer(mgr);
    }
    break;
  default:
    break;
  }
}

const CDamageVulnerability* CIng::GetDamageVulnerability() const {
  switch (mForm) {
  case kF_BodyProjectile:
    return CPatterned::GetDamageVulnerability();
  case kF_IngSpot:
    if (mGrappling) {
      return &mData.grapple.vulnerability;
    }
    return &mData.ingSpot.GetVulnerability();
  default:
    return &mData.triggerVulnerability;
  }
}

CVector3f CIng::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f position = CVector3f::Zero();
  switch (mForm) {
  case kF_Corporeal:
  case kF_BecomingCorporeal:
  case kF_BecomingIngSpot:
  case kF_BecomingBodyProjectile: {
    if (dt > 0.f) {
      const CMotionState motion = PredictMotion(dt);
      position = motion.GetTranslation();
    }
    if (mCollarSegment != CSegId::Invalid()) {
      const CTransform4f xf =
          GetTransform() * GetAnimationData()->GetLocatorTransform(mCollarSegment, nullptr);
      position += GetModelData()->GetScale() * xf.GetTranslation();
      if ((mForm == kF_BecomingCorporeal || mForm == kF_BecomingIngSpot) &&
          position.GetZ() < GetTranslation().GetZ()) {
        position = CPatterned::GetAimPosition(mgr, dt);
      }
    } else {
      position = CPatterned::GetAimPosition(mgr, dt);
    }
    break;
  }
  case kF_PossessingHost:
  case kF_ExitingHost:
  case kF_IngSpot:
  case kF_BodyProjectile:
  case kF_Evaporating:
  case kF_BecomingWallProjectile:
  default:
    position = CPatterned::GetAimPosition(mgr, dt);
    break;
  }
  return position;
}

void CIng::Touch(CActor& actor, CStateManager& mgr) {
  if (!mGrappling && mForm == kF_IngSpot && actor.GetUniqueId() == mTargetId &&
      mCurDamageRemTime <= 0.f) {
    mgr.ApplyDamage(
        GetUniqueId(), mTargetId, GetUniqueId(), GetContactDamage(),
        CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageMaterial), CMaterialList()),
        CVector3f::Zero());
    mCurDamageRemTime = mDamageWaitTime;
  }
  CPatterned::Touch(actor, mgr);
}

rstl::optional_object< CAABox > CIng::GetTouchBounds() const {
  rstl::optional_object< CAABox > bounds;
  switch (mForm) {
  case kF_IngSpot:
  case kF_Evaporating:
    bounds = mTouchBounds;
    break;
  case kF_PossessingHost:
  case kF_ExitingHost:
    bounds = rstl::optional_object< CAABox >();
    break;
  default:
    bounds = CPatterned::GetTouchBounds();
    break;
  }
  return bounds;
}

void CIng::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mForm == kF_Corporeal &&
      GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_KnockBack) {
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, true);
  } else {
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, false);
  }
  if (info.GetDamageInfo().GetWeaponMode().GetType() == kWT_Light) {
    KnockBackController().SetFlinchType(2);
  } else {
    KnockBackController().SetFlinchType(1);
  }
  CPatterned::KnockBack(mgr, info);
}

void CIng::TakeDamage(const CVector3f& direction, float magnitude) {
  switch (mForm) {
  case kF_PossessingHost:
  case kF_ExitingHost:
  case kF_Evaporating:
    break;
  default:
    mDamageCooldownTimer = skDamageHitTime;
    break;
  }
}

void CIng::UpdateHitDamageTime(float dt) {
  CEchoEmitter* emitter = EchoEmitter();
  mDamageCooldownTimer = CMath::Max(mDamageCooldownTimer - dt, 0.f);
  if (mDamageCooldownTimer > 0.f) {
    const float t = mDamageCooldownTimer / skDamageHitTime;
    const CColor& color = CColor::Lerp(CColor::Black(), skDamageColor, t);
    const uchar blue = color.GetBlueu8();
    const uchar green = color.GetGreenu8();
    mColor.SetRed(color.GetRedu8());
    mColor.SetGreen(green);
    mColor.SetBlue(blue);
    SetDamageHighlight(true);
    if (emitter != nullptr) {
      emitter->SetDamageExplicit(t);
    }
  } else {
    SetDamageHighlight(false);
    if (emitter != nullptr) {
      emitter->SetDamageExplicit(0.f);
    }
  }
}

void CIng::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list, CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (mForm == kF_BodyProjectile && mProjectileFlightTime >= 0.5f) {
    static const CMaterialList testList(skSplatMaterial1, skSplatMaterial2, skSplatMaterial3,
                                        skSplatMaterial4, skSplatMaterial5, skSplatMaterial6);
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (info.GetMaterialLeft().SharesMaterials(testList)) {
        mSplatNormal = info.GetNormalLeft();
        mProjectileSplat = true;
        break;
      }
    }
  }
}

void CIng::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (mAlive) {
    mBodyController->SetTimeScale(1.f);
    if (mStateMachine->HasState()) {
      mStateMachine->SetState(mgr, *this, rstl::string_l("Dead"));
    }
    RemoveMaterial(kMT_GroundCollider, mgr);
    if (!mBurning && !mLaggedBurnDeath) {
      mVerticalMovement = false;
    }
    mAlive = false;
    switch (mForm) {
    case kF_Corporeal:
    case kF_BecomingCorporeal:
    case kF_BecomingIngSpot:
    case kF_BecomingBodyProjectile:
    case kF_BecomingWallProjectile:
      IssueDeathBodyCommand(mgr, direction);
      break;
    default:
      break;
    }
    if (state != kSS_InvalidState) {
      SendScriptMsgs(state, mgr, kInvalidUniqueId, kSM_None);
    }
  }
}

bool CIng::Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) {
  bool heard = false;
  if (mAlive) {
    switch (type) {
    case kLNT_PathObstruction: {
      const CVector3f diff = position - GetTranslation();
      if (diff.MagSquared() < 1600.f) {
        mPathObstructed = heard = true;
      }
      break;
    }
    case kLNT_PlayerFire: {
      const float radiusSq = mData.hearingRadius * mData.hearingRadius;
      const CVector3f diff = position - GetTranslation();
      const float distSq = diff.MagSquared();
      if (distSq <= radiusSq) {
        const float rangeSq = mDetectionHeightRange * mDetectionHeightRange;
        if (mDetectionHeightRange == 0.f || distSq < rangeSq) {
          heard = true;
          mHeardShotTimer = 0.f;
        }
      }
      break;
    }
    case kLNT_SafeZone:
      UpdateLuringSafeZone(mgr, position);
      break;
    default:
      break;
    }
  }
  return heard;
}

void CIng::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

bool CIng::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.GetState() == CAnimationState::kAS_Over;
}

bool CIng::IsAlert(CStateManager& mgr, const CTriggerData& data) const { return mAlert; }

bool CIng::IsControllingHost(CStateManager& mgr, const CTriggerData& data) const {
  return mHostId != kInvalidUniqueId;
}

bool CIng::IsFacingTarget(CStateManager& mgr, const CTriggerData& data) const {
  CVector3f targetPosition = CVector3f::Zero();
  if (GetTargetAimPosition(mgr, targetPosition, 0.f)) {
    CVector3f toTarget = targetPosition - GetTranslation();
    toTarget.SetZ(0.f);
    return CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) <= 20.f * (M_PIF / 180.f);
  }
  return true;
}

bool CIng::HasTarget(CStateManager& mgr, const CTriggerData& data) const {
  return mgr.GetObjectById(mTargetId) != nullptr;
}

bool CIng::HasNewTarget(CStateManager& mgr, const CTriggerData& data) const {
  bool hasNewTarget = false;
  if (HasTarget(mgr, data) && mLastTargetId != mTargetId) {
    hasNewTarget = true;
  }
  return hasNewTarget;
}

bool CIng::HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  bool hasLineOfSight = false;
  if (mLineOfSight.HasLineOfSight() && mLineOfSight.GetClearTime() > 0.5f) {
    hasLineOfSight = true;
  }
  return hasLineOfSight;
}

bool CIng::UnderFire(CStateManager& mgr, const CTriggerData& data) const {
  return mUnderFireTimer < 0.75f;
}

bool CIng::HeardShot(CStateManager& mgr, const CTriggerData& data) const {
  return mHeardShotTimer < 1.5f;
}

bool CIng::AreaClear(CStateManager& mgr, const CTriggerData& data) const {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CVector3f position = GetTranslation();
  const CVector3f extent = GetModelData()->GetScale() * 10.f;
  const CAABox bounds(position - extent, position + extent);
  const CMaterialFilter filter =
      CMaterialFilter::MakeInclude(CMaterialList(skAreaClearMaterial1, skAreaClearMaterial2));
  mgr.BuildNearList(nearList, bounds, filter, this);
  return nearList.empty();
}

bool CIng::CoverLeash(CStateManager& mgr, const CTriggerData& data) const {
  const CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    const CVector3f diff = GetTranslation() - hint->GetTranslation();
    return diff.MagSquared() > mData.coverLeashDistance * mData.coverLeashDistance;
  }
  return true;
}

bool CIng::HasCoverPoint(CStateManager& mgr, const CTriggerData& data) const {
  return GetCoverHint(mgr) != nullptr;
}

bool CIng::HasWallCoverPoint(CStateManager& mgr, const CTriggerData& data) const {
  const CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    return hint->GetHintType() == CScriptAIHint::kHT_WallCover;
  }
  return false;
}

bool CIng::HasMiniPortals(CStateManager& mgr, const CTriggerData& data) const {
  return mMiniPortalCount != 0;
}

bool CIng::IsCorporeal(CStateManager& mgr, const CTriggerData& data) const {
  return mForm == kF_Corporeal;
}

bool CIng::IsIngSpot(CStateManager& mgr, const CTriggerData& data) const {
  return mForm == kF_IngSpot;
}

bool CIng::IsBodyProjectile(CStateManager& mgr, const CTriggerData& data) const {
  return mForm == kF_BodyProjectile;
}

bool CIng::IsFrustrated(CStateManager& mgr, const CTriggerData& data) const {
  return mFrustrationTimer > mData.frustrationTime;
}

bool CIng::IsAggressive(CStateManager& mgr, const CTriggerData& data) const {
  bool aggressive = false;
  if (mAggressive || mSafeZoneCount > 0) {
    aggressive = true;
  }
  return aggressive;
}

bool CIng::UseProjectileFSMEntry(CStateManager& mgr, const CTriggerData& data) const {
  return mUseProjectileFSMEntry;
}

bool CIng::IsLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const {
  bool lured = false;
  if (mSafeZoneId != kInvalidUniqueId) {
    switch (mForm) {
    case kF_Corporeal:
    case kF_IngSpot:
      lured = true;
      break;
    case kF_BodyProjectile:
    case kF_Evaporating:
    case kF_BecomingCorporeal:
    case kF_BecomingIngSpot:
    case kF_BecomingBodyProjectile:
    case kF_BecomingWallProjectile:
    default:
      lured = false;
      break;
    }
  }
  return lured;
}

bool CIng::StillLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const {
  return mSafeZoneId != kInvalidUniqueId;
}

bool CIng::TargetInSafeZone(CStateManager& mgr, const CTriggerData& data) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    return mgr.GetSafeZoneManager()->IsObjectInSafeZone(*target, mgr);
  }
  return false;
}

bool CIng::TargetInLightSafeZone(CStateManager& mgr, const CTriggerData& data) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    return mgr.GetSafeZoneManager()->IsObjectInHurtfulSafeZone(*target, mgr);
  }
  return false;
}

bool CIng::EmergePtInSafeZone(CStateManager& mgr, const CTriggerData& data) const {
  const CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    return mgr.GetSafeZoneManager()->PointIsInSafeZone(mgr, hint->GetTranslation());
  }
  return false;
}

bool CIng::CanChangeForm(CStateManager& mgr, const CTriggerData& data) const {
  switch (mForm) {
  case kF_Corporeal:
    return !mData.disableFormChange && mFormChangeTimer > mData.formChangeInterval;
  case kF_IngSpot:
    return !mData.disableFormChange;
  case kF_Evaporating:
  case kF_BecomingCorporeal:
  case kF_BecomingIngSpot:
  case kF_BecomingBodyProjectile:
  case kF_BecomingWallProjectile:
    return false;
  case kF_PossessingHost:
  case kF_ExitingHost:
  case kF_BodyProjectile:
  default:
    return true;
  }
}

bool CIng::ShouldEvaporate(CStateManager& mgr, const CTriggerData& data) const {
  if (mShouldEvaporate) {
    switch (mForm) {
    case kF_Corporeal:
    case kF_IngSpot:
    case kF_Evaporating:
      return true;
    case kF_PossessingHost:
    case kF_ExitingHost:
    case kF_BodyProjectile:
    case kF_BecomingCorporeal:
    case kF_BecomingIngSpot:
    case kF_BecomingBodyProjectile:
    case kF_BecomingWallProjectile:
    default:
      return false;
    }
  }
  return false;
}

bool CIng::ShouldFleeSafeZone(CStateManager& mgr, const CTriggerData& data) const {
  return mSafeZoneCount > 0;
}

bool CIng::ShouldBecomeCorporeal(CStateManager& mgr, const CTriggerData& data) const {
  return mNextForm == kF_Corporeal && !IsCorporeal(mgr, data);
}

bool CIng::ShouldBecomeIngSpot(CStateManager& mgr, const CTriggerData& data) const {
  return mNextForm == kF_IngSpot && !IsIngSpot(mgr, data);
}

bool CIng::ShouldArmSwipe(CStateManager& mgr, const CTriggerData& data) const {
  if (mForm == kF_Corporeal && !mData.disableArmSwipe && mTimeSinceArmSwipe > 1.f) {
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                         GetUniqueId())) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f diff = target->GetTranslation() - GetTranslation();
        const float distSq = diff.MagSquared();
        const float minSq = mMinAttackRange * mMinAttackRange;
        const float maxSq = mMaxAttackRange * mMaxAttackRange;
        if (distSq >= minSq && distSq <= maxSq) {
          return CMath::AbsF(diff.GetZ()) < 3.f;
        }
      }
    }
  }
  return false;
}

bool CIng::ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const { return mShouldTaunt; }

bool CIng::ShouldBodyProjectile(CStateManager& mgr, const CTriggerData& data) const {
  if (!mData.disableBodyProjectile && !mData.disableFormChange && mCanBodyProjectile) {
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                         GetUniqueId())) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f diff = target->GetTranslation() - GetTranslation();
        const float distSq = diff.MagSquared();
        const float minSq =
            mData.bodyProjectile.minAttackDistance * mData.bodyProjectile.minAttackDistance;
        const float maxSq =
            mData.bodyProjectile.maxAttackDistance * mData.bodyProjectile.maxAttackDistance;
        if (distSq >= minSq && distSq <= maxSq) {
          return diff.GetZ() >= -15.f && diff.GetZ() <= 3.f;
        }
      }
    }
  }
  return false;
}

bool CIng::ShouldOpenMiniPortal(CStateManager& mgr, const CTriggerData& data) const {
  if (!mData.disableMiniPortal && mFormChangeTimer >= mData.formChangeInterval) {
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                         GetUniqueId())) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f diff = target->GetTranslation() - GetTranslation();
        const float distSq = diff.MagSquared();
        const float minSq = mData.miniPortal.minAttackDistance * mData.miniPortal.minAttackDistance;
        const float maxSq = mData.miniPortal.maxAttackDistance * mData.miniPortal.maxAttackDistance;
        bool inRange = false;
        if (distSq >= minSq && distSq <= maxSq) {
          inRange = true;
        }
        return inRange;
      }
    }
  }
  return false;
}

bool CIng::ProjectileSplat(CStateManager& mgr, const CTriggerData& data) const {
  return mProjectileSplat;
}

bool CIng::ProjectileHitTarget(CStateManager& mgr, const CTriggerData& data) const {
  const CPhysicsActor* target = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const CPlayer* player = TCastToConstPtr< CPlayer >(target);
    if (player != nullptr) {
      const CPlayer::EPlayerMorphBallState state = player->GetMorphballTransitionState();
      if (state != CPlayer::kMS_Unmorphed && state != CPlayer::kMS_Unmorphing) {
        return false;
      }
    }
    const CAABox targetBounds = target->GetBoundingBox();
    const CVector3f position = GetTranslation();
    const CVector3f extent = GetModelData()->GetScale() * 2.5f;
    const CAABox bounds = CAABox(position - extent, position + extent);
    return bounds.DoBoundsOverlap(targetBounds);
  }
  return false;
}

bool CIng::FoundMovementPos(CStateManager& mgr, const CTriggerData& data) const {
  return mFoundMovementPos;
}

bool CIng::PathShagged(CStateManager& mgr, const CTriggerData& data) const {
  bool pathShagged = false;
  if (mPathObstructed || CPatterned::PathShagged(mgr, data)) {
    pathShagged = true;
  }
  return pathShagged;
}

bool CIng::PathOver(CStateManager& mgr, const CTriggerData& data) const {
  bool pathOver = false;
  if (!mFoundMovementPos || CPatterned::PathOver(mgr, data)) {
    pathOver = true;
  }
  return pathOver;
}

bool CIng::PointPathOver(CStateManager& mgr, const CTriggerData& data) const {
  return mPointNavigation.IsPathOver(*this);
}

bool CIng::FoundPointPath(CStateManager& mgr, const CTriggerData& data) const {
  return mPointNavigation.HasPath(*this);
}

bool CIng::IsOffPath(CStateManager& mgr, const CTriggerData& data) const {
  return mPathFindSearch.OnPath(GetTranslation()) != CPathFindSearch::kR_Success;
}

bool CIng::HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CIng::AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const {
  return mFollowingWaypoint && mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CIng::TargetIsBall(CStateManager& mgr, const CTriggerData& data) const {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId));
  if (player != nullptr) {
    return (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                ? player->GetMorphballTransitionState()
                : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed;
  }
  return false;
}

bool CIng::InBallPursuitRange(CStateManager& mgr, const CTriggerData& data) const {
  if (!mData.disableGrapple && mGrappleCooldown <= 0.f) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const CVector3f diff = target->GetTranslation() - GetTranslation();
      return diff.MagSquared() < mData.grapple.pursuitRange * mData.grapple.pursuitRange;
    }
  }
  return false;
}

bool CIng::HasPathToTarget(CStateManager& mgr, const CTriggerData& data) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    return mPathFindSearch.PathExists(GetTranslation(), target->GetTranslation()) ==
           CPathFindSearch::kR_Success;
  }
  return false;
}

bool CIng::InGrappleRange(CStateManager& mgr, const CTriggerData& data) const {
  if (!mData.disableGrapple) {
    const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId));
    if (player != nullptr && (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                                  ? player->GetMorphballTransitionState()
                                  : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
      if (mSurfaceAlignment.GetSurface().GetNormal().GetZ() >= 0.9f) {
        return player->GetBoundingBox().DoBoundsOverlap(*mTouchBounds);
      }
    }
  }
  return false;
}

bool CIng::CanGrappleTarget(CStateManager& mgr, const CTriggerData& data) const {
  if (!mData.disableGrapple) {
    const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId));
    if (player != nullptr) {
      return player->GetAttachedActorId() == kInvalidUniqueId;
    }
  }
  return false;
}

void CIng::Start(CStateManager& mgr, EStateMsg msg, float dt) {}

void CIng::WaitForFSMTransition(CStateManager& mgr, EStateMsg msg, float dt) {}

void CIng::ExitHost(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    AddMaterial(kMT_Target, mgr);
    mForm = kF_ExitingHost;
    if (mPossessionEffectId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mPossessionEffectId);
    }
    if (mExitHostEffectId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mExitHostEffectId);
    }
    mExitHostEffectId = mgr.AllocateUniqueId();
    CIngExitHostEffect* effect = rs_new CIngExitHostEffect(
        mExitHostEffectId, CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
        GetTransform(), mDestination, GetModelData()->GetScale(), mData.swarm.exitHostSwarmEffect,
        mData.swarm.exitHostTrailEffect, mData.swarm.exitHostTrailLength, mData.swarm.exitHostSpeed,
        mTargetId, mData.swarm.exitHostHomingTime, mData.swarm.exitHostHomingStrength,
        mData.swarm.swarmMoveSound);
    if (effect != nullptr) {
      mgr.AddObject(*effect);
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    const ushort sound = TargetInSafeZone(mgr, CTriggerData(0.f))
                             ? mData.swarm.exitHostSafeZoneSound
                             : mData.swarm.exitHostSound;
    CSfxManager::AddEmitter(sound, GetTranslation(), 127, GetCurrentAreaId().Value(), true, false,
                            CSfxManager::kMedPriority);
    break;
  }
  case kStateMsg_Update: {
    const CIngExitHostEffect* effect =
        static_cast< const CIngExitHostEffect* >(mgr.GetObjectById(mExitHostEffectId));
    if (effect != nullptr && !effect->HasArrived()) {
      SetTranslation(effect->GetTranslation());
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  }
  case kStateMsg_Deactivate:
    SetTranslation(mDestination);
    mSurfaceAlignment.AlignNearPosition(*this, mgr, mDestination, 1.f);
    AddMaterial(kMT_Character, kMT_Solid, kMT_Orbit, mgr);
    mGrappleCooldown = mData.grapple.postWaitTime;
    SpawnExitHostSmoke(mgr);
    mForm = kF_IngSpot;
    break;
  default:
    break;
  }
}

void CIng::BecomeCorporeal(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mUseProjectileFSMEntry = false;
    if (mForm == kF_IngSpot) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mUsePortalPlane = true;
      CUnitVector3f normal(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes);
      mPortalPlane = CPlane(GetTranslation(), normal);
      mForm = kF_BecomingCorporeal;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    mCollisionActorManager->SetActive(mgr, true);
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_WorldUp);
    mLineOfSight.SetSegment(mHeadSegment);
    FaceSafeZoneOrTarget(mgr);
    mMovingOnSurface = false;
    Stop();
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
      mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    }
    Stop();
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mUsePortalPlane = false;
    mBlobEffectActive = false;
    mVerticalMovement = false;
    AddMaterial(kMT_GroundCollider, mgr);
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mForm = kF_Corporeal;
    mNextForm = kF_Invalid;
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_None);
    mFormChangeTimer = 0.f;
    CAnimData* animData = AnimationData();
    animData->SetEffectState(rstl::string_l(skEyesName), true, mgr);
    break;
  }
  default:
    break;
  }
}

void CIng::BecomeIngSpot(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mForm == kF_Corporeal) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mUsePortalPlane = true;
      CUnitVector3f normal(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes);
      mPortalPlane = CPlane(GetTranslation(), normal);
      mForm = kF_BecomingIngSpot;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
    mLocomotionTime = 0.f;
    mIngSpotHurt = false;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mForm = kF_IngSpot;
    mNextForm = kF_Invalid;
    mLineOfSight.SetSegment(CSegId::Null());
    mVerticalMovement = true;
    RemoveMaterial(kMT_GroundCollider, mgr);
    mCollisionActorManager->SetActive(mgr, false);
    mUsePortalPlane = false;
    mBlobEffectActive = true;
    CAnimData* animData = AnimationData();
    animData->SetEffectState(rstl::string_l(skEyesName), false, mgr);
    break;
  }
  default:
    break;
  }
}

void CIng::BecomeBodyProjectile(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mForm = kF_BecomingBodyProjectile;
    mTakeOffReceived = false;
    mNextForm = kF_Invalid;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_LoopAttack)) {
      mVerticalMovement = true;
      RemoveMaterial(kMT_GroundCollider, mgr);
      const CBCLoopAttackCmd cmd(pas::kLAT_Zero);
      mBodyController->CommandMgr().DeliverCmd(cmd);
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_LoopAttack) {
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
      if (mTakeOffReceived || mForm == kF_BodyProjectile) {
        mCollisionActorManager->SetActive(mgr, false);
        mAnimationState.SetState(CAnimationState::kAS_Over);
      } else if (GetTargetAimPosition(mgr, mDestination, 0.f)) {
        const CVector3f position = GetTranslation();
        mDestination.SetZ(position.GetZ());
        mBodyController->CommandMgr().SetTargetVector(mDestination - position);
      }
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mVerticalMovement = true;
    RemoveMaterial(kMT_GroundCollider, mgr);
    mCollisionActorManager->SetActive(mgr, false);
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    CAnimData* animData = AnimationData();
    animData->SetEffectState(rstl::string_l(skEyesName), false, mgr);
    break;
  }
  default:
    break;
  }
}

void CIng::BecomeWallProjectile(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mForm = kF_BecomingWallProjectile;
    mWallProjectileVisible = false;
    mTakeOffReceived = false;
    mNextForm = kF_Invalid;
    const CUnitVector3f normal(mSurfaceAlignment.GetSurface().GetNormal(), CUnitVector3f::kN_No);
    const CVector3f right = CVector3f::Cross(normal, CVector3f::Up());
    if (right.IsMagnitudeSafe()) {
      const CVector3f origin = GetTranslation() + normal;
      SetTransform(
          CTransform4f::FromColumns(right.AsNormalized(), normal, CVector3f::Up(), origin));
    }
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_None);
    mUsePortalPlane = true;
    const float height = GetBoundingBox().GetHeight();
    mPortalPlane = CPlane(GetTranslation() - height * normal, normal);
    mBodyController->EnableAnimation(true);
    mMovingOnSurface = false;
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_LoopAttack)) {
      mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_One));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_LoopAttack) {
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
      mWallProjectileVisible = true;
      if (mTakeOffReceived || mForm == kF_BodyProjectile) {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      } else if (GetTargetAimPosition(mgr, mDestination, 0.f)) {
        const CVector3f position = GetTranslation();
        mDestination.SetZ(position.GetZ());
        const CVector3f normal = mSurfaceAlignment.GetSurface().GetNormal();
        CVector3f toTarget = mDestination - position;
        if (CVector3f::GetAngleDiff(toTarget, normal) > M_PIF / 4.f) {
          toTarget = CVector3f::Slerp(normal, toTarget.AsNormalized(),
                                      CRelAngle::FromRadians(M_PIF / 4.f));
        }
        mBodyController->CommandMgr().SetTargetVector(toTarget);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mUsePortalPlane = false;
    mBlobEffectActive = false;
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    CAnimData* animData = AnimationData();
    animData->SetEffectState(rstl::string_l(skEyesName), false, mgr);
    break;
  default:
    break;
  }
}

void CIng::Alert(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mLastTargetId = mTargetId;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    Stop();
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Taunt)) {
      mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Four));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mFormChangeTimer = 0.f;
    break;
  default:
    break;
  }
}

void CIng::SafeZoneReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    CSfxManager::AddEmitter(mData.swarm.exitHostSafeZoneSound, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    break;
  case kStateMsg_Update:
    if (mInHurtfulSafeZone || mShouldEvaporate) {
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_KnockBack)) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_One));
      }
    } else if (mStateMachine->GetTime() > 1.f) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CIng::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Taunt)) {
      mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Two));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mShouldTaunt = false;
    break;
  default:
    break;
  }
}

void CIng::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    if (mTargetId != kInvalidUniqueId) {
      mAlert = true;
    }
    const float tauntChance = mData.tauntChance;
    mShouldTaunt = mgr.Random()->Range(0.f, 100.f) <= tauntChance;
    bool aggressive = false;
    if (!mData.disableArmSwipe) {
      const float aggressiveness = mData.aggressiveness;
      if (mgr.Random()->Range(0.f, 100.f) <= aggressiveness) {
        aggressive = true;
      }
    }
    mAggressive = aggressive;
    bool canBodyProjectile = false;
    if (!mData.disableBodyProjectile) {
      bool allowed = true;
      if (!mData.disableMiniPortal) {
        const float odds = mData.bodyProjectile.odds;
        if (!(mgr.Random()->Range(0.f, 100.f) <= odds)) {
          allowed = false;
        }
      }
      if (allowed) {
        canBodyProjectile = true;
      }
    }
    mCanBodyProjectile = canBodyProjectile;
    break;
  }
  case kStateMsg_Update:
    if (mForm == kF_IngSpot) {
      Stop();
    }
    break;
  default:
    break;
  }
}

void CIng::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  bool moved = false;
  switch (msg) {
  case kStateMsg_Activate: {
    CBodyController* controller = BodyController();
    const float runSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    const float walkSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk);
    const float ratio = walkSpeed / runSpeed;
    const bool fullSpeed = mAggressive || mSafeZoneId != kInvalidUniqueId;
    const float speed = fullSpeed ? 1.f : ratio;
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(speed, speed);
    if (mSafeZoneId != kInvalidUniqueId) {
      GetSearchPath()->SetFlags(1);
    }
    if (!mData.disablePathMovement && mFoundMovementPos) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      moved = true;
    }
    mPathObstructed = false;
    break;
  }
  case kStateMsg_Update:
    if (!mData.disablePathMovement && mFoundMovementPos) {
      if (mAggressive || !HasLineOfSight(mgr, CTriggerData(0.f)) || mSafeZoneCount > 0 ||
          mSafeZoneId != kInvalidUniqueId) {
        mPathFindNavigation.PathFind(mgr, msg, dt, *this);
        ApplySeparation(mgr);
        moved = true;
      }
    }
    break;
  case kStateMsg_Deactivate:
    GetSearchPath()->SetFlags(0x301);
    break;
  default:
    break;
  }
  if (!moved) {
    Stop();
  }
}

void CIng::IngSpotPathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mPathObstructed = false;
    mMovingOnSurface = true;
  }
  if (!mData.disablePathMovement && mFoundMovementPos) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    ApplySeparation(mgr);
    const CVector3f direction = mBodyController->CommandMgr().GetMoveVector();
    mBodyController->CommandMgr().ClearLocomotionCmds();
    if (direction.IsMagnitudeSafe() && dt > 0.f) {
      const CVector3f heading = direction.AsNormalized();
      float speed = mIngSpotHurt ? mData.ingSpot.GetHurtSpeed() : mData.ingSpot.GetMaxSpeed();
      if (TargetIsBall(mgr, CTriggerData(0.f))) {
        speed = mData.ingSpot.GetBallPursuitSpeed();
      }
      MoveAlongSurface(direction, speed, dt);
      mBodyController->FaceDirection(heading, dt);
    }
  }
}

void CIng::IngSpotPointPathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mPathObstructed = false;
    mMovingOnSurface = true;
  }
  if (!mData.disablePathMovement && mFoundMovementPos) {
    mPointNavigation.PathFind(mgr, msg, dt, *this);
    const CVector3f direction = mBodyController->CommandMgr().GetMoveVector();
    mBodyController->CommandMgr().ClearLocomotionCmds();
    const float angle = CVector3f::GetAngleDiff(CVector3f::Up(), GetTransform().GetUp());
    const float slowSpeed =
        mIngSpotHurt ? mData.ingSpot.GetHurtSpeed() : mData.ingSpot.GetMaxSpeed();
    const float fastSpeed = mData.ingSpot.GetMaxWallSpeed();
    MoveAlongSurface(direction, angle / M_PIF * (fastSpeed - slowSpeed) + slowSpeed, dt);
  }
}

void CIng::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CBodyController* controller = BodyController();
    const float runSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    const float walkSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk);
    const float ratio = walkSpeed / runSpeed;
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(ratio, ratio);
    break;
  }
  default:
    break;
  }
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
}

void CIng::FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  if (mForm == kF_IngSpot) {
    const CVector3f direction = mBodyController->CommandMgr().GetMoveVector();
    mBodyController->CommandMgr().ClearLocomotionCmds();
    const CScriptAIWaypoint* waypoint = TCastToPtr< CScriptAIWaypoint >(
        const_cast< CEntity* >(mgr.GetObjectById(mWaypointNavigation.GetDestination())));
    const float waypointSpeed = waypoint != nullptr ? waypoint->GetSpeed() : 1.f;
    MoveAlongSurface(direction, waypointSpeed * mData.ingSpot.GetMaxSpeed(), dt);
  }
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId id = mPatrolWaypointId != kInvalidUniqueId
                             ? mPatrolWaypointId
                             : GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    mWaypointNavigation.SetDestination(id);
    mFollowingWaypoint = true;
    break;
  }
  case kStateMsg_Update: {
    const CScriptAIWaypoint* waypoint =
        TCastToPtr< CScriptAIWaypoint >(mgr.ObjectById(mWaypointNavigation.GetDestination()));
    if (waypoint != nullptr) {
      const uint flags = waypoint->GetFlags();
      if ((flags & 0x40) != 0 || (flags & 0x80) != 0) {
        mNextForm = kF_IngSpot;
      } else if ((flags & 0x100) != 0) {
        mNextForm = kF_Corporeal;
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    mFollowingWaypoint = false;
    mPatrolWaypointId = mWaypointNavigation.GetDestination();
    break;
  default:
    break;
  }
}

void CIng::BodyProjectileFlight(CStateManager& mgr, EStateMsg msg, float dt) {
  static CVector3f skLaunchPosition = CVector3f::Zero();     // Guessed name
  static CVector3f skLaunchDirection = CVector3f::Forward(); // Guessed name
  switch (msg) {
  case kStateMsg_Activate: {
    mProjectileSplat = false;
    mUseProjectileFSMEntry = true;
    skLaunchPosition = GetLctrTransform(mHeadSegment).GetTranslation();
    skLaunchDirection = CVector3f::Forward();
    if (GetTargetAimPosition(mgr, mDestination, 0.5f)) {
      const CVector3f toTarget = (mDestination - skLaunchPosition) +
                                 GetModelData()->GetScale() * CVector3f(0.f, 0.f, -1.f);
      if (toTarget.GetZ() < 0.f && toTarget.IsMagnitudeSafe()) {
        skLaunchDirection = GetTransform().TransposeRotate(toTarget.AsNormalized());
      }
    }
    mSfxBodyProjectile =
        CSfxManager::AddEmitter(mData.bodyProjectile.sound, GetTranslation(), 127,
                                GetCurrentAreaId().Value(), true, true, CSfxManager::kMedPriority);
    break;
  }
  case kStateMsg_Update: {
    CSfxManager::UpdateEmitter(mSfxBodyProjectile, GetTranslation(), GetTransform().GetForward(),
                               127);
    if (mForm == kF_BodyProjectile) {
      if (dt > 0.f) {
        const float dropTime = mData.bodyProjectile.dropTime;
        if (mStateMachine->GetTime() > dropTime) {
          const float fall = dt * GetGravityConstant();
          skLaunchDirection.SetZ(skLaunchDirection.GetZ() - fall);
        }
        const CVector3f step = dt * mData.bodyProjectile.speed * skLaunchDirection;
        MoveInOneFrameOR(step, dt);
      }
    } else if (dt > 0.f) {
      CVector3f movement = dt * mData.bodyProjectile.speed *
                           CVector3f(skLaunchDirection.GetX(), skLaunchDirection.GetY(), 0.f);
      const CVector3f toLaunch(0.f, 0.f, skLaunchPosition.GetZ() - GetTranslation().GetZ());
      if (toLaunch.IsMagnitudeSafe()) {
        movement += dt * (10.f * toLaunch.AsNormalized());
      }
      MoveInOneFrameOR(movement, dt);
    }
    break;
  }
  case kStateMsg_Deactivate:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    CSfxManager::RemoveEmitter(mSfxBodyProjectile);
    mSfxBodyProjectile.Clear();
    break;
  default:
    break;
  }
}

void CIng::SuckEnergy(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mForm = kF_PossessingHost;
    AnimationData()->GetParticleDB().DestroyAllActiveParticles();
    AnimationData()->GetParticleDB().ClearAllNonPersistentEffects(&mgr);
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
    RemoveMaterial(kMT_Character, kMT_GroundCollider, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    mHostId = mTargetId;
    if (mPossessionHudEffect.valid()) {
      mPossessionEffectId = mgr.AllocateUniqueId();
      CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
          rstl::optional_object< TToken< CGenDescription > >(*mPossessionHudEffect),
          rstl::optional_object< TToken< CElectricDescription > >(), mPossessionEffectId, true,
          rstl::string_l("Ing Possessed Mold"), CHUDBillboardEffect::GetNearClipDistance(mgr, 0),
          CHUDBillboardEffect::GetScaleForPOV(mgr), 0, CColor::White(), CVector3f::One(),
          CVector3f::Zero(), true);
      mgr.AddObject(*effect);
    }
    const SIngBodyProjectileData& projectile = mData.bodyProjectile;
    CSfxManager::AddEmitter(projectile.splatWallSound, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    mgr.ApplyDamage(GetUniqueId(), mHostId, GetUniqueId(), projectile.contactDamage,
                    CMaterialFilter(), GetTransform().GetForward());
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mHostId));
    if (player != nullptr) {
      player->EnableLeaveMorphBall(false);
      player->EnableEnterMorphBall(false);
      player->AttachActorToPlayer(GetUniqueId(), false);
      CEnvironmentVariable* warnings =
          gpGameState->SystemOptions().EnvVars().FindEnvironmentVariable("IngAttachedWarningCount");
      if (warnings->GetValue() != warnings->GetMaximum()) {
        warnings->Set(warnings->GetValue() + 1);
        CSamusHud::DisplayHudMemo(
            rstl::wstring_l(gpStringTable->GetString("IngAttachedWarning")),
            CHUDMemoParms(5.f, true, false, false, 1 << player->GetPlayerIndex(), true));
      }
    }
    mSfxHostInside =
        CSfxManager::SfxStart(mData.swarm.insideHostSound, CAudioSys::kMaxVolume, 64,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    break;
  }
  case kStateMsg_Update: {
    const float suckTime = mData.bodyProjectile.suckTime;
    if (mStateMachine->GetTime() >= suckTime) {
      mHostId = kInvalidUniqueId;
    } else {
      CPlayer* player = TCastToPtr< CPlayer >(const_cast< CEntity* >(mgr.GetObjectById(mHostId)));
      if (player != nullptr) {
        SetTransform(player->GetTransform());
        SetTranslation(player->GetAimPosition(mgr, 0.f));
        const CDamageInfo damage(CWeaponMode(kWT_AI, false, false, false),
                                 dt * mData.bodyProjectile.suckDamagePerSecond, 0.f, 0.f, true);
        mgr.ApplyDamage(
            GetUniqueId(), mHostId, GetUniqueId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageMaterial), CMaterialList()),
            CVector3f::Zero());
      } else {
        mHostId = kInvalidUniqueId;
      }
    }
    break;
  }
  case kStateMsg_Deactivate: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
    if (player != nullptr) {
      player->EnableLeaveMorphBall(true);
      player->EnableEnterMorphBall(true);
      if (player->GetAttachedActorId() == GetUniqueId()) {
        player->DetachActorFromPlayer();
      }
    }
    CSfxManager::RemoveEmitter(mSfxHostInside);
    mSfxHostInside.Clear();
    mHostId = kInvalidUniqueId;
    break;
  }
  default:
    break;
  }
}

void CIng::SeekWallPoint(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
  case kStateMsg_Update: {
    const CScriptAIHint* hint = GetCoverHint(mgr);
    if (hint != nullptr) {
      const CVector3f toHint = hint->GetTranslation() - GetTranslation();
      if (toHint.Magnitude() > dt * mData.ingSpot.GetMaxWallSpeed() && toHint.IsMagnitudeSafe()) {
        MoveAlongSurface(toHint.AsNormalized(), mData.ingSpot.GetMaxWallSpeed(), dt);
        mMovingOnSurface = true;
      } else {
        mMovingOnSurface = false;
      }
    }
    break;
  }
  default:
    break;
  }
}

void CIng::SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mgr.GetObjectById(mTargetId) == nullptr) {
      mTargetId = mgr.GetPlayer(0)->GetUniqueId();
      mLineOfSight.SetTarget(mTargetId);
      SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  default:
    break;
  }
}

void CIng::FaceTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (GetTargetAimPosition(mgr, mDestination, 0.f)) {
      mDestination.SetZ(GetTranslation().GetZ());
      const CVector3f toTarget = mDestination - GetTranslation();
      if (CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) > 20.f * (M_PIF / 180.f)) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    mMovingOnSurface = false;
    break;
  case kStateMsg_Update:
    switch (mForm) {
    case kF_Corporeal:
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Turn)) {
        const CVector3f toTarget = mDestination - GetTranslation();
        if (toTarget.IsMagnitudeSafe()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
        } else {
          mAnimationState.SetState(CAnimationState::kAS_Over);
        }
      }
      break;
    case kF_IngSpot:
    case kF_BecomingCorporeal:
    case kF_BecomingIngSpot:
    case kF_BecomingBodyProjectile:
    case kF_BecomingWallProjectile: {
      const CVector3f toTarget = mDestination - GetTranslation();
      if (toTarget.IsMagnitudeSafe()) {
        if (CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) >
            20.f * (M_PIF / 180.f)) {
          mBodyController->FaceDirection(toTarget.AsNormalized(), dt);
        } else {
          mAnimationState.SetState(CAnimationState::kAS_Over);
        }
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
      break;
    }
    default:
      mAnimationState.SetState(CAnimationState::kAS_Over);
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CIng::FindCoverPoint(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    ReleaseCoverHint(mgr);
    if (!GetTargetAimPosition(mgr, mDestination, 0.f)) {
      return;
    }
    static rstl::reserved_vector< rstl::pair< TUniqueId, float >, 4 > candidates;
    candidates.clear();
    const float nearRange = rstl::max_val(mMaxAttackRange, mData.bodyProjectile.minAttackDistance);
    const float nearRangeSq = nearRange * nearRange;
    const float farRange =
        rstl::max_val(mData.bodyProjectile.maxAttackDistance, mData.miniPortal.maxAttackDistance);
    const float farRangeSq = farRange * farRange;
    const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
    const bool inLightSafeZone = TargetInLightSafeZone(mgr, CTriggerData(0.f));
    const CSafeZoneManager* safeZones = mgr.GetSafeZoneManager();
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
      if (hint == nullptr || !hint->GetActive()) {
        continue;
      }
      if (hint->GetHintType() != CScriptAIHint::kHT_Cover &&
          hint->GetHintType() != CScriptAIHint::kHT_WallCover) {
        continue;
      }
      if (hint->GetInUse(GetUniqueId()) || hint->GetCurrentAreaId() != GetCurrentAreaId() ||
          hint->GetUniqueId() == mLastCoverHintId ||
          safeZones->PointIsInSafeZone(mgr, hint->GetTranslation())) {
        continue;
      }
      if (hint->GetHintType() == CScriptAIHint::kHT_WallCover &&
          (mData.disableBodyProjectile || inLightSafeZone)) {
        continue;
      }
      const CVector3f delta = mDestination - hint->GetTranslation();
      float distSq = delta.MagSquared();
      const float height = CVector3f::Dot(CVector3f::Up(), delta);
      if (hint->GetHintType() != CScriptAIHint::kHT_WallCover || height > 0.f) {
        distSq += 4.f * height * height;
      }
      if (distSq < nearRangeSq) {
        distSq += farRangeSq;
      }
      if (candidates.size() < 4) {
        candidates.push_back(rstl::pair< TUniqueId, float >(hint->GetUniqueId(), distSq));
      } else {
        rstl::pair< TUniqueId, float >& worst = candidates[candidates.size() - 1];
        if (distSq < worst.second) {
          worst = rstl::pair< TUniqueId, float >(hint->GetUniqueId(), distSq);
        }
      }
      for (int j = candidates.size() - 1; j > 0; --j) {
        if (candidates[j].second < candidates[j - 1].second) {
          rstl::swap(candidates[j], candidates[j - 1]);
        }
      }
    }
    if (candidates.size() != 0) {
      const TUniqueId pickedId = candidates[mgr.Random()->Range(0, candidates.size() - 1)].first;
      CScriptAIHint* hint = static_cast< CScriptAIHint* >(mgr.ObjectById(pickedId));
      if (hint != nullptr) {
        AssignCoverHint(*hint);
      }
    }
    break;
  }
  default:
    break;
  }
}

void CIng::ArmSwipe(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (GetTargetAimPosition(mgr, mDestination, 0.f)) {
      if (mTeamManagerId == kInvalidUniqueId ||
          CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                        GetUniqueId())) {
        mFormChangeTimer += mData.formChangeInterval;
        mAnimationState.SetState(CAnimationState::kAS_Ready);
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_MeleeAttack)) {
      static const pas::ESeverity skSwipeSeverity[] = {pas::kS_One, pas::kS_Two};
      mSwipeIndex = RollDoubleSwipe(mgr);
      mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(skSwipeSeverity[mSwipeIndex]));
      mSwipeDamagePending = true;
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
      const CVector3f right = GetTransform().GetRight();
      ApplySwipeDamage(mgr, mRightShoulderSegment, mRightElbowSegment, mRightForearmSegment,
                       mRightWristSegment, -right);
      if (mSwipeIndex == 1) {
        ApplySwipeDamage(mgr, mLeftShoulderSegment, mLeftElbowSegment, mLeftForearmSegment,
                         mLeftWristSegment, right);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mSwipeIndex = -1;
    mSwipeDamagePending = false;
    mTimeSinceArmSwipe = 0.f;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId, GetUniqueId(),
                                false);
    break;
  default:
    break;
  }
}

void CIng::FailSafeMode(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    const CPathFindPointSearchFilter filter(1000.f, 0, -1);
    int point = -1;
    if (mPointSearch.FindClosestPhysicalPoint(GetTranslation(), point, filter) ==
        CPathFindPointSearch::kCPR_Success) {
      SetTranslation(mPointSearch.GetArea()->GetPoint(point).GetPosition());
      mSurfaceAlignment.AlignNearPosition(*this, mgr, GetTranslation(), 1.f);
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  }
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CIng::GrappleBall(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mGrappling = true;
    mGrappleHold = 0.f;
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
    if (player != nullptr) {
      player->EnableLeaveMorphBall(false);
      player->EnableEnterMorphBall(false);
      player->AttachActorToPlayer(GetUniqueId(), false);
    }
    mSfxGrapple =
        CSfxManager::AddEmitter(mData.swarm.insideHostSound, GetTranslation(), 127,
                                GetCurrentAreaId().Value(), true, true, CSfxManager::kMedPriority);
    mMovingOnSurface = false;
    break;
  }
  case kStateMsg_Update: {
    CSfxManager::UpdateEmitter(mSfxGrapple, GetTranslation(), GetTransform().GetForward(), 127);
    const float maxHoldTime = mData.grapple.maxHoldTime;
    if (mStateMachine->GetTime() <= maxHoldTime && mGrappleHold >= 0.f) {
      CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
      if (player != nullptr && (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                                    ? player->GetMorphballTransitionState()
                                    : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
        player->Stop();
        const CVector3f offset = player->GetTranslation() - GetTranslation();
        if (offset.IsMagnitudeSafe()) {
          MoveAlongSurface(offset.AsNormalized(), mData.ingSpot.GetMaxWallSpeed(), dt);
        } else {
          Stop();
        }
        const CDamageInfo damage(CWeaponMode(kWT_AI, false, false, false),
                                 dt * mData.grapple.holdDamagePerSecond, 0.f, 0.f, true);
        mgr.ApplyDamage(
            GetUniqueId(), mTargetId, GetUniqueId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageMaterial), CMaterialList()),
            CVector3f::Zero());
        mGrappleHold = CMath::Min(1.f, 0.3f * dt + mGrappleHold);
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  }
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mGrappling = false;
    mGrappleCooldown = mData.grapple.postWaitTime;
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
    if (player != nullptr) {
      player->EnableLeaveMorphBall(true);
      player->EnableEnterMorphBall(true);
      if (player->GetAttachedActorId() == GetUniqueId()) {
        player->DetachActorFromPlayer();
      }
    }
    CSfxManager::RemoveEmitter(mSfxGrapple);
    mSfxGrapple.Clear();
    break;
  }
  default:
    break;
  }
}

void CIng::FindMiniPortals(CStateManager& mgr, EStateMsg msg, float dt) {
  static uint skAngleIndex = 0; // Guessed name
  static const float skAnglesDegrees[8] = {180.f, 0.f, 135.f, 45.f, -135.f, -45.f, 90.f, -90.f};
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mMiniPortalCount = 0;
    skAngleIndex = 0;
    if (!GetTargetAimPosition(mgr, mDestination, 0.f)) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mMiniPortalCount < 3 && skAngleIndex < 8) {
      const CTransform4f& headTransform = GetLctrTransform(mHeadSegment);
      const CVector3f scale = GetModelData()->GetScale();
      const CVector3f center =
          headTransform.GetTranslation() + scale.GetY() * (3.f * GetTransform().GetForward());
      const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(skPortalRayInclude),
          CMaterialList(skPortalRayExclude1, skPortalRayExclude2, skPortalRayExclude3,
                        skPortalRayExclude4));
      for (uint i = 0; i < 2; ++i) {
        const float angle = (M_PIF / 180.f) * skAnglesDegrees[skAngleIndex++];
        const float cosine = CMath::FastCosR(angle);
        const float sine = CMath::FastSinR(angle);
        const CVector3f point =
            center + GetTransform().Rotate(2.f * (scale * CVector3f(cosine, 0.f, sine)));
        if (mgr.RayCollideWorld(point, mDestination, filter, this)) {
          mMiniPortalPositions[mMiniPortalCount] = point;
          ++mMiniPortalCount;
          if (mMiniPortalCount == 3) {
            mAnimationState.SetState(CAnimationState::kAS_Over);
            break;
          }
        }
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CIng::MiniPortalAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mMiniPortalIndex = 0;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_ProjectileAttack)) {
      mBodyController->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(
          pas::kS_One, GetTranslation() + GetTransform().GetForward(), false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mMiniPortalIndex = -1;
    break;
  default:
    break;
  }
}

void CIng::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (mForm) {
  case kF_IngSpot:
  case kF_BodyProjectile:
  case kF_Evaporating:
    if (msg == kStateMsg_Activate) {
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      if (mIngSpotDeathEffect) {
        CExplosion* explosion =
            rs_new CExplosion(*mIngSpotDeathEffect, mgr.AllocateUniqueId(),
                              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                              rstl::string_l("Ing death Fx"), GetTransform(), 0,
                              GetModelData()->GetScale(), CColor::White(), -1);
        if (explosion != nullptr) {
          mgr.AddObject(explosion);
        }
      }
      CSfxManager::AddEmitter(mData.ingSpot.GetDeathSound(), GetTranslation(), 127,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
      DeathDelete(mgr);
    }
    break;
  case kF_PossessingHost:
  case kF_ExitingHost:
    if (msg == kStateMsg_Activate) {
      DeathDelete(mgr);
    }
    break;
  default:
    CPatterned::Dead(mgr, msg, dt);
    if (!mBurning && !mLaggedBurnDeath) {
      switch (msg) {
      case kStateMsg_Activate:
        mDeathDelayTimer = 2.f;
        RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
        mDamageCooldownTimer = skDamageHitTime;
        break;
      case kStateMsg_Update:
        mAlphaDelta = 0.f;
        mSurfaceAlignment.AlignNearPosition(*this, mgr, GetTranslation(), dt);
        if (mBodyController->GetCurrentStateId() == pas::kAS_Death) {
          CVector3f newScale = GetModelData()->GetScale();
          const float height = newScale.GetZ() - dt / 0.75f;
          newScale.SetZ(CMath::Max(height, 0.f));
          ModelData()->SetScale(newScale);
          if (height <= 0.f) {
            mDeathDelayTimer -= dt;
            mDrawModel = false;
            mBlobEffectActive = false;
          }
          if (mDeathDelayTimer <= 0.f) {
            DeathDelete(mgr);
          }
        }
        break;
      default:
        break;
      }
    }
    break;
  }
}

void CIng::Evaporate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mForm = kF_Evaporating;
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() > 2.f) {
      DeathDelete(mgr);
    }
    break;
  default:
    break;
  }
}

void CIng::SetTargetDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mFoundMovementPos = false;
  const CEntity* target = mgr.GetObjectById(mTargetId);
  if (target != nullptr) {
    destination = static_cast< const CActor* >(target)->GetTranslation();
    mFoundMovementPos = true;
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
  JoinTeam(mgr);
}

void CIng::SetCoverDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mFoundMovementPos = false;
  const CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    destination = hint->GetTranslation();
    mFoundMovementPos = true;
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
}

void CIng::SetRecoverDest(CStateManager& mgr, float dt) {
  mFoundMovementPos = false;
  mPointSearch.ClearWaypoints();
  int nearest = -1;
  const CPathFindPointSearchFilter nearestFilter(20.f, 0, -1);
  mPointSearch.FindClosestPhysicalPoint(GetTranslation(), nearest, nearestFilter);
  if (nearest != -1) {
    int target = -1;
    const CPathFindPointSearchFilter targetFilter(20.f, 1, nearest);
    mPointSearch.FindClosestPhysicalPoint(GetTranslation(), target, targetFilter);
    if (target != -1) {
      if (mPointSearch.Search(mPointSearch.GetArea()->GetPoint(nearest),
                              mPointSearch.GetArea()->GetPoint(target)) ==
          CPathFindPointSearch::kR_Success) {
        mFoundMovementPos = true;
      }
    }
  }
}

void CIng::SetPointCoverDest(CStateManager& mgr, float dt) {
  mFoundMovementPos = false;
  mPointSearch.ClearWaypoints();
  const CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    int nearest = -1;
    const CPathFindPointSearchFilter filter(20.f, 0, -1);
    mPointSearch.FindClosestPhysicalPoint(GetTranslation(), nearest, filter);
    if (nearest != -1) {
      const CVector3f hintPosition = hint->GetTranslation();
      int best = -1;
      float bestDistanceSq = 400.f;
      for (int i = 0; i < mPointSearch.GetArea()->GetNumPoints(); ++i) {
        const CPFPoint& point = mPointSearch.GetArea()->GetPoint(i);
        if ((point.GetFlags() & 1) && mPointSearch.GetArea()->PointPathExists(nearest, i)) {
          const CVector3f delta = hintPosition - point.GetPosition();
          const float distanceSq = CVector3f::Dot(delta, delta);
          if (distanceSq < bestDistanceSq &&
              mPathFindSearch.PathExists(point.GetPosition(), hintPosition) ==
                  CPathFindSearch::kR_Success) {
            bestDistanceSq = distanceSq;
            best = i;
          }
        }
      }
      if (best != -1) {
        if (mPointSearch.Search(mPointSearch.GetArea()->GetPoint(nearest),
                                mPointSearch.GetArea()->GetPoint(best)) ==
            CPathFindPointSearch::kR_Success) {
          mFoundMovementPos = true;
        }
      }
    }
  }
}

void CIng::SetWallPointCoverDest(CStateManager& mgr, float dt) {
  mFoundMovementPos = false;
  mPointSearch.ClearWaypoints();
  const CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    int nearest = -1;
    const CPathFindPointSearchFilter nearestFilter(20.f, 0, -1);
    mPointSearch.FindClosestPhysicalPoint(GetTranslation(), nearest, nearestFilter);
    if (nearest != -1) {
      const CPathFindPointSearchFilter targetFilter(20.f, 0, nearest);
      int target = -1;
      mPointSearch.FindClosestPhysicalPoint(hint->GetTranslation(), target, targetFilter);
      if (target != -1) {
        if (mPointSearch.Search(mPointSearch.GetArea()->GetPoint(nearest),
                                mPointSearch.GetArea()->GetPoint(target)) ==
            CPathFindPointSearch::kR_Success) {
          mFoundMovementPos = true;
        }
      }
    }
  }
}

void CIng::SetExitHostDest(CStateManager& mgr, float dt) {
  const CPlayer* player = mgr.GetPlayer(0);
  float bestWeight = 0.f;
  const CVector3f playerForward = player->GetTransform().GetForward();
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  CVector3f destination = player->GetTranslation();
  const CVector3f position = GetTranslation();
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetHintType() != CScriptAIHint::kHT_WallCover) {
      const CVector3f hintPosition = hint->GetTranslation();
      const CVector3f offset = hintPosition - position;
      const float angle = CVector3f::GetAngleDiff(playerForward, offset);
      const float weight = (1.f + angle) * offset.MagSquared();
      if ((weight < bestWeight && weight >= 400.f) ||
          (weight > bestWeight && bestWeight <= 400.f)) {
        bestWeight = weight;
        destination = hintPosition;
      }
    }
  }
  mDestination = destination;
}

void CIng::SetLuredDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mFoundMovementPos = false;
  CVector3f luredPosition = destination;
  if (FindSafeZoneFloorPoint(mgr, luredPosition)) {
    GetSearchPath()->SetFlags(1);
    if (mPathFindSearch.PathExists(GetTranslation(), luredPosition) ==
        CPathFindSearch::kR_Success) {
      destination = luredPosition;
      mFoundMovementPos = true;
    }
    GetSearchPath()->SetFlags(0x301);
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
}

void CIng::SetLuredPointDest(CStateManager& mgr, float dt) {
  mFoundMovementPos = false;
  mPointSearch.ClearWaypoints();
  const CVector3f position = GetTranslation();
  CVector3f luredPosition = position;
  if (FindSafeZoneFloorPoint(mgr, luredPosition)) {
    GetSearchPath()->SetFlags(1);
    int nearest = -1;
    const CPathFindPointSearchFilter filter(20.f, 0, -1);
    mPointSearch.FindClosestPhysicalPoint(GetTranslation(), nearest, filter);
    if (nearest != -1) {
      int best = -1;
      float bestDistanceSq = FLT_MAX;
      for (int i = 0; i < mPointSearch.GetArea()->GetNumPoints(); ++i) {
        const CPFPoint& point = mPointSearch.GetArea()->GetPoint(i);
        if ((point.GetFlags() & 1) && mPointSearch.GetArea()->PointPathExists(nearest, i)) {
          const CVector3f delta = position - point.GetPosition();
          const float distanceSq = CVector3f::Dot(delta, delta);
          if (distanceSq < bestDistanceSq &&
              mPathFindSearch.PathExists(point.GetPosition(), luredPosition) ==
                  CPathFindSearch::kR_Success) {
            bestDistanceSq = distanceSq;
            best = i;
          }
        }
      }
      if (best != -1) {
        if (mPointSearch.Search(mPointSearch.GetArea()->GetPoint(nearest),
                                mPointSearch.GetArea()->GetPoint(best)) ==
            CPathFindPointSearch::kR_Success) {
          mFoundMovementPos = true;
        }
      }
    }
    GetSearchPath()->SetFlags(0x301);
  }
}

void CIng::StartRangedAttack(CStateManager& mgr, float dt) {
  CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                GetUniqueId());
}

void CIng::EndRangedAttack(CStateManager& mgr, float dt) {
  CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId, GetUniqueId(),
                              false);
}

void CIng::SplatOntoMesh(CStateManager& mgr, float dt) {
  mForm = kF_IngSpot;
  mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
  const CVector3f normal = mSplatNormal;
  mSurfaceAlignment.OrientToSurfaceNormal(*this, normal, 1.f);
  CSfxManager::AddEmitter(mData.bodyProjectile.splatWallSound, GetTranslation(), 127,
                          GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
  if (mSplatEffect) {
    CExplosion* explosion =
        rs_new CExplosion(*mSplatEffect, mgr.AllocateUniqueId(),
                          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                          rstl::string_l("Ing Splat Wall Fx"), GetTransform(), 0,
                          GetModelData()->GetScale(), CColor::White(), -1);
    if (explosion != nullptr) {
      mgr.AddObject(explosion);
    }
  }
}

void CIng::ExpelGrappledBall(CStateManager& mgr, float dt) {
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
  if (player != nullptr && (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                                ? player->GetMorphballTransitionState()
                                : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
    if (mGrappleHold >= 0.f) {
      mgr.ApplyDamage(
          GetUniqueId(), mTargetId, GetUniqueId(), mData.grapple.exitDamage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageMaterial), CMaterialList()),
          CVector3f::Zero());
      mCurDamageRemTime = mDamageWaitTime;
    }
    player->Stop();
    const CVector3f direction = CVector3f::Slerp(CVector3f::Up(), GetTransform().GetForward(),
                                                 CRelAngle::FromDegrees(20.f));
    const CVector3f impulse = player->GetMass() * (mData.grapple.spitForce * direction);
    player->ApplyImpulseWR(impulse, CAxisAngle::Identity());
    player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    CSfxManager::AddEmitter(mData.grapple.exitSound, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
  }
  mGrappleHold = 0.f;
  mMovingOnSurface = false;
}

void CIng::SetInitialForm(CStateManager& mgr) {
  mForm = mData.startsAsIngSpot ? kF_IngSpot : kF_Corporeal;
  if (mData.startsAsIngSpot) {
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mSurfaceAlignment.AlignNearPosition(*this, mgr, GetTranslation(), 1.f);
  } else {
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
  }
}

void CIng::CreateLight(CStateManager& mgr) {
  const TUniqueId lightId = mgr.AllocateUniqueId();
  CGameLight* light =
      rs_new CGameLight(lightId, GetAreaIdForPersistence(), GetActive(),
                        rstl::string_l("Ing Light"), CTransform4f::Identity(), GetUniqueId(),
                        CLight::BuildPoint(CVector3f::Zero(), CColor::Black()), 0, 0, 0.f);
  if (light != nullptr) {
    mLightId = lightId;
    mgr.AddObject(light);
  }
}

void CIng::UpdateLight(float dt, CStateManager& mgr) {
  if (mAlive && mForm == kF_Corporeal) {
    mLightIntensity = CMath::Min(1.f, mLightIntensity + dt);
  } else {
    mLightIntensity = CMath::Max(mLightIntensity - dt, 0.f);
  }
  CGameLight* gameLight = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  if (gameLight != nullptr) {
    const CColor color = CColor::Lerp(CColor::Black(), mData.lightColor, mLightIntensity);
    const CVector3f position = GetLctrTransform(mHeadSegment).GetTranslation();
    CLight light = CLight::BuildPoint(position, color);
    light.SetAttenuation(0.f, 1.f / mData.lightAttenuation, 0.f);
    gameLight->SetLight(light);
  }
}

void CIng::SpawnBlobEffect(CStateManager& mgr, const TLockedToken< CGenDescription >& desc) {
  mBlobEffectId = mgr.AllocateUniqueId();
  if (mBlobEffectId != kInvalidUniqueId) {
    CCollisionTracker* effect = rs_new CCollisionTracker(
        desc, mBlobEffectId, GetCurrentAreaId(), true, rstl::string_l("IngBlobEffect"),
        CTransform4f::Translate(GetTranslation()), GetUniqueId(), 0,
        CCollisionTracker::skDefaultExtents);
    effect->SetParticleEmissionRateScalar(0.f);
    effect->SetNextDrawNode(GetUniqueId());
    mgr.AddObject(effect);
  }
}

void CIng::UpdateBlobEffect(float dt, CStateManager& mgr) {
  CCollisionTracker* effect = static_cast< CCollisionTracker* >(mgr.ObjectById(mBlobEffectId));
  if (effect != nullptr) {
    float rate = 0.f;
    switch (mForm) {
    case kF_IngSpot:
      rate = 1.f;
      break;
    case kF_BecomingCorporeal:
    case kF_BecomingIngSpot:
    case kF_BecomingBodyProjectile:
    case kF_BecomingWallProjectile:
      if (mBlobEffectActive) {
        rate = 1.f;
      }
      break;
    case kF_Corporeal:
      if (!mAlive && mBlobEffectActive) {
        rate = 1.f;
      }
      break;
    default:
      break;
    }
    if (rate > 0.f) {
      const CVector3f normal = mSurfaceAlignment.GetSurface().GetNormal();
      const CVector3f position = GetTranslation();
      const CVector3f& up =
          CMath::AbsF(normal.GetZ() < 0.95f) != 0.f ? CVector3f::Up() : CVector3f::Right();
      effect->SetTransform(CTransform4f::LookAt(position, position + normal, up));
      effect->SetModelFlags(
          CModelFlags(CModelFlags::kT_Two,
                      CColor(mColor.GetRedu8(), mColor.GetGreenu8(), mColor.GetBlueu8(), 255)));
    }
    effect->SetParticleEmissionRateScalar(rate);
    effect->SetDamageHighlight(mDamageCooldownTimer > 0.f);
  }
}

CScriptAIHint* CIng::GetCoverHint(CStateManager& mgr) const {
  CScriptAIHint* hint = nullptr;
  if (mCoverHintId != kInvalidUniqueId) {
    hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mCoverHintId));
  }
  return hint;
}

void CIng::ReleaseCoverHint(CStateManager& mgr) {
  CScriptAIHint* hint = GetCoverHint(mgr);
  if (hint != nullptr) {
    hint->SetInUse(false);
    hint->SetTimeRemaining(0.f);
    mCoverHintId = kInvalidUniqueId;
  }
}

void CIng::AssignCoverHint(CScriptAIHint& hint) {
  hint.SetInUse(true);
  mCoverHintId = hint.GetUniqueId();
  mLastCoverHintId = mCoverHintId;
}

bool CIng::GetTargetAimPosition(CStateManager& mgr, CVector3f& position, float dt) const {
  const CEntity* target = mgr.GetObjectById(mTargetId);
  if (target != nullptr) {
    position = static_cast< const CActor* >(target)->GetAimPosition(mgr, dt);
    return true;
  }
  return false;
}

void CIng::UpdateTimers(float dt, CStateManager& mgr) {
  mHeardShotTimer += dt;
  mTimeSinceArmSwipe += dt;
  mFormChangeTimer += dt;
  if (mForm == kF_BodyProjectile) {
    mProjectileFlightTime += dt;
  } else {
    mProjectileFlightTime = 0.f;
  }
  if (mForm == kF_Corporeal && PathShagged(mgr, CTriggerData(0.f))) {
    mFrustrationTimer += dt;
  } else {
    mFrustrationTimer = 0.f;
  }
  if (mForm == kF_IngSpot && mCurDamageRemTime > 0.f) {
    mCurDamageRemTime -= dt;
  }
  mGrappleCooldown -= dt;
  if (mHitByPlayerProjectile) {
    mUnderFireTimer = 0.f;
    mHitByPlayerProjectile = false;
  } else {
    mUnderFireTimer += dt;
  }
}

void CIng::UpdateTargetable(CStateManager& mgr) {
  switch (mForm) {
  case kF_PossessingHost:
  case kF_ExitingHost:
    SetValidTarget(0, false);
    break;
  case kF_IngSpot:
  case kF_Evaporating: {
    const bool darkVisor = mgr.GetPlayerState(0)->GetActiveVisor(mgr) == CPlayerState::kPV_Dark;
    SetValidTarget(0, darkVisor || mData.alwaysTargetable);
    break;
  }
  case kF_Corporeal:
  case kF_BodyProjectile:
  case kF_BecomingCorporeal:
  case kF_BecomingIngSpot:
  case kF_BecomingBodyProjectile:
  case kF_BecomingWallProjectile:
  default:
    SetValidTarget(0, true);
    break;
  }
}

int CIng::RollDoubleSwipe(CStateManager& mgr) { return mgr.Random()->Range(0.f, 100.f) >= 50.f; }

void CIng::ApplySwipeDamage(CStateManager& mgr, const CSegId& shoulder, const CSegId& elbow,
                            const CSegId& forearm, const CSegId& wrist,
                            const CVector3f& direction) {
  if (mSwipeDamagePending) {
    const CPhysicsActor* target =
        TCastToPtr< CPhysicsActor >(const_cast< CEntity* >(mgr.GetObjectById(mTargetId)));
    if (target != nullptr) {
      const CAABox targetBounds = target->GetBoundingBox();
      bool hit = true;
      const CVector3f shoulderPos = GetLctrTransform(shoulder).GetTranslation();
      const CVector3f elbowPos = GetLctrTransform(elbow).GetTranslation();
      const CAABox upperArmBounds = GetSwipeSegmentBounds(3.f, shoulderPos, elbowPos);
      if (!targetBounds.DoBoundsOverlap(upperArmBounds)) {
        const CVector3f forearmPos = GetLctrTransform(forearm).GetTranslation();
        const CAABox lowerArmBounds = GetSwipeSegmentBounds(3.f, elbowPos, forearmPos);
        if (!targetBounds.DoBoundsOverlap(lowerArmBounds)) {
          const CVector3f wristPos = GetLctrTransform(wrist).GetTranslation();
          const CAABox handBounds = GetSwipeSegmentBounds(3.f, forearmPos, wristPos);
          if (!targetBounds.DoBoundsOverlap(handBounds)) {
            const CVector3f delta = wristPos - forearmPos;
            if (delta.IsMagnitudeSafe()) {
              const CVector3f tip = wristPos + 6.f * delta.AsNormalized();
              const CAABox reachBounds = GetSwipeSegmentBounds(3.f, wristPos, tip);
              hit = targetBounds.DoBoundsOverlap(reachBounds);
            } else {
              hit = false;
            }
          }
        }
      }
      if (hit) {
        mgr.ApplyDamage(GetUniqueId(), mTargetId, GetUniqueId(), mData.armSwipeDamage,
                        CMaterialFilter::MakeIncludeExclude(CMaterialList(skSwipeDamageMaterial),
                                                            CMaterialList()),
                        direction);
        mCurDamageRemTime = mDamageWaitTime;
        mSwipeDamagePending = false;
      }
    }
  }
}

CAABox CIng::GetSwipeSegmentBounds(float radius, const CVector3f& a, const CVector3f& b) const {
  return CAABox(rstl::min_val(a.GetX(), b.GetX()), rstl::min_val(a.GetY(), b.GetY()),
                rstl::min_val(a.GetZ(), b.GetZ()) - radius, rstl::max_val(a.GetX(), b.GetX()),
                rstl::max_val(a.GetY(), b.GetY()), rstl::max_val(a.GetZ(), b.GetZ()) + radius);
}

void CIng::UpdateTouchBounds() {
  const CVector3f position = GetTranslation();
  mTouchBounds =
      CAABox(position + CVector3f(-2.5f, -2.5f, 0.f), position + CVector3f(2.5f, 2.5f, 1.5f));
}

void CIng::UpdateStateMachine(float dt, CStateManager& mgr) {
  if (mBodyController->GetCurrentStateId() != pas::kAS_Locomotion) {
    mLocomotionTime = 0.f;
  } else {
    mLocomotionTime += dt;
  }
  if (mLocomotionTime < 2.5f) {
    mBodyController->Update(dt, mgr);
    UpdateAnimation(dt, mgr, true);
  }
  if (!mStateMachine->HasState()) {
    InitializeStateMachine(mgr);
  } else {
    mStateMachine->Update(mgr, *this, dt);
  }
  mWaypointNavigation.Update(dt);
}

void CIng::MoveAlongSurface(const CVector3f& direction, float speed, float dt) {
  const CVector3f position = GetTranslation();
  const CVector3f normal = mSurfaceAlignment.GetSurface().GetNormal();
  const float dot = CVector3f::Dot(direction, normal);
  const CVector3f flat =
      dot * dot < 0.6f * direction.MagSquared() ? direction - dot * normal : direction;
  if (flat.IsMagnitudeSafe()) {
    const CVector3f heading = flat.AsNormalized();
    SetTranslation(position + heading * speed * dt);
    mMoveHeading = heading;
  } else {
    mMoveHeading = CVector3f::Zero();
  }
  Stop();
}

void CIng::ApplySeparation(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const float radius = 15.f * GetModelData()->GetScale().GetX();
        const CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), radius);
        if (separation.IsMagnitudeSafe()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(separation.AsNormalized(), CVector3f::Zero(), 0.5f));
        }
      }
    }
  }
}

void CIng::JoinTeam(CStateManager& mgr) {
  if (mTeamManagerId == kInvalidUniqueId) {
    mTeamManagerId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
    if (mTeamManagerId != kInvalidUniqueId) {
      CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamManagerId));
      if (team != nullptr) {
        team->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                       CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CIng::LeaveTeam(CStateManager& mgr) {
  if (mTeamManagerId != kInvalidUniqueId) {
    CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamManagerId));
    if (team != nullptr) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamManagerId = kInvalidUniqueId;
      }
    }
  }
}

void CIng::SetupCollision(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descriptions;
  descriptions.reserve(7);
  const CAnimData* animData = GetAnimationData();
  for (uint i = 0; i < 7; ++i) {
    const CSegId segId = animData->GetLocatorSegId(rstl::string_l(skJointSpheres[i].mName));
    const CJointCollisionDescription description = CJointCollisionDescription::SphereCollision(
        segId, CVector3f::Zero(), skJointSpheres[i].mRadius,
        rstl::string_l(skJointSpheres[i].mName), 1000.f);
    descriptions.push_back(description);
  }
  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), descriptions, false);
  UpdateCollisionVulnerabilities(mgr);
  AddMaterial(kMT_Unknown54, mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skColliderInclude),
      CMaterialList(skColliderExclude1, skColliderExclude2, skColliderExclude3)));
}

void CIng::UpdateCollisionVulnerabilities(CStateManager& mgr) {
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mCollisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId actorId = desc.GetCollisionActorId();
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(actorId))) {
      const CHealthInfo health = *GetHealthInfo();
      colAct->SetDamageVulnerability(*CPatterned::GetDamageVulnerability());
      *colAct->HealthInfo() = health;
    }
  }
}

void CIng::TouchDamage(CStateManager& mgr, TUniqueId senderId) {
  if (const CCollisionActor* colAct =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(senderId))) {
    const TUniqueId touched = colAct->GetLastTouchedObject();
    const CPlayer* player = mgr.GetPlayer(0);
    if (touched == player->GetUniqueId() && mCurDamageRemTime <= 0.f && !mSwipeDamagePending) {
      mgr.ApplyDamage(
          GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
      mCurDamageRemTime = mDamageWaitTime;
    }
  }
}

void CIng::HandleDamage(CStateManager& mgr, TUniqueId senderId) {
  if (mForm == kF_IngSpot) {
    IngSpotHit(mgr, senderId);
  } else {
    CollisionDamage(mgr, senderId);
  }
}

void CIng::IngSpotHit(CStateManager& mgr, TUniqueId senderId) {
  mGrappleHold = -1.f;
  mIngSpotHurt = true;
  if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(senderId))) {
    CExplosion* explosion = nullptr;
    if (weapon->GetCurrentDamageInfo().GetDamage(mData.ingSpot.GetVulnerability()) >= 25.f) {
      if (mIngSpotHeavyHitEffect) {
        explosion =
            rs_new CExplosion(*mIngSpotHeavyHitEffect, mgr.AllocateUniqueId(),
                              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                              rstl::string_l("Ing damage Fx"), GetTransform(), 0,
                              GetModelData()->GetScale(), CColor::White(), -1);
      }
      CSfxManager::AddEmitter(mData.ingSpot.GetHeavyHitSound(), GetTranslation(), 127,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    } else {
      if (mIngSpotNormalHitEffect) {
        explosion =
            rs_new CExplosion(*mIngSpotNormalHitEffect, mgr.AllocateUniqueId(),
                              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                              rstl::string_l("Ing damage Fx"), GetTransform(), 0,
                              GetModelData()->GetScale(), CColor::White(), -1);
      }
      CSfxManager::AddEmitter(mData.ingSpot.GetNormalHitSound(), GetTranslation(), 127,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    }
    if (explosion != nullptr) {
      mgr.AddObject(explosion);
    }
  }
}

void CIng::CollisionDamage(CStateManager& mgr, TUniqueId senderId) {
  if (mAlive) {
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      const TUniqueId touched = colAct->GetLastTouchedObject();
      CHealthInfo* colHealth = colAct->HealthInfo();
      CHealthInfo* health = HealthInfo();
      const float initialHP = health->GetInitialHP();
      const float hp = health->GetHP();
      const float damage = initialHP - colHealth->GetHP();
      health->SetHP(hp - damage);
      CDamageInfo damageInfo = CDamageInfo();
      CVector3f direction = GetTransform().GetForward();
      TUniqueId owner = touched;
      const CEntity* object = mgr.GetObjectById(touched);
      if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(object)) {
        damageInfo = weapon->GetCurrentDamageInfo();
        direction = weapon->GetTransform().GetForward();
        owner = weapon->GetOwnerId();
      } else if (const CPlayer* player = TCastToConstPtr< CPlayer >(object)) {
        direction = GetTranslation() - player->GetTranslation();
        if (player->GetMorphBall()->InScrewAttackMode()) {
          damageInfo = CDamageInfo(CWeaponMode(kWT_ScrewAttack, false, false, false), damage, 0.f,
                                   0.f, false, false);
        } else if (player->GetMorphBall()->IsBoosting()) {
          damageInfo = CDamageInfo(CWeaponMode(kWT_BoostBall, false, false, false), damage, 0.f,
                                   0.f, false, false);
        }
      }
      TakeDamage(direction, damage);
      if (hp <= damage) {
        Death(mgr, direction, kSS_DeathRattle);
        mgr.RecordDamageSource(*this, touched, damageInfo, true, false);
      }
      CKnockBackInfo knockBack(direction, touched, owner, damageInfo, true);
      KnockBack(mgr, knockBack);
      colHealth->SetHP(initialHP);
    }
  }
}

void CIng::SpawnDamageEffect(CStateManager& mgr) {
  CExplosion* explosion = nullptr;
  if (mIngSpotHeavyHitEffect) {
    explosion =
        rs_new CExplosion(*mIngSpotHeavyHitEffect, mgr.AllocateUniqueId(),
                          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                          rstl::string_l("Ing damage Fx"), GetTransform(), 0,
                          GetModelData()->GetScale(), CColor::White(), -1);
  } else if (mIngSpotNormalHitEffect) {
    explosion =
        rs_new CExplosion(*mIngSpotNormalHitEffect, mgr.AllocateUniqueId(),
                          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                          rstl::string_l("Ing damage Fx"), GetTransform(), 0,
                          GetModelData()->GetScale(), CColor::White(), -1);
  }
  if (explosion != nullptr) {
    mgr.AddObject(explosion);
  }
}

void CIng::UpdateSounds() {
  bool removeMove = true;
  bool removeIdle = true;
  if (mForm == kF_IngSpot) {
    if (mMovingOnSurface) {
      removeMove = false;
      if (!mSfxIngSpotMove) {
        mSfxIngSpotMove = CSfxManager::AddEmitter(mData.ingSpot.GetMoveSound(), GetTranslation(),
                                                  127, GetCurrentAreaId().Value(), true, true,
                                                  CSfxManager::kMedPriority);
      } else {
        CSfxManager::UpdateEmitter(mSfxIngSpotMove, GetTranslation(), GetTransform().GetForward(),
                                   127);
      }
    } else {
      removeIdle = false;
      if (!mSfxIngSpotIdle) {
        mSfxIngSpotIdle = CSfxManager::AddEmitter(mData.ingSpot.GetIdleSound(), GetTranslation(),
                                                  127, GetCurrentAreaId().Value(), true, true,
                                                  CSfxManager::kMedPriority);
      } else {
        CSfxManager::UpdateEmitter(mSfxIngSpotIdle, GetTranslation(), GetTransform().GetForward(),
                                   127);
      }
    }
  }
  if (removeMove && mSfxIngSpotMove) {
    CSfxManager::RemoveEmitter(mSfxIngSpotMove);
    mSfxIngSpotMove.Clear();
  }
  if (removeIdle && mSfxIngSpotIdle) {
    CSfxManager::RemoveEmitter(mSfxIngSpotIdle);
    mSfxIngSpotIdle.Clear();
  }
}

void CIng::StopSounds() {
  if (mSfxBodyProjectile) {
    CSfxManager::RemoveEmitter(mSfxBodyProjectile);
    mSfxBodyProjectile.Clear();
  }
  if (mSfxIngSpotIdle) {
    CSfxManager::RemoveEmitter(mSfxIngSpotIdle);
    mSfxIngSpotIdle.Clear();
  }
  if (mSfxIngSpotMove) {
    CSfxManager::RemoveEmitter(mSfxIngSpotMove);
    mSfxIngSpotMove.Clear();
  }
  if (mSfxHostInside) {
    CSfxManager::RemoveEmitter(mSfxHostInside);
    mSfxHostInside.Clear();
  }
  if (mSfxGrapple) {
    CSfxManager::RemoveEmitter(mSfxGrapple);
    mSfxGrapple.Clear();
  }
}

void CIng::SpawnExitHostSmoke(CStateManager& mgr) {
  if (mExitHostSmokeEffect) {
    CExplosion* explosion =
        rs_new CExplosion(*mExitHostSmokeEffect, mgr.AllocateUniqueId(),
                          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                          rstl::string_l("IngExitHostSmokeFx"), GetTransform(), 0,
                          GetModelData()->GetScale(), CColor::White(), -1);
    if (explosion != nullptr) {
      mgr.AddObject(explosion);
      CSfxManager::AddEmitter(mData.swarm.exitHostSmokeSound, GetTranslation(), 127,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    }
  }
}

void CIng::UpdateLuringSafeZone(CStateManager& mgr, const CVector3f& position) {
  const CVector3f diff = position - GetTranslation();
  const float distSq = diff.MagSquared();
  if (distSq <= mData.hearingRadius * mData.hearingRadius) {
    const TUniqueId zoneId = mgr.GetSafeZoneManager()->PointIsInWhichSafeZone(mgr, position);
    if (mSafeZoneId == kInvalidUniqueId) {
      mSafeZoneId = zoneId;
    } else {
      const CScriptSafeZone* current =
          TCastToConstPtr< CScriptSafeZone >(mgr.GetObjectById(mSafeZoneId));
      if (current != nullptr) {
        const CVector3f zoneDiff = current->GetTranslation() - GetTranslation();
        if (distSq < zoneDiff.MagSquared()) {
          mSafeZoneId = zoneId;
        }
      } else {
        mSafeZoneId = zoneId;
      }
    }
  }
}

bool CIng::FindSafeZoneFloorPoint(CStateManager& mgr, CVector3f& position) {
  bool found = false;
  const CScriptSafeZone* zone =
      TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(mSafeZoneId)));
  if (zone != nullptr) {
    const CVector3f above = zone->GetTranslation() + 5.f * CVector3f::Up();
    float height = 0.f;
    GetSearchPath()->SetFlags(3);
    if (mPathFindSearch.GetHeightOfPointAboveMesh(above, height, 0.f) ==
        CPathFindSearch::kR_Success) {
      position = above + height * CVector3f::Down();
      found = true;
    }
    GetSearchPath()->SetFlags(0x301);
  }
  return found;
}

void CIng::UpdateEchoSafeZone(CStateManager& mgr) {
  const CScriptSafeZone* zone =
      TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(mSafeZoneId)));
  if (zone == nullptr || !zone->GetActive() || zone->GetZoneType() != CScriptSafeZone::kZT_Echo) {
    mSafeZoneId = kInvalidUniqueId;
  }
}

void CIng::FaceSafeZoneOrTarget(CStateManager& mgr) {
  const CVector3f position = GetTranslation();
  const CScriptSafeZone* zone =
      TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(mSafeZoneId)));
  if (zone != nullptr) {
    const CVector3f zonePosition = zone->GetTranslation();
    const float dy = zonePosition.GetY() - position.GetY();
    const float dx = zonePosition.GetX() - position.GetX();
    const CVector3f toZone(dx, dy, 0.f);
    if (toZone.IsMagnitudeSafe()) {
      SetTransform(CTransform4f::LookAt(position, position + toZone, CVector3f::Up()));
    }
  } else {
    const CEntity* target = mgr.GetObjectById(mTargetId);
    if (target != nullptr) {
      const CVector3f targetPosition = static_cast< const CActor* >(target)->GetTranslation();
      const float dy = targetPosition.GetY() - position.GetY();
      const float dx = targetPosition.GetX() - position.GetX();
      const CVector3f toTarget(dx, dy, 0.f);
      if (toTarget.IsMagnitudeSafe()) {
        SetTransform(CTransform4f::LookAt(position, position + toTarget, CVector3f::Up()));
      }
    }
  }
}

CEntity* REL_LoadIng(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrIng sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrIng.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const SIngBodyProjectileData bodyProjectile(
      LdrToDamageInfo(sldrThis.bodyProjectileContactDamage), sldrThis.unknown_0xa0d63374,
      sldrThis.bodyProjectileSuckTime, sldrThis.bodyProjectileSplatEffect,
      sldrThis.sound_BodyProjectile, sldrThis.sound_BodyProjectileSplatWall,
      sldrThis.bodyProjectileSpeed, sldrThis.bodyProjectileDropTime,
      sldrThis.bodyProjectileMinAttackDist, sldrThis.bodyProjectileMaxAttackDist,
      sldrThis.bodyProjectileOdds);
  const CDamageVulnerability ingSpotVulnerability =
      LdrToDamageVulnerability(sldrThis.ingSpotVulnerability);
  const CIngSpotData ingSpot(
      sldrThis.ingSpotBlobEffect, sldrThis.ingSpotHitNormalDamage, sldrThis.ingSpotHitHeavyDamage,
      sldrThis.ingSpotDeath, sldrThis.ingSpotMaxSpeed, sldrThis.ingSpotMaxWallSpeed,
      sldrThis.ingSpotBallPursuitSpeed, sldrThis.unknown_0x50398a06, sldrThis.ingSpotTurnSpeed,
      ingSpotVulnerability, sldrThis.sound_IngSpotIdle, sldrThis.sound_IngSpotMove,
      sldrThis.sound_HitNormalDamage, sldrThis.sound_HitHeavyDamage, sldrThis.sound_IngSpotDeath);
  const CDamageVulnerability grappleVulnerability =
      LdrToDamageVulnerability(sldrThis.grappleBallVulnerability);
  const SIngGrappleData grapple(
      sldrThis.unknown_0x67f6c10e, LdrToDamageInfo(sldrThis.exitGrappleDamage),
      sldrThis.exitGrappleSpitForce, sldrThis.sound_ExitGrapple, sldrThis.sound_Grapple,
      sldrThis.maxGrappleBallHoldTime, sldrThis.postGrappleBallWaitTime,
      sldrThis.grappleBallPursuitRange, grappleVulnerability);
  const SIngMiniPortalData miniPortal(
      sldrThis.miniPortalMinAttackDist, sldrThis.miniPortalMaxAttackDist, sldrThis.miniPortalEffect,
      sldrThis.sound_MiniPortal, LdrToDamageInfo(sldrThis.miniPortalProjectileDamage),
      sldrThis.miniPortalBeamInfo);
  const SIngSwarmData swarm(
      sldrThis.pART, sldrThis.sRSC, sldrThis.pART_0x3da219c7, sldrThis.unknown_0x23271976,
      sldrThis.pART_0x081e9e6c, sldrThis.unknown_0xcb39eccb, sldrThis.unknown_0x587ca175,
      sldrThis.unknown_0x0bd7d5a9, sldrThis.sound_SwarmMove, sldrThis.sound_ExitHost,
      sldrThis.sound_ExitHostSafeZone, sldrThis.sound_InsideHost, sldrThis.sound);
  const CDamageVulnerability triggerVulnerability =
      LdrToDamageVulnerability(sldrThis.triggerVulnerability);
  const SIngData data(sldrThis.ingFlagsIng, sldrThis.hearingRadius, sldrThis.unknown_0x5d0d2c40,
                      sldrThis.unknown_0xc620183a, sldrThis.frustrationTime, sldrThis.tauntChance,
                      sldrThis.aggressiveness, sldrThis.lightColor, sldrThis.lightAttenuation,
                      LdrToDamageInfo(sldrThis.armSwipeDamage), triggerVulnerability,
                      bodyProjectile, ingSpot, grapple, miniPortal, swarm);

  return rs_new CIng(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                     LdrToEntityInfo(info, sldrThis.editorProperties),
                     LdrToTransform4f(sldrThis.editorProperties), *modelData,
                     LdrToActorParameters(sldrThis.actorInformation),
                     LdrToPatternedInfo(sldrThis.patterned, nullptr), data);
}

static void SetFuncPtrs() {
  static SIng_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadIng;
  SetSIng_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSIng_FuncPtrs(nullptr); }
