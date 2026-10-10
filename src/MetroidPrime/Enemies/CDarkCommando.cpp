#include "MetroidPrime/Enemies/CDarkCommando.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

#include "float.h"

static EMaterialTypes ChargeBeamMaterial = kMT_Solid;

typedef CPatterned::StateMachine::TriggerFunc TriggerFunc;
typedef CPatterned::StateMachine::StateFunc StateFunc;
typedef CPatterned::StateMachine::CodeFunc CodeFunc;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver", static_cast< TriggerFunc >(&CDarkCommando::StateOver)},
    {"ShouldTaunt", static_cast< TriggerFunc >(&CDarkCommando::ShouldTaunt)},
    {"ShouldFireEMP", static_cast< TriggerFunc >(&CDarkCommando::ShouldFireEMP)},
    {"ShouldFireChargeBeam", static_cast< TriggerFunc >(&CDarkCommando::ShouldFireChargeBeam)},
    {"ShouldMeleeAttack", static_cast< TriggerFunc >(&CDarkCommando::ShouldMeleeAttack)},
    {"ShouldWarpOut", static_cast< TriggerFunc >(&CDarkCommando::ShouldWarpOut)},
    {"ShouldGetUp", static_cast< TriggerFunc >(&CDarkCommando::ShouldGetUp)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< StateFunc >(&CDarkCommando::Start)},
    {"Dead", static_cast< StateFunc >(&CDarkCommando::Dead)},
    {"WarpIn", static_cast< StateFunc >(&CDarkCommando::WarpIn)},
    {"WarpOut", static_cast< StateFunc >(&CDarkCommando::WarpOut)},
    {"Lurk", static_cast< StateFunc >(&CDarkCommando::Lurk)},
    {"Taunt", static_cast< StateFunc >(&CDarkCommando::Taunt)},
    {"MeleeAttack", static_cast< StateFunc >(&CDarkCommando::MeleeAttack)},
    {"FireEMP", static_cast< StateFunc >(&CDarkCommando::FireEMP)},
    {"FireChargeBeam", static_cast< StateFunc >(&CDarkCommando::FireChargeBeam)},
    {"PostFireChargeBeam", static_cast< StateFunc >(&CDarkCommando::PostFireChargeBeam)},
    {"ShadowDash", static_cast< StateFunc >(&CDarkCommando::ShadowDash)},
    {"FaceTarget", static_cast< StateFunc >(&CDarkCommando::FaceTarget)},
    {"GetUp", static_cast< StateFunc >(&CDarkCommando::GetUp)},
};

static CPatterned::StateMachine::SCodeFunction skCodes[] = {
    {"SelectAttackTarget", static_cast< CodeFunc >(&CDarkCommando::SelectAttackTarget)},
    {"SelectDashTarget", static_cast< CodeFunc >(&CDarkCommando::SelectDashTarget)},
    {"SelectAction", static_cast< CodeFunc >(&CDarkCommando::SelectAction)},
    {"SetDashFaceVect", static_cast< CodeFunc >(&CDarkCommando::SetDashFaceVect)},
    {"SetTargetFaceVect", static_cast< CodeFunc >(&CDarkCommando::SetTargetFaceVect)},
    {"ActivateCloak", static_cast< CodeFunc >(&CDarkCommando::ActivateCloak)},
    {"DeactivateCloak", static_cast< CodeFunc >(&CDarkCommando::DeactivateCloak)},
};

static EMaterialTypes GrenadeSolidMaterial = kMT_Solid;
static EMaterialTypes GrenadeProjectileMaterial = kMT_Projectile;
static CMaterialList skGrenadeMaterials(GrenadeSolidMaterial, GrenadeProjectileMaterial);
static int sRelUseCount = 0;

static EMaterialTypes DecoyOrbitMaterial = kMT_Orbit;
static EMaterialTypes DecoyTargetMaterial = kMT_Target;

static EMaterialTypes DashWallMaterial = kMT_Wall;
static EMaterialTypes DashFloorMaterial = kMT_Floor;
static EMaterialTypes DashCeilingMaterial = kMT_Ceiling;
static EMaterialTypes DashCharacterMaterial = kMT_Character;
static EMaterialTypes DashCollisionActorMaterial = kMT_CollisionActor;
static EMaterialTypes DashImmovableMaterial = kMT_Immovable;
static EMaterialTypes DashRestoreWallMaterial = kMT_Wall;
static EMaterialTypes DashRestoreFloorMaterial = kMT_Floor;
static EMaterialTypes DashRestoreCeilingMaterial = kMT_Ceiling;
static EMaterialTypes DashRestoreCharacterMaterial = kMT_Character;
static EMaterialTypes DashRestoreCollisionActorMaterial = kMT_CollisionActor;
static EMaterialTypes DashRestoreImmovableMaterial = kMT_Immovable;

static EMaterialTypes MeleeSolidMaterial = kMT_Solid;

static int skLurkActionId = 0;
static int skTauntActionId = 1;

static EMaterialTypes LineOfSightSolidMaterial = kMT_Solid;
static EMaterialTypes LineOfSightCharacterMaterial = kMT_Character;
static EMaterialTypes LineOfSightPlayerMaterial = kMT_Player;
static EMaterialTypes LineOfSightCollisionActorMaterial = kMT_CollisionActor;
static EMaterialTypes LineOfSightNoPlatformMaterial = kMT_ProjectilePassthrough;
static EMaterialTypes LineOfSightExcludeMaterial = kMT_ExcludeFromLineOfSightTest;

static int skEMPActionId = 3;
static int skChargeBeamActionId = 2;

// Guessed name: the melee attack variants, selected by the angle to the target.
struct SMeleeVariant {
  pas::ESeverity mSeverity;
  float mMaxAngle;
  float mWeight;
};

static const SMeleeVariant skMeleeVariants[] = {
    {pas::kS_Zero, 1.5707964f, 30.f},
    {pas::kS_One, 1.0471976f, 30.f},
    {pas::kS_Three, 0.5235988f, 40.f},
};

static const float skRagDollParticleRadii[] = {0.45f, 0.52f, 0.35f, 0.1f,  0.15f, 0.35f, 0.1f,
                                               0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f};

static rstl::string skRELName = rstl::string_l("DarkCommando.rel");

CShadowDecoy::~CShadowDecoy() {}

