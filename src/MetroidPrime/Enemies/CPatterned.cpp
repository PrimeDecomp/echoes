#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGenericFSM2StateImpl.hpp"
#include "MetroidPrime/CPositionalParticleData.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/math.hpp"

#include <float.h>

#include <float.h>

const float CPatterned::skDamageHitTime = 0.33f;
const float CPatterned::skActorApproachDistance = 3.f;
const CColor CPatterned::skDamageColor(0.5f, 0.f, 0.f, 1.f);
const CColor CPatterned::skHitsWithoutDamageColor(0.5f, 0.5f, 0.f, 1.f);
const CColor CPatterned::skFrozenColor(0x321F50FF);
const CColor CPatterned::skDisintegrateColor(0xFFFFC0FF);
const CColor CPatterned::skBlackDeathColor(0xAA54FF00);
const CColor CPatterned::skDisintegrationColor(0xFFFFFF00);

static CMaterialList gkPatternedFlyerMaterialList(kMT_Character, kMT_Unknown59, kMT_Orbit,
                                                  kMT_Target, kMT_SeekerTarget);
static CMaterialList gkPatternedGroundMaterialList =
    CMaterialList(kMT_GroundCollider).Union(gkPatternedFlyerMaterialList);

CPatterned::CPatterned(EPatternedAI character, TUniqueId uid, const rstl::string& name,
                       EFlavorType flavor, const CEntityInfo& info, const CTransform4f& xf,
                       const CModelData& modelData, const CPatternedInfo& pinfo,
                       EMovementType movement, EColliderType collider, EBodyType body,
                       const CActorParameters& params)
: CAi(uid, name, info, 4, xf, modelData,
      CAABox(pinfo.mBodyOrigin.GetX() - pinfo.mHalfExtent * modelData.GetScale().GetX(),
             pinfo.mBodyOrigin.GetY() - pinfo.mHalfExtent * modelData.GetScale().GetY(),
             pinfo.mBodyOrigin.GetZ(),
             pinfo.mBodyOrigin.GetX() + pinfo.mHalfExtent * modelData.GetScale().GetX(),
             pinfo.mBodyOrigin.GetY() + pinfo.mHalfExtent * modelData.GetScale().GetY(),
             pinfo.mBodyOrigin.GetZ() + pinfo.mHeight * modelData.GetScale().GetZ()),
      pinfo.mMass, pinfo.mHealthInfo, pinfo.mDamageVulnerability,
      movement == kMT_Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
      pinfo.mStateMachineId, pinfo.mStateMachine2Id, params, pinfo.mStepUpHeight, 0.8f)
, mDestObj(kInvalidUniqueId)
, mDestPos(CVector3f::Zero())
, mReflectedDestPos(CVector3f::Zero())
, mInPosition(false)
, mVerticalMovement(movement == kMT_Flyer)
, mSolidCollision(false)
, mBlockingCollision(false)
, mOnGround(movement != kMT_Flyer)
, mOnStaticGround(false)
, mPrevOnGround(true)
, mEnergyAttractor(false)
, mLookAtDeathDir(true)
, x34d_25_(false)
, x34d_26_(true)
, mStateMachine(pinfo.mStateMachine2Id == kInvalidAssetId
                    ? static_cast< StateMachine* >(rs_new TStateMachineState< CPatterned >)
                    : static_cast< StateMachine* >(rs_new CGenericFSM2State< CPatterned >))
, mCharacterType(character)
, mCreatureSize(pinfo.mCreatureSize)
, mIngPossessionBlend(0.f)
, mIngPossessionTarget(0.f)
, mIngPossessionDelay(0.f)
, mIngPossessionDuration(0.5f)
, mIngVulnerability(LdrToDamageVulnerability(pinfo.mIngPossessionData.ingVulnerability))
, mMoveVec(CVector3f::Zero())
, mFaceVec(CVector3f::Zero())
, mInitialAnimation(pinfo.mAnimationParameters.GetInitialAnimation())
, mLatestLeashPosition(CVector3f::Zero())
, mSpeed(pinfo.mSpeed)
, mTurnSpeed(pinfo.mTurnSpeed)
, mDetectionRange(pinfo.mDetectionRange)
, mDetectionHeightRange(pinfo.mDetectionHeightRange)
, mDetectionAngle(cosf(CMath::Deg2Rad(pinfo.mDetectionAngle)))
, mMinAttackRange(pinfo.mMinAttackRange)
, mMaxAttackRange(pinfo.mMaxAttackRange)
, mAverageAttackTime(pinfo.mAverageAttackTime)
, mAttackTimeVariation(pinfo.mAttackTimeVariation)
, mLeashRadius(pinfo.mLeashRadius)
, mPlayerLeashRadius(pinfo.mPlayerLeashRadius)
, mPlayerLeashTime(pinfo.mPlayerLeashTime)
, mCurPlayerLeashTime(0.f)
, mXDamageThreshold(pinfo.mXDamageThreshold)
, mFrozenXDamageThreshold(pinfo.mFrozenXDamageThreshold)
, mXDamageDelay(pinfo.mXDamageDelay)
, mLastHP(0.f)
, mAlphaDelta(0.f)
, mPendingFireDamage(0.f)
, mPendingShockDamage(0.f)
, mBurnThinkRateTimer(0.f)
, mFlavor(flavor)
, mHitByPlayerProjectile(false)
, mAlive(true)
, x420_26_(false)
, mFadeToDeath(false)
, mPendingMassiveDeath(false)
, mPendingMassiveFrozenDeath(false)
, mIsFlyer(movement == kMT_Flyer)
, mPathOverCount(0)
, mBurning(false)
, mLaggedBurnDeath(false)
, mPendingDeath(false)
, mLostMassiveFrozenHP(false)
, mDieIf80PercFrozen(false)
, mIsMakingBigStrike(false)
, mDrawParticles(true)
, mEnableStateMachine(true)
, mStateControlledMassiveDeath(true)
, mDisabledAnimationDeltas(0)
, mUseDisintegrationPlane(false)
, mBlackDeath(false)
, mDisintegrating(false)
, mStopPhysics(false)
, x423_24_(false)
, mSuppressKnockBack(false)
, mContactDamage(pinfo.mContactDamageInfo)
, mCurDamageRemTime(0.f)
, mDamageWaitTime(pinfo.mDamageWaitTime)
, mDamageCooldownTimer(-1.f)
, mColor(0.f, 0.f, 0.f, 1.f)
, mDamageColor(skDamageColor)
, mAnimationDeltas(CVector3f::Zero(), CQuaternion::NoRotation())
, mNormalModel(GetAnimationData()->GetModelData())
, mDeathSfx(pinfo.mDeathSfx)
, mIceShatterSfx(pinfo.mIceShatterSfx)
, mIceVocalSfx(pinfo.mIceVocalSfx)
, mFrozenSfx(pinfo.mFrozenSfx)
, mIngPossessionData(pinfo.mIngPossessionData)
, mKnockBackController(pinfo.mKnockBackRules)
, mLatestPredictedTranslation(CVector3f::Zero())
, mPredictedLeashTime(0.f)
, mIntoFreezeDuration(pinfo.mIntoFreezeDuration)
, mOutOfFreezeDuration(pinfo.mOutOfFreezeDuration)
, mFreezeDuration(pinfo.mFreezeDuration)
, mPreThinkDt(0.f)
, mDamageDuration(0.f)
, mColliderType(collider)
, mFadeOnDeathTime(3.f)
, mDeathExplosionOffset(pinfo.mDeathExplosionOffset)
, mIceDeathExplosionOffset(pinfo.mIceDeathExplosionOffset)
, mMoveScale(1.f, 1.f, 1.f)
, mIngSnatchingPlane(CVector3f::Zero(), CVector3f::Forward())
, mDisintegrationOrigin(CVector3f::Zero()) {
  BuildIngModel(mIngPossessionData.ingPossessedModel, mIngPossessionData.ingPossessedSkinRules);
  if (pinfo.mDeathExplosionParticle != kInvalidAssetId) {
    mDeathExplosionParticle =
        gpSimplePool->GetObj(SObjectTag('PART', pinfo.mDeathExplosionParticle));
    mDeathExplosionParticle->Lock();
  }
  if (pinfo.mDeathExplosionElectric != kInvalidAssetId) {
    mDeathExplosionElectric =
        gpSimplePool->GetObj(SObjectTag('ELSC', pinfo.mDeathExplosionElectric));
    mDeathExplosionElectric->Lock();
  }
  if (pinfo.mIceDeathExplosionParticle != kInvalidAssetId) {
    mIceDeathExplosionParticle =
        gpSimplePool->GetObj(SObjectTag('PART', pinfo.mIceDeathExplosionParticle));
    mIceDeathExplosionParticle->Lock();
  }
  if (mContactDamage.GetRadius() > 0.f) {
    mContactDamage.SetRadius(0.f);
  }
  SetRenderParticleDatabaseInside(false);
  if (HasModelData()) {
    BuildBodyController(body);
    mLockOnTarget = GetAnimationData()->GetLocatorSegId(rstl::string("lockon_target_LCTR"));
  }
  if (mIngPossessionData.darkScanInfo != kInvalidAssetId) {
    mIngScanInfo = rs_new TLockedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', mIngPossessionData.darkScanInfo)));
  }
  // TODO: Enable the actor's damage/echo flags and apply pinfo.mEchoParameters.
}

