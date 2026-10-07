#include "MetroidPrime/Enemies/CTryclops.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CInternalCollisionStructure.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTryclops.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "REL/REL_Setup.h"

#include <math.h>

static EMaterialTypes SolidMaterial = kMT_Unknown59;

static const CDamageVulnerability::TWeaponVulnerability skPowerBombVulnerability =
    CDamageVulnerability::MakeWeaponVulnerability(kWT_PowerBomb,
                                                  CWeaponTypeVulnerability::Normal());

CVector3f CTryclops::kBombPosOffset(0.f, 0.f, -0.3f);
const char* const CTryclops::kMouthLctr = "ballGrab_locator";
static const char* const skRootLocator = "Skeleton_Root";

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"InDetectionRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::InDetectionRange)},
    {"InAttackPosition",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::InAttackPosition)},
    {"Inside", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::Inside)},
    {"InMaxRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::InMaxRange)},
    {"InRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::InRange)},
    {"InPosition", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::InPosition)},
    {"SpotPlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::SpotPlayer)},
    {"HearShot", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::HearShot)},
    {"CoverBlown", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::CoverBlown)},
    {"IsDizzy", static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::IsDizzy)},
    {"ShouldRetreat",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::ShouldRetreat)},
    {"HasRetreatPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CTryclops::HasRetreatPattern)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Patrol)},
    {"Suck", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Suck)},
    {"Crouch", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Crouch)},
    {"PathFindEx", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::PathFindEx)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::PathFind)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Attack)},
    {"TargetCover", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::TargetCover)},
    {"SelectTarget", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::SelectTarget)},
    {"JumpBack", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::JumpBack)},
    {"Approach", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Approach)},
    {"GetUp", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::GetUp)},
    {"Shuffle", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Shuffle)},
    {"TurnAround", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::TurnAround)},
    {"TargetPatrol", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::TargetPatrol)},
    {"TargetPlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::TargetPlayer)},
    {"Dizzy", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Dizzy)},
    {"Cover", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::Cover)},
    {"FixedDelay", static_cast< CPatterned::StateMachine::StateFunc >(&CTryclops::FixedDelay)},
};

static CDamageVulnerability MakePowerBombVulnerability() {
  return CDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty(), &skPowerBombVulnerability,
                              1, CDamageVulnerability::kOF_Normal);
}

CTryclops::CTryclops(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, float suckForceMultiplier, float suckAngle,
                     float suckRange, float launchSpeed)
: CPatterned(static_cast< EPatternedAI >(0x3f), uid, name, kFT_Zero, info, xf, mData, pInfo,
             kMT_Ground, kCT_One, kBT_BiPedal, actParms)
, mCollisionActorManager(nullptr)
, mPathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mPlayerRotation(CTransform4f::Identity())
, mSuckForceMultiplier(suckForceMultiplier)
, mMinSuckAngleProj(cosf(CRelAngle::FromDegrees(suckAngle * 0.5f).AsRadians()))
, mSuckRange(suckRange)
, mLaunchSpeed(launchSpeed)
, mIgnoreMorphballTimer(0.f)
, x8f4_(0)
, mBombId(kInvalidUniqueId)
, x8fa_(kInvalidUniqueId)
, mTargetPlayerId(kInvalidUniqueId)
, mPowerBombVulnerability(MakePowerBombVulnerability())
, mShotTarget(false)
, mTargetingBomb(false)
, mVulnerable(false)
, mDizzy(false)
, mDizzyTimer(0.f) {
  SetDrawShadow(false);
  mKnockBackController.EnableKnockBackPhysics(false);
  mLookAtDeathDir = false;
}

CTryclops::~CTryclops() {}

void CTryclops::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    SetupCollisionManager(mgr);
    break;
  case kSM_Delete:
    mCollisionActorManager->Destroy(mgr);
    break;
  case kSM_Deactivate:
    mCollisionActorManager->SetActive(mgr, false);
    break;
  case kSM_AreaLoaded: {
    const TAreaId areaId = GetCurrentAreaId();
    mPathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea);
  } break;
  default:
    break;
  }
}

