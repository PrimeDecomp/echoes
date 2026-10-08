#include "MetroidPrime/Enemies/CEyeBall.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEyeBall.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "REL/REL_Setup.h"

CEyeBall::CEyeBall(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                   const CActorParameters& actParams, float attackDelay, float attackStartTime,
                   CAssetId wpscId, const CDamageInfo& dInfo, CAssetId beamContactFxId,
                   CAssetId beamPulseFxId, CAssetId beamTextureId, CAssetId beamGlowTextureId,
                   uint anim0, uint anim1, uint anim2, uint anim3, uint beamSfx,
                   bool attackDisabled, const CColor& laserInnerColor,
                   const CColor& laserOuterColor, float maxAudibleDistance, float dropOff)
: CPatterned(kPAI_EyeBall, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
             kBT_Restricted, actParams)
, mAttackDelay(attackDelay)
, mAttackStartTime(attackStartTime)
, mBoneTracking(*GetModelData()->GetAnimationData(), rstl::string_l("Eye"), 0.7853982f, 3.1415927f,
                kBTF_NoParentOrigin)
, mLaserLocatorXf(CTransform4f::Identity())
, mTargetPosition(CVector3f::Zero())
, mProjectileInfo(wpscId, dInfo)
, mBeamContactFxId(beamContactFxId)
, mBeamPulseFxId(beamPulseFxId)
, mBeamTextureId(beamTextureId)
, mBeamGlowTextureId(beamGlowTextureId)
, mProjectileId(kInvalidUniqueId)
, mCurrentAnim(0)
, mBeamSfxId(beamSfx)
, mLaserInnerColor(laserInnerColor)
, mLaserOuterColor(laserOuterColor)
, mMaxAudibleDistance(maxAudibleDistance)
, mDropOff(dropOff)
, mCanAttack(false)
, mPlayerInRange(false)
, mAlert(false)
, mAttackDisabled(attackDisabled)
, mFiringBeam(false) {
  mAnimIndices[0] = anim0;
  mAnimIndices[1] = anim1;
  mAnimIndices[2] = anim2;
  mAnimIndices[3] = anim3;
  mKnockBackController.EnableKnockBackPhysics(false);
}

CEyeBall::~CEyeBall() {}

void CEyeBall::CreateBeam(CStateManager& mgr) {
  if (mProjectileId != kInvalidUniqueId) {
    return;
  }
  CBeamInfo beamInfo(3, mBeamContactFxId, mBeamPulseFxId, mBeamTextureId, mBeamGlowTextureId, 50.f,
                     0.05f, 1.f, 2.f, 20.f, 1.f, 1.f, 2.f, mLaserInnerColor, mLaserOuterColor,
                     150.f, kInvalidAssetId);

  mProjectileId = mgr.AllocateUniqueId();
  CEntity* proj = rs_new CPlasmaProjectile(
      mProjectileInfo.Token(), rstl::string_l("EyeBall_Beam"), kWT_AI, beamInfo,
      CTransform4f::Identity(), kMT_Character, mProjectileInfo.GetDamage(), mProjectileId,
      GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(), false, CWeapon::kPA_KeepInCinematic);
  mgr.AddObject(*proj);
}