static CElementGen* CreateShadowDecoyEffect(CAssetId effect) {
  if (effect == kInvalidAssetId) {
    return nullptr;
  }
  TToken< CGenDescription > token = gpSimplePool->GetObj(SObjectTag('PART', effect));
  return rs_new CElementGen(token, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
}

CShadowDecoy::CShadowDecoy(TUniqueId uid, const CEntityInfo& info, TUniqueId owner,
                           const CTransform4f& xf, const CAABox& bounds, CAssetId effect,
                           const CDamageVulnerability& vulnerability, SLdrAudioPlaybackParms sound,
                           const CVector3f& orbitPosition, const CVector3f& scale, float health)
: CActor(uid, rstl::string_l("Shadow_Decoy"), info, 0, xf, CModelData::CModelDataNull(),
         CMaterialList(DecoyOrbitMaterial, DecoyTargetMaterial), CActorParameters(),
         kInvalidUniqueId)
, mEffect(CreateShadowDecoyEffect(effect))
, mSfxHandle(0)
, mHealth(health)
, mBounds(bounds)
, mVulnerability(vulnerability)
, mOwnerId(owner)
, mOrbitPosition(orbitPosition)
, mRelToken(skRELName, 0) {
  if (mEffect.get()) {
    mEffect->SetParticleEmission(true);
    mEffect->SetOrientation(GetTransform().GetRotation());
    mEffect->SetTranslation(GetTranslation());
    mEffect->SetGlobalScale(scale);
    mSfxHandle = PlayCustomSound(GetTranslation(), GetTransform().GetColumn(kDY), sound, true);
  }
  SetDrawShadow(false);
  SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
  SetVisorOrbitableFlags(CVisorParameters::kVOF_Scan, false);
  SetVisorOrbitableFlags(CVisorParameters::kVOF_Dark, false);
}

void CShadowDecoy::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Delete:
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle = CSfxHandle();
    }
    break;
  default:
    break;
  }
}

void CShadowDecoy::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!mEffect.get() || mHealth <= 0.f || mEffect->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
    mgr.GetPlayer(0)->SetOrbitRequestForTarget(GetUniqueId(), CPlayer::kOR_ActivateOrbitSource,
                                               mgr);
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle = CSfxHandle();
    }
  } else if (mEffect.get()) {
    mEffect->Update(dt);
  }
}

void CShadowDecoy::AddToRenderer(const CStateManager& mgr) const {
  CActor::AddToRenderer(mgr);
  if (mEffect.get()) {
    if (mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Dark &&
        mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Echo) {
      gpRender->AddParticleGen(*mEffect);
    }
  }
}

void CShadowDecoy::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

const CDamageVulnerability* CShadowDecoy::GetDamageVulnerability() const { return &mVulnerability; }

rstl::optional_object< CAABox > CShadowDecoy::GetTouchBounds() const { return mBounds; }

void CShadowDecoy::Touch(CActor& actor, CStateManager&) {
  if (CWeapon* weapon = TCastToPtr< CWeapon >(&actor)) {
    mHealth -= weapon->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability());
  }
}

CVector3f CShadowDecoy::GetOrbitPosition(const CStateManager&) const { return mOrbitPosition; }

CVector3f CShadowDecoy::GetAimPosition(const CStateManager&, float) const { return mOrbitPosition; }

CDarkCommandoGrenade::CDarkCommandoGrenade(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, const CTransform4f& xf,
                                           const CModelData& modelData,
                                           const CActorParameters& actorParams, TUniqueId parentId,
                                           const SLdrDarkCommandoEMPData& data, float velocity)
: CBouncyGrenade(uid, name, info, xf, modelData, actorParams, parentId, velocity,
                 CBouncyGrenadeData(data.grenadeMass, data.unknown_0xed086ce0,
                                    LdrToDamageInfo(data.grenadeDamage), data.grenadeNumBounces,
                                    data.grenadeExplosion, data.grenadeExplosion, data.grenadeTrail,
                                    data.grenadeEffect, data.sound_GrenadeBounce,
                                    data.sound_GrenadeExplode, 0.1f, 150.f, 0.1f, 150.f, false),
                 0.f, CAABox::MakeMaxInvertedBox(), kInvalidUniqueId, 0.f, 0, &skGrenadeMaterials,
                 nullptr)
, mData(data)
, mEMPTime(0.f)
, mRelToken(skRELName, 0) {
  ++sRelUseCount;
}

CDarkCommandoGrenade::~CDarkCommandoGrenade() { --sRelUseCount; }

void CDarkCommandoGrenade::Think(float dt, CStateManager& mgr) {
  UpdateGrenadeFX(dt, mgr);
  if (HasExploded()) {
    mEMPTime += dt;
  }
  if (mData.eMPDuration > 0.f) {
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      const float distance = (mgr.GetPlayer(i)->GetTranslation() - GetTranslation()).Magnitude();
      if (distance < 20.f) {
        const float magnitude = CMath::Clamp(
            0.f, ((20.f - distance) / 20.f) * ((mData.eMPDuration - mEMPTime) / mData.eMPDuration),
            1.f);
        mgr.PlayerState(i)->StaticInterference().AddSource(GetUniqueId(), magnitude, 0.2f);
      }
    }
  }
  if (mEMPTime >= mData.eMPDuration) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

CDarkCommandoChargeBeam::CDarkCommandoChargeBeam(
    const TToken< CWeaponDescription >& description, const CTransform4f& xf,
    const CDamageInfo& damage, TUniqueId uid, TAreaId areaId, TUniqueId owner,
    const rstl::optional_object< TLockedToken< CGenDescription > >& moldEffect,
    const CDamageInfo& moldDamage)
: CEnergyProjectile(true, description, kWT_AI, xf, kMT_Character, damage, uid, areaId, owner,
                    kInvalidUniqueId, 0, false, CVector3f::One(), CImpactVisorEffect::None(), false,
                    true, false, 1.f, 4.f, 4.f)
, mMoldDamage(moldDamage)
, mMoldEffect(moldEffect)
, mBillboardId(kInvalidUniqueId)
, mTimer(0.f)
, mPlayerId(kInvalidUniqueId)
, mRelToken(skRELName, 0) {}

void CDarkCommandoChargeBeam::Think(float dt, CStateManager& mgr) {
  if (mBillboardId == kInvalidUniqueId) {
    CEnergyProjectile::Think(dt, mgr);
    return;
  }

  Projectile().UpdateParticleFX();
  mTimer += dt;
  CHUDBillboardEffect* effect = TCastToPtr< CHUDBillboardEffect >(mgr.ObjectById(mBillboardId));
  if (effect) {
    const CActor* player = static_cast< const CActor* >(mgr.GetObjectById(mPlayerId));
    if (mTimer >= 5.f || !player || mgr.GetSafeZoneManager()->IsObjectInSafeZone(*player, mgr)) {
      const float remaining = 5.f - effect->GetParticleGen()->GetEmitterTime() / 60.f;
      if (remaining > 0.3f) {
        effect->GetParticleGen()->Update(dt * (remaining / 0.3f));
      }
    } else {
      mgr.ApplyDamage(
          GetUniqueId(), mPlayerId, GetOwnerId(), CDamageInfo(mMoldDamage, dt),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(ChargeBeamMaterial), CMaterialList()),
          CVector3f::Zero());
    }
  } else {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CDarkCommandoChargeBeam::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEnergyProjectile::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Delete:
    mgr.DeleteObjectRequest(mBillboardId);
    break;
  default:
    break;
  }
}