void CPatterned::BuildBodyController(EBodyType body) {
  if (!mBodyController.null()) {
    return;
  }

  mBodyController = rs_new CBodyController(*this, mTurnSpeed, body);

  const CPASAnimParmData params(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > bestAnim =
      mBodyController->GetPASDatabase().FindBestAnimation(params, -1);
  mKnockBackController.EnableShock(bestAnim.first > 0.f);
}

static const CPatterned::StateMachine::STriggerFunction triggers[] = {
    {"Leash", &CPatterned::Leash},
    {"SpotPlayer", &CPatterned::SpotPlayer},
    {"PlayerSpot", &CPatterned::PlayerSpot},
    {"InRange", &CPatterned::InRange},
    {"InMaxRange", &CPatterned::InMaxRange},
    {"InDetectionRange", &CPatterned::InDetectionRange},
    {"PathShagged", &CPatterned::PathShagged},
    {"PathOver", &CPatterned::PathOver},
    {"PathFound", &CPatterned::PathFound},
    {"Delay", &CPatterned::Delay},
    {"RandomDelay", &CPatterned::RandomDelay},
    {"FixedDelay", &CPatterned::FixedDelay},
    {"HasPatrolPath", &CPatterned::HasPatrolPath},
    {"Attacked", &CPatterned::Attacked},
    {"OffLine", &CPatterned::OffLine},
    {"AnimOver", &CPatterned::AnimOver},
    {"NoPathNodes", &CPatterned::NoPathNodes},
    {"TooClose", &CPatterned::TooClose},
    {"Landed", &CPatterned::Landed},
    {"InPosition", &CPatterned::InPosition},
    {"Stuck", &CPatterned::Stuck},
    {"CodeTrigger", &CPatterned::CodeTrigger},
    {"Random", &CPatterned::Random},
    {"FixedRandom", &CPatterned::FixedRandom},
};
static const CPatterned::StateMachine::SStateFunction states[] = {
    {"Start", &CPatterned::Start},
    {"Dead", &CPatterned::Dead},
    {"PathFind", &CPatterned::PathFind},
    {"Patrol", &CPatterned::Patrol},
};

void CPatterned::SetupStateMachine(CStateManager&) {
  StateMachine* stateMachine = mStateMachine.get();
  stateMachine->SetTriggerFunctions(triggers, ARRAY_SIZE(triggers));
  stateMachine->SetStateFunctions(states, ARRAY_SIZE(states));
}

void CPatterned::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  CAi::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    if (mColliderType != kCT_One) {
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      CMaterialList charMat(kMT_Character);
      include.Remove(charMat);
      exclude.Add(charMat);
      SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }
    SetAngularEnabled(true);
    SetIngPossessed(mIngPossessionData.isAnEncounter, 0.f, mgr);
    UpdateIngPossession(1000.f);
    break;
  case kSM_LandedOnStaticGround:
    if (!mVerticalMovement) {
      mOnStaticGround = true;
    }
    break;
  case kSM_Landed:
    if (!mVerticalMovement) {
      SetMomentumWR(CVector3f::Zero());
      AddMaterial(kMT_GroundCollider, mgr);
    }
    mOnGround = true;
    break;
  case kSM_Falling:
    if (!mVerticalMovement && mBodyController->GetPercentageFrozen() == 0.f) {
      SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
      RemoveMaterial(kMT_GroundCollider, mgr);
    }
    mOnGround = false;
    mOnStaticGround = false;
    break;
  case kSM_Activate:
    mLatestLeashPosition = GetTranslation();
    break;
  case kSM_Delete:
    mStateMachine->Reset(mgr, *this);
    break;
  case kSM_Damage:
    if (TCastToPtr< CGameProjectile >(const_cast< CEntity* >(mgr.GetObjectById(sender)))) {
      mHitByPlayerProjectile = true;
    }
    break;
  case kSM_ResistedDamage:
    if (CGameProjectile* projectile =
            TCastToPtr< CGameProjectile >(const_cast< CEntity* >(mgr.GetObjectById(sender)))) {
      if (TCastToPtr< CPlayer >(
              const_cast< CEntity* >(mgr.GetObjectById(projectile->GetOwnerId())))) {
        mHitByPlayerProjectile = true;
      }
    }
    break;
  default:
    break;
  }
}

void CPatterned::SetDestPos(const CVector3f& position) { mDestPos = position; }

CVector3f CPatterned::GetGunEyePos() const {

  CVector3f translation = GetTranslation();
  const CAABox& bounds = GetBaseBoundingBox();
  translation[kDZ] += 0.6f * (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ());
  return translation;
}

bool CPatterned::ApplyBoneTracking() const {
  if (mAlive && !GetBodyController()->IsFrozen() &&
      !(mKnockBackController.GetFlinchRemainingTime() > 0.f)) {
    return true;
  }
  return false;
}

float CPatterned::GetAnimationDistance(const CPASAnimParmData& parms) const {
  float distance = 1.f;
  const rstl::pair< float, int > best =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
  if (best.first > FLT_EPSILON) {
    const CAnimData* animData = GetAnimationData();
    const float dur = animData->GetAnimationDuration(best.second);
    const float vel = animData->GetAverageVelocity(best.second);
    distance = vel * dur;
  }
  return distance;
}

float CPatterned::GetAnimationDuration(const CPASAnimParmData& parms) const {
  const rstl::pair< float, int > best =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
  if (best.first > FLT_EPSILON) {
    return GetAnimationData()->GetAnimationDuration(best.second);
  }
  return 0.f;
}

void CPatterned::SetupPlayerCollision(bool enabled) {
  if (enabled) {
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    const CMaterialList playerMaterial(kMT_Player);
    include.Add(playerMaterial);
    exclude.Remove(playerMaterial);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  } else {
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    const CMaterialList playerMaterial(kMT_Player);
    include.Remove(playerMaterial);
    exclude.Add(playerMaterial);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  }
}

CScriptCoverPoint* CPatterned::GetCoverPoint(CStateManager& mgr, const TUniqueId id) const {
  CScriptCoverPoint* point = nullptr;
  if (id != kInvalidUniqueId) {
    point = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id));
  }
  return point;
}

