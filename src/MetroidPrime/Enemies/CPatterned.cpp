#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGenericFSM2State.hpp"
#include "MetroidPrime/CPositionalParticleData.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include <float.h>

const float CPatterned::skDamageHitTime = 0.33f;
const float CPatterned::skActorApproachDistance = 3.f;
const CColor CPatterned::skDamageColor(0.5f, 0.f, 0.f, 1.f);
const CColor CPatterned::skHitsWithoutDamageColor(0.5f, 0.5f, 0.f, 1.f);
const CColor CPatterned::skFrozenColor(0x321F50FF);

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
, mIngVulnerability(pinfo.mIngPossessionData.ingVulnerability)
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
, x422_26_(0)
, x422_28_(false)
, x422_29_(false)
, x422_30_(false)
, mStopPhysics(false)
, x423_24_(false)
, mSuppressKnockBack(false)
, mContactDamage(pinfo.mContactDamageInfo)
, mCurDamageRemTime(0.f)
, mDamageWaitTime(pinfo.mDamageWaitTime)
, mDamageCooldownTimer(-1.f)
, mColor(0.f, 0.f, 0.f, 1.f)
, mDamageColor(skDamageColor)
, mPosDelta(CVector3f::Zero())
, mRotDelta(CQuaternion::NoRotation())
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
  fn_800747a4(mIngPossessionData.ingPossessedModel, mIngPossessionData.ingPossessedSkinRules);
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

void CPatterned::SetupStateMachine(CStateManager&) {
  static const StateMachine::STriggerFunction triggers[] = {
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
  static const StateMachine::SStateFunction states[] = {
      {"Start", &CPatterned::Start},
      {"Dead", &CPatterned::Dead},
      {"PathFind", &CPatterned::PathFind},
      {"Patrol", &CPatterned::Patrol},
  };
  mStateMachine->SetTriggerFunctions(triggers, ARRAY_SIZE(triggers));
  mStateMachine->SetStateFunctions(states, ARRAY_SIZE(states));
}

void CPatterned::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CAi::AcceptScriptMsg(mgr, msg);
  // TODO: Restore registration, floor, activation, deletion and damage-message handling.
}

void CPatterned::SetDestPos(const CVector3f& position) { mDestPos = position; }

CVector3f CPatterned::GetGunEyePos() const {
  const CAABox& bounds = GetBaseBoundingBox();
  return GetTranslation() +
         CVector3f(0.f, 0.f, 0.6f * (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()));
}

bool CPatterned::ApplyBoneTracking() const {
  if (!mAlive || mBodyController->IsFrozen() ||
      mKnockBackController.GetFlinchRemainingTime() > 0.f) {
    return false;
  }
  return true;
}

float CPatterned::GetAnimationDistance(const CPASAnimParmData& params) const {
  float distance = 1.f;
  const rstl::pair< float, int > bestAnim =
      GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(params, -1);

  if (bestAnim.first > FLT_EPSILON) {
    const CAnimData* animData = GetAnimationData();
    const float duration = animData->GetAnimationDuration(bestAnim.second);
    distance = animData->GetAverageVelocity(bestAnim.second);
    distance *= duration;
  }

  return distance;
}

