#include "MetroidPrime/Enemies/CSporbProjectile.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Particles/CParticleData.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CSporbTop.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSporbProjectile.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "rstl/math.hpp"
#include <stdio.h>

static const char* const skBallAttachLocator = "ball_attach_LCTR"; // Guessed name

static EMaterialTypes skExcludeCharacter = kMT_Character;         // Guessed name
static EMaterialTypes skExcludePlayer = kMT_Player;               // Guessed name
static EMaterialTypes skExcludeAIPassthrough = kMT_AIPassthrough; // Guessed name
static EMaterialTypes skSolid = kMT_Solid;                        // Guessed name
static EMaterialTypes skCeiling = kMT_Ceiling;                    // Guessed name
static EMaterialTypes skWall = kMT_Wall;                          // Guessed name
static EMaterialTypes skFloor = kMT_Floor;                        // Guessed name

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::AnimOver)},
    {"ShouldPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldPatrol)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldAttack)},
    {"ShouldFire",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldFire)},
    {"ShouldReload",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldReload)},
    {"ShouldLaunch",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldLaunch)},
    {"ShouldClose",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldClose)},
    {"ShouldOpen",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldOpen)},
    {"ShouldSpit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSporbProjectile::ShouldSpit)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Sleep", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Sleep)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Patrol)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Attack)},
    {"WakeUp", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::WakeUp)},
    {"GoToSleep", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::GoToSleep)},
    {"Fire", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Fire)},
    {"Reload", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Reload)},
    {"Launch", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Launch)},
    {"Close", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Close)},
    {"Open", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Open)},
    {"Spit", static_cast< CPatterned::StateMachine::StateFunc >(&CSporbProjectile::Spit)},
};

CSporbProjectile::CSporbProjectile(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                   const CTransform4f& xf, const CModelData& modelData,
                                   const CPatternedInfo& patternedInfo,
                                   const CActorParameters& actorParams, CAssetId ballSpitEffect,
                                   CAssetId ballEscapeEffect)
: CPatterned(kPAI_SporbProjectile, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_Floater, actorParams)
, mState(kS_Invalid)
, mSolidPhase(kSP_Solid)
, mSphere(CSphere(patternedInfo.GetBodyOrigin(),
                  rstl::max_val(patternedInfo.GetHalfExtent(), patternedInfo.GetHeight() * 0.5f)),
          GetMaterialList())
, mBallEscapeRadius(2.f * mSphere.GetSphere().GetRadius())
, mPassable(false)
, mBallSpitEffect(ballSpitEffect)
, mBallEscapeEffect(ballEscapeEffect)
, mEffectIndex(0)
, mTopId(kInvalidUniqueId) {
  SetDrawShadow(false);
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  include.Remove(CMaterialList(kMT_Player, kMT_Character));
  include.Add(CMaterialList(kMT_ProjectilePassthrough, kMT_CameraPassthrough));
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      include, CMaterialList(skExcludeCharacter, skExcludePlayer, skExcludeAIPassthrough)));
  mSphere.SetMaterial(include);

  const CAABox& baseBox = GetBaseBoundingBox();
  const CVector3f halfExtent = (baseBox.GetMaxPoint() - baseBox.GetMinPoint()) * 0.5f;
  SetBoundingBox(CAABox(-halfExtent, halfExtent));
  mDisabledAnimationDeltas = kADF_Translation | kADF_Rotation;

  rstl::vector< CAssetId > particles;
  particles.reserve(2);
  if (mBallSpitEffect != kInvalidAssetId) {
    particles.push_back_unsafe(mBallSpitEffect);
  }
  if (mBallEscapeEffect != kInvalidAssetId) {
    particles.push_back_unsafe(mBallEscapeEffect);
  }
  AnimationData()->GetParticleDB().CacheParticleDesc(CCharacterInfo::CParticleResData(
      particles, rstl::vector< CAssetId >(), rstl::vector< CAssetId >(), rstl::vector< CAssetId >(),
      rstl::vector< CAssetId >(), rstl::vector< CAssetId >()));
}

void CSporbProjectile::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CSporbProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
    for (; it != GetConnectionList().end(); ++it) {
    }
    break;
  }
  case kSM_Increment:
  case kSM_Decrement:
  case kSM_AIUpdateDisabled:
    break;
  case kSM_Create:
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Crouch);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }
    *DamageVulnerability() = CDamageVulnerability::ImmuneVulnerabilty();
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
    RemoveMaterial(kMT_ExcludeFromRadar, mgr);
    AddMaterial(kMT_CameraPassthrough, mgr);
    RemoveMaterial(kMT_Target, mgr);
    AddMaterial(kMT_Unknown54, mgr);
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

void CSporbProjectile::Think(float dt, CStateManager& mgr) {
  const CTransform4f locatorXf =
      GetTransform() * GetScaledLocatorTransform(rstl::string_l(skBallAttachLocator));
  const CVector3f locatorPos = locatorXf.GetTranslation();
  MoveCollisionPrimitive(locatorPos - GetTranslation());

  CPatterned::Think(dt, mgr);

  const CPlayer* player = mgr.GetPlayer(0);
  const CVector3f aimPos = player->GetAimPosition(mgr, 0.f);
  const bool morphed = (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                            ? player->GetMorphballTransitionState()
                            : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed;
  if (morphed && mState == kS_Launch) {
    const float distSq = (aimPos - locatorPos).MagSquared();
    if (mSolidPhase == kSP_Solid) {
      if (distSq <= mBallEscapeRadius * mBallEscapeRadius) {
        RemoveMaterial(kMT_Solid, mgr);
        mSolidPhase = kSP_BallInside;
        mPassable = true;
      }
    } else if (mSolidPhase == kSP_BallInside) {
      if (distSq > mBallEscapeRadius * mBallEscapeRadius) {
        mSolidPhase = kSP_Solid;
        AddMaterial(kMT_Solid, mgr);
        mPassable = false;
      }
    }
  }
}

void CSporbProjectile::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CSporbProjectile::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CSporbProjectile::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
}

