#include "MetroidPrime/Enemies/CSplitterMainChassis.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CIngSnatchingSwarm.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CSplitterCommandModule.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplitterMainChassis.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

#include <float.h>

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::StateOver)},
    {"IsIntact",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::IsIntact)},
    {"IsDisabled",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::IsDisabled)},
    {"ShouldWaitForSnatch", static_cast< CPatterned::StateMachine::TriggerFunc >(
                                &CSplitterMainChassis::ShouldWaitForSnatch)},
    {"ShotAt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::ShotAt)},
    {"LostHead",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::LostHead)},
    {"HasHead",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::HasHead)},
    {"HasRetreatPoint",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::HasRetreatPoint)},
    {"HasTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::HasTarget)},
    {"CanReachTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::CanReachTarget)},
    {"HasLineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::HasLineOfSight)},
    {"PathShagged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::PathShagged)},
    {"PathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::PathOver)},
    {"PatrolPathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::PatrolPathOver)},
    {"ShouldLegStab",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::ShouldLegStab)},
    {"ShouldMorphballStab", static_cast< CPatterned::StateMachine::TriggerFunc >(
                                &CSplitterMainChassis::ShouldMorphballStab)},
    {"ShouldSpinAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::ShouldSpinAttack)},
    {"ShouldLaserSweep",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::ShouldLaserSweep)},
    {"ShouldDodge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::ShouldDodge)},
    {"IsSpinAttackOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::IsSpinAttackOver)},
    {"SpunIntoPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::SpunIntoPlayer)},
    {"SpunIntoGeometry",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::SpunIntoGeometry)},
    {"BreakOutOfSpin",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::BreakOutOfSpin)},
    {"TooManyBounces",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::TooManyBounces)},
    {"DeployFromHoldingTube", static_cast< CPatterned::StateMachine::TriggerFunc >(
                                  &CSplitterMainChassis::DeployFromHoldingTube)},
    {"ShouldDeploy",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::ShouldDeploy)},
    {"DeployPathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplitterMainChassis::DeployPathOver)},
    {"PrepareForDocking", static_cast< CPatterned::StateMachine::TriggerFunc >(
                              &CSplitterMainChassis::PrepareForDocking)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Start)},
    {"Idle", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Idle)},
    {"Inactive",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Inactive)},
    {"Activate",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Activate)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Patrol)},
    {"Scanning",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Scanning)},
    {"PathFind",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::PathFind)},
    {"LegStabAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::LegStabAttack)},
    {"SpinAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::SpinAttack)},
    {"Dodge", static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::Dodge)},
    {"LaserSweepAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::LaserSweepAttack)},
    {"SpinCollisionReaction", static_cast< CPatterned::StateMachine::StateFunc >(
                                  &CSplitterMainChassis::SpinCollisionReaction)},
    {"SpinBounce",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::SpinBounce)},
    {"SpinTelegraph",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::SpinTelegraph)},
    {"SpinToIdle",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::SpinToIdle)},
    {"SpawnDrop",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::SpawnDrop)},
    {"HoldingTube",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::HoldingTube)},
    {"WaitForDocking",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::WaitForDocking)},
    {"WaitForSnatch",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::WaitForSnatch)},
    {"FollowDeployPath",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::FollowDeployPath)},
    {"DeploymentLanding",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::DeploymentLanding)},
    {"HeadExplosion",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplitterMainChassis::HeadExplosion)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SelectTarget",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::SelectTarget)},
    {"SetTargetDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::SetTargetDest)},
    {"SetRetreatDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::SetRetreatDest)},
    {"SetupSpinAttack",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::SetupSpinAttack)},
    {"EndSpinAttack",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::EndSpinAttack)},
    {"SetupLaserSweep",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::SetupLaserSweep)},
    {"SetMorphballStabLeg",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::SetMorphballStabLeg)},
    {"FindBestStabLeg",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::FindBestStabLeg)},
    {"FindBestDodgeDirection", static_cast< CPatterned::StateMachine::CodeFunc >(
                                   &CSplitterMainChassis::FindBestDodgeDirection)},
    {"BounceOffPlayer",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::BounceOffPlayer)},
    {"BounceOffGeometry",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSplitterMainChassis::BounceOffGeometry)},
};

static EMaterialTypes skSpinCollisionMaterial = kMT_Wall;              // Guessed name
static EMaterialTypes skDropRayMaterial1 = kMT_Solid;                  // Guessed name
static EMaterialTypes skDropRayMaterial2 = kMT_Floor;                  // Guessed name
static EMaterialTypes skNearListMaterial = kMT_Projectile;             // Guessed name
static EMaterialTypes skCollisionFilterMaterial1 = kMT_Solid;          // Guessed name
static EMaterialTypes skCollisionFilterMaterial2 = kMT_CollisionActor; // Guessed name
static EMaterialTypes skCollisionFilterMaterial3 = kMT_AIPassthrough;  // Guessed name
static EMaterialTypes skCollisionFilterMaterial4 = kMT_Player;         // Guessed name
static EMaterialTypes skPlayerDamageMaterial = kMT_Solid;              // Guessed name
static EMaterialTypes skObjectDamageMaterial = kMT_Solid;              // Guessed name
static EMaterialTypes skStepRayMaterial1 = kMT_Solid;                  // Guessed name
static EMaterialTypes skStepRayMaterial2 = kMT_CollisionActor;         // Guessed name

static const pas::ESeverity skLegStabSeverities[] = {pas::kS_Zero, pas::kS_One, pas::kS_Two,
                                                     pas::kS_Three, pas::kS_Four};

struct SSphereJoint {
  const char* mLocator;
  float mRadius;
};

struct SBoxJoint {
  const char* mFromLocator;
  const char* mToLocator;
  float mHalfExtent;
};

static const SSphereJoint skSphereJoints[] = {
    {"Skeleton_Root", 1.5f},
    {"target_LCTR", 0.38f},
};

static const SBoxJoint skBoxJoints[] = {
    {"L_F_knee_2", "L_F_ankle", 0.8f},
    {"L_B_knee_2", "L_B_ankle", 0.8f},
    {"R_F_knee_2", "R_F_ankle", 0.8f},
    {"R_B_knee_2", "R_B_ankle", 0.8f},
};

static const char* skBodyLocatorName = "target_LCTR";

SSplitterMainChassisData::SSplitterMainChassisData(const SLdrSplitterMainChassisData& loaded)
: legStabDamage(LdrToDamageInfo(loaded.legStabDamage))
, spinAttackDamage(LdrToDamageInfo(loaded.spinAttackDamage))
, spinAttackVulnerability(LdrToDamageVulnerability(loaded.spinAttackVulnerability))
, data(loaded) {}

CSplitterMainChassis::~CSplitterMainChassis() {}

CSplitterMainChassis::CSplitterMainChassis(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, const CTransform4f& xf,
                                           const CModelData& modelData,
                                           const CActorParameters& actorParams,
                                           const CPatternedInfo& patternedInfo,
                                           const SSplitterMainChassisData& data)