bool CTryclops::InMaxRange(CStateManager& mgr, const CTriggerData&) const {
  if (mBombId != kInvalidUniqueId) {
    return true;
  }

  const float detectionRange = mDetectionRange;
  const float detectionHeight = mDetectionHeightRange;
  float nearestDistSq = detectionRange * detectionRange;
  const float heightSq = detectionHeight * detectionHeight;
  const float negRange = -detectionRange;
  const CAABox bounds(GetTranslation() + CVector3f(negRange, negRange, 0.f),
                      GetTranslation() +
                          CVector3f(detectionRange, detectionRange, detectionHeight));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeInclude(CMaterialList(kMT_Bomb)), this);
  mBombId = kInvalidUniqueId;
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    if (const CBomb* const bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(*it))) {
      if (!bomb->IsBeingDragged()) {
        const CVector3f delta = bomb->GetTranslation() - GetTranslation();
        const float distSq = delta.MagSquared();
        if (distSq < nearestDistSq) {
          bool inHeightRange = true;
          if (detectionHeight > 0.f) {
            inHeightRange = delta.GetZ() * delta.GetZ() < heightSq;
          }
          if (inHeightRange &&
              mPathFindSearch.OnPath(bomb->GetTranslation()) == CPathFindSearch::kR_Success) {
            mBombId = bomb->GetUniqueId();
            nearestDistSq = distSq;
          }
        }
      }
    }
  }

  if (mBombId != kInvalidUniqueId) {
    if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
      bomb->SetFuseDisabled(true);
      bomb->SetIsBeingDragged(true);
      return true;
    }
  }

  return false;
}

bool CTryclops::InDetectionRange(CStateManager& mgr, const CTriggerData& data) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    const bool morphed = (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                              ? player->GetMorphballTransitionState()
                              : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed;
    const bool unattached = player->GetAttachedActorId() == kInvalidUniqueId;
    const bool ignore = mIgnoreMorphballTimer > 0.f;
    if (morphed && unattached && !ignore && CPatterned::InDetectionRange(mgr, data) &&
        mPathFindSearch.OnPath(player->GetBallPosition()) == CPathFindSearch::kR_Success) {
      return true;
    }
  }
  return false;
}

void CTryclops::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (mAlive && mIgnoreMorphballTimer > 0.f) {
    mIgnoreMorphballTimer -= dt;
  }
  mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetPlayerId))) {
    if (player->GetAttachedActorId() == GetUniqueId()) {
      if (!mDizzy) {
        mDizzy = player->GetAttachedActorStruggle() == 1.f && !BallCloseToCollision(*player, mgr);
      }
      if (mDizzyTimer > 0.f) {
        mDizzyTimer -= dt;
        if (mDizzyTimer <= 0.f) {
          mDizzy = true;
          mDizzyTimer = 0.f;
        }
      }
    }
  }
}

void CTryclops::TargetCover(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    if (mBombId != kInvalidUniqueId) {
      if (const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
        SetDestPos(bomb->GetTranslation());
      } else {
        mBombId = kInvalidUniqueId;
      }
    }
    break;
  }
}

bool CTryclops::InAttackPosition(CStateManager& mgr, const CTriggerData&) const {
  mTargetPlayerId = kInvalidUniqueId;
  float nearestDistSq = 3.4028235e38f;
  const CVector3f ownPos = GetTranslation();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
             ? player->GetMorphballTransitionState()
             : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed &&
        player->GetAttachedActorId() == kInvalidUniqueId) {
      const CVector3f pos = player->GetTranslation();
      const CVector3f center = pos + CVector3f(0.f, 0.f, player->GetMorphBall()->GetBallRadius());
      const CAABox bounds = player->GetBoundingBox();
      if (ObjectInVortexArea(pos, center, bounds, mgr)) {
        const CVector3f delta = pos - ownPos;
        const float distSq = delta.MagSquared();
        if (distSq < nearestDistSq) {
          nearestDistSq = distSq;
          mTargetPlayerId = player->GetUniqueId();
        }
      }
    }
  }
  return mTargetPlayerId != kInvalidUniqueId;
}

bool CTryclops::InRange(CStateManager& mgr, const CTriggerData&) const {
  if (mBombId != kInvalidUniqueId) {
    if (const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
      const CVector3f pos = bomb->GetTranslation();
      return ObjectInVortexArea(pos, pos, *bomb->GetTouchBounds(), mgr);
    }
  }
  return false;
}