void CPatterned::ReleaseCoverPoint(CStateManager& mgr, TUniqueId& id, bool retainCooldown) {
  if (CScriptCoverPoint* point = GetCoverPoint(mgr, id)) {
    point->SetInUse(false);
    if (!retainCooldown) {
      point->ResetCooldown();
    }
    id = kInvalidUniqueId;
  }
}

void CPatterned::SetCoverPoint(CScriptCoverPoint* point, TUniqueId& id) {
  point->SetInUse(true);
  id = point->GetUniqueId();
}

void CPatterned::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (mAlive) {
    mBodyController->SetTimeScale(1.f);
    if (!mBodyController->IsOnFire()) {
      const CWeaponMode deathWeapon = GetHealthInfo()->GetCauseOfDeathWeapon();
      if (deathWeapon.IsComboed() &&
          (deathWeapon.GetType() == kWT_Dark || deathWeapon.GetType() == kWT_Annihilator)) {
        mPendingMassiveFrozenDeath = false;
        mPendingMassiveDeath = false;
      } else {
        mLostMassiveFrozenHP = (mLastHP - GetHealthInfo()->GetHP()) >= mFrozenXDamageThreshold;
        if (mLostMassiveFrozenHP && mIceDeathExplosionParticle.valid() &&
            mBodyController->GetPercentageFrozen() > 0.8f) {
          mPendingMassiveFrozenDeath = true;
        } else if ((mLastHP - GetHealthInfo()->GetHP()) >= mXDamageThreshold) {
          mPendingMassiveDeath = true;
        }
      }
    }

    if (mPendingMassiveDeath || mPendingMassiveFrozenDeath) {
      if (mLookAtDeathDir && mXDamageDelay <= 0.f && direction.IsNonZero()) {
        const CVector3f pos = GetTranslation();
        const CVector3f target = pos - direction;
        const CTransform4f deathXf = CTransform4f::LookAt(pos, target) *
                                     CTransform4f::RotateX(CRelAngle::FromRadians(0.7853982f));
        SetTransform(deathXf);
      }
    } else {
      if (mStateMachine->HasState()) {
        mStateMachine->SetState(mgr, *this, rstl::string_l("Dead"));
      }
      RemoveMaterial(kMT_GroundCollider, mgr);
      if (!mBurning && !mLaggedBurnDeath) {
        mVerticalMovement = false;
      }
    }

    mAlive = false;
    SetHighlightedInDarkVisor(false);
    IssueDeathBodyCommand(mgr, direction);
    if (CanBeUnPossessed(mgr) == true) {
      SetIngPossessed(false, mgr);
    }
    if (state != kSS_InvalidState) {
      SendScriptMsgs(state, mgr, GetUniqueId(), kSM_None);
    }
  }
}

void CPatterned::IssueDeathBodyCommand(CStateManager& mgr, const CVector3f& direction) {
  if (!mBodyController->ShouldPlayDeathAnims()) {
    return;
  }

  if (mBodyController->HasBodyState(pas::kAS_Hurled) &&
      mBodyController->GetBodyType() == kBT_Flyer) {
    mBodyController->CommandMgr().DeliverCmd(CBCHurledCmd(-direction, CVector3f::Zero()));
  } else if (mBodyController->HasBodyState(pas::kAS_Fall)) {
    const EScriptObjectState state = IsIngPossessed() ? kSS_DarkXDamage : kSS_XDamage;
    if (!mPendingMassiveDeath || CheckConnectedObject(mgr, state, kSM_None) == kInvalidUniqueId) {
      mBodyController->CommandMgr().DeliverCmd(CBCKnockDownCmd(-direction, pas::kS_One));
    }
  }
}

void CPatterned::CreateXDamageParticles(CStateManager& mgr) const {
  const rstl::optional_object< TCachedToken< CGenDescription > >& deathParticle =
      GetDeathExplosionParticle();
  const rstl::optional_object< TCachedToken< CElectricDescription > >& deathElectric =
      mDeathExplosionElectric;

  if (deathParticle.valid() || deathElectric.valid()) {
    CTransform4f xf(GetTransform());
    const CVector3f offset =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), mDeathExplosionOffset);
    xf.SetTranslation(GetTransform() * offset);

    if (deathParticle.valid()) {
      CExplosion* explosion = rs_new CExplosion(
          TLockedToken< CGenDescription >(*deathParticle), mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), rstl::string_l(""),
          xf, 0, CVector3f(1.f, 1.f, 1.f), CColor::White(), -1);
      if (explosion) {
        mgr.AddObject(explosion);
      }
    }

    if (deathElectric.valid()) {
      CExplosion* explosion = rs_new CExplosion(
          TLockedToken< CElectricDescription >(*deathElectric), mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), rstl::string_l(""),
          xf, 0, CVector3f(1.f, 1.f, 1.f), CColor::White(), -1);
      if (explosion) {
        mgr.AddObject(explosion);
      }
    }
  }
}

void CPatterned::GenerateIceDeathExplosion(CStateManager& mgr) {
  const rstl::optional_object< TCachedToken< CGenDescription > >& deathParticle =
      mIceDeathExplosionParticle;
  if (deathParticle.valid()) {
    CTransform4f xf(GetTransform());
    const CVector3f offset =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), mIceDeathExplosionOffset);
    xf.SetTranslation(GetTransform() * offset);

    if (deathParticle.valid()) {
      CExplosion* explosion = rs_new CExplosion(
          TLockedToken< CGenDescription >(*deathParticle), mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), rstl::string_l(""),
          xf, 0, CVector3f(1.f, 1.f, 1.f), CColor::White(), -1);
      if (explosion) {
        mgr.AddObject(explosion);
      }
    }
  }
}

void CPatterned::MassiveDeath(CStateManager& mgr) {
  const ushort sfx = mDeathSfx;
  CSfxManager::AddEmitter(sfx, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                          CSfxManager::kMedPriority);

  if (!mBurning) {
    const CWeaponMode deathWeapon = GetHealthInfo()->GetCauseOfDeathWeapon();
    if (deathWeapon.GetType() == kWT_Dark && deathWeapon.IsCharged() == true) {
      SendScriptMsgs(kSS_IceXDamage, mgr, kInvalidUniqueId, kSM_None);
    } else if (IsIngPossessed() == true) {
      SendScriptMsgs(kSS_DarkXDamage, mgr, kInvalidUniqueId, kSM_None);
    } else {
      SendScriptMsgs(kSS_XDamage, mgr, kInvalidUniqueId, kSM_None);
    }
    CreateXDamageParticles(mgr);
  }

  DeathDelete(mgr);
  mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
  // Native post-death flag; no identified reader establishes its purpose.
  x423_24_ = true;
}

void CPatterned::MassiveFrozenDeath(CStateManager& mgr) {
  if (mIceShatterSfx == CSfxManager::kInternalInvalidSfxId) {
    mIceShatterSfx = mDeathSfx;
  }

  CSfxManager::AddEmitter(mIceShatterSfx, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                          CSfxManager::kMedPriority);
  CSfxManager::AddEmitter(mIceVocalSfx, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                          CSfxManager::kMedPriority);
  SendScriptMsgs(kSS_IceXDamage, mgr, kInvalidUniqueId, kSM_None);
  GenerateIceDeathExplosion(mgr);

  for (uint player = 0; player < mgr.GetNumPlayers(); ++player) {
    const CVector3f playerDelta = mgr.GetPlayer(player)->GetTranslation() - GetTranslation();
    const float toPlayerDist = playerDelta.Magnitude();
  }

  DeathDelete(mgr);
  mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
}