: CPatterned(kPAI_SplitterMainChassis, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Ground, kCT_One, kBT_BiPedal, actorParams)
, mData(data)
, mPathFindSearch(nullptr, 1, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mCollisionActorManager(nullptr)
, mCommandModuleId(kInvalidUniqueId)
, mDockingModuleId(kInvalidUniqueId)
, mLineOfSight(GetUniqueId(), CSegId::Invalid(), 0.1f, 0.05f)
, mTurnRate(patternedInfo.GetTurnSpeed())
, mAdditiveAnimation(-1)
, mAdditiveWeight(1.f)
, mSnatchId(kInvalidUniqueId)
, mTeamAiMgrId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mBodyCollisionId(kInvalidUniqueId)
, mPlayerHintId(kInvalidUniqueId)
, mWaypointId(kInvalidUniqueId)
, mAttachLocator(CSegId::Invalid())
, mTargetLocator(CSegId::Invalid())
, mLandingPoint(CVector3f::Zero())
, mSpinVelocity(CVector3f::Zero())
, mCollisionNormal(CVector3f::Zero())
, mBounceDirection(CVector3f::Zero())
, mBounceCount(0)
, mDeployState((data.data.unknown_0xcef5c2fe & 1) ? 0 : 5)
, mDodgeDirection(pas::kSD_Invalid)
, mStabLeg(-1)
, mLocomotionRatio(1.f)
, mLegStabTimer(0.f)
, mSpinAttackTimer(0.f)
, mLaserSweepTimer(0.f)
, mDodgeTimer(0.f)
, mAutoDestructTimer(0.f)
, mBounceTimer(0.f)
, mScanTimer(0.f)
, mSpinTime(0.f)
, mSpinElapsed(0.2f)
, mTimeSinceShot(1.f)
, mStepDistance(0.f)
, mPathBlockedTime(0.f)
, mSpinAttackAllowed((data.data.unknown_0xcef5c2fe >> 3) & 1)
, mDisabled(data.data.unknown_0xcef5c2fe & 1)
, mHasDestination(false)
, mNearDestination(false)
, mSpunIntoPlayer(false)
, mSpunIntoGeometry(false)
, mBreakOutOfSpin(false)
, mLegStabActive(false)
, mSpinAttackActive(false)
, mLaserSweepActive(false)
, mTargetFar(false)
, mDodging(false)
, mActivating(false)
, mShouldDeploy(false)
, mLostHead(false)
, mAutoDestructRequested(false) {
  const CBodyStateInfo& stateInfo = BodyController()->GetBodyStateInfo();
  mLocomotionRatio =
      stateInfo.GetLocomotionSpeed(pas::kLA_Walk) / stateInfo.GetLocomotionSpeed(pas::kLA_Run);

  mAttachLocator = AnimationData()->GetLocatorSegId(rstl::string_l("attach_LCTR"));
  mTargetLocator = AnimationData()->GetLocatorSegId(rstl::string_l(skBodyLocatorName));
  mLineOfSight.SetSegment(mAttachLocator);

  const CPASAnimParmData additiveParms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(5));
  mAdditiveAnimation = GetModelData()
                           ->GetAnimationData()
                           ->GetCharacterInfo()
                           .GetPASDatabase()
                           .FindBestAnimation(additiveParms, -1)
                           .second;
  KnockBackController().EnableKnockBackPhysics(false);

  const CPASAnimParmData stepParms(pas::kAS_Step, CPASAnimParm::FromEnum(1),
                                   CPASAnimParm::FromEnum(3));
  mStepDistance = GetModelData()->GetScale().GetX() * GetAnimationDistance(stepParms);
}

CVector3f CSplitterMainChassis::GetAttachPosition() const {
  const CTransform4f xf = GetLctrTransform(mAttachLocator);
  return xf.GetTranslation();
}

void CSplitterMainChassis::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId));
    if (team != nullptr) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CSplitterMainChassis::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
    if (mTeamAiMgrId != kInvalidUniqueId) {
      CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId));
      if (team != nullptr) {
        team->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                       CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CSplitterMainChassis::ResetAttackTimers() {
  mLegStabTimer = mData.data.legStabAttackInterval;
  mSpinAttackTimer = mData.data.spinAttackInterval;
  mLaserSweepTimer = mData.data.laserSweepInterval;
  mDodgeTimer = mData.data.minDodgeInterval;
}

CVector3f CSplitterMainChassis::CalculateSeparation(CStateManager& mgr) {
  CVector3f total = CVector3f::Zero();
  float count = 0.f;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CPatterned* other = TCastToConstPtr< CPatterned >(list[i]);
    if (other != nullptr && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CSplitterCommandModule* module = TCastToConstPtr< CSplitterCommandModule >(other);
      if (module != nullptr && module->GetMainChassisId() != kInvalidUniqueId) {
        continue;
      }
      const CVector3f separation = mSteeringBehaviors.Separation(
          *this, other->GetTranslation(), 6.5f * GetModelData()->GetScale().GetX());
      if (separation.IsMagnitudeSafe()) {
        total += separation.AsNormalized();
        count += 1.f;
      }
    }
  }
  if (count > 0.f) {
    total *= 1.f / count;
  }
  return total;
}

bool CSplitterMainChassis::CanStepInDirection(CStateManager& mgr, const CVector3f& direction,
                                              float distance) const {
  const CAABox box = GetBoundingBox();
  const CVector3f start = box.GetCenterPoint();
  const CVector3f end = start + direction * distance;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skStepRayMaterial1), CMaterialList(skStepRayMaterial2));
  if (!mgr.RayCollideWorld(start, end, filter, this)) {
    return false;
  }
  return !mPathFindSearch.OnPath(end);
}

pas::EStepDirection CSplitterMainChassis::FindBestDodgeStep(CStateManager& mgr,
                                                            const CVector3f& threatDirection) {
  bool canStepForward = true;
  bool canStepBackward = true;
  pas::EStepDirection result = pas::kSD_Invalid;
  pas::EStepDirection forwardStep = pas::kSD_Right;
  pas::EStepDirection backwardStep = pas::kSD_Left;
  CVector3f candidate = GetTransform().GetRight();
  const float radiusSq = mStepDistance * mStepDistance;
  const CVector3f position = GetTranslation();
  switch (FindBestStepDirection(threatDirection)) {
  case pas::kSD_Left:
    candidate = GetTransform().GetForward();
    forwardStep = pas::kSD_Forward;
    backwardStep = pas::kSD_Backward;
    break;
  case pas::kSD_Right:
    candidate = -GetTransform().GetForward();
    forwardStep = pas::kSD_Backward;
    backwardStep = pas::kSD_Forward;
    break;
  case pas::kSD_Forward:
    candidate = -candidate;
    forwardStep = pas::kSD_Left;
    backwardStep = pas::kSD_Right;
    break;
  default:
    break;
  }

  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CActor* other = static_cast< const CActor* >(list[i]);
    if (other != nullptr && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CVector3f offset = other->GetTranslation() - position;
      if (offset.MagSquared() < radiusSq) {
        if (CVector3f::Dot(offset, candidate) >= 0.f) {
          if (canStepForward && CVector3f::GetAngleDiff(candidate, offset) < (M_PIF / 3.f)) {
            canStepForward = false;
          }
        } else if (canStepBackward && CVector3f::GetAngleDiff(-candidate, offset) < (M_PIF / 3.f)) {
          canStepBackward = false;
        }
      }
    }
  }

  canStepForward = canStepForward && CanStepInDirection(mgr, candidate, mStepDistance);
  canStepBackward = canStepBackward && CanStepInDirection(mgr, -candidate, mStepDistance);
  if (canStepForward && canStepBackward) {
    if (mgr.Random()->Next() & 0x4000) {
      canStepBackward = false;
    } else {
      canStepForward = false;
    }
  }
  if (canStepBackward) {
    result = backwardStep;
  } else if (canStepForward) {
    result = forwardStep;
  }
  return result;
}

void CSplitterMainChassis::UpdateAdditive(float dt) {
  CAnimData* animData = AnimationData();
  const float rate = dt / (mTargetId != kInvalidUniqueId ? 0.25f : 1.f);
  if (mScanTimer > 0.f || mTargetId != kInvalidUniqueId) {
    mAdditiveWeight = CMath::Max(0.f, mAdditiveWeight - rate);
  } else {
    mAdditiveWeight = CMath::Min(1.f, mAdditiveWeight + rate);
  }
  if (mAdditiveWeight > 0.f) {
    animData->AddAdditiveAnimation(mAdditiveAnimation, mAdditiveWeight, false, false);
  } else if (animData->IsAdditiveAnimationActive(mAdditiveAnimation)) {
    animData->DelAdditiveAnimation(mAdditiveAnimation);
  }
}