void CEyeBall::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  bool skipForward = false;
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();
  switch (message) {
  case kSM_Damage: {
    if (const CGameProjectile* proj =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(sender))) {
      if (proj->GetOwnerId() == mgr.GetPlayer(0)->GetUniqueId()) {
        const CDamageVulnerability* vuln = GetDamageVulnerability();
        if (vuln->GetVulnerability(proj->GetCurrentDamageInfo().GetWeaponMode()).WeaponHurts()) {
          mHitByPlayerProjectile = true;
        }
      }
    }
    skipForward = true;
  } break;
  case kSM_ResistedDamage: {
    if (const CGameProjectile* proj =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(sender))) {
      if (proj->GetOwnerId() == mgr.GetPlayer(0)->GetUniqueId()) {
        const CDamageVulnerability* vuln = GetDamageVulnerability();
        if (vuln->GetVulnerability(proj->GetCurrentDamageInfo().GetWeaponMode()).WeaponHurts()) {
          mHitByPlayerProjectile = true;
        }
      }
    }
    skipForward = true;
  } break;
  case kSM_Alert:
    mAlert = true;
    break;
  case kSM_Create:
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    SetDrawShadow(false);
    CreateBeam(mgr);
    break;
  case kSM_Delete:
    if (mProjectileId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mProjectileId);
      if (mBeamSfx) {
        CSfxManager::RemoveEmitter(mBeamSfx);
        mBeamSfx.Clear();
      }
    }
    mProjectileId = kInvalidUniqueId;
    break;
  default:
    break;
  }

  if (!skipForward) {
    CPatterned::AcceptScriptMsg(mgr, msg);
  }
}

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldFire", static_cast< CPatterned::StateMachine::TriggerFunc >(&CEyeBall::ShouldFire)},
    {"ScriptingTriggered",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CEyeBall::ScriptingTriggered)},
    {"CloseDelay", static_cast< CPatterned::StateMachine::TriggerFunc >(&CEyeBall::CloseDelay)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"InActive", static_cast< CPatterned::StateMachine::StateFunc >(&CEyeBall::InActive)},
    {"Active", static_cast< CPatterned::StateMachine::StateFunc >(&CEyeBall::Active)},
    {"Cover", static_cast< CPatterned::StateMachine::StateFunc >(&CEyeBall::Cover)},
    {"Flinch", static_cast< CPatterned::StateMachine::StateFunc >(&CEyeBall::Flinch)},
};

const char* const CEyeBall::skEyeLocator = "Particle_LCTR";

void CEyeBall::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void CEyeBall::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt) {
  bool skipForward = false;
  switch (type) {
  case kUE_DamageOn: {
    if (mCanAttack) {
      skipForward = true;
      CTransform4f xf = GetLctrTransform(node.GetLocatorName());
      TurnLaserOn(mgr, xf);
    }
  } break;
  case kUE_DamageOff: {
    skipForward = true;
    TurnLaserOff(mgr);
  } break;
  default:
    break;
  }
  if (!skipForward) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CEyeBall::InActive(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CEyeBall::Active(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mHitByPlayerProjectile = false;
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mCanAttack = false;
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() > mAttackStartTime) {
      mCanAttack = true;
    }
    UpdateCycleAnimation(dt);
    break;
  case kStateMsg_Deactivate:
    mStateMachine->SetDelay(mAttackDelay);
    if (CPlasmaProjectile* proj =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
      proj->ResetBeam(mgr, true);
    }
    mCanAttack = false;
    CSfxManager::RemoveEmitter(mBeamSfx);
    mBeamSfx.Clear();
    break;
  }
}

void CEyeBall::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->GetScale(),
                          ApplyBoneTracking());
  mLaserLocatorXf = GetLctrTransform(rstl::string_l(skEyeLocator));
}

void CEyeBall::PreThink(float dt, CStateManager& mgr) {
  mBoneTracking.PreThink(*AnimationData());
  CPatterned::PreThink(dt, mgr);
}

void CEyeBall::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  const CPlayer* player = mgr.GetPlayer(0);
  static float minAngle = CMath::FastCosR(0.7853982f);
  const CVector3f direction = (player->GetTranslation() - GetTranslation()).AsNormalized();
  const float angle = CVector3f::Dot(GetTransform().GetForward(), direction);

  mPlayerInRange =
      player->GetMorphballTransitionState() == CPlayer::kMS_Morphed && angle > minAngle;
  if (mPlayerInRange) {
    mBoneTracking.SetActive(true);
    mTargetPosition = player->GetTranslation() - (player->GetVelocityWR() * 0.5f);
    mBoneTracking.SetTargetPosition(mTargetPosition);
  } else {
    mBoneTracking.SetActive(false);
  }
  mBoneTracking.Think(dt);

  if (GetActive()) {
    CPlasmaProjectile* projectile =
        static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId));
    if (projectile && projectile->GetActive()) {
      projectile->UpdateFx(mLaserLocatorXf, dt, mgr);
    }
  }

  if (!mFiringBeam) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    TurnLaserOff(mgr);
  }
}