void CDarkCommandoChargeBeam::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                        CStateManager& mgr) {
  if (TCastToPtr< CPlayer >(&actor) && mMoldEffect.valid()) {
    mBillboardId = mgr.AllocateUniqueId();
    mPlayerId = actor.GetUniqueId();
    const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
    if (player && !mgr.GetSafeZoneManager()->IsObjectInSafeZone(*player, mgr)) {
      CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
          rstl::optional_object< TToken< CGenDescription > >(*mMoldEffect),
          rstl::optional_object< TToken< CElectricDescription > >(), mBillboardId, true,
          rstl::string_l("DarkChargeMold"), CHUDBillboardEffect::GetNearClipDistance(mgr, 0),
          CHUDBillboardEffect::GetScaleForPOV(mgr), 0, CColor::White(), CVector3f::One(),
          CVector3f::Zero(), true);
      if (effect) {
        effect->SetRunIndefinitely(true);
        effect->SetFinishing();
        mgr.AddObject(*effect);
      }
    }
  }
  CEnergyProjectile::ResolveCollisionWithActor(result, actor, mgr);
}

CDarkCommandoChargeBeam::~CDarkCommandoChargeBeam() {}

CDarkCommando::CDarkCommando(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& modelData,
                             const CActorParameters& actorParams,
                             const CPatternedInfo& patternedInfo, const SLdrDarkCommandoData& data)
: CPatterned(kPAI_DarkCommando, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_BiPedal, actorParams)
, mData(data)
, mCurrentAction(-1)
, mAttackState(-1)
, mRagDoll(nullptr)
, mRagDollTimer(0.f)
, mBoneTracking(*AnimationData(), rstl::string_l("Head_1"), 1.2217305f, 3.1415927f, kBTF_None)
, mChargeBeamInfo(data.chargeBeamAttackInfo.projectile,
                  LdrToDamageInfo(data.chargeBeamAttackInfo.damage))
, mShadowDashVulnerability(LdrToDamageVulnerability(data.shadowDashInfo.shadowDashVulnerability))
, mBladeDamage(LdrToDamageInfo(data.bladeDamage))
, mCloakAlpha(0.f)
, mCloakTargetAlpha(0.f)
, mCloakRate(1.f)
, mHintId(kInvalidUniqueId)
, mPrevHintId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mDecoyId(kInvalidUniqueId)
, mTeamAiMgrId(kInvalidUniqueId)
, mFaceVector(CVector3f::Zero())
, mSfxHandle(0)
, mMoldEffect(data.chargeBeamAttackInfo.moldEffect != kInvalidAssetId
                  ? rstl::optional_object< TLockedToken< CGenDescription > >(gpSimplePool->GetObj(
                        SObjectTag('PART', data.chargeBeamAttackInfo.moldEffect)))
                  : rstl::optional_object_null())
, mMeleeAttacking(false)
, mFiringEMP(false)
, mFiringChargeBeam(false)
, mWarpingIn(false)
, mWarpingOut(false)
, mCloakFading(false)
, mShadowDashing(false)
, mMeleeDamageApplied(false)
, mDashMoving(false)
, mWarpOutRequested(false)
, mKnockedBack(false)
, mHurled(false) {
  mChargeBeamInfo.Token().Lock();
  KnockBackController().EnableKnockBackPhysics(false);
  mBoneTracking.SetDisableTrackingDistance(25.f * GetModelData()->GetScale().GetZ());
  mMeleeSegId = AnimationData()->GetLocatorSegId(rstl::string_l("gun_LCTR"));
  mWristSegId = AnimationData()->GetLocatorSegId(rstl::string_l("R_wrist"));
  mGrenadeSegId = AnimationData()->GetLocatorSegId(rstl::string_l("Mid_Launch_LCTR"));
  mChargeBeamSegId = AnimationData()->GetLocatorSegId(rstl::string_l("gun_LCTR"));
}

CDarkCommando::~CDarkCommando() {}

void CDarkCommando::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    mBodyController->Activate(mgr, pas::kAS_Invalid);
    mBodyController->SetLocomotionType(pas::kLT_Combat);
    SetCloakTarget(0.f, 0.f);
    mColor.SetAlpha(0.f);
    mAlphaDelta = 0.f;
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    break;
  case kSM_Delete:
    ReleaseHint(mgr);
    mgr.DeleteObjectRequest(mDecoyId);
    LeaveTeam(mgr);
    break;
  case kSM_Deactivate:
    LeaveTeam(mgr);
    break;
  case kSM_Escape:
    mWarpOutRequested = true;
    break;
  case kSM_OffGround:
    if (!mBodyController->IsFrozen()) {
      const float mass = GetMass();
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * mass));
    }
    break;
  default:
    break;
  }
}

void CDarkCommando::PreThink(float dt, CStateManager& mgr) {
  if (!mRagDoll.get() || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreThink(*AnimationData());
  }
  CPatterned::PreThink(dt, mgr);
}

void CDarkCommando::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const bool noRagDoll = !mRagDoll.get();
  if (noRagDoll || !mRagDoll->IsPrimed()) {
    CPatterned::Think(dt, mgr);
    if (!mBodyController->IsFrozen()) {
      mBoneTracking.Think(dt);
    }
    UpdateAdditiveAim(mgr);
  } else {
    CActor::Think(dt, mgr);
    UpdateAlphaDelta(mgr, dt);
    UpdateHitDamageTime(dt);
    if (mBodyController->IsFrozen()) {
      mBodyController->UnFreeze();
    }
  }

  UpdateCloak(dt, mgr);
  ThinkRagDoll(dt, mgr, noRagDoll);
  UpdateSfx();
}