void CSplitterMainChassis::MoveSpinning(float dt, const CVector3f& desired) {
  if (dt > 0.f) {
    CVector3f direction = mSpinVelocity;
    if (mSpinVelocity.IsMagnitudeSafe()) {
      if (desired.IsMagnitudeSafe()) {
        const float limit = mData.data.spinAttackTurnSpeed * dt;
        const float angle = CVector3f::GetAngleDiff(mSpinVelocity, desired);
        const float radians = (M_PIF / 180.f) * limit;
        if (angle > radians) {
          direction = CVector3f::Slerp(mSpinVelocity.AsNormalized(), desired.AsNormalized(),
                                       CRelAngle::FromRadians(radians));
        } else {
          direction = desired.AsNormalized();
        }
      }
    } else if (desired.IsMagnitudeSafe()) {
      direction = desired.AsNormalized();
    }

    float acceleration;
    if (CVector3f::GetAngleDiff(desired, mSpinVelocity) > (10.f * (M_PIF / 180.f))) {
      acceleration = -mData.data.spinAttackLinearDeceleration * dt;
    } else {
      acceleration = mData.data.spinAttackLinearAcceleration * dt;
    }
    float speed = CMath::Min(mData.data.spinAttackLinearVelocity,
                             (1.f / dt) * mSpinVelocity.Magnitude() + acceleration);
    speed = CMath::Max(0.f, speed);
    mSpinVelocity = dt * (speed * direction);
    MoveInOneFrameOR(GetTransform().TransposeRotate(mSpinVelocity), dt);
  }
}

void CSplitterMainChassis::UpdateLocomotion(CStateManager& mgr) {
  const CVector3f separation = CalculateSeparation(mgr);
  if (separation != CVector3f::Zero()) {
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(separation, CVector3f::Zero(), 0.5f));
  }

  const CVector3f move = BodyController()->CommandMgr().GetMoveVector();
  BodyController()->CommandMgr().ClearLocomotionCmds();
  const pas::EStepDirection step = FindBestStepDirection(move);
  if (move.IsMagnitudeSafe() && step != pas::kSD_Forward) {
    CVector3f facing = CVector3f::Zero();
    switch (step) {
    case pas::kSD_Backward:
      facing = -move;
      break;
    case pas::kSD_Left:
      facing = CVector3f(move.GetY(), -move.GetX(), move.GetZ());
      break;
    case pas::kSD_Right:
      facing = CVector3f(-move.GetY(), move.GetX(), move.GetZ());
      break;
    default:
      break;
    }
    BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, facing.AsNormalized(), 1.f));
  } else {
    BodyController()->CommandMgr().SetSteeringSpeedRange(mLocomotionRatio, mLocomotionRatio);
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
  }
}

void CSplitterMainChassis::SetCollisionVulnerabilities(
    CStateManager& mgr, const CDamageVulnerability& bodyVulnerability,
    const CDamageVulnerability& otherVulnerability) {
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id));
    if (actor != nullptr) {
      if (id == mBodyCollisionId) {
        actor->SetDamageVulnerability(bodyVulnerability);
      } else {
        actor->SetDamageVulnerability(otherVulnerability);
      }
    }
  }
}

void CSplitterMainChassis::HandleHitObject(CStateManager& mgr, const TUniqueId& collisionActorId) {
  if (mAlive && !mDisabled) {
    const CCollisionActor* actor =
        TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(collisionActorId));
    if (actor != nullptr) {
      const TUniqueId touchedId = actor->GetLastTouchedObject();
      CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(touchedId));
      if (player != nullptr) {
        if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                 ? player->GetMorphballTransitionState()
                 : CPlayer::kMS_Unmorphed) != CPlayer::kMS_Morphed ||
            !player->GetMorphBall()->IsBoostShieldActive()) {
          if (!mBreakOutOfSpin) {
            CDamageInfo damage = GetContactDamage();
            if (mLegStabActive) {
              damage = mData.legStabDamage;
            } else if (mSpinAttackActive) {
              damage = mData.spinAttackDamage;
              mSpunIntoPlayer = true;
            }
            if (mCurDamageRemTime <= 0.f) {
              mgr.ApplyDamage(GetUniqueId(), touchedId, GetUniqueId(), damage,
                              CMaterialFilter::MakeIncludeExclude(
                                  CMaterialList(skPlayerDamageMaterial), CMaterialList()),
                              GetTransform().GetForward());
              mCurDamageRemTime = mDamageWaitTime;
            }
          }
        } else if (collisionActorId == mBodyCollisionId) {
          const CDamageInfo damage = gpTweakBall->GetBoostBallDamage();
          player->GetMorphBall()->ApplyBoostBallDamage(mgr, mBodyCollisionId, damage, 0.f);
        }
        AcquirePlayerHint(mgr);
      } else if (mSpinAttackActive &&
                 TCastToConstPtr< CAi >(mgr.GetObjectById(touchedId)) == nullptr) {
        mgr.ApplyDamage(GetUniqueId(), touchedId, GetUniqueId(), mData.spinAttackDamage,
                        CMaterialFilter::MakeIncludeExclude(CMaterialList(skObjectDamageMaterial),
                                                            CMaterialList()),
                        GetTransform().GetForward());
      }
    }
  }
}

void CSplitterMainChassis::HandleResistedDamage(CStateManager& mgr,
                                                const TUniqueId& collisionActorId) {
  if (mAlive) {
    if (mSpinAttackActive) {
      CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(collisionActorId));
      if (actor != nullptr) {
        const TUniqueId touchedId = actor->GetLastTouchedObject();
        const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(touchedId));
        if (player != nullptr &&
            (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                 ? player->GetMorphballTransitionState()
                 : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed &&
            player->GetMorphBall()->IsBoostShieldActive()) {
          mBounceCount += mData.data.unknown_0xd5f34476;
          mBreakOutOfSpin = true;
        }
      }
    }
    AcquirePlayerHint(mgr);
  }
}

void CSplitterMainChassis::HandleDamage(CStateManager& mgr, const TUniqueId& collisionActorId) {
  if (mAlive) {
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(collisionActorId));
    if (actor != nullptr) {
      const TUniqueId touchedId = actor->GetLastTouchedObject();
      CHealthInfo* actorHealth = actor->HealthInfo();
      CHealthInfo* health = HealthInfo();
      const float initialHP = actorHealth->GetInitialHP();
      const float damageAmount = initialHP - actorHealth->GetHP();
      const float currentHP = health->GetHP();
      CVector3f direction = GetTransform().GetForward();
      TUniqueId ownerId = kInvalidUniqueId;
      CDamageInfo damage;
      const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touchedId));
      if (weapon != nullptr) {
        direction = weapon->GetTransform().GetForward();
        ownerId = weapon->GetOwnerId();
        damage = weapon->GetCurrentDamageInfo();
      }
      health->SetHP(currentHP - damageAmount);
      TakeDamage(direction, damageAmount);
      if (currentHP <= damageAmount) {
        Death(mgr, direction, kSS_DeathRattle);
        mgr.RecordDamageSource(*this, actor->GetUniqueId(), damage, true, false);
      }
      const CKnockBackInfo knockBack(direction, ownerId, touchedId, damage, true);
      KnockBack(mgr, knockBack);
      actorHealth->SetHP(initialHP);
    }
    AcquirePlayerHint(mgr);
  }
}

void CSplitterMainChassis::SyncCollisionActorHealth(CStateManager& mgr) {
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id));
    if (actor != nullptr) {
      const CHealthInfo health = *GetHealthInfo();
      *actor->HealthInfo() = health;
    }
  }
}