void CPatterned::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  const CHealthInfo* health = GetHealthInfo();
  if (!mBurning && health != nullptr && !mSuppressKnockBack) {
    mKnockBackController.KnockBack(mgr, *this, info);
  }
}

void CPatterned::ApplyKnockBackFollowUp(CStateManager& mgr, const CVector3f& direction,
                                        CKnockBackMgr::EFollowUp followUp, float duration,
                                        float secondaryDuration, TUniqueId source,
                                        TUniqueId owner) {
  if (mPendingMassiveDeath || mPendingMassiveFrozenDeath) {
    return;
  }

  switch (followUp) {
  case CKnockBackMgr::kFU_Slow:
    if (mBodyController->IsFrozen()) {
      CUnitVector3f dir = GetTransform().TransposeRotate(direction);
      Freeze(mgr, CVector3f::Zero(), dir, secondaryDuration, 0.f);
    } else {
      mBodyController->SetTimeScale(rstl::max_val(mBodyController->GetTimeScale() - duration, 0.f));
      if (mBodyController->GetTimeScale() == 0.f) {
        CUnitVector3f dir = GetTransform().TransposeRotate(direction);
        Freeze(mgr, CVector3f::Zero(), dir, secondaryDuration, 0.f);
        mBodyController->SetTimeScale(1.f);
      }
    }
    break;
  case CKnockBackMgr::kFU_BurnPhase:
    if (source != kInvalidUniqueId) {
      if (mBodyController->IsOnFire()) {
        Burn(mgr, secondaryDuration, gpTweakPlayerGun->GetAIBurnDamage());
        mBodyController->SetFireDamageBuildup(0.f);
      } else {
        mBodyController->SetFireDamageBuildup(
            rstl::min_val(duration + mBodyController->GetFireDamageBuildup(), 1.f));
        if (mBodyController->GetFireDamageBuildup() == 1.f) {
          Burn(mgr, secondaryDuration, gpTweakPlayerGun->GetAIBurnDamage());
          mBodyController->SetFireDamageBuildup(0.f);
        }
      }
    }
    break;
  case CKnockBackMgr::kFU_Freeze: {
    CVector3f pos = CVector3f::Zero();
    CUnitVector3f dir = GetTransform().TransposeRotate(direction);
    Freeze(mgr, pos, dir, duration, -1.f);
    break;
  }
  case CKnockBackMgr::kFU_Shock:
    Shock(mgr, duration, -1.f);
    break;
  case CKnockBackMgr::kFU_Burn:
    Burn(mgr, duration, gpTweakPlayerGun->GetAIBurnDamage());
    break;
  case CKnockBackMgr::kFU_ImmediateExplosion:
    Shock(mgr, duration, -1.f);
    break;
  case CKnockBackMgr::kFU_BlackDeath:
  case CKnockBackMgr::kFU_ImmediateDisintegration: {
    const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(source));
    if (!weapon) {
      break;
    }

    mDisintegrationOrigin = weapon->GetTranslation();
    if (followUp == CKnockBackMgr::kFU_BlackDeath) {
      mBlackDeath = true;
    } else {
      mDisintegrating = true;
      mFadeOnDeathTime = 1.5f;
    }
    mStopPhysics = true;
    mUseDisintegrationPlane = true;
    mKnockBackController.EnableExplodeDeath(false);
    Burn(mgr, duration, -1.f);
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
    mFadeToDeath = mBurning = true;
    mBurnThinkRateTimer = 1.5f;
    mDrawParticles = false;
    mBodyController->DouseFlames();

    CActorModelParticles* particles = mgr.ActorModelParticles();
    particles->StopFire(*this);
    particles->StartBurnDeath(*this, mgr);
    particles->StartImplosion(*this, mDisintegrationOrigin, mBlackDeath);
    const CColor& color = CColor::White();
    mColor.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), mColor.GetAlphau8());
    break;
  }
  case CKnockBackMgr::kFU_LaggedBurnDeath:
    mLaggedBurnDeath = true;
  case CKnockBackMgr::kFU_BurnDeath: {
    Burn(mgr, duration, -1.f);
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
    mFadeToDeath = mBurning = true;
    mBurnThinkRateTimer = 1.5f;
    mDrawParticles = false;
    mBodyController->DouseFlames();

    CActorModelParticles* particles = mgr.ActorModelParticles();
    particles->StopFire(*this);
    particles->StartBurnDeath(*this, mgr);
    if (!mLaggedBurnDeath) {
      particles->DoFirePop(*this);
      particles->StartAsh(*this);
    }
    break;
  }
  case CKnockBackMgr::kFU_Death:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    break;
  case CKnockBackMgr::kFU_ExplodeDeath:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    if (GetDeathExplosionParticle().valid() || mDeathExplosionElectric.valid()) {
      mBodyController->CommandMgr().Reset();
      MassiveDeath(mgr);
    } else if (mBodyController->IsFrozen()) {
      mBodyController->FrozenBreakout();
    }
    break;
  case CKnockBackMgr::kFU_IceDeath:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    if (mIceDeathExplosionParticle.valid()) {
      mBodyController->CommandMgr().Reset();
      MassiveFrozenDeath(mgr);
    } else if (mBodyController->IsFrozen()) {
      mBodyController->FrozenBreakout();
    }
    break;
  default:
    break;
  }
}

void CPatterned::UpdateAlphaDelta(CStateManager& mgr, float dt) {
  if (mAlphaDelta == 0.f) {
    return;
  }
  float alpha = mColor.GetAlpha() + dt * mAlphaDelta;
  if (alpha > 1.f) {
    mAlphaDelta = 0.f;
    alpha = 1.f;
  } else if (alpha < 0.f) {
    mAlphaDelta = 0.f;
    alpha = 0.f;
    if (mFadeToDeath) {
      DeathDelete(mgr);
    }
  }
  Shadow()->SetUserAlpha(alpha);
  mColor.SetAlpha(alpha);
  AnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(CColor(1.f, 1.f, 1.f, alpha));
}

void CPatterned::UpdateHitDamageTime(float dt) {
  CEchoEmitter* emitter = EchoEmitter();
  if (mDamageCooldownTimer > 0.f) {
    CColor baseColor = CColor::Black();
    if (mBodyController->IsFrozen()) {
      baseColor =
          CColor::Lerp(CColor::Black(), skFrozenColor, mBodyController->GetPercentageFrozen());
    }

    mDamageCooldownTimer = rstl::max_val(mDamageCooldownTimer - dt, 0.f);
    const float t = rstl::min_val(1.f, mDamageCooldownTimer / skDamageHitTime);
    if (emitter) {
      emitter->SetDamageExplicit(t);
    }
    const CColor& color = CColor::Lerp(baseColor, mDamageColor, t);
    mColor.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), mColor.GetAlphau8());
    SetDamageHighlight(mDamageCooldownTimer > 0.f);
  } else if (mBodyController->IsFrozen()) {
    const CColor& color =
        CColor::Lerp(CColor::Black(), skFrozenColor, mBodyController->GetPercentageFrozen());
    mColor.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), mColor.GetAlphau8());
    if (emitter) {
      emitter->SetDamageExplicit(0.f);
    }
  }
}

bool rstl::operator==(const char* lhs, const rstl::string& rhs) {
  return rhs.compare(lhs, -1) == 0;
}