bool CTryclops::ObjectInVortexArea(const CVector3f& pos, const CVector3f& center,
                                   const CAABox& bounds, const CStateManager& mgr) const {
  const CAABox ownBounds = GetBoundingBox();
  if (bounds.DoBoundsOverlap(ownBounds)) {
    return true;
  }
  const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
  const CVector3f forward = GetTransform().GetForward();
  const CVector3f delta = center - (xf.GetTranslation() - 1.f * forward);
  const float distance = delta.Magnitude();
  const float projection = CVector3f::Dot(delta.AsNormalized(), forward);
  if (distance < mSuckRange) {
    const CVector3f angleDelta = pos - (xf.GetTranslation() - 4.f * forward);
    const float angleProjection = CVector3f::Dot(angleDelta.AsNormalized(), forward);
    if (projection > 0.f && angleProjection > mMinSuckAngleProj) {
      if (distance > 2.f) {
        static const CMaterialFilter kSolidFilter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Unknown59),
            CMaterialList(kMT_Character, kMT_Player, kMT_NoPlatformCollision));
        if (!CGameCollision::RayStaticLineOfSightTest(
                mgr, xf.GetTranslation(), (1.f / distance) * delta,
                distance - gpTweakPlayerA->GetBallRadius(), kSolidFilter)) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

bool CTryclops::TargetCaught(const CVector3f& pos, float range) const {
  const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
  const CVector3f delta = pos - xf.GetTranslation();
  return delta.MagSquared() <= range;
}

bool CTryclops::Inside(CStateManager& mgr, const CTriggerData& data) const {
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetPlayerId))) {
    const CTransform4f& xf = player->GetTransform();
    const CVector3f pos =
        xf.GetTranslation() + CVector3f(0.f, 0.f, player->GetMorphBall()->GetBallRadius());
    mPlayerRotation = xf.GetRotation();
    return TargetCaught(pos, data.GetFloat());
  }
  return false;
}

bool CTryclops::InPosition(CStateManager& mgr, const CTriggerData& data) const {
  if (mBombId != kInvalidUniqueId) {
    if (const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
      return TargetCaught(bomb->GetTranslation(), data.GetFloat());
    }
  }
  return false;
}

bool CTryclops::HearShot(CStateManager& mgr, const CTriggerData&) const {
  mVulnerable = false;
  if (mBombId != kInvalidUniqueId) {
    if (TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
      mVulnerable = true;
      return false;
    }
    mBombId = kInvalidUniqueId;
    return true;
  }
  return true;
}

void CTryclops::Suck(CStateManager& mgr, EStateMsg msg, float dt) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
    switch (msg) {
    case kStateMsg_Activate:
      player->EnableLeaveMorphBall(false);
      player->GetMorphBall()->DisableHalfPipeStatus();
      BodyController()->SetLocomotionType(pas::kLT_Internal6);
      break;
    case kStateMsg_Update:
      AttractPlayer(*player, mgr, dt);
      break;
    case kStateMsg_Deactivate:
      ReleasePlayer(*player, mgr);
      break;
    }
  }
}

void CTryclops::SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
      ReleasePlayer(*player, mgr);
    }
    mTargetingBomb = true;
    BodyController()->SetLocomotionType(pas::kLT_Internal6);
    break;
  case kStateMsg_Update:
    AttractBomb(mgr, dt);
    break;
  }
}

void CTryclops::AttractPlayer(CPlayer& player, CStateManager& mgr, float dt) {
  const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
  const CVector3f delta = player.GetTranslation() - xf.GetTranslation();
  if ((player.GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
           ? player.GetMorphballTransitionState()
           : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
    const float distance = delta.Magnitude();
    if (distance < 3.f) {
      player.Stop();
      CenterPlayer(player, mgr, xf.GetTranslation(), dt);
    } else {
      const float force = mSuckForceMultiplier * (mSuckRange / (distance * distance));
      const CVector3f massDirection = player.GetMass() * -delta;
      const CVector3f forceVector = force * massDirection;
      player.ApplyForceWR(forceVector, CAxisAngle::Identity());
    }
  }
}

void CTryclops::AttractBomb(CStateManager& mgr, float dt) {
  if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
    const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
    const CVector3f target = xf.GetTranslation() + kBombPosOffset;
    const CVector3f direction = (target - bomb->GetTranslation()).AsNormalized();
    bomb->SetVelocityWR((1.f / (2.f * dt)) * direction);
  }
}