void CSplitterMainChassis::SetupCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descriptions;
  descriptions.reserve(6);
  CAnimData* animData = AnimationData();
  for (uint i = 0; i < ARRAY_SIZE(skSphereJoints); ++i) {
    const SSphereJoint& joint = skSphereJoints[i];
    const CSegId segment = animData->GetLocatorSegId(rstl::string_l(joint.mLocator));
    const CJointCollisionDescription description = CJointCollisionDescription::SphereCollision(
        segment, CVector3f::Zero(), joint.mRadius, rstl::string_l(joint.mLocator), 1000.f);
    descriptions.push_back_unsafe(description);
  }
  for (uint i = 0; i < ARRAY_SIZE(skBoxJoints); ++i) {
    const SBoxJoint& joint = skBoxJoints[i];
    const CSegId from = animData->GetLocatorSegId(rstl::string_l(joint.mFromLocator));
    const CSegId to = animData->GetLocatorSegId(rstl::string_l(joint.mToLocator));
    const CJointCollisionDescription description = CJointCollisionDescription::OBBAutoSizeCollision(
        from, to, CVector3f(joint.mHalfExtent, joint.mHalfExtent, joint.mHalfExtent),
        CJointCollisionDescription::kOT_BetweenJoints, rstl::string_l(joint.mFromLocator), 1000.f);
    descriptions.push_back_unsafe(description);
  }

  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), descriptions, true);
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& description =
        mCollisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = description.GetCollisionActorId();
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id));
    if (actor != nullptr) {
      if (description.GetName() == rstl::string_l(skBodyLocatorName)) {
        mBodyCollisionId = id;
        const CVector3f scale = GetModelData()->GetScale();
        actor->SetExtendedTouchBounds(
            CVector3f(scale.GetX() * 1.5f, scale.GetY() * 1.5f, scale.GetZ() * 1.5f));
      }
      actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
    }
  }
  SyncCollisionActorHealth(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(skCollisionFilterMaterial1),
                                                        CMaterialList(skCollisionFilterMaterial2,
                                                                      skCollisionFilterMaterial3,
                                                                      skCollisionFilterMaterial4)));
}

void CSplitterMainChassis::SetLocomotionTypeFromFlags() {
  if (mData.data.unknown_0xcef5c2fe & 2) {
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
  } else if (mData.data.unknown_0xcef5c2fe & 1) {
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
  } else {
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  }
}

void CSplitterMainChassis::ReleasePlayerHint(CStateManager& mgr) {
  CScriptPlayerHint* hint = TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(mPlayerHintId));
  if (hint != nullptr && hint->GetActorId() == GetUniqueId()) {
    hint->SetActorId(kInvalidUniqueId);
    mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), hint->GetUniqueId(), kSM_Decrement,
                                    mgr.GetPlayer(0)->GetUniqueId(), kSS_InvalidState));
  }
}

void CSplitterMainChassis::AcquirePlayerHint(CStateManager& mgr) {
  CScriptPlayerHint* hint = TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(mPlayerHintId));
  if (hint != nullptr && hint->GetActorId() != GetUniqueId()) {
    CPlayer* player = mgr.GetPlayer(0);
    switch (player->GetMorphballTransitionState()) {
    case CPlayer::kMS_Unmorphed:
    case CPlayer::kMS_Morphed:
      hint->SetActorId(GetUniqueId());
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), hint->GetUniqueId(), kSM_Increment,
                                      player->GetUniqueId(), kSS_InvalidState));
      break;
    default:
      break;
    }
  }
}

void CSplitterMainChassis::FindConnectedObjects(CStateManager& mgr) {
  mPlayerHintId = CheckConnectedObject(mgr, kSS_Connect, kSM_Attach);
  mWaypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
}

void CSplitterMainChassis::UpdateCommandModule(float dt, CStateManager& mgr) {
  if (mCommandModuleId != kInvalidUniqueId) {
    CSplitterCommandModule* module =
        TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
    if (module != nullptr) {
      const CTransform4f attachXf = GetLctrTransform(mAttachLocator);
      const CVector3f attachPosition = attachXf.GetTranslation();
      if (mSpinAttackActive || mActivating || mDeployState != 5 ||
          module->GetFacingDirection() == CVector3f::Zero()) {
        const CVector3f moduleForward = module->GetTransform().GetForward();
        const CVector3f attachForward = attachXf.GetForward();
        const float limit = (2.f * M_2PIF) * dt;
        if (mDeployState == 5 && CVector3f::GetAngleDiff(moduleForward, attachForward) >= limit) {
          const CVector3f direction =
              CVector3f::Slerp(moduleForward, attachForward, CRelAngle::FromRadians(limit));
          module->SetTransform(
              CTransform4f::LookAt(attachPosition, attachPosition + direction, attachXf.GetUp()));
        } else {
          module->SetTransform(attachXf);
        }
      } else {
        module->SetTransform(CTransform4f::LookAt(
            attachPosition, attachPosition + module->GetFacingDirection(), attachXf.GetUp()));
      }
    } else {
      mCommandModuleId = kInvalidUniqueId;
      mLostHead = true;
      AddMaterial(kMT_Target, kMT_Orbit, mgr);
    }
  } else if (mDockingModuleId != kInvalidUniqueId &&
             TCastToConstPtr< CSplitterCommandModule >(mgr.GetObjectById(mDockingModuleId)) ==
                 nullptr) {
    mDockingModuleId = kInvalidUniqueId;
  }
}

void CSplitterMainChassis::UpdateTimers(float dt, CStateManager& mgr) {
  mLegStabTimer -= dt;
  mSpinAttackTimer -= dt;
  mLaserSweepTimer -= dt;
  mDodgeTimer -= dt;
  mScanTimer -= dt;
  mTimeSinceShot += dt;
  mSpinElapsed += dt;
  mSpinTime += dt;
  if (mTargetId != kInvalidUniqueId && mPathFindSearch.IsShagged()) {
    mPathBlockedTime += dt;
  } else {
    mPathBlockedTime = 0.f;
  }
  if (mAutoDestructRequested && mAlive) {
    mAutoDestructTimer -= dt;
    if (mAutoDestructTimer <= 0.f) {
      CPatterned::Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
    }
  }
}

void CSplitterMainChassis::AutoDestruct(float time) {
  if (mAlive && !mAutoDestructRequested) {
    mAutoDestructRequested = true;
    mAutoDestructTimer = time;
  }
}

bool CSplitterMainChassis::DockCommandModule(CStateManager& mgr, TUniqueId moduleId) {
  if (mCommandModuleId == kInvalidUniqueId) {
    mCommandModuleId = moduleId;
    mDockingModuleId = kInvalidUniqueId;
    mDisabled = false;
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    ResetAttackTimers();
    return true;
  }
  return false;
}

void CSplitterMainChassis::CancelDocking(TUniqueId moduleId) {
  if (mDockingModuleId == moduleId) {
    mDockingModuleId = kInvalidUniqueId;
  }
}

bool CSplitterMainChassis::RequestDocking(TUniqueId moduleId) {
  if (mAlive && GetActive() && mCommandModuleId == kInvalidUniqueId &&
      mDockingModuleId == kInvalidUniqueId && (mDeployState == 5 || mDeployState == 0) &&
      !mSpinAttackActive && !mLegStabActive) {
    mDockingModuleId = moduleId;
  }
  return mDockingModuleId == moduleId;
}

void CSplitterMainChassis::SetAttackTarget(CStateManager& mgr, TUniqueId target) {
  if (TCastToConstPtr< CIngSnatchingSwarm >(mgr.GetObjectById(target)) != nullptr) {
    mSnatchId = target;
  }
  mHitByPlayerProjectile = true;
}

void CSplitterMainChassis::SetIngPossessed(bool possessed, float duration, CStateManager& mgr) {
  CPatterned::SetIngPossessed(possessed, duration, mgr);
  if (possessed) {
    CSplitterCommandModule* module =
        TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
    if (module != nullptr) {
      module->SetIngPossessed(true, 0.f, mgr);
    }
  }
}

void CSplitterMainChassis::SetIngPossessed(bool possessed, CStateManager& mgr) {
  CPatterned::SetIngPossessed(possessed, mgr);
  if (possessed) {
    CSplitterCommandModule* module =
        TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
    if (module != nullptr) {
      module->SetIngPossessed(true, 0.f, mgr);
    }
  }
}

bool CSplitterMainChassis::CanBeIngPossessed(CStateManager& mgr) const {
  if (!(mSpinAttackActive || mLegStabActive || mLaserSweepActive)) {
    return CPatterned::CanBeIngPossessed(mgr);
  }
  return false;
}

void CSplitterMainChassis::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