void CDarkCommando::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_BecomeRagDoll:
    if (GetHealthInfo()->GetHP() <= 0.f) {
      mRagDollTimer = 0.05f * mgr.Random()->Float() + 0.001f;
    }
    handled = true;
    break;
  case kUE_FadeIn:
    DeactivateCloak(mgr, dt);
    handled = true;
    break;
  case kUE_TakeOff:
    if (mShadowDashing) {
      mDashMoving = true;
    }
    handled = true;
    break;
  case kUE_Landing:
    if (mShadowDashing) {
      mDashMoving = false;
    }
    handled = true;
    break;
  case kUE_Projectile:
    if (mFiringEMP) {
      LaunchGrenade(mgr);
    } else if (mFiringChargeBeam) {
      FireChargeBeamShot(mgr, dt);
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

void CDarkCommando::Render(const CStateManager& mgr) const {
  const float alpha = mColor.GetAlpha();
  if (mAlive && mCloakFading) {
    const float warp = CMath::FastSinR(3.1415927f * alpha);
    if (warp > 0.f) {
      mgr.DrawSpaceWarp(GetBoundingBox().GetCenterPoint(), warp);
    }
  }
  if (alpha > 0.f) {
    CPatterned::Render(mgr);
  }
}

void CDarkCommando::PreRender(CStateManager& mgr) {
  if (mRagDoll.get() && mRagDoll->IsPrimed()) {
    mRagDoll->PreRender(GetTranslation(), *ModelData());
  }

  if (!mFadeToDeath) {
    if (!mWarpingIn && !mWarpingOut &&
        mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
      mColor.SetAlpha(1.f);
    } else {
      mColor.SetAlpha(mCloakAlpha);
    }
  }

  CPatterned::PreRender(mgr);

  if (!mRagDoll.get() || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->GetScale(),
                            *mBodyController);
  }
}

void CDarkCommando::PreRenderAllViewports(CStateManager& mgr) {
  if (mRagDoll.get() && mRagDoll->IsPrimed()) {
    mRagDoll->PreRenderAllViewports(*this, 0.2f);
    UpdatePortalSystemState(mgr);
  } else {
    CPatterned::PreRenderAllViewports(mgr);
  }
}

void CDarkCommando::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
}

void CDarkCommando::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mRagDoll.get()) {
    return;
  }

  KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_Flinch, true);
  KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack,
                                           !mKnockedBack && !mShadowDashing);
  KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_Hurled, !(mAlive && mShadowDashing));
  CPatterned::KnockBack(mgr, info);

  if (mAlive) {
    if (KnockBackController().GetFollowUp() == CKnockBackMgr::kFU_Freeze) {
      SetCloakTarget(1.f, 0.f);
      SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
      SetDrawShadow(true);
    }
    if (KnockBackController().GetActiveReaction() != CKnockBackMgr::kAR_Flinch) {
      mKnockedBack = true;
      if (KnockBackController().GetActiveReaction() == CKnockBackMgr::kAR_Hurled) {
        mHurled = true;
      }
    }
  } else if (KnockBackController().GetActiveReaction() == CKnockBackMgr::kAR_Hurled &&
             KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_LaggedBurnDeath &&
             KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_BurnDeath &&
             KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_ExplodeDeath &&
             KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_IceDeath) {
    mSfxHandle = PlayCustomSound(GetTranslation(), GetTransform().GetColumn(kDY),
                                 mData.sound_HurledDeath, false);
  }
}

const CDamageVulnerability* CDarkCommando::GetDamageVulnerability() const {
  if (mWarpingIn || mWarpingOut) {
    return &CDamageVulnerability::PassThroughVulnerabilty();
  }
  if (mShadowDashing) {
    return &mShadowDashVulnerability;
  }
  return CPatterned::GetDamageVulnerability();
}

CVector3f CDarkCommando::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

CProjectileInfo* CDarkCommando::ProjectileInfo() { return &mChargeBeamInfo; }

void CDarkCommando::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodes, ARRAY_SIZE(skCodes));
}

bool CDarkCommando::StateOver(CStateManager&, const CTriggerData&) const {
  return mAnimationState.GetState() == CAnimationState::kAS_Over;
}

bool CDarkCommando::ShouldTaunt(CStateManager&, const CTriggerData&) const {
  return mCurrentAction == 1;
}

bool CDarkCommando::ShouldFireEMP(CStateManager&, const CTriggerData&) const {
  return mCurrentAction == 3;
}

bool CDarkCommando::ShouldFireChargeBeam(CStateManager&, const CTriggerData&) const {
  return mCurrentAction == 2;
}

bool CDarkCommando::ShouldMeleeAttack(CStateManager& mgr, const CTriggerData&) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                         GetUniqueId())) {
      bool inRange = false;
      const float minSq = mMinAttackRange * mMinAttackRange;
      const float maxSq = mMaxAttackRange * mMaxAttackRange;
      const CVector3f diff = target->GetTranslation() - GetTranslation();
      const float distanceSquared = diff.MagSquared();
      if (distanceSquared >= minSq && distanceSquared <= maxSq) {
        inRange = true;
      }
      return inRange;
    }
  }
  return false;
}

bool CDarkCommando::ShouldWarpOut(CStateManager&, const CTriggerData&) const {
  return mWarpOutRequested;
}

bool CDarkCommando::ShouldGetUp(CStateManager&, const CTriggerData&) const {
  if (mAlive) {
    const pas::EAnimationState state = mBodyController->GetCurrentStateId();
    if (state == pas::kAS_Hurled || state == pas::kAS_LieOnGround) {
      return true;
    }
  }
  return false;
}

void CDarkCommando::Start(CStateManager&, EStateMsg, float) {}

void CDarkCommando::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    if (mCloakAlpha < 1.f) {
      SetCloakTarget(1.f, 0.f);
      SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
      SetDrawShadow(true);
      mColor.SetAlpha(mCloakAlpha);
    }
  } else if (msg == kStateMsg_Update) {
    if (mCloakAlpha >= 1.f) {
      CPatterned::Dead(mgr, msg, dt);
    }
  }
}

void CDarkCommando::WarpIn(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mWarpingIn = true;
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate && !mCloakFading) {
      SetCloakTarget(1.f, mBodyController->GetAnimTimeRemaining());
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
    mWarpingIn = false;
  }
}

void CDarkCommando::WarpOut(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mWarpingOut = false;
  } else if (msg == kStateMsg_Update) {
    if (mWarpingOut) {
      if (mBodyController->GetCurrentStateId() != pas::kAS_Generate) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
      mWarpingOut = true;
      SetCloakTarget(0.f, mBodyController->GetAnimTimeRemaining());
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    } else if (mCloakAlpha >= 1.f && !mBodyController->IsFrozen()) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    }
  }
}

void CDarkCommando::Lurk(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBoneTracking.SetActive(false);
  } else if (msg == kStateMsg_Update) {
    if (mKnockedBack) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Taunt)) {
      mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Five));
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    if (mBodyController->GetCurrentStateId() == pas::kAS_Taunt) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
    }
  }
}

void CDarkCommando::Taunt(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
  } else if (msg == kStateMsg_Update) {
    if (mKnockedBack) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Taunt)) {
      mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Two));
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
  }
}