void CTryclops::CenterPlayer(CPlayer& player, CStateManager& mgr, const CVector3f& pos, float dt) {
  const float ballRadius = player.GetMorphBall()->GetBallRadius();
  const CVector3f center = player.GetTranslation() + CVector3f(0.f, 0.f, ballRadius);
  const CVector3f direction = (pos - center).AsNormalized();
  const CVector3f velocity = (1.f / (2.f * dt)) * direction;
  player.SetVelocityWR(velocity);
}

void CTryclops::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
    switch (msg) {
    case kStateMsg_Activate: {
      const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
      if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
        SetDestPos(waypoint->GetTranslation());
      }
      GrabPlayer(*player, mgr);
      SendScriptMsgs(kSS_Inside, mgr, kInvalidUniqueId, kSM_None);
      player->AttachActorToPlayer(GetUniqueId(), true);
      player->EnableLeaveMorphBall(false);
      player->GetMorphBall()->DisableHalfPipeStatus();
      BodyController()->SetLocomotionType(pas::kLT_Combat);
      break;
    }
    case kStateMsg_Update: {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
      SetPlayerPosition(*player, mgr, xf.GetTranslation());
      break;
    }
    case kStateMsg_Deactivate:
      if (player->GetAttachedActorId() == GetUniqueId()) {
        player->DetachActorFromPlayer();
      }
      ReleasePlayer(*player, mgr);
      break;
    }
  }
}

void CTryclops::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate: {
    const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
    if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
      SetDestPos(waypoint->GetTranslation());
    }
    if (mBombId != kInvalidUniqueId) {
      if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
        bomb->SetFuseDisabled(false);
      } else {
        mBombId = kInvalidUniqueId;
      }
    }
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  }
  case kStateMsg_Update:
    SetBombPosition(mgr);
    break;
  }
}

void CTryclops::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  const CVector3f forward = GetTransform().GetForward();
  const CVector3f move = GetBodyController()->GetCommandMgr().GetMoveVector();
  if (CVector3f::Dot(forward, move) < 0.f && move.CanBeNormalized()) {
    BodyController()->CommandMgr().ClearLocomotionCmds();
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), move.AsNormalized(), 1.f));
  }
  ApplySeparationBehavior(mgr);
}

void CTryclops::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) { PathFind(mgr, msg, dt); }

void CTryclops::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  ApplySeparationBehavior(mgr);
  switch (msg) {
  case kStateMsg_Activate:
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Update:
    SetBombPosition(mgr);
    break;
  }
}

void CTryclops::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
    switch (msg) {
    case kStateMsg_Activate:
      GrabPlayer(*player, mgr);
      player->EnableLeaveMorphBall(false);
      player->GetMorphBall()->DisableHalfPipeStatus();
      player->AttachActorToPlayer(GetUniqueId(), true);
      mDizzyTimer = 15.f;
      mDizzy = false;
      break;
    case kStateMsg_Update: {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
      SetPlayerPosition(*player, mgr, xf.GetTranslation());
      break;
    }
    case kStateMsg_Deactivate:
      mDizzyTimer = 0.f;
      break;
    }
  }
}

void CTryclops::SetPlayerPosition(CPlayer& player, CStateManager& mgr, const CVector3f&) {
  GrabPlayer(player, mgr);
  CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr)) * mPlayerRotation;
  xf.AddTranslation(CVector3f(0.f, 0.f, -0.6f));
  player.SetTransform(xf);
}

void CTryclops::SetBombPosition(CStateManager& mgr) {
  if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
    CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
    xf.AddTranslation(kBombPosOffset);
    bomb->SetTransform(xf);
  }
}