float CPatterned::GetAnimationDuration(const CPASAnimParmData& params) const {
  const rstl::pair< float, int > bestAnim =
      GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(params, -1);
  if (bestAnim.first > FLT_EPSILON) {
    return GetAnimationData()->GetAnimationDuration(bestAnim.second);
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
        const CTransform4f deathXf =
            CTransform4f::LookAt(pos, target) *
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

void CPatterned::ApplyKnockBackFollowUp(CStateManager&, const CVector3f&, CKnockBackMgr::EFollowUp,
                                        float, float, TUniqueId, TUniqueId) {
  // TODO: Apply the knockback rule's follow-up (freeze, burn, shock, death or disintegration).
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

void CPatterned::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  // TODO: Restore death, body/animation, continuous damage, movement and leash updates.
}

void CPatterned::InitializeStateMachine(CStateManager& mgr) {
  if (mStateMachine->HasState()) {
    return;
  }
  if (mStateMachine->GetType() == 1) {
    CGenericFSM2* machine = GetStateMachine2();
    if (!machine) {
      return;
    }
    static_cast< CGenericFSM2State< CPatterned >* >(mStateMachine.get())->Setup(*machine);
  } else {
    CStateMachine* machine = GetStateMachine();
    if (!machine) {
      return;
    }
    static_cast< TStateMachineState< CPatterned >* >(mStateMachine.get())->Setup(machine);
  }
  SetupStateMachine(mgr);
  mStateMachine->SetState(mgr, *this, rstl::string("Start"));
}

void CPatterned::Touch(CActor& actor, CStateManager& mgr) {
  if (mAlive) {
    if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
      if (TCastToPtr< CPlayer >(
              const_cast< CEntity* >(mgr.GetObjectById(projectile->GetOwnerId())))) {
        mHitByPlayerProjectile = true;
      }
    }
  }
}

void CPatterned::CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&) {
  // TODO: Recover ground/static-ground flags, collision response and linked script messages.
}

void CPatterned::ThinkAboutMove(float) {
  // TODO: Apply scaled animation translation/rotation and account for frozen/disabled movement.
}

void CPatterned::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                 float dt) {
  // TODO: Restore projectile, material, damage-window, movement and body-state events.
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
  return CMath::Max(0.1f, mBurning ? mBurnThinkRateTimer / 1.5f : 1.f);
}

void CPatterned::DeathDelete(CStateManager& mgr) {
  mSuppressKnockBack = true;
  if (!mStateMachine->HasState()) {
    InitializeStateMachine(mgr);
  }
  SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);

  if (mBodyController->IsElectrocuting()) {
    mPendingShockDamage = 0.f;
    mBodyController->DouseElectrocuting();
    mgr.ActorModelParticles()->StopElectric(*this);
  }

  mgr.DeleteObjectRequest(GetUniqueId());
}

CDamageInfo CPatterned::GetContactDamage() const { return mAlive ? mContactDamage : CDamageInfo(); }

CTransform4f CPatterned::GetLctrTransform(const rstl::string& name) const {
  return GetLctrTransform(GetAnimationData()->GetLocatorSegId(name));
}

CTransform4f CPatterned::GetLctrTransform(const CSegId& id) const {
  CTransform4f locator = GetAnimationData()->GetLocatorTransform(id, nullptr);
  locator.SetTranslation(
      CVector3f::ByElementMultiply(GetModelData()->GetScale(), locator.GetTranslation()));
  return GetTransform() * locator;
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
  CActor::PreRender(mgr);
  // TODO: Restore actor lighting, frozen/possession models and particle preparation.
}

bool CPatterned::CanRenderUnsorted(const CStateManager& mgr) const {
  if (GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel()) {
    return false;
  }
  return CActor::CanRenderUnsorted(mgr);
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
  // TODO: Restore model flags, damage color and possession-transition rendering.
  CPhysicsActor::Render(mgr);
}