void CDarkCommando::MeleeAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mAttackState = SelectMeleeVariant(mgr);
    mMeleeDamageApplied = false;
    if (mAttackState != -1) {
      if (mTeamAiMgrId == kInvalidUniqueId ||
          CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                        GetUniqueId())) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
        mMeleeAttacking = true;
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_MeleeAttack)) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCMeleeAttackCmd(skMeleeVariants[mAttackState].mSeverity));
    } else {
      CPhysicsActor* target = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mTargetId));
      if (target) {
        mBodyController->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
        ApplyMeleeDamage(mgr, *target);
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    mMeleeAttacking = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
  }
}

void CDarkCommando::FireEMP(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                      GetUniqueId())) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mFiringEMP = true;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
  } else if (msg == kStateMsg_Update) {
    if (mStateMachine->GetTime() >= mData.eMPGrenadeAttackInfo.preFireIdleTime) {
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_ProjectileAttack)) {
        const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
        if (target) {
          const CVector3f aim = target->GetAimPosition(mgr, 0.f);
          mBodyController->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::kS_One, aim, false));
        } else {
          mAnimationState.SetState(CAnimationState::kAS_Over);
        }
      }
    } else if (mKnockedBack) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (!target) {
        return;
      }
      const CVector3f toTarget = target->GetTranslation() - GetTranslation();
      if (CVector3f::GetAngleDiff(GetTransform().GetColumn(kDY), toTarget) >= 0.5235988f &&
          toTarget.IsMagnitudeSafe()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mFiringEMP = false;
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
  }
}

void CDarkCommando::FireChargeBeam(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                      GetUniqueId())) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mFiringChargeBeam = true;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_ProjectileAttack)) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target) {
        const CVector3f aim = target->GetAimPosition(mgr, 0.f);
        mBodyController->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::kS_Two, aim, false));
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mFiringChargeBeam = false;
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
  }
}

void CDarkCommando::PostFireChargeBeam(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
  } else if (msg == kStateMsg_Update) {
    if (!mKnockedBack && mStateMachine->GetTime() < mData.chargeBeamAttackInfo.postFireIdleTime) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target) {
        const CVector3f toTarget = target->GetTranslation() - GetTranslation();
        if (CVector3f::GetAngleDiff(GetTransform().GetColumn(kDY), toTarget) >= 0.5235988f &&
            toTarget.IsMagnitudeSafe()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
        }
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
  }
}

void CDarkCommando::ShadowDash(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mShadowDashing = true;
    mDashMoving = false;
    if (!GetHint(mgr)) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Add(CMaterialList(DashWallMaterial, DashFloorMaterial, DashCeilingMaterial,
                                           DashCharacterMaterial, DashCollisionActorMaterial,
                                           DashImmovableMaterial));
    SetMaterialFilter(filter);
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_LoopAttack)) {
      mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Four));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_LoopAttack && mDashMoving) {
      const CScriptAIHint* hint = GetHint(mgr);
      if (hint) {
        const CVector3f toHint = hint->GetTranslation() - GetTranslation();
        const float distance = toHint.Magnitude();
        const float step = mData.shadowDashInfo.shadowDashSpeed * dt;
        if (distance > step) {
          SetTranslation(GetTranslation() + toHint.AsNormalized() * step);
        } else {
          SetTranslation(hint->GetTranslation());
          mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
        }
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    if (mBodyController->GetCurrentStateId() == pas::kAS_LoopAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
    }
    mDashMoving = false;
    mShadowDashing = false;
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Remove(
        CMaterialList(DashRestoreWallMaterial, DashRestoreFloorMaterial, DashRestoreCeilingMaterial,
                      DashRestoreCharacterMaterial, DashRestoreCollisionActorMaterial,
                      DashRestoreImmovableMaterial));
    SetMaterialFilter(filter);
  }
}

void CDarkCommando::FaceTarget(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    if (CVector3f::GetAngleDiff(GetTransform().GetColumn(kDY), mFaceVector) <= 0.17453292f) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Turn)) {
      if (mFaceVector.IsMagnitudeSafe()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), mFaceVector.AsNormalized(), 1.f));
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
  }
}

void CDarkCommando::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    DeactivateCloak(mgr, dt);
    mVerticalMovement = false;
    mHurled = false;
  } else if (msg == kStateMsg_Update) {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Getup)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mVerticalMovement = true;
  }
}

void CDarkCommando::SelectAttackTarget(CStateManager& mgr, float) {
  mTargetId = mgr.GetPlayer(0)->GetUniqueId();
  mBoneTracking.SetActive(true);
  mBoneTracking.SetTarget(mTargetId);
}

void CDarkCommando::SelectDashTarget(CStateManager& mgr, float) {
  ReleaseHint(mgr);
  mBoneTracking.SetActive(false);

  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target) {
    const float minRange = mData.shadowDashInfo.unknown_0xa0d037ee;
    const float maxRange = mData.shadowDashInfo.unknown_0x4f522994;
    const float minRangeSq = minRange * minRange;
    const float maxRangeSq = maxRange * maxRange;
    rstl::reserved_vector< TUniqueId, 128 > candidates;
    bool foundPreferred = false;
    bool foundAny = false;
    const CSafeZoneManager* safeZones = mgr.GetSafeZoneManager();
    const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
      if (hint && hint->GetActive() && hint->GetHintType() == CScriptAIHint::kHT_ShadowDashPoint &&
          !hint->GetInUse(GetUniqueId()) && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
          hint->GetUniqueId() != mPrevHintId &&
          !safeZones->PointIsInSafeZone(mgr, hint->GetTranslation())) {
        const float targetDistSq = (target->GetTranslation() - hint->GetTranslation()).MagSquared();
        const float selfDistSq = (hint->GetTranslation() - GetTranslation()).MagSquared();
        if (targetDistSq >= minRangeSq && targetDistSq <= maxRangeSq && selfDistSq >= 100.f) {
          foundAny = true;
          if (!foundPreferred) {
            foundPreferred = true;
            candidates.clear();
          }
          candidates.push_back(hint->GetUniqueId());
        } else if (!foundPreferred) {
          if (!foundAny && selfDistSq >= 100.f) {
            candidates.clear();
            foundAny = true;
            candidates.push_back(hint->GetUniqueId());
          } else if (!foundAny || selfDistSq >= 100.f) {
            candidates.push_back(hint->GetUniqueId());
          }
        }
      }
      if (candidates.size() >= 128) {
        break;
      }
    }

    if (candidates.size() != 0) {
      ReserveHint(mgr, candidates[mgr.Random()->Range(0, candidates.size() - 1)]);
    }
  }
  JoinTeam(mgr);
}