void CTryclops::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
    switch (msg) {
    case kStateMsg_Activate:
      GrabPlayer(*player, mgr);
      player->EnableLeaveMorphBall(false);
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mShotTarget = false;
      break;
    case kStateMsg_Update:
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
        BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
      }
      if (!mShotTarget) {
        const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
        SetPlayerPosition(*player, mgr, xf.GetTranslation());
      }
      break;
    case kStateMsg_Deactivate:
      mAnimationState.SetState(CAnimationState::kAS_NotReady);
      if (player->GetAttachedActorId() == GetUniqueId()) {
        player->DetachActorFromPlayer();
      }
      ReleasePlayer(*player, mgr);
      break;
    }
  } else {
    mAnimationState.SetState(CAnimationState::kAS_Over);
  }
}

void CTryclops::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mShotTarget = false;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
    }
    if (!mShotTarget) {
      SetBombPosition(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CTryclops::ShootPlayer(CPlayer& player, CStateManager& mgr, const CTransform4f& xf,
                            float speed) {
  const CVector3f direction = xf.GetForward().AsNormalized();
  player.EnableLeaveMorphBall(true);
  if ((player.GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
           ? player.GetMorphballTransitionState()
           : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
    mShotTarget = true;
    mIgnoreMorphballTimer = 1.5f;
    player.Stop();
    CTransform4f playerXf = xf * mPlayerRotation;
    playerXf.AddTranslation(CVector3f(0.f, 0.f, -0.5f));
    player.Teleport(playerXf, mgr, false);
    player.ApplyImpulseWR(speed * (player.GetMass() * direction), CAxisAngle::Identity());
    player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    player.AddMaterial(kMT_Unknown59, mgr);
    mgr.ApplyDamage(
        GetUniqueId(), player.GetUniqueId(), GetUniqueId(), GetContactDamage(),
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
        CVector3f::Zero());
  }
}

void CTryclops::ShootBomb(CStateManager& mgr, const CTransform4f& xf) {
  if (mBombId != kInvalidUniqueId) {
    const CVector3f direction = xf.GetForward().AsNormalized();
    if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
      bomb->SetVelocityWR((mgr.Random()->Float() * 5.f + 20.f) * direction);
      bomb->SetConstantAccelerationWR(CVector3f(0.f, 0.f, -CPhysicsActor::GravityConstant()));
    }
  }
  mVulnerable = false;
  mShotTarget = true;
  mBombId = kInvalidUniqueId;
}

bool CTryclops::SpotPlayer(CStateManager& mgr, const CTriggerData&) const {
  if (mBombId != kInvalidUniqueId) {
    if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        const CPlayer* player = mgr.GetPlayer(i);
        if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                 ? player->GetMorphballTransitionState()
                 : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
          const CVector3f playerDelta = player->GetTranslation() - GetTranslation();
          const CVector3f bombDelta = bomb->GetTranslation() - GetTranslation();
          if (playerDelta.MagSquared() < bombDelta.MagSquared()) {
            bomb->SetFuseDisabled(false);
            bomb->SetIsBeingDragged(false);
            mBombId = kInvalidUniqueId;
            return true;
          }
        }
      }
      return false;
    }
  }
  return true;
}

void CTryclops::TurnAround(CStateManager& mgr, EStateMsg msg, float) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
    switch (msg) {
    case kStateMsg_Activate: {
      if (mBombId == kInvalidUniqueId) {
        GrabPlayer(*player, mgr);
        player->EnableLeaveMorphBall(false);
      }
      TUniqueId waypointId = GetConnectedObject(mgr, kSS_Modify, kSM_Follow);
      bool retreat = false;
      if (waypointId == kInvalidUniqueId) {
        waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
        retreat = true;
      }
      if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
        CVector3f direction = retreat ? waypoint->GetTransform().GetForward()
                                      : waypoint->GetTranslation() - GetTranslation();
        CVector3f forward = GetTransform().GetForward();
        forward.SetZ(0.f);
        direction.Normalize();
        direction.SetZ(0.f);
        SetDestPos(GetTranslation() + direction);
        if (CMath::AbsF(CVector3f::Dot(forward, direction)) < 0.9998f) {
          mAnimationState.SetState(CAnimationState::kAS_Ready);
        }
      }
      break;
    }
    case kStateMsg_Update:
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Turn)) {
        const CVector3f dir = mDestPos - GetTranslation();
        if (dir.IsMagnitudeSafe()) {
          BodyController()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), dir.AsNormalized(), 1.f));
        } else {
          mAnimationState.SetState(CAnimationState::kAS_Over);
        }
      }
      if (mBombId == kInvalidUniqueId) {
        const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
        SetPlayerPosition(*player, mgr, xf.GetTranslation());
      } else {
        SetBombPosition(mgr);
      }
      break;
    case kStateMsg_Deactivate:
      mAnimationState.SetState(CAnimationState::kAS_NotReady);
      ReleasePlayer(*player, mgr);
      if (player->GetAttachedActorId() == GetUniqueId()) {
        player->DetachActorFromPlayer();
      }
      break;
    }
  } else {
    mAnimationState.SetState(CAnimationState::kAS_Over);
  }
}