bool CSplitterMainChassis::Listen(CStateManager& mgr, const CVector3f& position,
                                  EListenNoiseType type) {
  bool handled = false;
  if (mAlive) {
    switch (type) {
    case kLNT_PathObstruction:
      const CVector3f offset = position - GetTranslation();
      if (offset.MagSquared() < 400.f) {
        handled = true;
        mNearDestination = true;
      }
      break;
    case kLNT_PlayerFire:
      handled = true;
      mTimeSinceShot = 0.f;
      break;
    default:
      break;
    }
  }
  return handled;
}

void CSplitterMainChassis::Death(CStateManager& mgr, const CVector3f& direction,
                                 EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id));
    if (actor != nullptr) {
      actor->SetMaterialFilter(GetMaterialFilter());
    }
  }
  RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  CSplitterCommandModule* module =
      TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
  if (module != nullptr) {
    module->SetNormalState(mgr);
    mCommandModuleId = kInvalidUniqueId;
  }
}

void CSplitterMainChassis::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (!mSpinAttackActive) {
    if (info.GetDamageInfo().GetWeaponMode1() == kWT_BoostBall) {
      KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, true);
    } else {
      KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, false);
    }
  } else {
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_Flinch, false);
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, false);
  }
  CPatterned::KnockBack(mgr, info);
}

void CSplitterMainChassis::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                        CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (mSpinAttackActive && mSpinElapsed > 0.2f) {
    if (id == kInvalidUniqueId) {
      static CMaterialList skWallList(skSpinCollisionMaterial);
      for (int i = 0; i < list.GetCount(); ++i) {
        const CCollisionInfo& info = list[i];
        if (info.GetMaterialLeft().SharesMaterials(skWallList) ||
            CVector3f::Dot(GetTransform().GetUp(), info.GetNormalLeft()) < 0.707f) {
          mSpunIntoGeometry = true;
          mCollisionNormal = info.GetNormalLeft();
          break;
        }
      }
    } else {
      const CEntity* object = mgr.GetObjectById(id);
      const CActor* actor = static_cast< const CActor* >(object);
      if (TCastToConstPtr< CAi >(object) == nullptr) {
        if (actor->GetHealthInfo() != nullptr &&
            mData.spinAttackDamage.GetDamage(*actor->GetDamageVulnerability()) <= 0.f) {
          return;
        }
      }
      const CVector3f offset = GetTranslation() - actor->GetTranslation();
      if (offset.IsMagnitudeSafe()) {
        mSpunIntoGeometry = true;
        mCollisionNormal = offset.AsNormalized();
      }
    }
  }
}

const CDamageVulnerability* CSplitterMainChassis::GetDamageVulnerability() const {
  return &CDamageVulnerability::PassThroughVulnerabilty();
}

void CSplitterMainChassis::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
}

void CSplitterMainChassis::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CSplitterMainChassis::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CSplitterMainChassis::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                           EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_BreakLockOn:
    if (mActivating) {
      CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
      if (player != nullptr) {
        player->SetOrbitRequestForTarget(GetUniqueId(), CPlayer::kOR_ActivateOrbitSource, mgr);
        player->SetOrbitRequestForTarget(mCommandModuleId, CPlayer::kOR_ActivateOrbitSource, mgr);
        handled = true;
      }
    }
    break;
  case kUE_ChangeMaterial:
    if (mSpinAttackActive) {
      SetCollisionVulnerabilities(mgr, *CPatterned::GetDamageVulnerability(),
                                  CDamageVulnerability::ReflectVulnerabilty());
      CSplitterCommandModule* module =
          TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
      if (module != nullptr) {
        module->SetNormalState(mgr);
      }
      handled = true;
    }
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CSplitterMainChassis::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    if (mCollisionActorManager.get() == nullptr) {
      SetupCollisionActors(mgr);
    } else {
      mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    }
    mLineOfSight.Update(dt, mgr);
    UpdateTimers(dt, mgr);
    UpdateCommandModule(dt, mgr);
    UpdateAdditive(dt);
  }
}

void CSplitterMainChassis::PreThink(float dt, CStateManager& mgr) { CPatterned::PreThink(dt, mgr); }

void CSplitterMainChassis::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool wasActive = GetActive();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_Create:
    SetLocomotionTypeFromFlags();
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    break;
  case kSM_Delete:
    if (mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->Destroy(mgr);
    }
    ReleasePlayerHint(mgr);
    QuitTeam(mgr);
    break;
  case kSM_Activate:
    if (!wasActive && mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->SetActive(mgr, true);
    }
    break;
  case kSM_Deactivate:
    if (wasActive && mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->SetActive(mgr, false);
    }
    QuitTeam(mgr);
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Alert:
    mHitByPlayerProjectile = true;
    break;
  case kSM_Start:
    mShouldDeploy = true;
    break;
  case kSM_Action:
    mScanTimer = mData.data.scanDuration > 0.f ? mData.data.scanDuration : FLT_MAX;
    break;
  case kSM_AreaLoaded:
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    FindConnectedObjects(mgr);
    break;
  case kSM_Damage:
    HandleDamage(mgr, msg.GetSenderId());
    mHitByPlayerProjectile = true;
    break;
  case kSM_ResistedDamage:
  case kSM_ReflectedDamage:
    HandleResistedDamage(mgr, msg.GetSenderId());
    mHitByPlayerProjectile = true;
    break;
  case kSM_HitObject:
    HandleHitObject(mgr, msg.GetSenderId());
    break;
  case kSM_Landed:
    mDeployState = 5;
    break;
  case kSM_InternalMessage0:
    mSpinAttackAllowed = false;
    if (mSpinAttackActive) {
      mSpinTime += mData.data.spinAttackMaxTime;
      mBounceCount += mData.data.unknown_0xd5f34476;
    }
    break;
  case kSM_InternalMessage1:
    mSpinAttackAllowed = true;
    break;
  default:
    break;
  }
}

bool CSplitterMainChassis::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

bool CSplitterMainChassis::IsIntact(CStateManager& mgr, const CTriggerData& data) const {
  return mCommandModuleId != kInvalidUniqueId;
}

bool CSplitterMainChassis::IsDisabled(CStateManager& mgr, const CTriggerData& data) const {
  return mDisabled;
}

bool CSplitterMainChassis::ShouldWaitForSnatch(CStateManager& mgr, const CTriggerData& data) const {
  return mSnatchId != kInvalidUniqueId || IsBeingSnatched();
}

bool CSplitterMainChassis::ShotAt(CStateManager& mgr, const CTriggerData& data) const {
  return mHitByPlayerProjectile;
}

bool CSplitterMainChassis::LostHead(CStateManager& mgr, const CTriggerData& data) const {
  return mLostHead;
}

bool CSplitterMainChassis::HasHead(CStateManager& mgr, const CTriggerData& data) const {
  return mCommandModuleId != kInvalidUniqueId;
}

bool CSplitterMainChassis::HasRetreatPoint(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointId != kInvalidUniqueId;
}

bool CSplitterMainChassis::HasTarget(CStateManager& mgr, const CTriggerData& data) const {
  return mTargetId != kInvalidUniqueId;
}

bool CSplitterMainChassis::CanReachTarget(CStateManager& mgr, const CTriggerData& data) const {
  return mPathBlockedTime < 3.f;
}

bool CSplitterMainChassis::HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return mLineOfSight.HasLineOfSight();
}

bool CSplitterMainChassis::PathShagged(CStateManager& mgr, const CTriggerData& data) const {
  return mNearDestination || CPatterned::PathShagged(mgr, data);
}

bool CSplitterMainChassis::PathOver(CStateManager& mgr, const CTriggerData& data) const {
  return !mHasDestination || CPatterned::PathOver(mgr, data);
}

bool CSplitterMainChassis::PatrolPathOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CSplitterMainChassis::ShouldLegStab(CStateManager& mgr, const CTriggerData& data) const {
  if ((mData.data.unknown_0xcef5c2fe & 4) && mLegStabTimer <= 0.f) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const CVector3f offset = target->GetTranslation() - GetTranslation();
      const float distanceSq = offset.MagSquared();
      return distanceSq >= mData.data.legStabMinAttackRange * mData.data.legStabMinAttackRange &&
             distanceSq <= mData.data.legStabMaxAttackRange * mData.data.legStabMaxAttackRange;
    }
  }
  return false;
}