void CPatterned::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  if (mDieIf80PercFrozen && mBodyController->GetPercentageFrozen() > 0.8f) {
    mPendingMassiveFrozenDeath = true;
  }

  if (!mAlive) {
    if ((mPendingMassiveDeath || mPendingMassiveFrozenDeath) && mXDamageDelay <= 0.f) {
      if (mPendingMassiveFrozenDeath) {
        SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kInvalidUniqueId, kSM_None);
        MassiveFrozenDeath(mgr);
      } else {
        SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kInvalidUniqueId, kSM_None);
        MassiveDeath(mgr);
      }
      return;
    }

    mXDamageDelay -= dt;
    if (mStateControlledMassiveDeath && mStateMachine->GetName() != nullptr) {
      const bool isDead = mStateMachine->GetName() == rstl::string_l("Dead");
      if (isDead && mStateMachine->GetTime() > 15.f) {
        MassiveDeath(mgr);
      }
    }
  }

  UpdateAlphaDelta(mgr, dt);
  UpdateIngPossession(dt);
  mLastHP = GetHealthInfo()->GetHP();
  if (!mStateMachine->HasState()) {
    InitializeStateMachine(mgr);
  }

  CVector3f diffVec = mLatestPredictedTranslation - GetTranslation();
  if (!mVerticalMovement) {
    diffVec.SetZ(0.f);
  }
  if (CVector3f::Dot(diffVec, diffVec) > 0.1f * dt) {
    mPredictedLeashTime += dt;
  } else {
    mPredictedLeashTime = 0.f;
  }

  if (mKnockBackController.IsShockEnabled()) {
    if (mBodyController->IsElectrocuting()) {
      mgr.ActorModelParticles()->StartElectric(*this);
      if (mPendingShockDamage > 0.f && mAlive) {
        const CDamageInfo shockDamage =
            CDamageInfo(CWeaponMode(kWT_Annihilator), mPendingShockDamage, 0.f, 0.f, false, false);
        mgr.ApplyDamage(
            kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, CDamageInfo(shockDamage, dt),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    } else if (mPendingShockDamage != 0.f) {
      mPendingShockDamage = 0.f;
      mBodyController->DouseElectrocuting();
      mgr.ActorModelParticles()->StopElectric(*this);
    }
  }

  if (mBodyController->IsOnFire()) {
    if (mAlive) {
      mgr.ActorModelParticles()->LightDudeOnFire(*this);
      const CDamageInfo fireDamage =
          CDamageInfo(CWeaponMode(kWT_Light), mPendingFireDamage, 0.f, 0.f, false, false);
      mgr.ApplyDamage(
          kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, CDamageInfo(fireDamage, dt),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  } else {
    if (mPendingFireDamage > 0.f) {
      mPendingFireDamage = 0.f;
    }
    if (mBodyController->IsFrozen()) {
      mgr.ActorModelParticles()->StopFire(*this);
    }
  }

  if (mBurning) {
    mAlphaDelta = -1.f / GetFadeOnDeathTime();
  }
  if (mPendingDeath) {
    mPendingDeath = false;
    Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
  }

  float thinkDt;
  if (mAlive) {
    thinkDt = dt;
  } else {
    thinkDt = dt * GetDeathTimeScale();
  }
  mBodyController->Update(thinkDt, mgr);
  mBodyController->MultiplyPlaybackRate(mSpeed * mBodyController->GetTimeScale());

  mAnimationDeltas =
      UpdateAnimation(thinkDt, mgr, !(mBodyController->GetPercentageFrozen() >= 1.f));
  if (mEnableStateMachine && mBodyController->GetPercentageFrozen() < 1.f) {
    mStateMachine->Update(mgr, *this, thinkDt);
  }
  ThinkAboutMove(thinkDt);
  mKnockBackController.Update(thinkDt, mgr, *this);

  const CMotionState motion = PredictMotion(thinkDt);
  mLatestPredictedTranslation = GetTranslation() + motion.GetTranslation();
  mSolidCollision = false;
  mBlockingCollision = false;
  if (mCurDamageRemTime > 0.f) {
    mCurDamageRemTime -= dt;
  }
  if (mBurning && mBurnThinkRateTimer > dt) {
    mBurnThinkRateTimer -= dt;
  }
  if (!mBlackDeath && !mDisintegrating) {
    UpdateHitDamageTime(dt);
  }

  if (mBodyController->GetPercentageFrozen() != 1.f) {
    if (mLatestLeashPosition == CVector3f::Zero()) {
      mLatestLeashPosition = GetTranslation();
    }
    float playerLeashRadius = mPlayerLeashRadius;
    if (playerLeashRadius != 0.f) {
      if ((GetTranslation() - mgr.GetPlayer(0)->GetTranslation()).MagSquared() >
          playerLeashRadius * playerLeashRadius) {
        mCurPlayerLeashTime += dt;
      } else {
        mCurPlayerLeashTime = 0.f;
      }
    } else {
      mCurPlayerLeashTime = 0.f;
    }
  } else {
    StopLoopedSounds();
  }

  mWaypointNavigation.Update(dt);
  if (mStopPhysics) {
    Stop();
  }
}

void CPatterned::InitializeStateMachine(CStateManager& mgr) {
  if (mStateMachine->HasState()) {
    return;
  }
  switch (mStateMachine->GetType()) {
  case 0: {
    CStateMachine* machine = GetStateMachine();
    if (machine) {
      static_cast< TStateMachineState< CPatterned >* >(mStateMachine.get())->Setup(machine);
      SetupStateMachine(mgr);
      mStateMachine->SetState(mgr, *this, rstl::string_l("Start"));
    }
    break;
  }
  case 1: {
    CGenericFSM2* machine = GetStateMachine2();
    if (machine) {
      static_cast< CGenericFSM2State< CPatterned >* >(mStateMachine.get())->Setup(*machine);
      SetupStateMachine(mgr);
      mStateMachine->SetState(mgr, *this, rstl::string_l("Start"));
    }
    break;
  }
  }
}

void CPatterned::Touch(CActor& actor, CStateManager& mgr) {
  if (!mAlive) {
    return;
  }
  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    if (TCastToPtr< CPlayer >(
            const_cast< CEntity* >(mgr.GetObjectById(projectile->GetOwnerId())))) {
      mHitByPlayerProjectile = true;
    }
  }
}

void CPatterned::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                              CStateManager& mgr) {
  if (mCurDamageRemTime <= 0.f) {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(id));
    if (player != nullptr) {
      bool jumpOnHead = player->GetTimeSinceJump() < 5.f && list.GetCount() != 0 &&
                        list[0].GetNormalLeft().GetZ() > 0.707f;

      if (mAlive || jumpOnHead) {
        CDamageInfo contactDamage = GetContactDamage();
        if (!mAlive || mBodyController->IsFrozen()) {
          contactDamage.SetDamage(0.f);
        }

        if (jumpOnHead) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), contactDamage,
                          CMaterialFilter::GetPassEverything(), -player->GetVelocityWR());
          player->SetTimeSinceJump(1000.f);
        } else if (mAlive && mBodyController->GetPercentageFrozen() != 1.f) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), contactDamage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
        }

        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  }

  static CMaterialList skSolidTypes(kMT_Unknown59, kMT_Ceiling, kMT_Wall, kMT_Floor, kMT_Character);

  mSolidCollision = true;
  for (int i = 0; i < list.GetCount(); ++i) {
    const CCollisionInfo& info = list[i];
    if (info.GetMaterialLeft().SharesMaterials(skSolidTypes)) {
      if (info.GetMaterialLeft().HasMaterial(kMT_Floor)) {
        if (!mIsFlyer) {
          continue;
        }
      } else if (GetVelocityWR().IsNonZero() &&
                 CVector3f::Dot(info.GetNormalLeft(), GetVelocityWR()) >= 0.f) {
        continue;
      }

      mBlockingCollision = true;
      return;
    }
  }

  CPhysicsActor::CollidedWith(id, list, mgr);
}

