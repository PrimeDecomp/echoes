#include "MetroidPrime/Enemies/CAmbientAI.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAmbientAI.hpp"

#include "Kyoto/Animation/IAnimReader.hpp"

#include <float.h>

CAmbientAI::CAmbientAI(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& model, const CAABox& bounds,
                       const CMaterialList& materials, float mass, const CHealthInfo& health,
                       const CDamageVulnerability& vulnerability, const CActorParameters& params,
                       float detectRadius, float explodeRadius, int reactAnim, int damagedAnim)
: CPhysicsActor(uid, name, info, 0, xf, model, materials, bounds, SMoverData(mass), params,
                skDefaultStepData)
, mInitialHealthInfo(health)
, mHealthInfo(health)
, mDVuln(vulnerability)
, mAnimState(kAS_Ready)
, mDetectRadius(detectRadius)
, mExplodeRadius(explodeRadius)
, mCurrentAnim(GetModelData()->GetAnimationData()->GetCurrentAnimation())
, mReactAnim(reactAnim)
, mDamagedAnim(damagedAnim)
, mDead(false)
, mAnimating(false) {
  ModelData()->EnableLooping(true);
}

CHealthInfo* CAmbientAI::HealthInfo() { return &mHealthInfo; }

const CDamageVulnerability* CAmbientAI::GetDamageVulnerability() const { return &mDVuln; }

void CAmbientAI::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CAmbientAI::GetTouchBounds() const {
  if (GetActive()) {
    return rstl::optional_object< CAABox >(GetBoundingBox());
  }
  return rstl::optional_object_null();
}

void CAmbientAI::RandomizePlaybackRate(CStateManager& mgr) {
  ModelData()->AnimationData()->MultiplyPlaybackRate(0.4f * mgr.Random()->Float() + 0.8f);
}

void CAmbientAI::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (HasAnimation()) {
    const bool hasAnimTime = GetModelData()->GetAnimationData()->IsAnimTimeRemaining(
        dt - FLT_EPSILON, rstl::string_l("Whole Body"));
    const bool isLooping = GetModelData()->GetIsLoop();
    if (hasAnimTime || isLooping) {
      mAnimating = true;
      CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
      MoveToOR(deltas.GetOffsetDelta(), dt);
      RotateToOR(deltas.GetOrientationDelta(), dt);
    }
    if (!hasAnimTime && mAnimating && !isLooping) {
      SendScriptMsgs(kSS_MaxReached, mgr);
      mAnimating = false;
    }
  }

  bool inDetectRange = false;
  bool inExplodeRange = false;
  const uint playerCount = mgr.GetNumPlayers();
  for (int i = 0; i < playerCount; ++i) {
    const CVector3f& delta = mgr.GetPlayer(i)->GetTranslation() - GetTranslation();
    const float distanceSquared = delta.MagSquared();
    if (distanceSquared < mDetectRadius * mDetectRadius) {
      inDetectRange = true;
    }
    if (distanceSquared < mExplodeRadius * mExplodeRadius) {
      inExplodeRange = true;
    }
  }

  switch (mAnimState) {
  case kAS_Ready:
    if (inDetectRange) {
      mAnimState = kAS_Alert;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mReactAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  case kAS_Alert:
    if (!inDetectRange) {
      mAnimState = kAS_Ready;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mCurrentAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    } else if (inExplodeRange) {
      SendScriptMsgs(kSS_Dead, mgr);
      StopLoopedSounds();
      SetActive(false);
    }
    break;
  case kAS_Impact:
    if (!mAnimating) {
      mAnimState = kAS_Ready;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mCurrentAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }

  if (mDead) {
    return;
  }
  if (GetHealthInfo()->GetHP() <= 0.f) {
    mDead = true;
    SendScriptMsgs(kSS_Dead, mgr);
    StopLoopedSounds();
    SetActive(false);
  }
}

void CAmbientAI::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Reset:
    if (!GetActive()) {
      SetActive(true);
    }
    mAnimState = kAS_Ready;
    ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mCurrentAnim, -1, 1.f, true),
                                               false);
    ModelData()->EnableLooping(true);
    RandomizePlaybackRate(mgr);
    mDead = false;
    mHealthInfo = mInitialHealthInfo;
    break;
  case kSM_AreaLoaded:
    RandomizePlaybackRate(mgr);
    break;
  case kSM_Damage:
    if (GetActive()) {
      mAnimState = kAS_Impact;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mDamagedAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(false);
      RandomizePlaybackRate(mgr);
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadAmbientAI(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrAmbientAI sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrAmbientAI.inc"

  const rstl::optional_object< CModelData > modelData =
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.animationInformation, true);
  if (!modelData) {
    return nullptr;
  }

  CAABox bounds =
      LoadCAABox(mgr, info.GetAreaId(), sldrThis.collisionBox, sldrThis.collisionOffset);
  CMaterialList materials(kMT_Immovable, kMT_NonSolidDamageable);
  return rs_new CAmbientAI(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, bounds, materials, sldrThis.mass, LdrToHealthInfo(sldrThis.health),
      LdrToDamageVulnerability(sldrThis.vulnerability),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.detectRadius,
      sldrThis.explodeRadius, sldrThis.animation_React, sldrThis.animation_Damaged);
}

CAmbientAI::~CAmbientAI() {}