void CSporbProjectile::Sleep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    mState = kS_Sleeping;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    mState = kS_Patrolling;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    mState = kS_Attacking;
    mSolidPhase = kSP_Solid;
    AddMaterial(kMT_Solid, mgr);
    mPassable = false;
    mCarryingBall = false;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::WakeUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::GoToSleep(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Launch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Internal8);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Close(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mPassable) {
      BodyController()->SetLocomotionType(pas::kLT_Internal6);
    } else {
      BodyController()->SetLocomotionType(pas::kLT_Crouch);
    }
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Open(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Fire(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, -1));
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSporbProjectile::Spit(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CSporbProjectile::Reload(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Four, -1));
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Four, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mState = kS_Attacking;
    break;
  }
}

bool CSporbProjectile::ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const {
  return mState >= kS_Patrolling;
}

bool CSporbProjectile::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mState >= kS_Attacking;
}

bool CSporbProjectile::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

bool CSporbProjectile::ShouldFire(CStateManager& mgr, const CTriggerData& data) const {
  return mState >= kS_Fire;
}

bool CSporbProjectile::ShouldReload(CStateManager& mgr, const CTriggerData& data) const {
  return mState >= kS_Reload;
}

bool CSporbProjectile::ShouldLaunch(CStateManager& mgr, const CTriggerData& data) const {
  return mState == kS_Launch;
}

bool CSporbProjectile::ShouldClose(CStateManager& mgr, const CTriggerData& data) const {
  return mState == kS_Close;
}

bool CSporbProjectile::ShouldOpen(CStateManager& mgr, const CTriggerData& data) const {
  return mState == kS_Open;
}

bool CSporbProjectile::ShouldSpit(CStateManager& mgr, const CTriggerData& data) const {
  return mState == kS_Spit;
}

void CSporbProjectile::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                    CStateManager& mgr) {
  static CMaterialList skSolidTypes(skSolid, skCeiling, skWall, skFloor);
  if (id == kInvalidUniqueId) {
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (info.GetMaterialLeft().SharesMaterials(skSolidTypes) &&
          !info.GetMaterialLeft().HasMaterial(kMT_AIPassthrough)) {
        mSolidPhase = kSP_Landed;
        mPassable = false;
      }
    }
  }
  CPatterned::CollidedWith(id, list, mgr);
}

void CSporbProjectile::PlayBallSpitEffect(CStateManager& mgr) {
  char name[100];
  sprintf(name, "SPORB_TOP_EFFECT%d-%d", mBallSpitEffect, mEffectIndex++);
  const CSegId locator = AnimationData()->GetLocatorSegId(rstl::string_l(skBallAttachLocator));
  AnimationData()->GetParticleDB().AddParticleEffect(
      CPOINode::GetHashForString(name), 0x40,
      CParticleData(0, SObjectTag('PART', mBallSpitEffect),
                    locator == CSegId::Invalid() ? CSegId(0) : locator, 1.f,
                    CParticleData::kPM_ContinuousSystem),
      GetModelData()->GetScale(), &mgr, GetCurrentAreaId(), false, 0);
}

void CSporbProjectile::PlayBallEscapeEffect(CStateManager& mgr) {
  char name[100];
  sprintf(name, "SPORB_TOP_EFFECT%d-%d", mBallEscapeEffect, mEffectIndex++);
  const CSegId locator = AnimationData()->GetLocatorSegId(rstl::string_l(skBallAttachLocator));
  AnimationData()->GetParticleDB().AddParticleEffect(
      CPOINode::GetHashForString(name), 0x40,
      CParticleData(0, SObjectTag('PART', mBallEscapeEffect),
                    locator == CSegId::Invalid() ? CSegId(0) : locator, 1.f,
                    CParticleData::kPM_ContinuousEmitter),
      GetModelData()->GetScale(), &mgr, GetCurrentAreaId(), false, 0);
}

rstl::optional_object< CAABox > CSporbProjectile::GetTouchBounds() const {
  return GetCollisionPrimitive()->CalculateAABox(GetTransform());
}

CAABox CSporbProjectile::GetScanVisorRenderBounds(const CStateManager& mgr) const {
  if (mTopId != kInvalidUniqueId) {
    if (const CSporbTop* top = TCastToConstPtr< CSporbTop >(mgr.GetObjectById(mTopId))) {
      return top->GetScanVisorRenderBounds(mgr);
    }
  }
  return CPatterned::GetScanVisorRenderBounds(mgr);
}

void CSporbProjectile::ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                                       const CModelFlags& flags) const {
  if (mTopId != kInvalidUniqueId) {
    if (const CSporbTop* top = TCastToConstPtr< CSporbTop >(mgr.GetObjectById(mTopId))) {
      top->ScanVisorRender(mgr, xf, flags);
    }
  }
}

CEntity* REL_LoadSporbProjectile(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSporbProjectile sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSporbProjectile.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CSporbProjectile(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.ballSpitParticleEffect,
      sldrThis.ballEscapeParticleEffect);
}

CSporbProjectile::~CSporbProjectile() {}