void CDarkCommando::SelectAction(CStateManager& mgr, float) {
  mCurrentAction = 0;
  rstl::reserved_vector< TActionChoice, 4 > choices;
  BuildActionChoices(mgr, choices);

  float total = 0.f;
  for (int i = 0; i < choices.size(); ++i) {
    total += choices[i].second;
  }

  float roll = mgr.Random()->Range(0.f, total);
  for (int i = 0; i < choices.size(); ++i) {
    if (roll <= choices[i].second) {
      mCurrentAction = choices[i].first;
      break;
    }
    roll -= choices[i].second;
  }
  mKnockedBack = false;
}

void CDarkCommando::SetDashFaceVect(CStateManager& mgr, float) {
  mFaceVector = GetTransform().GetColumn(kDY);
  if (const CScriptAIHint* hint = GetHint(mgr)) {
    const CVector3f toHint(hint->GetTranslation().GetX() - GetTranslation().GetX(),
                           hint->GetTranslation().GetY() - GetTranslation().GetY(), 0.f);
    if (toHint.IsMagnitudeSafe()) {
      mFaceVector = toHint.AsNormalized();
    }
  }
}

void CDarkCommando::SetTargetFaceVect(CStateManager& mgr, float) {
  mFaceVector = GetTransform().GetColumn(kDY);
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const CVector3f toTarget(target->GetTranslation().GetX() - GetTranslation().GetX(),
                             target->GetTranslation().GetY() - GetTranslation().GetY(), 0.f);
    if (toTarget.IsMagnitudeSafe()) {
      mFaceVector = toTarget.AsNormalized();
    }
  }
  mBoneTracking.SetActive(true);
  mBoneTracking.SetTarget(mTargetId);
}

void CDarkCommando::ActivateCloak(CStateManager& mgr, float) {
  if (mHurled) {
    return;
  }
  if (mCloakAlpha > 0.f || mCloakTargetAlpha > 0.f) {
    SetCloakTarget(0.f, 0.4f);
    SetVisorOrbitableFlags(CVisorParameters::kVOF_All, false);
    SetVisorOrbitableFlags(CVisorParameters::kVOF_Scan, true);
    SetDrawShadow(false);
    PlayCustomSound(GetTranslation(), GetTransform().GetColumn(kDY),
                    mData.shadowDashInfo.sound_Cloak, false);
  }

  if (mDecoyId != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mDecoyId);
  }
  mDecoyId = mgr.AllocateUniqueId();

  CShadowDecoy* decoy = rs_new CShadowDecoy(
      mDecoyId, CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), GetUniqueId(),
      GetTransform(), GetBoundingBox(), mData.shadowDashInfo.shadowDecoyFx,
      LdrToDamageVulnerability(mData.shadowDashInfo.shadowDecoyVulnerability),
      mData.shadowDashInfo.sound_ShadowDecoy, GetOrbitPosition(mgr), GetModelData()->GetScale(),
      mData.shadowDashInfo.shadowDecoyHP);
  if (decoy) {
    mgr.AddObject(*decoy);
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId));
    if (player && player->GetOrbitTargetId() == GetUniqueId() &&
        player->GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Dark) {
      player->SetOrbitTargetId(mDecoyId, mgr);
    }
  }
}

void CDarkCommando::DeactivateCloak(CStateManager&, float) {
  if (mCloakAlpha < 1.f || mCloakTargetAlpha < 1.f) {
    SetCloakTarget(1.f, 1.f);
    SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
    SetDrawShadow(true);
    PlayCustomSound(GetTranslation(), GetTransform().GetColumn(kDY),
                    mData.shadowDashInfo.sound_DeCloak, false);
  }
}

void CDarkCommando::SetCloakTarget(float target, float duration) {
  target = CMath::Min(1.f, target);
  target = CMath::Max(0.f, target);
  if (duration > 0.f) {
    mCloakTargetAlpha = target;
    mCloakRate = 1.f / duration;
    mCloakFading = true;
  } else {
    mCloakTargetAlpha = target;
    mCloakAlpha = target;
    mCloakRate = 0.f;
    mCloakFading = false;
  }
}

void CDarkCommando::UpdateCloak(float dt, CStateManager& mgr) {
  if (mCloakFading) {
    const float step = mCloakRate * dt;
    if (mCloakAlpha < mCloakTargetAlpha) {
      mCloakAlpha = CMath::Min(mCloakTargetAlpha, mCloakAlpha + step);
    } else if (mCloakAlpha > mCloakTargetAlpha) {
      mCloakAlpha = CMath::Max(mCloakTargetAlpha, mCloakAlpha - step);
    }
    if (CMath::AbsF(mCloakAlpha - mCloakTargetAlpha) < 0.00001f) {
      mCloakAlpha = mCloakTargetAlpha;
      mCloakRate = 0.f;
      mCloakFading = false;
    }
  }

  if (mCloakAlpha > 0.f || mgr.GetPlayerState(0)->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    AddMaterial(kMT_Target, mgr);
  } else {
    RemoveMaterial(kMT_Target, mgr);
  }

  if (mDecoyId != kInvalidUniqueId && !mgr.GetObjectById(mDecoyId)) {
    mDecoyId = kInvalidUniqueId;
  }
}

void CDarkCommando::ApplyMeleeDamage(CStateManager& mgr, CPhysicsActor& target) {
  if (mMeleeAttacking && !mMeleeDamageApplied) {
    const CAABox targetBounds = target.GetBoundingBox();
    const CVector3f wristPos = GetLctrTransform(mWristSegId).GetTranslation();
    const CVector3f bladePos = GetLctrTransform(mMeleeSegId).GetTranslation();
    const CVector3f bladeDir = bladePos - wristPos;
    if (bladeDir.IsMagnitudeSafe()) {
      const CVector3f tipPos =
          bladePos + bladeDir.AsNormalized() * (3.f * GetModelData()->GetScale().GetY());
      const CAABox bladeBounds(CVector3f(CMath::Min(tipPos.GetX(), wristPos.GetX()),
                                         CMath::Min(tipPos.GetY(), wristPos.GetY()),
                                         CMath::Min(tipPos.GetZ(), wristPos.GetZ()) - 0.5f),
                               CVector3f(CMath::Max(tipPos.GetX(), wristPos.GetX()),
                                         CMath::Max(tipPos.GetY(), wristPos.GetY()),
                                         CMath::Max(tipPos.GetZ(), wristPos.GetZ()) + 0.5f));
      if (targetBounds.DoBoundsOverlap(bladeBounds) && mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), target.GetUniqueId(), GetUniqueId(), mBladeDamage,
                        CMaterialFilter::MakeInclude(CMaterialList(MeleeSolidMaterial)),
                        GetTransform().GetColumn(kDY));
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  }
}