void CPatterned::ThinkAboutMove(float dt) {
  if (dt > 0.f) {
    if (!(mDisabledAnimationDeltas & kADF_Translation) &&
        mBodyController->GetBodyStateInfo().GetCurrentState()->ApplyAnimationDeltas()) {
      const CVector3f scale = GetModelData()->GetScale();
      const CVector3f scaledDelta = CVector3f::ByElementMultiply(
          CVector3f::ByElementMultiply(scale, mAnimationDeltas.GetOffsetDelta()), mMoveScale);
      if (!mVerticalMovement && !mOnGround) {
        MoveInOneFrameOR(scaledDelta, dt);
      } else {
        MoveToOR(scaledDelta, dt);
      }
    }
    if (!(mDisabledAnimationDeltas & kADF_Rotation)) {
      RotateToOR(mAnimationDeltas.GetOrientationDelta(), dt);
    }
  }
}

void CPatterned::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                 float dt) {
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f lctrXf = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
    const CVector3f forward = lctrXf.GetForward();

    if (CVector3f::Dot(forward, (aimPos - lctrXf.GetTranslation()).AsNormalized()) > 0.f) {
      const CTransform4f lookAtXf = CTransform4f::LookAt(lctrXf.GetTranslation(), aimPos);
      LaunchProjectile(lookAtXf, mgr, 1, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                       CVector3f(1.f, 1.f, 1.f));
    } else {
      LaunchProjectile(lctrXf, mgr, 1, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                       CVector3f(1.f, 1.f, 1.f));
    }
    break;
  }
  case kUE_DamageOn: {
    const CVector3f scale = GetModelData()->GetScale();
    const CTransform4f& lctrXf = GetLocatorTransform(node.GetLocatorName());
    CVector3f xfOrigin = CVector3f::ByElementMultiply(scale, lctrXf.GetTranslation());
    xfOrigin = GetTransform() * xfOrigin;
    const CVector3f margin = CVector3f::ByElementMultiply(scale, CVector3f(1.f, 1.f, 0.5f));
    const CAABox touchBounds(xfOrigin - margin, xfOrigin + margin);

    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      CPlayer* player = mgr.GetPlayer(i);
      if (touchBounds.DoBoundsOverlap(player->GetBoundingBox())) {
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    }
    break;
  }
  case kUE_Delete:
    if (!mAlive) {
      if (!mFadeToDeath) {
        mAlphaDelta = -1.f / GetFadeOnDeathTime();
        mFadeToDeath = true;
      }
      RemoveMaterial(kMT_Character, kMT_Unknown59, kMT_Target, kMT_Orbit, mgr);
      AddMaterial(kMT_NoPlatformCollision, mgr);
    } else {
      DeathDelete(mgr);
    }
    break;
  case kUE_BreakLockOn:
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    break;
  case kUE_BecomeShootThrough:
    AddMaterial(kMT_NoPlatformCollision, mgr);
    break;
  case kUE_RemoveCollision:
    RemoveMaterial(kMT_Unknown59, mgr);
    break;
  default:
    break;
  }

  CActor::DoUserAnimEvent(mgr, node, type, dt);
}

void CPatterned::Burn(CStateManager&, float duration, float damage) {
  if (mKnockBackController.IsBurnEnabled() &&
      GetDamageVulnerability()->WeaponHits(CWeaponMode(kWT_Light), 0)) {
    mBodyController->SetOnFire(duration);
    mPendingFireDamage = damage;
  }
}

void CPatterned::Shock(CStateManager&, float duration, float damage) {
  if (mKnockBackController.IsShockEnabled() &&
      GetDamageVulnerability()->WeaponHits(CWeaponMode(kWT_Annihilator), 0)) {
    mBodyController->SetElectrocuting(duration);
    mPendingShockDamage = damage;
  }
}

void CPatterned::Freeze(CStateManager&, const CVector3f&, CUnitVector3f, float duration,
                        float intoFreezeDuration) {
  if (mLostMassiveFrozenHP) {
    mDieIf80PercFrozen = true;
  }

  bool playSfx = false;
  if (intoFreezeDuration < 0.f) {
    intoFreezeDuration = mIntoFreezeDuration;
  }
  if (mBodyController->IsFrozen()) {
    mBodyController->Freeze(intoFreezeDuration, duration, mOutOfFreezeDuration);
  } else if (!mBodyController->IsElectrocuting() && !mBodyController->IsOnFire()) {
    mBodyController->Freeze(intoFreezeDuration, duration, mOutOfFreezeDuration);
    playSfx = true;
  }

  if (playSfx) {
    CSfxManager::AddEmitter(mFrozenSfx, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                            CSfxManager::kMedPriority);
  }
}

float CPatterned::GetDeathTimeScale() const {
  return rstl::max_val(mBurning ? mBurnThinkRateTimer / 1.5f : 1.f, 0.1f);
}

void CPatterned::DeathDelete(CStateManager& mgr) {
  mSuppressKnockBack = true;
  if (!mStateMachine->HasState()) {
    InitializeStateMachine(mgr);
  }
  SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
  if (GetBodyController()->IsElectrocuting()) {
    mPendingShockDamage = 0.f;
    BodyController()->DouseElectrocuting();
    mgr.ActorModelParticles()->StopElectric(*this);
  }
  mgr.DeleteObjectRequest(GetUniqueId());
}

CDamageInfo CPatterned::GetContactDamage() const {
  if (!mAlive) {
    return CDamageInfo();
  }
  return mContactDamage;
}

CTransform4f CPatterned::GetLctrTransform(const rstl::string& name) const {
  return GetTransform() * GetScaledLocatorTransform(name);
}

CTransform4f CPatterned::GetLctrTransform(const CSegId& id) const {
  CTransform4f locator = GetAnimationData()->GetLocatorTransform(id, nullptr);
  CVector3f scaled =
      CVector3f::ByElementMultiply(GetModelData()->GetScale(), locator.GetTranslation());
  return GetTransform() * CTransform4f(locator.BuildMatrix3f(), scaled);
}

CVector3f CPatterned::GetAimPosition(const CStateManager&, float dt) const {
  CVector3f offset = CVector3f::Zero();
  if (dt > 0.f) {
    const CMotionState motion = PredictMotion(dt);
    offset = motion.GetTranslation();
  }

  if (mLockOnTarget.val() != 0xff) {
    const CAnimData* animData = GetModelData()->GetAnimationData();
    const CTransform4f locatorXf = animData->GetLocatorTransform(mLockOnTarget, 0);
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f scaledOrigin = CVector3f::ByElementMultiply(scale, locatorXf.GetTranslation());

    if (GetTouchBounds()) {
      offset += GetTouchBounds()->ClampToBox(GetTransform() * scaledOrigin);
    } else {
      const CAABox& baseBox = GetBaseBoundingBox();
      const CAABox primBox(baseBox.GetMinPoint() + GetPrimitiveOffset(),
                           baseBox.GetMaxPoint() + GetPrimitiveOffset());
      offset += GetTransform() * primBox.ClampToBox(scaledOrigin);
    }
  } else {
    offset += GetBoundingBox().GetCenterPoint();
  }

  return offset;
}

CVector3f CPatterned::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