void CTryclops::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile:
    if (mBombId == kInvalidUniqueId) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
        mCollisionActorManager->SetActive(mgr, false);
        ShootPlayer(*player, mgr, GetLctrTransform(node.GetLocatorName()),
                    mDizzy ? 5.f : mLaunchSpeed);
      }
    } else {
      ShootBomb(mgr, GetLctrTransform(node.GetLocatorName()));
    }
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

const CDamageVulnerability* CTryclops::GetDamageVulnerability() const {
  if (mVulnerable) {
    return CPatterned::GetDamageVulnerability();
  }
  return &mPowerBombVulnerability;
}

const CDamageVulnerability* CTryclops::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                              const CDamageInfo&) const {
  if (mVulnerable) {
    return CPatterned::GetDamageVulnerability();
  }
  return &mPowerBombVulnerability;
}

void CTryclops::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (mAlive) {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId));
    if (player && player->GetAttachedActorId() == GetUniqueId()) {
      ReleasePlayer(*player, mgr);
      player->DetachActorFromPlayer();
    } else if (mBombId != kInvalidUniqueId) {
      if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
        bomb->SetFuseDisabled(false);
        bomb->SetIsBeingDragged(false);
      }
      mBombId = kInvalidUniqueId;
    }
  }
  CPatterned::Death(mgr, direction, state);
}

void CTryclops::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    SetDestPos(mWaypointNavigation.GetDestinationPosition());
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CTryclops::Cover(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mTargetingBomb) {
      mIgnoreMorphballTimer = 1.5f;
    }
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId));
    if (player && player->GetAttachedActorId() == GetUniqueId()) {
      ReleasePlayer(*player, mgr);
      player->DetachActorFromPlayer();
    }
    break;
  }
}

void CTryclops::FixedDelay(CStateManager& mgr, EStateMsg msg, float dt) {}

bool CTryclops::CoverBlown(CStateManager&, const CTriggerData&) const {
  switch (mPathFindSearch.OnPath(GetTranslation())) {
  case CPathFindSearch::kR_InvalidArea:
    return false;
  default:
    return true;
  }
}

bool CTryclops::IsDizzy(CStateManager&, const CTriggerData&) const { return mDizzy; }

bool CTryclops::ShouldRetreat(CStateManager& mgr, const CTriggerData&) const {
  const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Modify, kSM_Next);
  if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
    const_cast< CTryclops* >(this)->SetDestPos(waypoint->GetTranslation());
    return true;
  }
  return false;
}

bool CTryclops::HasRetreatPattern(CStateManager& mgr, const CTriggerData&) const {
  return GetConnectedObject(mgr, kSS_Retreat, kSM_Follow) != kInvalidUniqueId;
}

void CTryclops::ApplySeparationBehavior(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), 8.f);
        if (separation.IsNonZero()) {
          BodyController()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CTryclops::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    ApplySeparationBehavior(mgr);
    break;
  }
}

void CTryclops::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId playerId = CScriptTeamAiMgr::ChoosePlayer(mgr, *this);
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(playerId))) {
      mDestObj = playerId;
      SetDestPos(player->GetTranslation());
      mReflectedDestPos = GetTranslation();
      mInPosition = false;
    }
    if (mBombId != kInvalidUniqueId) {
      if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
        bomb->SetFuseDisabled(false);
        bomb->SetIsBeingDragged(false);
        mBombId = kInvalidUniqueId;
      }
    }
    break;
  }
  }
}