int CDarkCommando::SelectMeleeVariant(CStateManager& mgr) const {
  int variant = -1;
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target) {
    const CVector3f forward = GetTransform().GetColumn(kDY);
    CVector3f toTarget = target->GetTranslation() - GetTranslation();
    toTarget.SetZ(0.f);
    const float angle = CVector3f::GetAngleDiff(toTarget, forward);

    float totalWeight = 0.f;
    for (int i = 0; i < 3; ++i) {
      if (skMeleeVariants[i].mMaxAngle >= angle) {
        totalWeight += skMeleeVariants[i].mWeight;
      }
    }

    float roll = mgr.Random()->Float() * totalWeight;
    for (int i = 0; i < 3; ++i) {
      if (skMeleeVariants[i].mMaxAngle >= angle) {
        if (skMeleeVariants[i].mWeight >= roll) {
          variant = i;
          break;
        }
        roll -= skMeleeVariants[i].mWeight;
      }
    }
  }
  return variant;
}

void CDarkCommando::BuildActionChoices(CStateManager& mgr,
                                       rstl::reserved_vector< TActionChoice, 4 >& choices) {
  choices.clear();
  if (mCloakAlpha <= 0.f) {
    choices.push_back(TActionChoice(skLurkActionId, mData.lurkChance));
  }
  choices.push_back(TActionChoice(skTauntActionId, mData.tauntChance));

  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (!target) {
    return;
  }
  if (mTeamAiMgrId != kInvalidUniqueId &&
      !CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                        GetUniqueId())) {
    return;
  }

  const CVector3f start = GetLctrTransform(mGrenadeSegId).GetTranslation();
  const CVector3f end = target->GetAimPosition(mgr, 0.f);
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(LineOfSightSolidMaterial),
      CMaterialList(LineOfSightCharacterMaterial, LineOfSightPlayerMaterial,
                    LineOfSightCollisionActorMaterial, LineOfSightNoPlatformMaterial,
                    LineOfSightExcludeMaterial));
  if (mgr.RayCollideWorld(start, end, filter, this)) {
    return;
  }

  if (InEMPRange(mgr)) {
    choices.push_back(TActionChoice(skEMPActionId, mData.eMPAttackChance));
  }
  if (InChargeBeamRange(mgr)) {
    choices.push_back(TActionChoice(skChargeBeamActionId, mData.chargeBeamAttackChance));
  }
}

bool CDarkCommando::InEMPRange(const CStateManager& mgr) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target) {
    bool inRange = false;
    const float minSq =
        mData.eMPGrenadeAttackInfo.minAttackRange * mData.eMPGrenadeAttackInfo.minAttackRange;
    const float maxSq =
        mData.eMPGrenadeAttackInfo.maxAttackRange * mData.eMPGrenadeAttackInfo.maxAttackRange;
    const CVector3f diff = target->GetTranslation() - GetTranslation();
    const float distanceSquared = diff.MagSquared();
    if (distanceSquared >= minSq && distanceSquared <= maxSq) {
      inRange = true;
    }
    return inRange;
  }
  return false;
}

bool CDarkCommando::InChargeBeamRange(const CStateManager& mgr) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target) {
    bool inRange = false;
    const float minSq =
        mData.chargeBeamAttackInfo.minAttackRange * mData.chargeBeamAttackInfo.minAttackRange;
    const float maxSq =
        mData.chargeBeamAttackInfo.maxAttackRange * mData.chargeBeamAttackInfo.maxAttackRange;
    const CVector3f diff = target->GetTranslation() - GetTranslation();
    const float distanceSquared = diff.MagSquared();
    if (distanceSquared >= minSq && distanceSquared <= maxSq) {
      inRange = true;
    }
    return inRange;
  }
  return false;
}

void CDarkCommando::UpdateAdditiveAim(CStateManager& mgr) {
  if (mAlive && mFiringChargeBeam) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target) {
      const CVector3f aim = target->GetAimPosition(mgr, 0.f);
      mBodyController->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
      CTransform4f xf = GetTransform();
      const CTransform4f gunXf = GetLctrTransform(mGrenadeSegId);
      xf.SetTranslation(gunXf.GetTranslation());
      mBodyController->CommandMgr().DeliverAdditiveTargetVector(
          xf.TransposeRotate(aim - gunXf.GetTranslation()));
    }
  } else {
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
  }
}

void CDarkCommando::FireChargeBeamShot(CStateManager& mgr, float dt) {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (!target) {
    return;
  }

  const CTransform4f gunXf = GetLctrTransform(mChargeBeamSegId);
  const CVector3f gunPos = gunXf.GetTranslation();
  CVector3f aim = target->GetAimPosition(mgr, 0.f);
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(target)) {
    aim = ProjectileInfo()->PredictInterceptPos(gunPos, player->GetAimPosition(mgr, 0.f), *player,
                                                false, dt);
  }

  CVector3f dir = aim - gunPos;
  const CTransform4f wristXf = GetLctrTransform(mWristSegId);
  const CVector3f wristDir = gunPos - wristXf.GetTranslation();
  if (CVector3f::GetAngleDiff(dir, wristDir) > 0.5235988f) {
    dir = CVector3f::Slerp(wristDir.AsNormalized(), dir.AsNormalized(),
                           CRelAngle::FromRadians(0.5235988f));
  }
  const CTransform4f xf = CTransform4f::LookAt(gunPos, gunPos + dir, CVector3f::Up());

  if (ProjectileInfo()->Token().IsLoaded() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 6)) {
    CDamageInfo moldDamage = LdrToDamageInfo(mData.chargeBeamAttackInfo.moldDamage);
    moldDamage.SetDamageLoopSfxId(mData.chargeBeamAttackInfo.sound_Mold.sound_Id);
    CDarkCommandoChargeBeam* beam = rs_new CDarkCommandoChargeBeam(
        ProjectileInfo()->Token(), xf, ProjectileInfo()->GetDamage(), mgr.AllocateUniqueId(),
        GetCurrentAreaId(), GetUniqueId(), mMoldEffect, moldDamage);
    if (beam) {
      mgr.AddObject(*beam);
    }
  }
}