void CPatterned::PreRender(CStateManager& mgr) {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
    SetCalculateLighting(false);
    ActorLights()->BuildConstantAmbientLighting(CColor::White());
  } else {
    SetCalculateLighting(true);
  }

  CColor color = mColor;
  const uchar alpha = GetModelAlphau8(mgr);
  if (alpha < 255) {
    if (color.GetRedu8() == 0 && color.GetGreenu8() == 0 && color.GetBlueu8() == 0) {
      color = CColor::White();
    }

    if (mBlackDeath || mDisintegrating) {
      SetModelFlags(CModelFlags(CModelFlags::kT_ColorLerp,
                                mBlackDeath
                                    ? skBlackDeathColor.WithAlphaOf((255 - alpha) / 255.f)
                                    : skDisintegrationColor.WithAlphaOf((255 - alpha) / 255.f)));

      const CAABox bounds = GetOtherBounds();
      const CVector3f center = bounds.GetCenterPoint();
      const CUnitVector3f normal(center - mDisintegrationOrigin);
      const uchar planeAlpha = GetModelAlphau8(mgr);
      const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
      float width = extent.GetX();
      if (CMath::AbsF(normal.GetY()) > CMath::AbsF(normal.GetX())) {
        width = extent.GetY();
      }
      if (CMath::AbsF(normal.GetZ()) > CMath::AbsF(normal.GetY())) {
        width = extent.GetZ();
      }
      mIngSnatchingPlane = CPlane(center - (planeAlpha / 255.f - 0.5f) * width * normal, normal);
    } else if (mLaggedBurnDeath) {
      const uchar stripedAlpha = alpha > 127 ? (alpha - 128) * 2 : 255;
      SetModelFlags(
          CModelFlags(CModelFlags::kT_ColorLerp,
                      CColor(skDisintegrateColor.GetRedu8(), skDisintegrateColor.GetGreenu8(),
                             skDisintegrateColor.GetBlueu8(), (stripedAlpha * stripedAlpha) >> 8)));
    } else if (mBurning) {
      SetModelFlags(CModelFlags::AlphaBlended(CColor::Black()));
    } else {
      SetModelFlags(CModelFlags::AlphaBlended(
          CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), alpha)));
    }
  } else if (color.GetRedu8() != 0 || color.GetGreenu8() != 0 || color.GetBlueu8() != 0) {
    SetModelFlags(CModelFlags(
        CModelFlags::kT_Two, CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), 255)));
  } else {
    SetModelFlags(CModelFlags::Normal());
  }

  CActor::PreRender(mgr);
  if (mUseDisintegrationPlane || (mIngPossessionBlend > 0.f && mIngPossessionBlend < 1.f)) {
    SetModelFlags(
        CModelFlags(GetModelFlags(), GetModelFlags().GetOtherFlags() | CModelFlags::kF_Unknown80));
  }
}

bool CPatterned::CanRenderUnsorted(const CStateManager& mgr) const {
  return GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel()
             ? false
             : CActor::CanRenderUnsorted(mgr);
}

void CPatterned::PreRenderAllViewports(CStateManager& mgr) {
  CActor::PreRenderAllViewports(mgr);
  if (GetEchoEmitterEnabled()) {
    const CAABox& bounds =
        HasModelData() ? GetModelData()->GetBounds(GetTransform()) : GetOtherBounds();
    const CVector3f center = bounds.GetCenterPoint();
    const CVector3f halfExtent = 0.375f * (bounds.GetMaxPoint() - bounds.GetMinPoint());
    EchoEmitter()->SetBounds(CAABox(center - halfExtent, center + halfExtent));
  }
}

void CPatterned::Render(const CStateManager& mgr) const {
  uint mask = 0;
  uint target = 0;
  if (mDrawParticles) {
    mgr.GetCharacterRenderMaskAndTarget(mask, target);
  }
  RenderSystemsToBeDrawnFirst(mgr, mask, target);

  if (mUseDisintegrationPlane) {
    GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), mIngSnatchingPlane);
  }

  if (mBurning) {
    const CTexture* ashyTexture = mgr.GetActorModelParticles()->GetAshyTexture(*this);
    const uchar alpha = GetModelAlphau8(mgr);
    if (ashyTexture && ((!mLaggedBurnDeath && alpha <= 255) || alpha <= 127)) {
      if (GetPointGeneratorParticles()) {
        mgr.SetupParticleHook(*this);
      }

      if (mBlackDeath || mDisintegrating) {
        CPhysicsActor::Render(mgr);
      } else if (HasModelData()) {
        const CColor disColor = mLaggedBurnDeath ? skDisintegrateColor : CColor::Black();
        const float t = (mLaggedBurnDeath ? 0.0078740157f : 0.0039215689f) * CCast::ToReal32(alpha);
        GetModelData()->DisintegrateDraw(mgr, GetTransform(), *ashyTexture, disColor, t);
      }

      if (GetPointGeneratorParticles()) {
        CSkinnedModel::ClearPointGeneratorFunc();
        mgr.GetActorModelParticles()->Render(mgr, *this);
      }
    } else {
      CPhysicsActor::Render(mgr);
    }
  } else if (IsBeingSnatched() == true) {
    RenderIngSnatchingTransition(mgr);
  } else {
    CPhysicsActor::Render(mgr);
  }

  if (mBodyController->IsFrozen() && !mBurning) {
    RenderIceModelWithFlags(CModelFlags::Normal());
  }

  RenderSystemsToBeDrawnLast(mgr, mask, target);
}

bool CPatterned::IsBeingSnatched() const {
  return mIngPossessionBlend > 0.f && mIngPossessionBlend < 1.f && mIngModel.valid() == true;
}

void CPatterned::RenderSystemsToBeDrawnFirst(const CStateManager&, uint mask, uint target) const {
  if (mDrawParticles) {
    GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirstPOICheck(mask, target);
  }
}

void CPatterned::RenderSystemsToBeDrawnLast(const CStateManager&, uint mask, uint target) const {
  if (mDrawParticles) {
    GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLastPOICheck(mask, target);
  }
}

void CPatterned::RenderIceModelWithFlags(const CModelFlags& flags) const {
  const CAnimData* animData = GetAnimationData();
  CModelFlags useFlags = flags.UseShaderSet(0);
  const rstl::optional_object< TLockedToken< CSkinnedModel > >& iceModel = animData->GetIceModel();
  if (iceModel.valid()) {
    animData->Render(***iceModel, useFlags);
  }
}

void CPatterned::RenderIngSnatchingTransition(const CStateManager& mgr) const {
  const CVector3f normal = GetIngSnatchingNormal(mIngPossessionBlend);
  const CVector3f point = GetIngSnatchingPoint(mIngPossessionBlend);
  CAnimData* animData = const_cast< CAnimData* >(GetAnimationData());
  const CVector3f& overlap = (0.5f * GetIngSnatchingModelOverlapSize()) * normal;

  const CVector3f& ingPoint = point - overlap;
  const CPlane ingPlane(ingPoint, CUnitVector3f(normal, CUnitVector3f::kN_No));
  GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), ingPlane);
  animData->SetSkinnedModel(*mIngModel);
  CPhysicsActor::Render(mgr);

  const CVector3f& normalPoint = point + overlap;
  const CPlane normalPlane(normalPoint, CUnitVector3f(-1.f * normal, CUnitVector3f::kN_No));
  GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), normalPlane);
  animData->SetSkinnedModel(mNormalModel);
  CPhysicsActor::Render(mgr);
}

CVector3f CPatterned::GetIngSnatchingNormal(float) const { return CVector3f::Up(); }

CVector3f CPatterned::GetIngSnatchingPoint(float t) const {
  const CAABox bounds = GetBoundingBox();
  const float height = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
  return GetTranslation() + height * ((1.f - t) * GetIngSnatchingNormal(t));
}

float CPatterned::GetIngSnatchingModelOverlapSize() const { return 0.f; }

void CPatterned::BuildIngModel(CAssetId model, CAssetId skinRules) {
  if (model != kInvalidAssetId && skinRules != kInvalidAssetId) {
    mIngModel = rstl::optional_object< TLockedToken< CSkinnedModel > >(
        rs_new CSkinnedModel(gpSimplePool->GetObj(SObjectTag('CMDL', model)),
                             gpSimplePool->GetObj(SObjectTag('CSKR', skinRules)),
                             GetAnimationData()->GetModelData()->GetLayoutInfo()));
    (*mIngModel)->SetLayoutInfo(GetAnimationData()->GetModelData()->GetLayoutInfo());
  }
}