void CEyeBall::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CEyeBall::Flinch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetColumn(kDX), pas::kS_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CEyeBall::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    mCanAttack = false;
    mStateMachine->SetDelay(mAttackDelay);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CEyeBall::CloseDelay(CStateManager& mgr, const CTriggerData& data) const {
  return mStateMachine->GetTime() > mAttackDelay;
}

bool CEyeBall::ShouldFire(CStateManager& mgr, const CTriggerData& data) const {
  return !mAttackDisabled;
}

bool CEyeBall::ScriptingTriggered(CStateManager& mgr, const CTriggerData& data) const {
  return mAlert;
}

void CEyeBall::UpdateCycleAnimation(float dt) {
  if (!close_enough(
          GetModelData()->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")),
          0.f)) {
    return;
  }

  int i = 0;
  mCurrentAnim = (mCurrentAnim + 1) % 4;
  for (; mAnimIndices[mCurrentAnim] == -1 && i < 4; i++) {
    mCurrentAnim = (mCurrentAnim + 1) % 4;
  }
  const int animIdx = mAnimIndices[mCurrentAnim];
  if (animIdx != -1) {
    CBodyController* controller = BodyController();
    controller->CommandMgr().DeliverCmd(CBCScriptedCmd(animIdx, false, false, 0.f));
  }
}

void CEyeBall::Touch(CActor&, CStateManager&) {}

void CEyeBall::TurnLaserOn(CStateManager& mgr, const CTransform4f& xf) {
  CPlasmaProjectile* proj = static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId));
  if (!proj) {
    return;
  }
  if (proj->GetActive()) {
    return;
  }

  proj->Fire(xf, mgr, false);
  mFiringBeam = true;
  if (mBeamSfx) {
    return;
  }
  TAreaId id = GetCurrentAreaId();

  CAudioSys::C3DEmitterParmData parmData(mMaxAudibleDistance, mDropOff, 1, 127, 20);
  parmData.mPos = GetTranslation();
  parmData.mDir = CVector3f::Zero();
  parmData.mSfxId = mBeamSfxId;
  mBeamSfx = CSfxManager::AddEmitter(parmData, id.Value(), true, true, CSfxManager::kMedPriority);
}

void CEyeBall::TurnLaserOff(CStateManager& mgr) {
  if (CPlasmaProjectile* proj = static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
    proj->ResetBeam(mgr, true);
  }
  mFiringBeam = false;
  if (!mBeamSfx) {
    return;
  }
  CSfxManager::RemoveEmitter(mBeamSfx);
  mBeamSfx.Clear();
}

void CEyeBall::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  const CTransform4f xf = GetTransform();
  CPatterned::Death(mgr, direction, state);
  SetTransform(xf);
}

CEntity* LoadEyeBall(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrEyeBall sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrEyeBall.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CEyeBall(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.closeTime, sldrThis.fireWaitTime,
      sldrThis.projectile, LdrToDamageInfo(sldrThis.rayDamage), sldrThis.plasmaBurn,
      sldrThis.plasmaPulse, sldrThis.plasmaTexture, sldrThis.plasmaGlow,
      sldrThis.unknown_0x81d14be8, sldrThis.unknown_0x6e1320d6, sldrThis.unknown_0x85249bd5,
      sldrThis.unknown_0x6ae6f0eb, sldrThis.laserSound, sldrThis.shouldBeTriggered,
      sldrThis.laserInnerColor, sldrThis.laserOuterColor, sldrThis.maxAudibleDistance,
      sldrThis.dropOff);
}

#ifndef MONOLITHIC
SEyeBall_FuncPtrs REL_loader_EyeBall;

void SetRelLoaderFunctionToLoader() {
  REL_loader_EyeBall.mLoader = LoadEyeBall;
  SetSEyeBall_FuncPtrs(&REL_loader_EyeBall);
}

extern "C" void RELMain() { SetRelLoaderFunctionToLoader(); }

extern "C" void RELExit() { SetSEyeBall_FuncPtrs(nullptr); }
#endif