bool CSplitterMainChassis::ShouldMorphballStab(CStateManager& mgr, const CTriggerData& data) const {
  if ((mData.data.unknown_0xcef5c2fe & 4) && mLegStabTimer <= 0.f) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const CVector3f offset = target->GetTranslation() - GetTranslation();
      const float distanceSq = offset.MagSquared();
      return distanceSq < mData.data.legStabMinAttackRange * mData.data.legStabMinAttackRange;
    }
  }
  return false;
}

bool CSplitterMainChassis::ShouldSpinAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mSpinAttackAllowed && mSpinAttackTimer <= 0.f) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                         GetUniqueId())) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f offset = target->GetTranslation() - GetTranslation();
        const float distanceSq = offset.MagSquared();
        return distanceSq >=
                   mData.data.spinAttackMinAttackRange * mData.data.spinAttackMinAttackRange &&
               distanceSq <=
                   mData.data.spinAttackMaxAttackRange * mData.data.spinAttackMaxAttackRange;
      }
    }
  }
  return false;
}

bool CSplitterMainChassis::ShouldLaserSweep(CStateManager& mgr, const CTriggerData& data) const {
  if ((mData.data.unknown_0xcef5c2fe & 0x40) && mLaserSweepTimer <= 0.f) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                         GetUniqueId())) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f offset = target->GetTranslation() - GetTranslation();
        const float distanceSq = offset.MagSquared();
        return distanceSq >=
                   mData.data.laserSweepMinAttackRange * mData.data.laserSweepMinAttackRange &&
               distanceSq <=
                   mData.data.laserSweepMaxAttackRange * mData.data.laserSweepMaxAttackRange;
      }
    }
  }
  return false;
}

bool CSplitterMainChassis::ShouldDodge(CStateManager& mgr, const CTriggerData& data) const {
  return mDodgeDirection != pas::kSD_Invalid;
}

bool CSplitterMainChassis::IsSpinAttackOver(CStateManager& mgr, const CTriggerData& data) const {
  return !mSpinAttackActive;
}

bool CSplitterMainChassis::SpunIntoPlayer(CStateManager& mgr, const CTriggerData& data) const {
  return mSpunIntoPlayer;
}

bool CSplitterMainChassis::SpunIntoGeometry(CStateManager& mgr, const CTriggerData& data) const {
  return mSpunIntoGeometry;
}

bool CSplitterMainChassis::BreakOutOfSpin(CStateManager& mgr, const CTriggerData& data) const {
  return mBreakOutOfSpin;
}

bool CSplitterMainChassis::TooManyBounces(CStateManager& mgr, const CTriggerData& data) const {
  return mBounceCount >= mData.data.unknown_0xd5f34476;
}

bool CSplitterMainChassis::DeployFromHoldingTube(CStateManager& mgr,
                                                 const CTriggerData& data) const {
  return (mData.data.unknown_0xcef5c2fe & 2) != 0;
}

bool CSplitterMainChassis::ShouldDeploy(CStateManager& mgr, const CTriggerData& data) const {
  return mShouldDeploy;
}

bool CSplitterMainChassis::DeployPathOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CSplitterMainChassis::PrepareForDocking(CStateManager& mgr, const CTriggerData& data) const {
  return mDockingModuleId != kInvalidUniqueId;
}

void CSplitterMainChassis::Start(CStateManager& mgr, EStateMsg msg, float dt) {}

void CSplitterMainChassis::Idle(CStateManager& mgr, EStateMsg msg, float dt) {}

void CSplitterMainChassis::Inactive(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    RemoveMaterial(kMT_Orbit, mgr);
    break;
  default:
    break;
  }
}

void CSplitterMainChassis::Activate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mActivating = true;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_Generate) {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mActivating = false;
    break;
  }
}

void CSplitterMainChassis::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyController()->CommandMgr().SetSteeringSpeedRange(mLocomotionRatio, mLocomotionRatio);
    break;
  case kStateMsg_Update: {
    const CScriptWaypoint* lastWaypoint = TCastToConstPtr< CScriptWaypoint >(
        mgr.GetObjectById(mWaypointNavigation.GetLastDestination()));
    const CScriptWaypoint* waypoint =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mWaypointNavigation.GetDestination()));
    if (lastWaypoint != nullptr && waypoint != nullptr) {
      UpdateLocomotion(mgr);
      mWaypointNavigation.SetFaceVector(BodyController()->CommandMgr().GetFaceVector());
    }
    break;
  }
  default:
    break;
  }
}

void CSplitterMainChassis::Scanning(CStateManager& mgr, EStateMsg msg, float dt) {}

void CSplitterMainChassis::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyController()->CommandMgr().SetSteeringSpeedRange(mLocomotionRatio, mLocomotionRatio);
    if (mHasDestination) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    }
    mNearDestination = false;
    break;
  case kStateMsg_Update:
    if (mHasDestination) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    }
    break;
  default:
    break;
  }
  if (msg != kStateMsg_Deactivate) {
    UpdateLocomotion(mgr);
  }
}

void CSplitterMainChassis::LegStabAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mStabLeg >= 0 && mStabLeg <= 4) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mLegStabActive = true;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(skLegStabSeverities[mStabLeg]));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mLegStabTimer = mData.data.legStabAttackInterval;
    mLegStabActive = false;
    break;
  }
}

void CSplitterMainChassis::SpinAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mSpinTime > mData.data.spinAttackMaxTime) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else if (dt > 0.f) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f toTarget = target->GetTranslation() - GetTranslation();
        mTargetFar = toTarget.MagSquared() > 100.f;
        CVector3f desired =
            toTarget.IsMagnitudeSafe() ? toTarget.AsNormalized() : CVector3f::Zero();
        desired += 0.5f * CalculateSeparation(mgr);
        desired.SetZ(0.f);
        if (mTargetFar && desired.IsMagnitudeSafe()) {
          MoveSpinning(dt, dt * (mData.data.spinAttackLinearVelocity * desired.AsNormalized()));
        } else {
          MoveInOneFrameOR(GetTransform().TransposeRotate(mSpinVelocity), dt);
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterMainChassis::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mDodging = true;
    if (mDodgeDirection == pas::kSD_Down) {
      CSplitterCommandModule* module =
          TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
      if (module != nullptr) {
        module->SetReflectiveState(mgr);
      }
    }
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDirection, pas::kStep_Dodge));
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mDodgeTimer = mData.data.minDodgeInterval;
    mDodging = false;
    CSplitterCommandModule* module =
        TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
    if (module != nullptr) {
      module->SetNormalState(mgr);
    }
    break;
  }
  }
}

void CSplitterMainChassis::LaserSweepAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mLaserSweepActive = true;
    SetCollisionVulnerabilities(mgr, *CPatterned::GetDamageVulnerability(),
                                CDamageVulnerability::ReflectVulnerabilty());
    break;
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Zero));
    }
    const CSplitterCommandModule* module =
        TCastToConstPtr< CSplitterCommandModule >(mgr.GetObjectById(mCommandModuleId));
    if (module == nullptr || !module->IsLaserSweepActive()) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    }
    break;
  }
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    SetCollisionVulnerabilities(mgr, CDamageVulnerability::ReflectVulnerabilty(),
                                CDamageVulnerability::ReflectVulnerabilty());
    mLaserSweepTimer = mData.data.laserSweepInterval;
    mLaserSweepActive = false;
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    break;
  }
}