bool CPatterned::IsBeingSnatched() const {
  return mIngPossessionBlend > 0.f && mIngPossessionBlend < 1.f && mIngModel.valid();
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

void CPatterned::fn_80074e54(const CModelFlags&) const {
  // TODO: Draw the animation's ice model with the adjusted model flags.
}

void CPatterned::RenderIngSnatchingTransition(const CStateManager&) const {
  // TODO: Render the normal/possessed models on opposite sides of the snatching plane.
}

CVector3f CPatterned::GetIngSnatchingNormal(float) const { return CVector3f::Up(); }

CVector3f CPatterned::GetIngSnatchingPoint(float t) const {
  const CAABox bounds = GetBoundingBox();
  const float height = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
  return GetTranslation() + height * (1.f - t) * GetIngSnatchingNormal(t);
}

float CPatterned::GetIngSnatchingModelOverlapSize() const { return 0.f; }

void CPatterned::fn_800747a4(CAssetId, CAssetId) {
  // TODO: Build the possessed skinned model using the actor's shared character layout.
}

bool CPatterned::CanBeIngPossessed(CStateManager&) const { return mAlive && mIngModel.valid(); }

bool CPatterned::CanBeUnPossessed(CStateManager&) const { return true; }

void CPatterned::SetIngPossessed(bool possessed, CStateManager& mgr) {
  SetIngPossessed(possessed, 1.f, mgr);
  // TODO: Derive possession delay/duration from the current animation's EventStart/EventStop POIs.
}

void CPatterned::SetIngPossessed(bool possessed, float duration, CStateManager&) {
  if (!IsIngPossessed() && possessed) {
    if (mIngPossessionData.unknown_0xb68c0aa3) {
      *HealthInfo() = CHealthInfo(mIngPossessionData.ingPossessedHealth.health,
                                  mIngPossessionData.ingPossessedHealth.hI_KnockBackResistance);
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
    if (mIngPossessionDelay > 0.f) {
      mIngPossessionDelay -= dt;
    } else {
      mIngPossessionBlend = CMath::Min(1.f, mIngPossessionBlend + delta);
      if (mIngPossessionBlend == 1.f && mIngModel) {
        AnimationData()->SetSkinnedModel(*mIngModel);
      }
    }
  } else if (mIngPossessionBlend > mIngPossessionTarget) {
    mIngPossessionBlend = CMath::Max(0.f, mIngPossessionBlend - dt);
    if (mIngPossessionBlend == 0.f) {
      AnimationData()->SetSkinnedModel(mNormalModel);
    }
  }
}

const CDamageVulnerability* CPatterned::GetDamageVulnerability() const {
  if (mIngPossessionBlend < mIngPossessionTarget) {
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
  return IsIngPossessed() && !mIngScanInfo.null() ? **mIngScanInfo
                                                  : CActor::GetScannableObjectInfo();
}

CEnergyProjectile* CPatterned::LaunchProjectile(const CTransform4f& xf, CStateManager& mgr,
                                                int maxProjectiles, uint attributes, bool homing,
                                                const CImpactVisorEffect& visorEffect,
                                                const CVector3f& scale) {
  CEnergyProjectile* projectile = nullptr;
  CProjectileInfo* projectileInfo = ProjectileInfo();
  if (projectileInfo->Token().IsLoaded()) {
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
  if (mBodyController->IsFrozen() && mode.GetRawType() == kWT_Dark) {
    return kWCR_None;
  }
  return CAi::GetCollisionResponseType(position, direction, mode, attributes);
}

void CPatterned::PreThink(float dt, CStateManager& mgr) {
  mPreThinkDt = dt;
  CEntity::PreThink(dt, mgr);
}

void CPatterned::AddToRenderer(const CStateManager& mgr) const {
  // TODO: Queue the animation particle database with the current render mask/target.
  CActor::AddToRenderer(mgr);
}

bool CPatterned::IsOnStaticGround() const { return mOnStaticGround; }

bool CPatterned::TryToBeCaptured(CStateManager&) { return false; }

CCharAnimTime CPatterned::GetTimeOfUserEventForAnimation(const CPASAnimParmData& params,
                                                         EUserEventType event) const {
  const rstl::pair< float, int > bestAnim =
      GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(params, -1);
  if (bestAnim.first > FLT_EPSILON) {
    return GetAnimationData()->GetTimeOfUserEventForAnimation(bestAnim.second, event);
  }
  return CCharAnimTime::Infinity();
}

int CPatterned::GetNumUserEventsForAnimation(const CPASAnimParmData& params,
                                             EUserEventType event) const {
  const rstl::pair< float, int > bestAnim =
      GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(params, -1);
  if (bestAnim.first > FLT_EPSILON) {
    return GetAnimationData()->CountUserEventsForAnimation(bestAnim.second, event);
  }
  return 0;
}

float CPatterned::GetAverageAttackTime() const {
  const float timeScale = mBodyController->GetTimeScale();
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

CAABox CPatterned::GetScanVisorRenderBounds(const CStateManager&) const {
  return CAABox::Identity();
}

CPatterned::~CPatterned() {}