bool CPatterned::CanBeIngPossessed(CStateManager&) const { return mAlive && mIngModel.valid(); }

bool CPatterned::CanBeUnPossessed(CStateManager&) const { return true; }

void CPatterned::SetIngPossessed(bool possessed, CStateManager&) {
  if (!IsIngPossessed() && possessed) {
    if (mIngPossessionData.unknown_0xb68c0aa3) {
      *HealthInfo() = LdrToHealthInfo(mIngPossessionData.ingPossessedHealth);
    }
    mIngPossessionDelay = 0.f;
    mIngPossessionDuration = 1.f;

    const int animation = mIngPossessionData.unknown_0x2befc1bf;
    if (animation != -1) {
      const CCharAnimTime start =
          GetAnimationData()->GetTimeOfUserEventForAnimation(animation, kUE_EventStart);
      if (start != CCharAnimTime::Infinity()) {
        mIngPossessionDelay = start.GetSeconds();
      }
      const CCharAnimTime stop =
          GetAnimationData()->GetTimeOfUserEventForAnimation(animation, kUE_EventStop);
      if (stop != CCharAnimTime::Infinity()) {
        mIngPossessionDuration = rstl::max_val(0.f, stop.GetSeconds() - mIngPossessionDelay);
      } else {
        mIngPossessionDuration = rstl::max_val(
            0.f, GetAnimationData()->GetAnimationDuration(animation) - mIngPossessionDelay);
      }
    }
  }
  mIngPossessionTarget = possessed ? 1.f : 0.f;
}

void CPatterned::SetIngPossessed(bool possessed, float duration, CStateManager&) {
  if (!IsIngPossessed() && possessed) {
    if (mIngPossessionData.unknown_0xb68c0aa3) {
      *HealthInfo() = LdrToHealthInfo(mIngPossessionData.ingPossessedHealth);
    }
    mIngPossessionDelay = 0.f;
    mIngPossessionDuration = duration;
  }
  mIngPossessionTarget = possessed ? 1.f : 0.f;
}

bool CPatterned::IsIngPossessed() const {
  return mIngPossessionBlend > 0.f || mIngPossessionTarget > 0.f;
}

void CPatterned::UpdateIngPossession(float dt) {
  if (mIngPossessionBlend < mIngPossessionTarget) {
    const float delta = mIngPossessionDuration > 0.f ? dt / mIngPossessionDuration : 1.f;
    if (mIngPossessionDelay <= 0.f) {
      mIngPossessionBlend = rstl::min_val(mIngPossessionBlend + delta, 1.f);
      if (mIngPossessionBlend == 1.f && mIngModel) {
        AnimationData()->SetSkinnedModel(*mIngModel);
      }
    } else {
      mIngPossessionDelay -= dt;
    }
  } else if (mIngPossessionBlend > mIngPossessionTarget) {
    mIngPossessionBlend = rstl::max_val(0.f, mIngPossessionBlend - dt);
    if (mIngPossessionBlend == 0.f) {
      AnimationData()->SetSkinnedModel(mNormalModel);
    }
  }
}

const CDamageVulnerability* CPatterned::GetDamageVulnerability() const {
  if (mIngPossessionTarget > mIngPossessionBlend) {
    return &CDamageVulnerability::ImmuneVulnerabilty();
  }
  if (IsIngPossessed() && mIngPossessionData.unknown_0xb68c0aa3) {
    return &mIngVulnerability;
  }
  return CAi::GetDamageVulnerability();
}

const CDamageVulnerability* CPatterned::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                               const CDamageInfo&) const {
  return GetDamageVulnerability();
}

CScannableObjectInfo* CPatterned::GetScannableObjectInfo() const {
  if (IsIngPossessed() && !mIngScanInfo.null()) {
    return **mIngScanInfo;
  }
  return CActor::GetScannableObjectInfo();
}

CEnergyProjectile* CPatterned::LaunchProjectile(const CTransform4f& xf, CStateManager& mgr,
                                                int maxProjectiles, uint attributes, bool homing,
                                                const CImpactVisorEffect& visorEffect,
                                                const CVector3f& scale) {
  CEnergyProjectile* projectile = nullptr;
  CProjectileInfo* projectileInfo = ProjectileInfo();
  if (projectileInfo->Token().TryCache()) {
    if (mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, maxProjectiles)) {
      projectile = rs_new CEnergyProjectile(
          true, ProjectileInfo()->Token(), kWT_AI, xf, kMT_Character, ProjectileInfo()->GetDamage(),
          mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
          homing ? mgr.GetPlayer(0)->GetUniqueId() : kInvalidUniqueId, attributes, false, scale,
          visorEffect, false, true, false, 1.f, 4.f, 4.f);

      if (projectile != nullptr) {
        mgr.AddObject(projectile);
      }
    }
  }

  return projectile;
}

EWeaponCollisionResponseTypes CPatterned::GetCollisionResponseType(const CVector3f& position,
                                                                   const CVector3f& direction,
                                                                   const CWeaponMode& mode,
                                                                   int attributes) const {
  if (GetBodyController()->IsFrozen() && mode.GetType() == kWT_Dark) {
    return kWCR_None;
  }
  return CAi::GetCollisionResponseType(position, direction, mode, attributes);
}

void CPatterned::PreThink(float dt, CStateManager& mgr) {
  mPreThinkDt = dt;
  CEntity::PreThink(dt, mgr);
}

void CPatterned::AddToRenderer(const CStateManager& mgr) const {
  if (mDrawParticles && HasModelData()) {
    uint mask;
    uint target;
    mgr.GetCharacterRenderMaskAndTarget(mask, target);
    if (const CAnimData* animData = GetAnimationData()) {
      animData->GetParticleDB().AddToRendererClippedMasked(mgr.GetFrustumPlanes(), mask, target);
    }
  }
  CActor::AddToRenderer(mgr);
}

bool CPatterned::IsOnStaticGround() const { return mOnStaticGround; }

bool CPatterned::TryToBeCaptured(CStateManager&) { return false; }

CCharAnimTime CPatterned::GetTimeOfUserEventForAnimation(const CPASAnimParmData& parms,
                                                         EUserEventType type) const {
  const rstl::pair< float, int > best =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
  if (best.first > FLT_EPSILON) {
    return GetAnimationData()->GetTimeOfUserEventForAnimation(best.second, type);
  }
  return CCharAnimTime(CCharAnimTime::kT_Infinity, 1.f);
}

int CPatterned::GetNumUserEventsForAnimation(const CPASAnimParmData& parms,
                                             EUserEventType type) const {
  const rstl::pair< float, int > best =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
  if (best.first > FLT_EPSILON) {
    return GetAnimationData()->CountUserEventsForAnimation(best.second, type);
  }
  return 0;
}

float CPatterned::GetAverageAttackTime() const {
  const float timeScale = GetBodyController()->GetTimeScale();
  if (timeScale > 0.f) {
    return mAverageAttackTime / timeScale;
  }
  return mAverageAttackTime;
}

void CPatterned::AddParticleEffect(CStateManager& mgr, const CTransform4f& xf, float particleScale,
                                   CAssetId particle, uint name, int flags) {
  if (HasAnimation()) {
    AnimationData()->GetParticleDB().AddParticleEffect(
        name, flags, CPositionalParticleData(0, SObjectTag('PART', particle), xf, particleScale),
        GetModelData()->GetScale(), &mgr, GetCurrentAreaId(), 0);
  }
}

bool CPatterned::IsScanVisorSelfRender() const { return false; }

CAABox CPatterned::GetScanVisorRenderBounds(const CStateManager&) const {
  return CAABox::Identity();
}

void CPatterned::ScanVisorRender(const CStateManager&, const CTransform4f&,
                                 const CModelFlags&) const {}