void CTryclops::Dizzy(CStateManager& mgr, EStateMsg msg, float) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetPlayerId))) {
    switch (msg) {
    case kStateMsg_Activate:
      GrabPlayer(*player, mgr);
      player->EnableLeaveMorphBall(false);
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mShotTarget = false;
      break;
    case kStateMsg_Update:
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
        BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
      }
      if (!mShotTarget) {
        const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
        SetPlayerPosition(*player, mgr, xf.GetTranslation());
      }
      break;
    case kStateMsg_Deactivate:
      mAnimationState.SetState(CAnimationState::kAS_NotReady);
      if (player->GetAttachedActorId() == GetUniqueId()) {
        ReleasePlayer(*player, mgr);
        player->DetachActorFromPlayer();
        mDizzy = false;
      }
      break;
    }
  } else {
    mAnimationState.SetState(CAnimationState::kAS_Over);
  }
}

bool CTryclops::BallCloseToCollision(const CPlayer& player, const CStateManager& mgr) const {
  const CVector3f rayStart = GetLctrTransform(rstl::string_l(skRootLocator)).GetTranslation();
  if (!CGameCollision::RayStaticLineOfSightTest(mgr, rayStart, GetTransform().GetForward(), 3.f,
                                                CMaterialFilter::GetPassEverything())) {
    return true;
  }
  const float radius = player.GetMorphBall()->GetBallRadius();
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59), CMaterialList(kMT_NoPlatformCollision));
  const CCollidableSphere sphere(
      CSphere(player.GetTranslation() + CVector3f(0.f, 0.f, radius), radius),
      CMaterialList(kMT_Player, kMT_Unknown59));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildColliderList(nearList, player, sphere.CalculateLocalAABox());
  if (CGameCollision::DetectStaticCollisionBoolean(mgr, sphere, CTransform4f::Identity(), filter)) {
    return true;
  }
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    const TUniqueId& id = *it;
    if (id != GetUniqueId()) {
      if (const CPhysicsActor* const actor =
              TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(id))) {
        if (CCollisionPrimitive::CollideBoolean(
                CInternalCollisionStructure::CPrimDesc(sphere, filter, CTransform4f::Identity()),
                CInternalCollisionStructure::CPrimDesc(*actor->GetCollisionPrimitive(),
                                                       CMaterialFilter::GetPassEverything(),
                                                       actor->GetPrimitiveTransform()))) {
          return true;
        }
      }
    }
  }
  return false;
}

void CTryclops::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CTryclops::ReleasePlayer(CPlayer& player, CStateManager& mgr) {
  mCollisionActorManager->SetActive(mgr, false);
  player.EnableLeaveMorphBall(true);
  player.AddMaterial(kMT_Unknown59, mgr);
}

void CTryclops::GrabPlayer(CPlayer& player, CStateManager& mgr) {
  mCollisionActorManager->SetActive(mgr, true);
  player.Stop();
  player.RemoveMaterial(kMT_Unknown59, mgr);
}

void CTryclops::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(1);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  const CSegId segId = animData->GetLocatorSegId(rstl::string_l(kMouthLctr));
  if (segId != CSegId::Invalid()) {
    const float radius = 1.5f * gpTweakPlayerA->GetBallRadius();
    const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
        segId, CVector3f::Zero(), radius, rstl::string_l(kMouthLctr), 50.f);
    joints.push_back_unsafe(desc);
  }
  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mCollisionActorManager->SetActive(mgr, false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59),
                                                        CMaterialList(kMT_CollisionActor)));
}

CEntity* REL_LoadTryclops(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTryclops sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTryclops.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CTryclops(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                          LdrToEntityInfo(info, sldrThis.editorProperties),
                          LdrToTransform4f(sldrThis.editorProperties), *modelData,
                          LdrToPatternedInfo(sldrThis.patterned, nullptr),
                          LdrToActorParameters(sldrThis.actorInformation), sldrThis.attractForce,
                          sldrThis.attractAngle, sldrThis.attractDistance, sldrThis.shotForce);
}

static void SetFuncPtrs() {
  static STryclops_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadTryclops;
  SetSTryclops_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSTryclops_FuncPtrs(nullptr); }