void CSplitterMainChassis::SpinCollisionReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCKnockBackCmd(
          GetTransform().GetForward(), mSpunIntoPlayer ? pas::kS_Two : pas::kS_Five));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterMainChassis::SpinBounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mSpinElapsed = 0.f;
    ++mBounceCount;
    if (mBounceDirection.IsMagnitudeSafe()) {
      mSpinVelocity = mBounceDirection.AsNormalized();
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mSpunIntoGeometry = false;
      mSpunIntoPlayer = false;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() > mBounceTimer) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else {
      const CVector3f velocity = (mData.data.spinAttackLinearVelocity * dt) * mSpinVelocity;
      MoveInOneFrameOR(GetTransform().TransposeRotate(velocity), dt);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterMainChassis::SpinTelegraph(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() > mData.data.spinAttackTelegraphTime) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mSpinTime = 0.f;
    break;
  }
}

void CSplitterMainChassis::SpinToIdle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCGenerateCmd(mBreakOutOfSpin ? pas::kGType_Five : pas::kGType_Three, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSplitterMainChassis::SpawnDrop(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mDeployState = 3;
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    const CVector3f position = GetTranslation();
    const CRayCastResult result = mgr.RayStaticIntersection(
        position, CVector3f::Down(), 1000.f,
        CMaterialFilter::MakeInclude(CMaterialList(skDropRayMaterial1, skDropRayMaterial2)));
    if (result.IsValid()) {
      mLandingPoint = position + result.GetTime() * CVector3f::Down();
    } else {
      mLandingPoint = position;
    }
    mSpinVelocity = CVector3f::Zero();
    break;
  }
  case kStateMsg_Update:
    if (GetTranslation().GetZ() - mLandingPoint.GetZ() <= 10.f) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mHitByPlayerProjectile = true;
    break;
  }
}

void CSplitterMainChassis::HoldingTube(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mDeployState = 1;
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    mVerticalMovement = true;
    break;
  default:
    break;
  }
}

void CSplitterMainChassis::WaitForDocking(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CSplitterCommandModule* module =
        TCastToConstPtr< CSplitterCommandModule >(mgr.ObjectById(mDockingModuleId));
    if (module != nullptr && mCommandModuleId == kInvalidUniqueId &&
        module->GetDockingTargetId() == GetUniqueId()) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      SetCollisionVulnerabilities(mgr, *CPatterned::GetDamageVulnerability(),
                                  CDamageVulnerability::ReflectVulnerabilty());
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  }
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_One));
    }
    const CSplitterCommandModule* module =
        TCastToConstPtr< CSplitterCommandModule >(mgr.ObjectById(mDockingModuleId));
    if (module == nullptr || mCommandModuleId != kInvalidUniqueId ||
        module->GetDockingTargetId() != GetUniqueId()) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    }
    break;
  }
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    SetCollisionVulnerabilities(mgr, CDamageVulnerability::ReflectVulnerabilty(),
                                CDamageVulnerability::ReflectVulnerabilty());
    break;
  }
}

void CSplitterMainChassis::WaitForSnatch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CSplitterCommandModule* module =
        TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
    if (module != nullptr) {
      module->SetInvulnerableState(mgr);
    }
    break;
  }
  case kStateMsg_Update:
    if (TCastToConstPtr< CIngSnatchingSwarm >(mgr.GetObjectById(mSnatchId)) == nullptr) {
      mSnatchId = kInvalidUniqueId;
    }
    break;
  case kStateMsg_Deactivate: {
    CSplitterCommandModule* module =
        TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
    if (module != nullptr) {
      module->SetNormalState(mgr);
    }
    mSnatchId = kInvalidUniqueId;
    mSpinAttackTimer = mData.data.spinAttackInterval;
    break;
  }
  }
}

void CSplitterMainChassis::FollowDeployPath(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  const CVector3f move = BodyController()->CommandMgr().GetMoveVector();
  BodyController()->CommandMgr().ClearLocomotionCmds();
  switch (msg) {
  case kStateMsg_Activate:
    mWaypointNavigation.SetDestination(GetConnectedObject(mgr, kSS_Attack, kSM_Follow));
    mSpinVelocity = CVector3f::Zero();
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Nine, -1));
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    BodyController()->SetTurnSpeed(180.f);
    mDeployState = 2;
    break;
  case kStateMsg_Update:
    if (dt > 0.f) {
      if (move.IsMagnitudeSafe()) {
        CVector3f direction = move.AsNormalized();
        if (mSpinVelocity.IsMagnitudeSafe()) {
          const float limit = mData.data.spinAttackTurnSpeed * dt;
          if (CVector3f::GetAngleDiff(mSpinVelocity, direction) > limit) {
            direction = CVector3f::Slerp(mSpinVelocity.AsNormalized(), direction,
                                         CRelAngle::FromRadians((M_PIF / 180.f) * limit));
          }
        }
        mSpinVelocity = dt * (mData.data.deploymentSpeed * direction);
      }
      MoveInOneFrameOR(GetTransform().TransposeRotate(mSpinVelocity), dt);
      BodyController()->FaceDirectionOnSurface(mSpinVelocity, -GetTransform().GetUp(), dt);
    }
    break;
  case kStateMsg_Deactivate:
    BodyController()->SetTurnSpeed(mTurnRate);
    break;
  }
}

void CSplitterMainChassis::DeploymentLanding(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mVerticalMovement = false;
    mDeployState = 4;
    mDisabledAnimationDeltas = 0;
    break;
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(CBCJumpCmd(GetTranslation() + CVector3f::Down(),
                                                           pas::kJT_Ambush, pas::kJS_Loop, 0,
                                                           CBCJumpCmd::kFF_AmbushJump));
    } else {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    }
    if (mDeployState != 5) {
      MoveInOneFrameOR(GetTransform().TransposeRotate(mSpinVelocity), dt);
      const CVector3f forward = GetTransform().GetForward();
      if (CMath::AbsF(forward.GetZ()) >= 0.00001f) {
        CVector3f direction(forward.GetX(), forward.GetY(), 0.f);
        if (direction.IsMagnitudeSafe()) {
          const float limit = 180.f * dt;
          if (CVector3f::GetAngleDiff(forward, direction) > limit) {
            direction = CVector3f::Slerp(forward, direction,
                                         CRelAngle::FromRadians((M_PIF / 180.f) * limit));
          }
          const CVector3f position = GetTranslation();
          SetTransform(CTransform4f::LookAt(position, position + direction, CVector3f::Up()));
        }
      }
    }
    break;
  }
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mDeployState = 5;
    mDisabledAnimationDeltas = 0;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    Stop();
    const CVector3f forward = GetTransform().GetForward();
    if (CMath::AbsF(forward.GetZ()) >= 0.00001f) {
      const CVector3f position = GetTranslation();
      SetTransform(CTransform4f::LookAt(
          position, position + CVector3f(forward.GetX(), forward.GetY(), 0.f), CVector3f::Up()));
    }
    break;
  }
  }
}

void CSplitterMainChassis::HeadExplosion(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mLostHead = false;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Eight));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mSpinAttackTimer = mData.data.spinAttackInterval;
    break;
  }
}

void CSplitterMainChassis::SelectTarget(CStateManager& mgr, float dt) {
  mTargetId = mgr.GetPlayer(0)->GetUniqueId();
  mLineOfSight.SetTarget(mTargetId);
  if (mCommandModuleId != kInvalidUniqueId) {
    CSfxManager::AddEmitter(mData.data.sound_Alerted, GetTranslation(), 0x7f,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
  }
  ResetAttackTimers();
  SendScriptMsgs(kSS_Attack, mgr, GetUniqueId(), kSM_None);
}

void CSplitterMainChassis::SetTargetDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mHasDestination = false;
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    if (!mLineOfSight.HasLineOfSight()) {
      destination = target->GetTranslation();
      mHasDestination = true;
    } else {
      const CTeamAiRole* role = CScriptTeamAiMgr::GetTeamAiRole(mgr, mTeamAiMgrId, GetUniqueId());
      if (role != nullptr && role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
        const float limit =
            0.5f * (mData.data.spinAttackMinAttackRange + mData.data.spinAttackMaxAttackRange);
        if ((target->GetTranslation() - GetTranslation()).MagSquared() >= limit * limit) {
          destination = target->GetTranslation();
          mHasDestination = true;
        }
      } else {
        destination = target->GetTranslation();
        mHasDestination = true;
      }
    }
  }
  mPathFindNavigation.SetDestination(destination);
  mDestObj = kInvalidUniqueId;
  JoinTeam(mgr);
}