void CDarkCommando::LaunchGrenade(CStateManager& mgr) {
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const CTransform4f launchXf = GetLctrTransform(mGrenadeSegId);
    const CVector3f origin = launchXf.GetTranslation();
    float angle = 0.34906584f;
    float speed = mData.eMPGrenadeAttackInfo.grenadeMinLaunchSpeed;
    const CVector3f aim = GetGrenadeTargetPosition(mgr, *target);
    SolveGrenadeLaunch(aim, origin, angle, speed);

    CVector3f flat = aim - origin;
    flat.SetZ(0.f);
    const CVector3f forward = GetTransform().GetColumn(kDY);
    CVector3f horizontal = flat.CanBeNormalized() ? flat.AsNormalized() : forward;
    if (CVector3f::GetAngleDiff(forward, horizontal) > 1.0471976f) {
      horizontal = CVector3f::Slerp(forward, horizontal, CRelAngle::FromRadians(1.0471976f));
    }
    const CVector3f launchDir =
        CVector3f::Slerp(horizontal, CVector3f::Up(), CRelAngle::FromRadians(angle));
    const CTransform4f grenadeXf =
        CTransform4f::LookAt(origin, origin + launchDir, CVector3f::Up());

    CDarkCommandoGrenade* grenade = rs_new CDarkCommandoGrenade(
        mgr.AllocateUniqueId(), rstl::string_l("Commando E-Grenade"),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), grenadeXf,
        CModelData::CModelDataNull(), CActorParameters::None(), GetUniqueId(),
        mData.eMPGrenadeAttackInfo, speed);
    if (grenade) {
      mgr.AddObject(grenade);
    }
  }
}

CVector3f CDarkCommando::GetGrenadeTargetPosition(const CStateManager& mgr,
                                                  const CActor& target) const {
  CVector3f aim = target.GetAimPosition(mgr, 0.5f);
  const CPlayer* player = TCastToConstPtr< CPlayer >(target);
  if (player && player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    aim -= CVector3f(0.f, 0.f, 0.5f * player->GetEyeHeight());
  }
  const CVector3f diff = GetTranslation() - aim;
  const float distance = diff.Magnitude();
  if (distance > 6.f) {
    aim += (3.f / distance) * diff;
  }
  return aim;
}

void CDarkCommando::SolveGrenadeLaunch(const CVector3f& target, const CVector3f& origin,
                                       float& angle, float& speed) const {
  const float heightDelta = target.GetZ() - origin.GetZ();
  const float distance =
      CVector2f(target.GetX() - origin.GetX(), target.GetY() - origin.GetY()).Magnitude();
  const float halfGravityDistSq = 0.5f * kDefaultGravityAccel * distance * distance;
  const float minSpeedSq = mData.eMPGrenadeAttackInfo.grenadeMinLaunchSpeed *
                           mData.eMPGrenadeAttackInfo.grenadeMinLaunchSpeed;
  const float maxSpeedSq = mData.eMPGrenadeAttackInfo.grenadeMaxLaunchSpeed *
                           mData.eMPGrenadeAttackInfo.grenadeMaxLaunchSpeed;

  float startAngle = 0.34906584f;
  float stepAngle = 0.043633234f;
  if (target.GetZ() > origin.GetZ()) {
    startAngle = 0.7853982f;
    stepAngle = -stepAngle;
  }

  float bestError = FLT_MAX;
  for (float i = 0.f; i < 10.f; i += 1.f) {
    const float candidate = stepAngle * i + startAngle;
    const float cosine = CMath::FastCosR(candidate);
    const float sine = CMath::FastSinR(candidate);
    const float denominator = distance * (cosine * sine) - heightDelta * (cosine * cosine);
    if (denominator > 1.1920929e-7f) {
      const float speedSq = halfGravityDistSq / denominator;
      if (speedSq >= minSpeedSq && speedSq <= maxSpeedSq) {
        angle = candidate;
        speed = CMath::SqrtF(speedSq);
        break;
      }
      const float error = speedSq > maxSpeedSq ? speedSq - maxSpeedSq : minSpeedSq - speedSq;
      if (error < bestError) {
        angle = candidate;
        speed = CMath::SqrtF(speedSq);
        bestError = error;
      }
    }
  }
}

const CScriptAIHint* CDarkCommando::GetHint(const CStateManager& mgr) const {
  const CScriptAIHint* hint = nullptr;
  if (mHintId != kInvalidUniqueId) {
    hint = TCastToConstPtr< CScriptAIHint >(mgr.GetObjectById(mHintId));
  }
  return hint;
}

void CDarkCommando::ReleaseHint(CStateManager& mgr) {
  if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mHintId))) {
    hint->SetInUse(false);
    hint->SetTimeRemaining(0.f);
    mHintId = kInvalidUniqueId;
  }
}

void CDarkCommando::ReserveHint(CStateManager& mgr, TUniqueId id) {
  if (CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(id))) {
    hint->SetInUse(true);
    mHintId = hint->GetUniqueId();
    mPrevHintId = mHintId;
  }
}

void CDarkCommando::ThinkRagDoll(float dt, CStateManager& mgr, bool noRagDoll) {
  if (!noRagDoll) {
    if (!mRagDoll->IsPrimed()) {
      mRagDoll->Prime(mgr, GetTransform(), *ModelData());
      const CVector3f position = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(position);
      mBodyController->SetPlaybackRate(0.f);
    } else {
      float waterTop = -FLT_MAX;
      if (InFluidId() != kInvalidUniqueId) {
        const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
        if (water && water->GetActive()) {
          waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
        }
      }
      mRagDoll->Update(mgr, dt * GetDeathTimeScale(), waterTop);
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }

    if (mRagDoll->IsOver() && !mRagDoll->WillContinueSmallMovements() && !mFadeToDeath &&
        mCloakAlpha >= 1.f) {
      mFadeToDeath = true;
      mAlphaDelta = -0.33333334f;
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      SetMomentumWR(CVector3f::Zero());
      Stop();
    }
  }

  if (mRagDollTimer > 0.f) {
    mRagDollTimer -= dt;
    if (mRagDollTimer <= 0.f) {
      if (!mRagDoll.get()) {
        const rstl::reserved_vector< float, 14 > radii(
            skRagDollParticleRadii, skRagDollParticleRadii + ARRAY_SIZE(skRagDollParticleRadii));
        mRagDoll = rs_new CPirateRagDoll(mgr, this, ushort(mData.sound_ImpactRagDoll.sound_Id), 0,
                                         GetGravityConstant(), -3.f, radii);
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      }
      mRagDollTimer = 0.f;
    }
  }
}

void CDarkCommando::UpdateSfx() {
  if (mSfxHandle) {
    if (CSfxManager::IsPlaying(mSfxHandle) || CSfxManager::IsQueued(mSfxHandle)) {
      CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), GetTransform().GetColumn(kDY), 127);
    } else {
      mSfxHandle.Clear();
    }
  }
}

void CDarkCommando::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
  }
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->JoinTeam(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Invalid,
                     CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CDarkCommando::LeaveTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

CEntity* LoadDarkCommando(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDarkCommando sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDarkCommando.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CDarkCommando(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.darkCommandoProperties);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SDarkCommando_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadDarkCommando;
  SetSDarkCommando_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSDarkCommando_FuncPtrs(nullptr); }
#endif