void CSplitterMainChassis::SetRetreatDest(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  mHasDestination = false;
  const CScriptWaypoint* waypoint =
      TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mWaypointId));
  if (waypoint != nullptr) {
    destination = waypoint->GetTranslation();
    mHasDestination = true;
  }
  mPathFindNavigation.SetDestination(destination);
  mDestObj = kInvalidUniqueId;
}

void CSplitterMainChassis::SetupSpinAttack(CStateManager& mgr, float dt) {
  mSpinVelocity = CVector3f::Zero();
  mBounceCount = 0;
  mSpinTime = 0.f;
  mSpinAttackActive = true;
  mSpunIntoPlayer = false;
  mSpunIntoGeometry = false;
  mBreakOutOfSpin = false;
  BodyController()->SetLocomotionType(pas::kLT_Combat);
  SetCollisionVulnerabilities(mgr, mData.spinAttackVulnerability, mData.spinAttackVulnerability);
  CSplitterCommandModule* module =
      TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
  if (module != nullptr) {
    module->SetChassisShieldState(mgr);
  }
  mCurDamageRemTime = 0.f;
  CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId());
}

void CSplitterMainChassis::EndSpinAttack(CStateManager& mgr, float dt) {
  mSpinAttackActive = false;
  mSpunIntoPlayer = false;
  mSpunIntoGeometry = false;
  mBreakOutOfSpin = false;
  mSpinAttackTimer = mData.data.spinAttackInterval;
  BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  SetCollisionVulnerabilities(mgr, CDamageVulnerability::ReflectVulnerabilty(),
                              CDamageVulnerability::ReflectVulnerabilty());
  CSplitterCommandModule* module =
      TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
  if (module != nullptr) {
    module->SetNormalState(mgr);
  }
  CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId(), false);
}

void CSplitterMainChassis::SetupLaserSweep(CStateManager& mgr, float dt) {
  CSplitterCommandModule* module =
      TCastToPtr< CSplitterCommandModule >(mgr.ObjectById(mCommandModuleId));
  if (module != nullptr) {
    const CVector3f beamPosition = module->GetBeamPosition();
    CVector3f endPosition = beamPosition + 30.f * GetTransform().GetForward();
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      endPosition = target->GetAimPosition(mgr, 0.f);
    }
    CVector3f direction = endPosition - beamPosition;
    const CVector3f flat(direction.GetX(), direction.GetY(), 0.f);
    if (direction.IsMagnitudeSafe() && flat.IsMagnitudeSafe()) {
      const float angle = CVector3f::GetAngleDiff(direction, flat);
      if (direction.GetZ() > 0.f) {
        if (angle > (M_PIF / 4.f)) {
          direction = direction.Magnitude() * CVector3f::Slerp(flat.AsNormalized(),
                                                               direction.AsNormalized(),
                                                               CRelAngle::FromRadians(M_PIF / 4.f));
        }
      } else if (angle > (M_PIF / 12.f)) {
        direction =
            direction.Magnitude() * CVector3f::Slerp(flat.AsNormalized(), direction.AsNormalized(),
                                                     CRelAngle::FromRadians(M_PIF / 12.f));
      }
    }
    const float cosine = CMath::FastCosR(M_PIF / 4.f);
    const float sine = CMath::FastSinR(M_PIF / 4.f);
    const CVector3f start(
        beamPosition.GetX() + (cosine * direction.GetX() + sine * direction.GetY()),
        beamPosition.GetY() + (cosine * direction.GetY() - sine * direction.GetX()),
        beamPosition.GetZ() + direction.GetZ());
    const CVector3f end(beamPosition.GetX() + (cosine * direction.GetX() - sine * direction.GetY()),
                        beamPosition.GetY() + (cosine * direction.GetY() + sine * direction.GetX()),
                        beamPosition.GetZ() + direction.GetZ());
    module->RequestLaserSweep(start, end);
    CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                  GetUniqueId());
  }
}

void CSplitterMainChassis::SetMorphballStabLeg(CStateManager& mgr, float dt) { mStabLeg = 0; }

void CSplitterMainChassis::FindBestStabLeg(CStateManager& mgr, float dt) {
  mStabLeg = -1;
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const CVector3f offset = target->GetTranslation() - GetTranslation();
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f right = GetTransform().GetRight();
    if (CVector3f::Dot(forward, offset) > 0.f) {
      mStabLeg = CVector3f::Dot(right, offset) > 0.f ? 1 : 2;
    } else {
      mStabLeg = CVector3f::Dot(right, offset) > 0.f ? 3 : 4;
    }
  }
}

void CSplitterMainChassis::FindBestDodgeDirection(CStateManager& mgr, float dt) {
  mDodgeDirection = pas::kSD_Invalid;
  if ((mData.data.unknown_0xcef5c2fe & 0x30) && mTimeSinceShot < 1.f && mDodgeTimer <= 0.f) {
    if (mgr.Random()->Range(0.f, 100.f) <= mData.data.dodgeChance) {
      const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId));
      if (player != nullptr && (player->GetOrbitTargetId() == GetUniqueId() ||
                                player->GetOrbitTargetId() == mCommandModuleId)) {
        const CVector3f position = GetTranslation();
        const CAABox box(position - CVector3f(50.f, 50.f, 50.f),
                         position + CVector3f(50.f, 50.f, 50.f));
        rstl::reserved_vector< TUniqueId, 1024 > nearList;
        mgr.BuildNearList(nearList, box,
                          CMaterialFilter::MakeInclude(CMaterialList(skNearListMaterial)), nullptr);
        for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
             it != nearList.end(); ++it) {
          const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(*it));
          if (weapon != nullptr && weapon->GetType() == kWT_Missile &&
              weapon->GetCurrentAreaId() == GetCurrentAreaId()) {
            if (mData.data.unknown_0xcef5c2fe & 0x20) {
              mDodgeDirection = pas::kSD_Down;
            }
            if (!(mData.data.unknown_0xcef5c2fe & 0x10)) {
              return;
            }
            mDodgeDirection = FindBestDodgeStep(mgr, position - weapon->GetTranslation());
            return;
          }
        }
      }
    } else {
      mDodgeTimer = mData.data.minDodgeInterval;
    }
  }
}

void CSplitterMainChassis::BounceOffPlayer(CStateManager& mgr, float dt) {
  mBounceDirection = -mSpinVelocity;
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    mBounceDirection = GetTranslation() - target->GetTranslation();
  }
  mBounceDirection.SetZ(0.f);
  mBounceTimer = mBreakOutOfSpin ? 0.75f : 0.5f;
}

void CSplitterMainChassis::BounceOffGeometry(CStateManager& mgr, float dt) {
  mBounceDirection = -mSpinVelocity;
  if (mCollisionNormal.IsMagnitudeSafe()) {
    const CVector3f normal = mCollisionNormal.AsNormalized();
    mBounceDirection = mSpinVelocity - 2.f * CVector3f::Dot(mSpinVelocity, normal) * normal;
  }
  mBounceDirection.SetZ(0.f);
  mBounceTimer = 0.5f;
}

CEntity* REL_LoadSplitterMainChassis(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSplitterMainChassis sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSplitterMainChassis.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const SSplitterMainChassisData data(sldrThis.splitterMainChassisData);
  return rs_new CSplitterMainChassis(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, &sldrThis.splitterMainChassisData.ingPossessionData),
      data);
}

static void SetFuncPtrs() {
  static SSplitterMainChassis_FuncPtrs funcPtrs;
  funcPtrs.mLoadMainChassis = &REL_LoadSplitterMainChassis;
  funcPtrs.mLoadCommandModule = &REL_LoadSplitterCommandModule;
  funcPtrs.mAutoDestruct = reinterpret_cast< void (CSplitterMainChassis::*)(float) >(
      &CSplitterCommandModule::AutoDestruct);
  SetSSplitterMainChassis_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSplitterMainChassis_FuncPtrs(nullptr); }
