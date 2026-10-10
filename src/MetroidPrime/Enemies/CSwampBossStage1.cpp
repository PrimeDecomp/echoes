#include "MetroidPrime/Enemies/CSwampBossStage1.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "REL/REL_Setup.h"
#include "rstl/algorithm.hpp"

static EMaterialTypes SolidMaterial = kMT_Solid;   // Guessed name
static EMaterialTypes PlayerMaterial = kMT_Player; // Guessed name

static const char* const skBossName = "BossSwampBossStage1";
static const char* const skTongueLocatorName = "tongue_LCTR_SDK";

static const CSwampBossStage1::SCollisionJoint skCollisionJoints[16] = {
    {"head", 5.f, 5.5f, CSwampBossStage1::kCJT_Normal, 0},
    {skTongueLocatorName, 3.2f, 3.2f, CSwampBossStage1::kCJT_Tongue, 0},
    {"L_upper_1", 2.f, 2.f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_NoSortingBounds},
    {"R_upper_1", 2.f, 2.f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_NoSortingBounds},
    {"Spine_2", 5.f, 5.f, CSwampBossStage1::kCJT_Normal, 0},
    {"Spine_3", 5.f, 5.f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_UnusedMarker},
    {"Spine_4", 4.1f, 4.1f, CSwampBossStage1::kCJT_Normal, 0},
    {"Spine_5", 3.7f, 15.f, CSwampBossStage1::kCJT_Normal,
     CSwampBossStage1::kCJF_Passthrough | CSwampBossStage1::kCJF_NoSortingBounds},
    {"Spine_6", 3.f, 15.f, CSwampBossStage1::kCJT_Normal,
     CSwampBossStage1::kCJF_Passthrough | CSwampBossStage1::kCJF_NoSortingBounds},
    {"Spine_7", 2.5f, 2.5f, CSwampBossStage1::kCJT_Normal, 0},
    {"Spine_8", 2.f, 2.f, CSwampBossStage1::kCJT_Normal, 0},
    {"Spine_9", 1.8f, 1.8f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_NoSortingBounds},
    {"Spine_11", 1.6f, 1.6f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_NoSortingBounds},
    {"Spine_13", 1.5f, 1.5f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_NoSortingBounds},
    {"Spine_15", 1.5f, 1.5f, CSwampBossStage1::kCJT_Normal, CSwampBossStage1::kCJF_NoSortingBounds},
    {"L_middle2_boob_LCTR", 2.5f, 0.1f, CSwampBossStage1::kCJT_WeakSpot, 0},
};

CSwampBossStage1::SFsm2::SFsm2(CAssetId stateMachine)
: mToken(gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine))) {}

CSwampBossStage1::SWeakSpot::SWeakSpot(const CDamageVulnerability& vulnerability)
: mStartHealth(-1000.f), mVulnerability(vulnerability) {
  mDamageTaken = 0.f;
  mHeavyHit = false;
}

CSwampBossStage1::CSwampBossStage1(const TUniqueId& uid, const rstl::string& name,
                                   CEntityInfo& info, const CTransform4f& xf,
                                   const CModelData& modelData, const CActorParameters& actorParams,
                                   const CPatternedInfo& patternedInfo, CAssetId stateMachine2,
                                   const SLdrSwampBossStage1Data& swampBossStage1Properties)
: CPatterned(kPAI_SwampBossStage1, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_Flyer, actorParams)
, mProperties(swampBossStage1Properties)
, mPreviousStates()
, mUnusedState(-1)
, mState(kBS_None)
, mElapsedTime(0.f)
, mWaterTime(0.f)
, mRootPosition(CVector3f::Zero())
, mHeadPosition(CVector3f::Zero())
, mTelegraphSfx()
, mPainDamage(10000.f)
, mSortingBounds(CAABox::MakeNullBox())
, mScanned(false)
, mTargetable(true)
, mLastTouchedProjectile(kInvalidUniqueId)
, mShredderState(1)
, mAlertPoint(CVector3f::Zero())
, mAnimationVariant(0)
, mStateMachine2(stateMachine2)
, mNextBobTime(-1000.f)
, mCollisionManager()
, mPhase(0)
, mAttackIndex(0)
, mNextAttackTime(-1000.f)
, mSplashAttack(swampBossStage1Properties.splashShockWave,
                swampBossStage1Properties.preJumpTelegraphEffect)
, mConnectedIndex(-1)
, mTargetIndex(-1)
, mPlatformReadyTime(0.f)
, mMissCount(0)
, mTongueAttempts(0)
, mBeachAttackOver(false)
, mTongue(swampBossStage1Properties.tongueParticleModel, swampBossStage1Properties.tongueTipModel)
, mAdditive()
, mWeakSpot(LdrToDamageVulnerability(mProperties.weakSpotVulnerability).MakeIgnoreRadius())
, mBarf(swampBossStage1Properties.pART)
, mSavedStepUpHeight(-1.f)
, mWaterRing(swampBossStage1Properties.darkWaterRingEffect)
, mWaitStartTime(-1000.f)
, mSpit(mProperties.spitProjectile, LdrToDamageInfo(mProperties.spitDamage),
        mProperties.spitVisorEffect) {
  KnockBackController().EnableFreeze(false);
  KnockBackController().EnableBurn(false);
  KnockBackController().EnableBurnDeath(false);
  KnockBackController().EnableExplodeDeath(false);
  KnockBackController().EnableLaggedBurnDeath(false);
  KnockBackController().EnableKnockBackPhysics(false);
  KnockBackController().EnableSlow(false);
}

CSwampBossStage1::~CSwampBossStage1() {}

pas::ELocomotionType CSwampBossStage1::GetSurfaceLocomotion() const {
  if (mAnimationVariant == 0) {
    return pas::kLT_Relaxed;
  }
  return pas::kLT_Internal14;
}

pas::ELocomotionType CSwampBossStage1::GetSwimLocomotion() const {
  if (mAnimationVariant == 0) {
    return pas::kLT_Internal10;
  }
  return pas::kLT_Internal11;
}

CScannableObjectInfo* CSwampBossStage1::GetScannableObjectInfo() const {
  if (mBodyController->GetCurrentStateId() == pas::kAS_MeleeAttack || !mTargetable) {
    return nullptr;
  }
  return CPatterned::GetScannableObjectInfo();
}

bool CSwampBossStage1::IsDamageableState() const {
  switch (mState) {
  case kBS_Beach:
  case kBS_Exposed:
  case kBS_PlatformReady:
  case kBS_TonguePull:
  case kBS_SlideIntoWater:
  case kBS_TongueLoop:
  case kBS_Vomit:
    return true;
  default:
    return false;
  }
}

void CSwampBossStage1::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    break;
  case kSM_Activate:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetActive(mgr, true);
    }
    mgr.SetBossParams(GetUniqueId(), GetHealthInfo()->GetInitialHP(),
                      gpStringTable->GetStringIndex(skBossName));
    break;
  case kSM_Deactivate:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetActive(mgr, false);
    }
    break;
  case kSM_Delete:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->Destroy(mgr);
      mCollisionManager = nullptr;
    }
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_AreaLoaded:
    mNextAttackTime = mWaterTime;
    if (mgr.IsRandomAvailable() == true) {
      const SLdrSwampBossStage1Struct* phase = GetCurrentPhase();
      if (phase != nullptr) {
        mNextAttackTime +=
            mgr.Random()->Range(phase->minTimeBetweenAttacks, phase->maxTimeBetweenAttacks);
      }
    }
    mNextBobTime = mWaterTime + mgr.Random()->Range(mProperties.unknown_0x27a06f6a,
                                                    mProperties.unknown_0x233a5e40);
    {
      CPlayer* player = mgr.GetPlayer(0);
      mSavedStepUpHeight = player->GetStepUpHeight();
      player->SetStepUpHeight(1.f);
    }
    mWaterRing.mWaterId = FindConnectedObject(mgr, kSS_InternalState8, kSM_None);
    AddMaterial(kMT_ProjectilePassthrough, kMT_ExcludeFromRadar, mgr);
    RemoveMaterial(kMT_Solid, kMT_CollisionActor, kMT_Player, mgr);
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    break;
  case kSM_Damage:
    if (GetAlive()) {
      CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
      if (actor != nullptr) {
        const TUniqueId objectId = actor->GetLastTouchedObject();
        const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(objectId));
        const TUniqueId touchedId = actor->GetLastTouchedObject();
        if (mLastTouchedProjectile == kInvalidUniqueId || touchedId != mLastTouchedProjectile) {
          mLastTouchedProjectile = touchedId;
          if (projectile != nullptr && actor->GetHealthInfo()->GetHP() < 10000.f &&
              !actor->GetMaterialList().HasMaterial(kMT_ProjectilePassthrough)) {
            CDamageInfo damageInfo = projectile->GetCurrentDamageInfo();
            bool canTakeDamage = IsDamageableState();
            const bool weakSpotHit = IsCollisionActorOfType(mgr, actor, kCJT_WeakSpot);
            float damage = damageInfo.GetDamage(*GetDamageVulnerability());
            if (weakSpotHit == true) {
              const float rawDamage = damageInfo.GetDamage();
              damageInfo.SetDamage(rawDamage * mProperties.weakSpotDamageMultiplier);
              damage = damageInfo.GetDamage(mWeakSpot.mVulnerability);
              damageInfo = CDamageInfo(CWeaponMode(kWT_UnknownSource), damage, 0.f, 0.f);
              if (mState == kBS_Exposed) {
                canTakeDamage = true;
                if (rawDamage >= mProperties.unknown_0xee6b6f47) {
                  mWeakSpot.mHeavyHit = true;
                  damage += mProperties.unknown_0x3ce96c9d;
                  const CDamageInfo heavyDamage(CWeaponMode(kWT_UnknownSource),
                                                mProperties.unknown_0x3ce96c9d, 0.f, 0.f);
                  mgr.ApplyDamage(touchedId, GetUniqueId(), touchedId, heavyDamage,
                                  CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial)),
                                  CVector3f::Zero());
                }
              }
            }
            damageInfo.SetRadius(0.f);
            damageInfo.SetRadiusDamage(0.f);
            if (damage > 0.f) {
              mDamageCooldownTimer = skDamageHitTime;
              const CVector3f direction = -1.f * projectile->GetTransform().GetForward();
              const SLdrAudioPlaybackParms* sound = nullptr;
              if (mWeakSpot.mHeavyHit) {
                PlayCustomSound(projectile->GetTranslation(), direction,
                                mProperties.sounds.painSound_OneShot, false);
                sound = &mProperties.sounds.weakSpotHitLarge_OneShot;
              } else if (weakSpotHit == true) {
                PlayCustomSound(projectile->GetTranslation(), direction,
                                mProperties.sounds.painSound_OneShot, false);
                sound = &mProperties.sounds.weakSpotHitSmall_OneShot;
              } else {
                switch (mState) {
                case kBS_None:
                case kBS_Beach:
                case kBS_Bob:
                case kBS_Dive:
                case kBS_PlatformReady:
                case kBS_TonguePull:
                case kBS_TongueLoop:
                  mPainDamage += damage;
                  break;
                default:
                  break;
                }
                if (mPainDamage > mProperties.sounds.painSoundDamageThreshold) {
                  sound = &mProperties.sounds.painSound_OneShot;
                  mPainDamage = 0.f;
                }
              }
              if (sound != nullptr) {
                PlayCustomSound(projectile->GetTranslation(), direction, *sound, false);
              }
            }
            if (!canTakeDamage) {
              if (GetHealthInfo()->GetHP() - damage < 1.f) {
                damageInfo.SetDamage(rstl::max_val(0.f, GetHealthInfo()->GetHP() - 1.f));
              }
            }
            mgr.ApplyDamage(touchedId, GetUniqueId(), touchedId, damageInfo,
                            CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial)),
                            CVector3f::Zero());
            switch (mState) {
            case kBS_TongueLoop:
            case kBS_TonguePull:
            case kBS_Beach:
            case kBS_Vomit:
            case kBS_PlatformReady:
              mWeakSpot.mDamageTaken += damage;
              if (mTongueAttempts < 1) {
                const float maxDamage = 0.5f * mProperties.unknown_0x74e1a041;
                if (mWeakSpot.mDamageTaken > maxDamage) {
                  mWeakSpot.mDamageTaken = maxDamage;
                }
              }
              break;
            default:
              break;
            }
          }
          actor->HealthInfo()->SetHP(10000.f);
        } else {
          return;
        }
      }
    }
    break;
  case kSM_HitObject: {
    const CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
    if (actor != nullptr) {
      const TUniqueId playerId = mgr.GetPlayer(0)->GetUniqueId();
      if (actor->GetLastTouchedObject() == playerId && mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), playerId, GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial)),
                        CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
    break;
  }
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

bool CSwampBossStage1::IsCollisionActorOfType(const CStateManager& mgr,
                                              const CCollisionActor* actor, int type) const {
  for (uint i = 0; i < 16; ++i) {
    const CCollisionActor* candidate = TCastToConstPtr< CCollisionActor >(
        mgr.GetObjectById(mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    if (candidate == actor) {
      return DoesTypeMatch(skCollisionJoints[i].type, type);
    }
  }
  return false;
}

void CSwampBossStage1::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                    CStateManager& mgr) {
  if (mState == kBS_TonguePull) {
    for (int i = 0; i < list.GetCount(); ++i) {
      if (list[i].GetMaterialLeft().HasMaterial(kMT_Player) == true) {
        mTongue.mCollided = true;
      }
    }
  }
  CPatterned::CollidedWith(id, list, mgr);
}

bool CSwampBossStage1::DoesTypeMatch(int jointType, int type) const {
  if (jointType == kCJT_Any || type == kCJT_Any) {
    return true;
  }
  return jointType == type;
}

bool CSwampBossStage1::HasRadarCollisionActor(CStateManager& mgr) const {
  if (mCollisionManager.get() == nullptr || !GetActive()) {
    return false;
  }
  for (uint i = 0; i < 16; ++i) {
    const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(
        mgr.ObjectById(mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    if (actor != nullptr && actor->GetMaterialList().HasMaterial(kMT_RadarObject)) {
      return true;
    }
  }
  return false;
}

void CSwampBossStage1::SetCollisionActorState(CStateManager& mgr, int type, int flags) {
  if (mCollisionManager.get() == nullptr) {
    return;
  }
  for (uint i = 0; i < 16; ++i) {
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(
        mgr.ObjectById(mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    if (actor != nullptr && DoesTypeMatch(skCollisionJoints[i].type, type)) {
      if (skCollisionJoints[i].type == kCJT_WeakSpot && !mScanned) {
        actor->RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
      } else if ((flags & kCF_AddTargetMaterials) != 0) {
        actor->AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
      } else if ((flags & kCF_RemoveTargetMaterials) != 0) {
        actor->RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
      }
      if ((flags & kCF_SetVulnerability) != 0) {
        if (skCollisionJoints[i].type == kCJT_WeakSpot) {
          actor->SetDamageVulnerability(mWeakSpot.mVulnerability);
          actor->SetResponseType(kWCR_SwampBossStage1Reflect);
        } else {
          actor->SetDamageVulnerability(GetDamageVulnerability()->MakeIgnoreRadius());
          actor->SetResponseType(kWCR_Unknown48);
        }
      } else if ((flags & kCF_SetReflect) != 0) {
        actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
        actor->SetResponseType(kWCR_SwampBossStage1Reflect);
      }
      if ((flags & kCF_AddRadarObject) != 0) {
        actor->AddMaterial(kMT_RadarObject, mgr);
      } else if ((flags & kCF_RemoveRadarObject) != 0) {
        actor->RemoveMaterial(kMT_RadarObject, mgr);
      }
    }
  }
}

void CSwampBossStage1::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    mTargetable = false;
  } else if (!mStateMachine->HasState()) {
    SetupStateMachineHelper(mgr);
  } else {
    if (mCollisionManager.get() == nullptr) {
      SetupCollisionManager(mgr);
    }
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    }
    CPatterned::Think(dt, mgr);
    mElapsedTime += dt;
    UpdateEffects(mgr, dt);
    UpdateShockWaveSfx(mgr);
    const SLdrSwampBossStage1Struct* phase = GetCurrentPhase();
    if (phase != nullptr) {
      if (GetHealthInfo()->GetHP() < phase->unknown_0x98106ee2 && mPhase < 2) {
        ChooseNextAttack(mgr);
      }
    }
    mTargetable = HasRadarCollisionActor(mgr);
  }
}

void CSwampBossStage1::AlertNearbyActors(CStateManager& mgr) {
  if (mShredderState != 0) {
    return;
  }
  CVector3f direction = mHeadPosition - mRootPosition;
  direction.SetZ(0.f);
  direction = direction.AsNormalized();
  mAlertPoint = mHeadPosition + 20.f * direction;
  CObjectList& list = mgr.ObjectListById(kOL_PhysicsActor);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CActor* actor = TCastToConstPtr< CActor >(list[i]);
    if (actor != nullptr && actor->GetActive() && actor != this &&
        actor->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CPatterned* patterned = TCastToConstPtr< CPatterned >(actor);
      if (patterned != nullptr) {
        const CVector3f offset = patterned->GetTranslation() - mAlertPoint;
        if (offset.GetX() * offset.GetX() + offset.GetY() * offset.GetY() < 400.f) {
          mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), patterned->GetUniqueId(), kSM_Alert));
        }
      }
    }
  }
}

void CSwampBossStage1::UpdateShockWaveSfx(CStateManager& mgr) {
  if (mSplashAttack.mShockWaveId != kInvalidUniqueId && mSplashAttack.mShockWaveSfx) {
    const CShockWave* shockWave =
        TCastToConstPtr< CShockWave >(mgr.GetObjectById(mSplashAttack.mShockWaveId));
    if (shockWave != nullptr) {
      const CVector3f playerPosition = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
      CVector3f direction = playerPosition - shockWave->GetTranslation();
      if (direction.CanBeNormalized()) {
        if (direction.Magnitude() < shockWave->GetRadius()) {
          CSfxManager::UpdateEmitter(mSplashAttack.mShockWaveSfx, playerPosition, CVector3f::Up(),
                                     127);
          mSplashAttack.mShockWavePosition = playerPosition;
        } else {
          direction.Normalize();
          const CVector3f position =
              shockWave->GetTranslation() + shockWave->GetRadius() * direction;
          CSfxManager::UpdateEmitter(mSplashAttack.mShockWaveSfx, position, direction, 127);
          mSplashAttack.mShockWavePosition = position;
        }
        return;
      }
    }
  }
  if (mSplashAttack.mShockWaveSfx) {
    CSfxManager::RemoveEmitter(mSplashAttack.mShockWaveSfx);
    mSplashAttack.mShockWaveSfx = CSfxHandle();
  }
  mSplashAttack.mShockWaveId = kInvalidUniqueId;
  mSplashAttack.mShockWavePosition = CVector3f::Zero();
}

void CSwampBossStage1::UpdateEffects(CStateManager& mgr, float dt) {
  if (mWaterRing.mEffect.get() != nullptr) {
    if (mWaterRing.mEnabled) {
      const float height = GetWaterSurfaceHeight(mgr);
      mWaterRing.mEffect->SetParticleEmission(true);
      mWaterRing.mDirection.SetZ(0.f);
      if (mWaterRing.mDirection.CanBeNormalized()) {
        mWaterRing.mDirection.Normalize();
        const CTransform4f orientation = CTransform4f::LookAt(
            CVector3f::Zero(),
            CVector3f(mWaterRing.mDirection.GetY(), -mWaterRing.mDirection.GetX(), 0.f),
            CVector3f::Up());
        mWaterRing.mEffect->SetOrientation(orientation.GetRotation());
      }
      mWaterRing.mEffect->SetTranslation(
          CVector3f(mRootPosition.GetX(), mRootPosition.GetY(), height + 0.2f));
    } else {
      mWaterRing.mEffect->SetParticleEmission(false);
    }
    mWaterRing.mEffect->Update(dt * mWaterRing.mSpeed);
  }
  if (mSplashAttack.mTelegraphEffect.get() != nullptr) {
    mSplashAttack.mTelegraphEffect->Update(dt);
  }
}

float CSwampBossStage1::GetWaterSurfaceHeight(const CStateManager& mgr) const {
  if (mWaterRing.mWaterId == kInvalidUniqueId) {
    return -10000.f;
  }
  const CScriptWater* water =
      TCastToConstPtr< CScriptWater >(mgr.GetObjectById(mWaterRing.mWaterId));
  if (water == nullptr) {
    return -10000.f;
  }
  return water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
}

void CSwampBossStage1::BuildCollisionDescriptions(const SCollisionJoint* joints, int count,
                                                  rstl::vector< CJointCollisionDescription >& out) {
  CAnimData* animData = AnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId segId = animData->GetLocatorSegId(rstl::string_l(joints[i].name));
    if (segId != CSegId::Invalid()) {
      CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          segId, CVector3f::Zero(), joints[i].radius, rstl::string_l(joints[i].name), 10000.f);
      out.push_back_unsafe(desc);
    }
  }
}

void CSwampBossStage1::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descriptions;
  descriptions.reserve(16);
  BuildCollisionDescriptions(skCollisionJoints, 16, descriptions);
  mCollisionManager = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(),
                                                    descriptions, GetActive());
  for (uint i = 0; i < 16; ++i) {
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(
        mgr.ObjectById(mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    CHealthInfo* health = actor->HealthInfo();
    health->SetKnockbackResistance(GetHealthInfo()->GetKnockBackResistance());
    health->SetHP(10000.f);
    switch (skCollisionJoints[i].type) {
    case kCJT_Tongue:
      actor->AddMaterial(kMT_RadarObject, mgr);
    case kCJT_Normal:
      actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
      actor->SetResponseType(kWCR_SwampBossStage1Reflect);
      break;
    case kCJT_WeakSpot:
      actor->SetDamageVulnerability(
          LdrToDamageVulnerability(mProperties.weakSpotVulnerability).MakeIgnoreRadius());
      actor->AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
      actor->SetResponseType(kWCR_Unknown78);
      break;
    default:
      break;
    }
    CMaterialFilter filter = actor->GetMaterialFilter();
    filter.ExcludeList().Add(kMT_Character);
    actor->SetMaterialFilter(filter);
  }
}

void CSwampBossStage1::SetCollisionRadii(CStateManager& mgr, int radiusSet) {
  if (mCollisionManager.get() == nullptr) {
    return;
  }
  for (uint i = 0; i < 16; ++i) {
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(
        mgr.ObjectById(mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    if (actor != nullptr) {
      switch (radiusSet) {
      case 0:
        actor->SetSphereRadius(skCollisionJoints[i].radius);
        if ((skCollisionJoints[i].flags & kCJF_Passthrough) != 0) {
          actor->RemoveMaterial(kMT_ProjectilePassthrough, mgr);
        }
        break;
      case 1:
        actor->SetSphereRadius(skCollisionJoints[i].altRadius);
        if ((skCollisionJoints[i].flags & kCJF_Passthrough) != 0) {
          actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
        }
        break;
      }
    }
  }
}

void CSwampBossStage1::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                       EUserEventType type, float dt) {
  switch (type) {
  case kUE_BreakLockOn:
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    if (mState == kBS_Splash) {
      SetCollisionActorState(mgr, kCJT_Tongue, kCF_RemoveRadarObject);
    }
    return;
  case kUE_DamageOn:
    if (mState == kBS_Vomit) {
      BarfVisorGoo(mgr);
    }
    return;
  case kUE_Landing:
    if (mState == kBS_Splash) {
      SpawnSplashShockWave(mgr);
    }
    break;
  case kUE_BeginAction:
    if (mState == kBS_Beach) {
      GrabPlayer(mgr);
    }
    break;
  case kUE_EndAction:
    ReleasePlayer(mgr);
    break;
  case kUE_EventStart:
    if (mState == kBS_TongueLoop && mTongue.mState == kTS_Retracted) {
      ExtendTongue(mgr);
    }
    return;
  case kUE_EventStop:
    RetractTongue();
    return;
  case kUE_EffectOn:
    if (GetAlive() == true) {
      mWaterRing.mEnabled = true;
    }
    break;
  case kUE_EffectOff:
    mWaterRing.mEnabled = false;
    break;
  case kUE_Projectile:
    if (mState == kBS_Splash) {
      LaunchSpit(mgr);
    }
    break;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CSwampBossStage1::LaunchSpit(CStateManager& mgr) {
  const CVector3f aimPosition = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
  const CTransform4f locator = GetLctrTransform(rstl::string_l(skTongueLocatorName));
  const CTransform4f xf =
      CTransform4f::LookAt(locator.GetTranslation(), aimPosition, CVector3f::Up());
  CEnergyProjectile* projectile = LaunchProjectile(
      xf, mgr, 64, CWeapon::kPA_BigProjectile, true,
      mSpit.mVisorEffect ? CImpactVisorEffect::ParticleEffect(mSpit.mVisorEffect,
                                                              mProperties.sound_SpitVisor, false)
                         : CImpactVisorEffect::None(),
      CVector3f(1.f, 1.f, 1.f));
  if (projectile != nullptr) {
    projectile->SetProjExtent(mProperties.spitProjectileRadius);
  }
}

CProjectileInfo* CSwampBossStage1::ProjectileInfo() { return &mSpit.mProjectile; }

CShockWaveInfo CSwampBossStage1::BuildShockWaveInfo() const {
  return CShockWaveInfo(mSplashAttack.mShockWave);
}

void CSwampBossStage1::SpawnSplashShockWave(CStateManager& mgr) {
  SendScriptMsgs(kSS_InternalState14, mgr);
  if (mSplashAttack.mShockWave.shockWaveEffect != kInvalidAssetId) {
    CTransform4f xf = GetTransform();
    xf.SetTranslation(
        CVector3f(mRootPosition.GetX(), mRootPosition.GetY(), GetWaterSurfaceHeight(mgr) - 4.f));
    CShockWave* shockWave =
        rs_new CShockWave(mgr.AllocateUniqueId(), rstl::string_l("Swamp Boss 1 Shockwave"),
                          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), xf,
                          GetUniqueId(), BuildShockWaveInfo(), 2.f, 0.4f);
    if (shockWave != nullptr) {
      mgr.AddObject(shockWave);
      mSplashAttack.mShockWaveId = shockWave->GetUniqueId();
      if (mSplashAttack.mShockWaveSfx) {
        CSfxManager::RemoveEmitter(mSplashAttack.mShockWaveSfx);
        mSplashAttack.mShockWaveSfx = CSfxHandle();
      }
      mSplashAttack.mShockWaveSfx = PlayCustomSound(
          xf.GetTranslation(), CVector3f::Up(), mProperties.sounds.shockWaveVolumetric_Loop, true);
    }
  }
}

void CSwampBossStage1::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (mTongue.mSegmentModel && mTongue.mState != kTS_Retracted) {
    RenderTongue(mgr);
  }
  if (mTongue.mTipModel && mTongue.mExtension > 0.f) {
    const CVector3f direction = mTongue.mTipPosition - mTongue.mRootPosition;
    if (direction.CanBeNormalized()) {
      const CTransform4f orientation =
          CTransform4f::LookAt(CVector3f::Zero(), direction.AsNormalized(), CVector3f::Up());
      CTransform4f xf = GetTransform();
      xf.SetTranslation(mTongue.mTipPosition);
      xf.SetRotation(orientation.GetRotation());
      mTongue.mTipModel->Render(mgr, xf, GetActorLights(), GetModelFlags());
    }
  }
}

void CSwampBossStage1::PreRenderAllViewports(CStateManager& mgr) {
  CPatterned::PreRenderAllViewports(mgr);
  if (mCollisionManager.get() != nullptr) {
    mSortingBounds = CAABox::MakeMaxInvertedBox();
    for (uint i = 0; i < 16; ++i) {
      if ((skCollisionJoints[i].flags & kCJF_NoSortingBounds) == 0) {
        const TUniqueId id = mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
        const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(id));
        if (actor != nullptr) {
          const CAABox bounds = *actor->GetTouchBounds();
          const CVector3f halfExtent = (bounds.GetMaxPoint() - bounds.GetCenterPoint()) * 0.85f;
          const CAABox shrunk(bounds.GetCenterPoint() - halfExtent,
                              bounds.GetCenterPoint() + halfExtent);
          mSortingBounds.AccumulateBounds(shrunk.GetMinPoint());
          mSortingBounds.AccumulateBounds(shrunk.GetMaxPoint());
        }
      }
    }
  }
}

CAABox CSwampBossStage1::GetSortingBounds(const CStateManager& mgr) const { return mSortingBounds; }

CVector3f CSwampBossStage1::GetPlayerAimPoint(const CStateManager& mgr) const {
  const CPlayer* player = mgr.GetPlayer(0);
  CVector3f position = player->GetAimPosition(mgr, 0.f);
  if (player->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    position += CVector3f(0.f, 0.f, -1.5f);
  }
  return position;
}

void CSwampBossStage1::CheckTongueHit(CStateManager& mgr, const CVector3f& tip) {
  if (mTongue.mState != kTS_Extending) {
    return;
  }
  CVector3f direction = tip - mTongue.mRootPosition;
  if (!direction.CanBeNormalized()) {
    return;
  }
  const float length = 5.f + direction.Magnitude();
  direction.Normalize();
  static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(PlayerMaterial));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, mTongue.mRootPosition, direction, length, filter, nullptr);
  CVector3f side1(direction.GetY(), -direction.GetX(), 0.f);
  CVector3f side2(-direction.GetY(), direction.GetX(), 0.f);
  if (side1.CanBeNormalized()) {
    side1.Normalize();
  }
  if (side2.CanBeNormalized()) {
    side2.Normalize();
  }
  side1 = side1 * 0.7f;
  side2 = side2 * 0.7f;
  const CVector3f origins[3] = {mTongue.mRootPosition, mTongue.mRootPosition + side1,
                                mTongue.mRootPosition + side2};
  for (int i = 0; i < 3; ++i) {
    TUniqueId hitId = kInvalidUniqueId;
    const CRayCastResult result = CGameCollision::RayDynamicIntersection(
        mgr, hitId, origins[i], direction, length, filter, nearList);
    if (hitId == mgr.GetPlayer(0)->GetUniqueId()) {
      mTongue.mState = kTS_Hit;
      PlayCustomSound(mTongue.mRootPosition, direction, mProperties.sounds.tongueHitPlayer_OneShot,
                      false);
      mTongue.mPullSfx = PlayCustomSound(mTongue.mRootPosition, direction,
                                         mProperties.sounds.tonguePullPlayer_Loop, true);
      return;
    }
  }
}

void CSwampBossStage1::UpdateTongueExtend(CStateManager& mgr, float dt) {
  mTongue.mExtension += dt / 0.4f;
  mTongue.mExtension = CMath::Min(mTongue.mExtension, 1.f);
  if (mTongue.mExtension < 0.6f) {
    mTongue.mTargetPosition = GetPlayerAimPoint(mgr);
  }
}

void CSwampBossStage1::UpdateTongueRetract(CStateManager& mgr, float dt) {
  mTongue.mExtension -= dt / 0.25f;
  mTongue.mExtension = CMath::Max(mTongue.mExtension, 0.f);
  if (mTongue.mExtension < 0.1f) {
    RetractTongue();
  } else if (!mTongue.mRetractSfx) {
    CVector3f direction = mTongue.mTargetPosition - mTongue.mRootPosition;
    direction.SetZ(0.f);
    if (direction.CanBeNormalized()) {
      direction.Normalize();
    }
    mTongue.mRetractSfx = PlayCustomSound(mTongue.mRootPosition, direction,
                                          mProperties.sounds.tongueRetract_Loop, true);
  }
}

void CSwampBossStage1::FadeInAdditive(float dt) {
  mAdditive.mBlend += dt / 0.3f;
  mAdditive.mBlend = CMath::Min(mAdditive.mBlend, 1.f);
}

void CSwampBossStage1::FadeOutAdditive(float dt) {
  mAdditive.mBlend -= dt / 0.2f;
  mAdditive.mBlend = CMath::Max(mAdditive.mBlend, 0.f);
}

void CSwampBossStage1::UpdateTongueTip(CStateManager& mgr, bool checkHit, float amplitude) {
  if (mTongue.mState != kTS_Retracted) {
    const float extension = CMath::Max(mTongue.mExtension, 0.01f);
    const CVector3f tip =
        mTongue.mRootPosition * (1.f - extension) + mTongue.mTargetPosition * extension;
    mTongue.mTipPosition = tip;
    mTongue.mWaveAmplitude = amplitude;
    if (checkHit) {
      CheckTongueHit(mgr, tip);
    }
  }
}

void CSwampBossStage1::RenderTongue(const CStateManager& mgr) const {
  const CColor startColor = CColor::White();
  const CColor endColor = CColor::Lerp(CColor::White(), CColor::Red(), 0.8f);
  const float extension = CMath::Max(mTongue.mExtension, 0.01f);
  const CVector3f toTip =
      (mTongue.mRootPosition * (1.f - extension) + mTongue.mTargetPosition * extension) -
      mTongue.mRootPosition;
  const CTransform4f orientation = CTransform4f::LookAt(CVector3f::Zero(), toTip, CVector3f::Up());
  const float length = CMath::Min(toTip.Magnitude(), 100.f);
  const float phase = 6.2831855f * (2.f * mElapsedTime);
  for (int i = 0; i < 256; ++i) {
    const float t = i - 0.5f;
    const float taper = (1.f - 0.00390625f * t) * 0.4f;
    const float angle = phase + 0.8f * (0.18f * t - 46.08f);
    const float sideWave = mTongue.mWaveAmplitude * (taper * CMath::FastSinR(angle));
    const float upWave = mTongue.mWaveAmplitude * (0.2f * (taper * CMath::FastSinR(1.f + angle)));
    const CVector3f offset = orientation * CVector3f(sideWave, length * t / 256.f, upWave);
    CTransform4f xf = GetTransform();
    xf.SetTranslation(mTongue.mRootPosition + offset);
    xf.SetRotation(orientation.GetRotation());
    const float s = t / 256.f;
    float blend = 1.f - (1.f - s) * (1.f - s);
    blend = CMath::Clamp(0.f, blend, 1.f);
    CColor color = CColor::Lerp(startColor, endColor, blend);
    color.SetAlpha(blend);
    if (s < 0.4f) {
      const float scale = 1.f - s / 0.4f;
      xf.ScaleBy(4.f * (scale * scale) + 1.f);
    }
    mTongue.mSegmentModel->Render(
        mgr, xf, GetActorLights(),
        CModelFlags(CModelFlags::kT_Blend, 0,
                    CModelFlags::EFlags(CModelFlags::kF_DepthCompare | CModelFlags::kF_DepthUpdate),
                    color));
  }
}

void CSwampBossStage1::ExtendTongue(CStateManager& mgr) {
  StopTongueSounds();
  mTongue.mTargetPosition = GetPlayerAimPoint(mgr);
  mTongue.mState = kTS_Extending;
  CVector3f direction = mTongue.mTargetPosition - mTongue.mRootPosition;
  direction.SetZ(0.f);
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  }
  PlayCustomSound(mTongue.mRootPosition, direction, mProperties.sounds.tongueOut_OneShot, false);
}

void CSwampBossStage1::RetractTongue() {
  if (mState != kBS_TonguePull && mState != kBS_TongueLoop) {
    mTongue.mState = kTS_Retracted;
    StopTongueSounds();
  }
}

void CSwampBossStage1::StopTongueSounds() {
  if (mTongue.mRetractSfx) {
    CSfxManager::RemoveEmitter(mTongue.mRetractSfx);
  }
  if (mTongue.mPullSfx) {
    CSfxManager::RemoveEmitter(mTongue.mPullSfx);
    mTongue.mPullSfx = CSfxHandle();
  }
}

void CSwampBossStage1::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  mRootPosition = GetLctrTransform(rstl::string_l("Skeleton_Root")).GetTranslation();
  mTongue.mRootPosition = GetLctrTransform(rstl::string_l(skTongueLocatorName)).GetTranslation();
  mHeadPosition = GetLctrTransform(rstl::string_l("head")).GetTranslation();
  const CVector3f spinePosition = GetLctrTransform(rstl::string_l("Spine_5")).GetTranslation();
  mWaterRing.mDirection = mHeadPosition - spinePosition;
}

const CDamageVulnerability* CSwampBossStage1::GetDamageVulnerability() const {
  return CPatterned::GetDamageVulnerability();
}

CVector3f CSwampBossStage1::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

const CGenericFSM2* CSwampBossStage1::GetStateMachine2() const {
  if (!mStateMachine2.mToken->IsLoaded()) {
    return nullptr;
  }
  CToken token = *mStateMachine2.mToken;
  return static_cast< const CGenericFSM2* >(token.GetObj()->GetContents());
}

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"BeachAttackOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::BeachAttackOver)},
    {"EnoughSplashing",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::EnoughSplashing)},
    {"GivingUp", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::GivingUp)},
    {"HeavyHit", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::HeavyHit)},
    {"InVomitRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::InVomitRange)},
    {"LightHit", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::LightHit)},
    {"MinHealth",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::MinHealth)},
    {"ReadyToTongue",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::ReadyToTongue)},
    {"ShouldBeach",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::ShouldBeach)},
    {"ShouldBob",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::ShouldBob)},
    {"ShouldExposeBelly",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::ShouldExposeBelly)},
    {"ShouldSplash",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::ShouldSplash)},
    {"SplashAttackOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::SplashAttackOver)},
    {"TongueHit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::TongueHit)},
    {"TongueMissed",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::TongueMissed)},
    {"TooMuchPulling",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::TooMuchPulling)},
    {"Wait", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSwampBossStage1::Wait)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Beach", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Beach)},
    {"BeachBubbles",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::BeachBubbles)},
    {"BeachRise", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::BeachRise)},
    {"BeachWait", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::BeachWait)},
    {"Bob", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Bob)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Dead)},
    {"Dive", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Dive)},
    {"ExposeBelly",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::ExposeBelly)},
    {"HeavyHitReact",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::HeavyHitReact)},
    {"LightHitReact",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::LightHitReact)},
    {"Null", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Null)},
    {"PlatformReady",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::PlatformReady)},
    {"ResetPlatform",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::ResetPlatform)},
    {"SetSplashOver",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SetSplashOver)},
    {"SlideIntoWater",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SlideIntoWater)},
    {"SplashDescend",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SplashDescend)},
    {"SplashJump",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SplashJump)},
    {"SplashRise",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SplashRise)},
    {"SplashTelegraph",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SplashTelegraph)},
    {"SplashWait",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::SplashWait)},
    {"StopBeaching",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::StopBeaching)},
    {"Swim", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Swim)},
    {"TongueLoop",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::TongueLoop)},
    {"TonguePull",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::TonguePull)},
    {"Vomit", static_cast< CPatterned::StateMachine::StateFunc >(&CSwampBossStage1::Vomit)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"IncrementMisses",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSwampBossStage1::IncrementMisses)},
    {"SetSplashRotation",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSwampBossStage1::SetSplashRotation)},
    {"ShreddersOn",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSwampBossStage1::ShreddersOn)},
    {"ShreddersOff",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CSwampBossStage1::ShreddersOff)},
};

void CSwampBossStage1::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  mStateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  mStateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

bool CSwampBossStage1::ReadyToTongue(CStateManager& mgr, const CTriggerData& data) const {
  if (mMissCount >= 3) {
    return false;
  }
  if (mPlatformReadyTime < 0.65f) {
    return false;
  }
  if (GetPlayerAngle(mgr) > 85.f) {
    return false;
  }
  return IsPlayerInsideTongueHint(mgr);
}

float CSwampBossStage1::GetPlayerAngle(const CStateManager& mgr) const {
  const CPlayer* player = mgr.GetPlayer(0);
  CVector3f toPlayer = player->GetTranslation() - mTongue.mRootPosition;
  toPlayer.SetZ(0.f);
  CVector3f toBoss = GetTranslation() - mTongue.mRootPosition;
  toBoss.SetZ(0.f);
  if (!toPlayer.CanBeNormalized() || !toBoss.CanBeNormalized()) {
    return 360.f;
  }
  return CMath::Rad2Deg(CVector3f::GetAngleDiff(toPlayer.AsNormalized(), toBoss.AsNormalized()));
}

bool CSwampBossStage1::TongueHit(CStateManager& mgr, const CTriggerData& data) const {
  return mTongue.mState == kTS_Hit;
}

bool CSwampBossStage1::TongueMissed(CStateManager& mgr, const CTriggerData& data) const {
  if (GetPlayerAngle(mgr) > 85.f) {
    return true;
  }
  if (!IsPlayerInsideTongueHint(mgr)) {
    return true;
  }
  return mTongue.mExtension >= 1.f && mTongue.mState != kTS_Hit;
}

bool CSwampBossStage1::GivingUp(CStateManager& mgr, const CTriggerData& data) const {
  if (mPlatformReadyTime > 8.f) {
    return true;
  }
  if (mMissCount < 3) {
    return false;
  }
  return mPlatformReadyTime > 0.5f;
}

bool CSwampBossStage1::HeavyHit(CStateManager& mgr, const CTriggerData& data) const {
  return mWeakSpot.mHeavyHit;
}

bool CSwampBossStage1::LightHit(CStateManager& mgr, const CTriggerData& data) const {
  return GetHealthInfo()->GetHP() <= mWeakSpot.mStartHealth - mProperties.unknown_0x1f4e7c2c;
}

bool CSwampBossStage1::MinHealth(CStateManager& mgr, const CTriggerData& data) const {
  return GetHealthInfo()->GetHP() <= 1.01f;
}

bool CSwampBossStage1::BeachAttackOver(CStateManager& mgr, const CTriggerData& data) const {
  return mBeachAttackOver;
}

bool CSwampBossStage1::ShouldBob(CStateManager& mgr, const CTriggerData& data) const {
  if (GetHealthInfo()->GetHP() <= 1.f) {
    return false;
  }
  return mNextBobTime < mWaterTime;
}

bool CSwampBossStage1::ShouldExposeBelly(CStateManager& mgr, const CTriggerData& data) const {
  if (MinHealth(mgr, CTriggerData(0.f))) {
    return mWeakSpot.mDamageTaken > mProperties.unknown_0x74e1a041;
  }
  return mWeakSpot.mDamageTaken > mProperties.unknown_0x78755da3;
}

bool CSwampBossStage1::ShouldSplash(CStateManager& mgr, const CTriggerData& data) const {
  if (GetHealthInfo()->GetHP() <= 1.f) {
    return false;
  }
  if (mNextAttackTime > mWaterTime) {
    return false;
  }
  return GetCurrentAttack() == kAttack_Splash;
}

bool CSwampBossStage1::ShouldBeach(CStateManager& mgr, const CTriggerData& data) const {
  if (MinHealth(mgr, CTriggerData(0.f))) {
    return true;
  }
  if (mNextAttackTime > mWaterTime) {
    return false;
  }
  return GetCurrentAttack() == kAttack_Beach;
}

bool CSwampBossStage1::InVomitRange(CStateManager& mgr, const CTriggerData& data) const {
  if (mTongue.mCollided) {
    return true;
  }
  CVector3f toPlayer = mTongue.mRootPosition - GetPlayerAimPoint(mgr);
  const CPlayer* player = mgr.GetPlayer(0);
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    toPlayer.SetZ(0.f);
    return toPlayer.Magnitude() < 6.f;
  }
  return toPlayer.Magnitude() < 8.5f;
}

void CSwampBossStage1::Swim(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(GetSurfaceLocomotion());
    SetCollisionRadii(mgr, 0);
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddRadarObject);
    break;
  case kStateMsg_Update:
    mWaterTime += dt;
    AlertNearbyActors(mgr);
    break;
  }
}

void CSwampBossStage1::Bob(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Bob, msg);
  switch (msg) {
  case kStateMsg_Activate:
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_SetVulnerability);
    mNextBobTime = mWaterTime + mgr.Random()->Range(mProperties.unknown_0x27a06f6a,
                                                    mProperties.unknown_0x233a5e40);
    break;
  case kStateMsg_Update:
    AlertNearbyActors(mgr);
    break;
  case kStateMsg_Deactivate:
    SetCollisionActorState(mgr, kCJT_Normal, kCF_RemoveTargetMaterials | kCF_SetReflect);
    break;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Zero));
}

void CSwampBossStage1::SplashDescend(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Splash, msg);
  switch (msg) {
  case kStateMsg_Activate:
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    mSplashAttack.mSplashCount = 0;
    mSplashAttack.mRotation = mgr.Random()->Range(0.f, 360.f);
    mSplashAttack.mSplashOver = false;
    mSplashAttack.mStartPhase = mPhase;
    mSplashAttack.mTargetSplashCount = mgr.Random()->Range(GetCurrentPhase()->unknown_0xbb0ffdd6,
                                                           GetCurrentPhase()->unknown_0x60b0ae31);
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_SetVulnerability);
    break;
  case kStateMsg_Update:
    BodyController()->SetLocomotionType(GetSwimLocomotion());
    break;
  case kStateMsg_Deactivate:
    SetCollisionActorState(mgr, kCJT_Normal, kCF_SetReflect);
    break;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eleven));
}

bool CSwampBossStage1::EnoughSplashing(CStateManager& mgr, const CTriggerData& data) const {
  if (MinHealth(mgr, CTriggerData(0.f))) {
    return true;
  }
  if (mPhase != mSplashAttack.mStartPhase) {
    return true;
  }
  return mSplashAttack.mTargetSplashCount <= mSplashAttack.mSplashCount;
}

void CSwampBossStage1::SetSplashOver(CStateManager& mgr, EStateMsg msg, float dt) {
  SplashWait(mgr, msg, dt);
  mSplashAttack.mSplashOver = true;
  if (msg == kStateMsg_Activate) {
    BodyController()->SetLocomotionType(GetSwimLocomotion());
    if (mPhase == mSplashAttack.mStartPhase) {
      ++mAttackIndex;
      ChooseNextAttack(mgr);
    }
  }
}

void CSwampBossStage1::RandomizeSwimVariant(CStateManager& mgr) {
  if (mgr.Random()->Range(0.f, 1.f) < 0.7f) {
    if (mAnimationVariant == 0) {
      mAnimationVariant = 1;
    } else {
      mAnimationVariant = 0;
    }
  }
  BodyController()->SetLocomotionType(GetSwimLocomotion());
}

bool CSwampBossStage1::SplashAttackOver(CStateManager& mgr, const CTriggerData& data) const {
  return mSplashAttack.mSplashOver;
}

void CSwampBossStage1::SetSplashRotation(CStateManager& mgr, float dt) {
  SetTransform(CQuaternion::ZRotation(CRelAngle::FromDegrees(mSplashAttack.mRotation))
                   .BuildTransform4f(GetTranslation()));
  mSplashAttack.mRotation += mgr.Random()->Range(140.f, 220.f);
  RandomizeSwimVariant(mgr);
}

void CSwampBossStage1::SplashJump(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Splash, msg);
  if (mScanned) {
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_RemoveTargetMaterials);
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_AddTargetMaterials);
  } else {
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddTargetMaterials);
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_RemoveTargetMaterials);
  }
  if (msg == kStateMsg_Activate) {
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
    if (mTelegraphSfx) {
      CSfxManager::RemoveEmitter(mTelegraphSfx);
      mTelegraphSfx = CSfxHandle();
    }
    ++mSplashAttack.mSplashCount;
    mWaterRing.mSpeed = 2.f;
  } else if (msg == kStateMsg_Deactivate) {
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    mWaterRing.mSpeed = 1.f;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Seven));
}

void CSwampBossStage1::SplashRise(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Splash, msg);
  if (msg == kStateMsg_Activate) {
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    ChooseNextAttack(mgr);
  } else {
    BodyController()->SetLocomotionType(GetSurfaceLocomotion());
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eight));
}

void CSwampBossStage1::BeachRise(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Splash, msg);
  if (msg == kStateMsg_Activate) {
    BodyController()->SetLocomotionType(GetSurfaceLocomotion());
    SetCollisionActorState(mgr, kCJT_Any,
                           kCF_RemoveTargetMaterials | kCF_SetReflect | kCF_RemoveRadarObject);
    ChooseNextAttack(mgr);
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eight));
}

void CSwampBossStage1::SlideIntoWater(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_SlideIntoWater, msg);
  if (msg == kStateMsg_Activate) {
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    RumblePlayer(mgr);
  }
  if (msg == kStateMsg_Activate) {
    RetractTongue();
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Three));
}

void CSwampBossStage1::ChooseNextAttack(CStateManager& mgr) {
  if (mAttackIndex >= 4 || GetCurrentAttack() == kAttack_Restart) {
    mAttackIndex = 0;
  }
  const SLdrSwampBossStage1Struct* phase = GetCurrentPhase();
  while (GetHealthInfo()->GetHP() < phase->unknown_0x98106ee2 && mPhase < 2) {
    ++mPhase;
    mAttackIndex = 0;
    phase = GetCurrentPhase();
  }
  mNextAttackTime = mWaterTime;
  if (mgr.IsRandomAvailable()) {
    mNextAttackTime +=
        mgr.Random()->Range(phase->minTimeBetweenAttacks, phase->maxTimeBetweenAttacks);
  }
}

static void PushPreviousState(rstl::reserved_vector< CSwampBossStage1::EBossState, 4 >& states,
                              CSwampBossStage1::EBossState state) {
  if (states.size() == 3) {
    states.erase(states.begin());
  }
  states.push_back(state);
}

void CSwampBossStage1::UpdateState(EBossState state, EStateMsg msg) {
  if (msg == kStateMsg_Deactivate) {
    PushPreviousState(mPreviousStates, mState);
    mState = kBS_None;
  } else {
    mState = state;
  }
}

const SLdrSwampBossStage1Struct* CSwampBossStage1::GetCurrentPhase() const {
  switch (mPhase) {
  case 0:
    return &mProperties.swampBossStage1Struct;
  case 1:
    return &mProperties.swampBossStage1Struct_0x3e1e7597;
  case 2:
    return &mProperties.swampBossStage1Struct_0xa1c4f609;
  default:
    return &mProperties.swampBossStage1Struct_0xa1c4f609;
  }
}

int CSwampBossStage1::GetCurrentAttack() const {
  const SLdrSwampBossStage1Struct* phase = GetCurrentPhase();
  switch (mAttackIndex) {
  case 0:
    return phase->firstAttack;
  case 1:
    return phase->secondAttack;
  case 2:
    return phase->thirdAttack;
  case 3:
    return phase->fourthAttack;
  }
  return phase->firstAttack;
}

void CSwampBossStage1::Dive(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mBeachAttackOver = false;
    mWeakSpot.mDamageTaken = 0.f;
    mWeakSpot.mHeavyHit = false;
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_SetVulnerability);
  } else if (msg == kStateMsg_Update) {
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
  }
  UpdateState(kBS_Dive, msg);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eleven));
}

void CSwampBossStage1::Null(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->SetLocomotionType(pas::kLT_Internal7);
  if (msg == kStateMsg_Activate) {
    mWaitStartTime = mElapsedTime;
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_RemoveRadarObject);
  }
}

void CSwampBossStage1::BeachWait(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->SetLocomotionType(pas::kLT_Internal7);
  if (msg == kStateMsg_Activate) {
    mWaitStartTime = mElapsedTime;
    mWeakSpot.mDamageTaken = 0.f;
    mWeakSpot.mHeavyHit = false;
    ChooseTargetIndex(mgr);
    RotateToIndex();
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveRadarObject);
  }
}

void CSwampBossStage1::BeachBubbles(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->SetLocomotionType(pas::kLT_Internal7);
  if (msg == kStateMsg_Activate) {
    mWaitStartTime = mElapsedTime;
    CVector3f toRoot = mRootPosition - GetTranslation();
    toRoot.SetZ(0.f);
    StartTelegraph(mgr, GetTranslation() + toRoot * 2.f);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddRadarObject);
    RumblePlayer(mgr);
  }
}

void CSwampBossStage1::RumblePlayer(CStateManager& mgr) {
  mgr.RumbleManager(mgr.MaskUIdNumPlayers(mgr.GetPlayer(0)->GetUniqueId()))
      ->Rumble(mgr, kRFX_TwentyThree, 1.f, kRP_Two);
}

void CSwampBossStage1::SplashTelegraph(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddRadarObject);
    mWaitStartTime = mElapsedTime;
    mWaterRing.mEnabled = false;
    StartTelegraph(mgr, mRootPosition);
    RumblePlayer(mgr);
  }
  UpdateState(kBS_SplashTelegraph, msg);
}

void CSwampBossStage1::StartTelegraph(CStateManager& mgr, const CVector3f& position) {
  const CVector3f surfacePosition(position.GetX(), position.GetY(), GetWaterSurfaceHeight(mgr));
  if (mSplashAttack.mTelegraphEffectId != kInvalidAssetId) {
    mSplashAttack.mTelegraphEffect = rstl::auto_ptr< CParticleGen >(rs_new CElementGen(
        gpSimplePool->GetObj(SObjectTag('PART', mSplashAttack.mTelegraphEffectId)),
        CElementGen::kMOT_Normal, CElementGen::kOSF_One));
    mSplashAttack.mTelegraphEffect->SetParticleEmission(true);
    mSplashAttack.mTelegraphEffect->SetTranslation(surfacePosition);
  }
  if (mTelegraphSfx) {
    CSfxManager::RemoveEmitter(mTelegraphSfx);
    mTelegraphSfx = CSfxHandle();
  }
  mTelegraphSfx =
      PlayCustomSound(surfacePosition, CVector3f::Up(), mProperties.sounds.telegraph_Loop, true);
}

void CSwampBossStage1::SplashWait(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_None, msg);
  mWaterRing.mEnabled = false;
  if (msg == kStateMsg_Activate) {
    mWaitStartTime = mElapsedTime;
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_RemoveRadarObject);
    const CPASAnimParmData parms(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(6),
                                 CPASAnimParm::FromEnum(GetSwimLocomotion()),
                                 CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(),
                                 CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter());
    BodyController()->CommandMgr().Reset();
    BodyController()->LoopBestAnimation(parms, *mgr.Random());
  }
}

bool CSwampBossStage1::Wait(CStateManager& mgr, const CTriggerData& data) const {
  return mElapsedTime > mWaitStartTime + data.GetFloat();
}

void CSwampBossStage1::StopBeaching(CStateManager& mgr, EStateMsg msg, float dt) {
  BodyController()->SetLocomotionType(pas::kLT_Internal7);
  mBeachAttackOver = true;
  if (msg == kStateMsg_Activate && mPhase == mBeachStartPhase) {
    ++mAttackIndex;
    ChooseNextAttack(mgr);
  }
}

void CSwampBossStage1::Beach(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Beach, msg);
  if (msg == kStateMsg_Activate) {
    mAdditive = SAdditive();
    UpdateTongueAdditive(mgr, 1000.f);
    mMissCount = 0;
    mTongueAttempts = 0;
    mBeachStartPhase = mPhase;
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddTargetMaterials | kCF_SetVulnerability);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    SetCollisionRadii(mgr, 1);
    ChooseNextAttack(mgr);
    RumblePlayer(mgr);
    if (mTelegraphSfx) {
      CSfxManager::RemoveEmitter(mTelegraphSfx);
      mTelegraphSfx = CSfxHandle();
    }
  } else if (msg == kStateMsg_Deactivate) {
    SetCollisionRadii(mgr, 0);
  }
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Six));
}

void CSwampBossStage1::PlatformReady(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_PlatformReady, msg);
  if (msg == kStateMsg_Activate) {
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    mPlatformReadyTime = 0.f;
    mTongue.mCollided = false;
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddTargetMaterials | kCF_SetVulnerability);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
  } else {
    mPlatformReadyTime += dt;
    UpdateTongueRetract(mgr, dt);
    UpdateTongueTip(mgr, false, 1.f);
    FadeInAdditive(dt);
    UpdateTongueAdditive(mgr, dt);
  }
}

void CSwampBossStage1::SelectOrbitTarget(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->GetOrbitTargetId() == kInvalidUniqueId) {
    for (uint i = 0; i < 16; ++i) {
      if (DoesTypeMatch(skCollisionJoints[i].type, kCJT_Tongue)) {
        const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(
            mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
        if (actor != nullptr) {
          player->SetOrbitState(CPlayer::kOS_ForcedOrbitObject, mgr);
          player->SetOrbitTargetId(actor->GetUniqueId(), mgr);
          break;
        }
      }
    }
  }
}

void CSwampBossStage1::ClearPlayerOrbit(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->GetOrbitState() == CPlayer::kOS_ForcedOrbitObject) {
    player->SetOrbitState(CPlayer::kOS_NoOrbit, mgr);
  }
}

void CSwampBossStage1::TonguePull(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_TonguePull, msg);
  CPlayer* player = mgr.GetPlayer(0);
  switch (msg) {
  case kStateMsg_Activate:
    mTongue.mPullTime = 0.f;
    mTongue.mAmplitudeScale = 1.f;
    mTongue.mCollided = false;
    mTongue.mExtension = 1.f;
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddTargetMaterials | kCF_SetVulnerability);
    BeginTonguePull(mgr);
    BodyController()->SetLocomotionType(pas::kLT_Internal6);
    mTongue.mRetractDelay = 0.f;
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      HideOrbit(mgr);
    }
    SelectOrbitTarget(mgr);
    break;
  case kStateMsg_Update: {
    mTongue.mPullTime += dt;
    mTongue.mAmplitudeScale -= dt / 0.3f;
    mTongue.mAmplitudeScale = CMath::Max(mTongue.mAmplitudeScale, 0.1f);
    const CVector3f aimPoint = GetPlayerAimPoint(mgr);
    CVector3f direction = mTongue.mRootPosition - aimPoint;
    if (direction.CanBeNormalized()) {
      direction.Normalize();
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        direction = direction * (7.5f * dt);
        PullPlayerMorphed(mgr, player->GetTranslation() + direction, dt);
      } else {
        float speed = 7.5f;
        if (player->GetBackwardInput() > 0.1f) {
          speed *= 0.4f;
        }
        direction = direction * (dt * speed);
        PullPlayer(mgr, player->GetTranslation() + direction, dt);
      }
    }
    mTongue.mTargetPosition = aimPoint;
    UpdateTongueTip(mgr, false, mTongue.mAmplitudeScale);
    UpdateTongueAdditive(mgr, dt);
    break;
  }
  case kStateMsg_Deactivate:
    player->EnableLeaveMorphBall(true);
    player->GetMorphBall()->SetBoostEnabled(true);
    player->Stop();
    ShowOrbit(mgr);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mTongue.mRetractDelay = 1.2f;
    ClearPlayerOrbit(mgr);
    break;
  }
}

bool CSwampBossStage1::TooMuchPulling(CStateManager& mgr, const CTriggerData& data) const {
  return mTongue.mPullTime > 8.f;
}

void CSwampBossStage1::Vomit(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Vomit, msg);
  BodyController()->SetLocomotionType(pas::kLT_Combat);
  UpdateTongueRetract(mgr, dt);
  UpdateTongueTip(mgr, false, 1.f);
  FadeOutAdditive(dt);
  UpdateTongueAdditive(mgr, dt);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_One));
  switch (msg) {
  case kStateMsg_Activate:
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddTargetMaterials | kCF_SetVulnerability);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
    mBarf.mCount = 0;
    if (mTongue.mRetractDelay <= 0.f) {
      DecrementAttachedActors(mgr);
    }
    RumblePlayer(mgr);
    StopTongueSounds();
    break;
  case kStateMsg_Update:
    if (mTongue.mRetractDelay > 0.f) {
      mTongue.mRetractDelay -= dt;
      if (mTongue.mRetractDelay < 0.f) {
        DecrementAttachedActors(mgr);
        mTongue.mRetractDelay = 0.f;
      }
    }
    break;
  case kStateMsg_Deactivate:
    DecrementAttachedActors(mgr);
    mTongue.mRetractDelay = 0.f;
    RetractTongue();
    break;
  }
}

void CSwampBossStage1::BarfVisorGoo(CStateManager& mgr) {
  if (mBarf.mCount > 0 || GetPlayerAngle(mgr) > 100.f) {
    return;
  }
  const CVector3f toPlayer = mTongue.mRootPosition - GetPlayerAimPoint(mgr);
  if (toPlayer.Magnitude() > 10.5f) {
    return;
  }

  CDamageInfo damage = LdrToDamageInfo(mProperties.damageInfo);
  damage.SetKnockBackPower(0.f);
  damage.SetRadiusDamage(0.f);
  mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), damage,
                  CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial)), CVector3f::Zero());
  if (mBarf.mCount == 0) {
    const float nearClip = CHUDBillboardEffect::GetNearClipDistance(mgr, 0);
    const CVector3f scale = CHUDBillboardEffect::GetScaleForPOV(mgr);
    CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
        mBarf.mEffect, rstl::optional_object< TToken< CElectricDescription > >(),
        mgr.AllocateUniqueId(), true, rstl::string_l("Swamp Boss 1 barf visor goo"), nearClip,
        scale, 0, CColor::White(), CVector3f::One(), CVector3f::Zero(), false);
    mgr.AddObject(effect);
  }
  ++mBarf.mCount;
  if (toPlayer.CanBeNormalized()) {
    CVector3f direction = toPlayer.AsNormalized();
    direction *= -12.f;
    direction.SetZ(12.f);
    LaunchPlayer(mgr, direction);
  }
}

void CSwampBossStage1::DecrementAttachedActors(CStateManager& mgr) {
  SendMessageToAttachedActors(mgr, kSM_Decrement);
}

void CSwampBossStage1::BeginTonguePull(CStateManager& mgr) {
  SendMessageToAttachedActors(mgr, kSM_Increment);
  mgr.GetPlayer(0)->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
}

void CSwampBossStage1::SendMessageToAttachedActors(CStateManager& mgr, EScriptObjectMessage msg) {
  rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
  for (; it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Connect && it->msg == kSM_Attach) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(it->objId)))) {
        actor->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), actor->GetUniqueId(), msg));
      }
    }
  }
}

void CSwampBossStage1::PullPlayer(CStateManager& mgr, const CVector3f& position, float dt) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->IsOnGround()) {
    player->Stop();
    CVector3f forward = player->GetTransform().GetForward();
    forward.SetZ(0.f);
    if (forward.CanBeNormalized()) {
      forward.Normalize();
      CTransform4f xf = CTransform4f::LookAt(CVector3f::Zero(), forward, CVector3f::Up());
      xf.SetTranslation(position);
      player->SetTransform(xf);
    }
  } else {
    player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }

  const CRelAngle angle = CRelAngle::FromDegrees(220.f * dt);
  const CVector3f aimPosition = player->GetAimPosition(mgr, 0.f);
  const CVector3f toTongue = mTongue.mRootPosition - aimPosition;
  if (toTongue.CanBeNormalized()) {
    const CVector3f normalized = toTongue.AsNormalized();
    const CQuaternion rotation = CQuaternion::LookAt(
        CUnitVector3f(player->GetTransform().GetForward(), CUnitVector3f::kN_No),
        CUnitVector3f(normalized, CUnitVector3f::kN_No), angle);
    const CQuaternion localRotation = CQuaternion::ScalarVector(
        rotation.GetScalar(), player->GetTransform().TransposeRotate(rotation.GetVector()));
    player->RotateInOneFrameOR(localRotation, dt);
  }
}

void CSwampBossStage1::PullPlayerMorphed(CStateManager& mgr, const CVector3f& position, float dt) {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f direction = position - player->GetTranslation();
  direction.SetZ(0.f);
  if (direction.CanBeNormalized()) {
    direction.Normalize();
    player->SetVelocityWR(direction * 30.f);
    const float gravityForce = player->GetMass() * player->GetGravity();
    player->SetConstantForceWR(CVector3f(0.f, 0.f, gravityForce));
    player->SetMoveState(NPlayer::kMS_FallingMorphed, mgr);
  }
}

void CSwampBossStage1::HideOrbit(CStateManager& mgr) {
  if (mTongue.mOrbitHidden != true) {
    mTongue.mOrbitHidden = true;
    SendScriptMsgs(kSS_InternalState10, mgr, GetUniqueId(), kSM_None);
  }
}

void CSwampBossStage1::ShowOrbit(CStateManager& mgr) {
  if (mTongue.mOrbitHidden) {
    mTongue.mOrbitHidden = false;
    SendScriptMsgs(kSS_InternalState11, mgr, GetUniqueId(), kSM_None);
  }
}

void CSwampBossStage1::ResetPlatform(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    ReleasePlayer(mgr);
    mWeakSpot.mHeavyHit = false;
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
    SetCollisionActorState(mgr, kCJT_Any,
                           kCF_RemoveTargetMaterials | kCF_SetReflect | kCF_RemoveRadarObject);
  }
}

void CSwampBossStage1::ExposeBelly(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Exposed, msg);
  BodyController()->SetLocomotionType(pas::kLT_Internal5);
  switch (msg) {
  case kStateMsg_Activate:
    mWeakSpot.mStartHealth = GetHealthInfo()->GetHP();
    DecrementAttachedActors(mgr);
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_AddTargetMaterials | kCF_SetVulnerability);
    StopTongueSounds();
    break;
  case kStateMsg_Update:
    UpdateTongueRetract(mgr, dt);
    UpdateTongueTip(mgr, false, 1.f);
    FadeOutAdditive(dt);
    UpdateTongueAdditive(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    RetractTongue();
    break;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Nine));
}

void CSwampBossStage1::UpdateHitReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_Exposed, msg);
  BodyController()->SetLocomotionType(pas::kLT_Internal5);
  if (msg == kStateMsg_Update) {
    UpdateTongueRetract(mgr, dt);
    UpdateTongueTip(mgr, false, 1.f);
    FadeOutAdditive(dt);
    UpdateTongueAdditive(mgr, dt);
  }
}

void CSwampBossStage1::HeavyHitReact(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    SetCollisionActorState(mgr, kCJT_Any, kCF_RemoveTargetMaterials | kCF_SetReflect);
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
  }
  UpdateHitReaction(mgr, msg, dt);
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Four));
}

void CSwampBossStage1::LightHitReact(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateHitReaction(mgr, msg, dt);
  if (msg == kStateMsg_Activate) {
    BodyController()->CommandMgr().DeliverCmd(CBCAdditiveReactionCmd(pas::kART_Five, 1.f, false));
  }
}

void CSwampBossStage1::TongueLoop(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateState(kBS_TongueLoop, msg);
  BodyController()->SetLocomotionType(pas::kLT_Combat);
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().ClearLocomotionCmds();
    mTongue.Reset();
    ++mTongueAttempts;
    SetCollisionActorState(mgr, kCJT_WeakSpot, kCF_RemoveTargetMaterials | kCF_SetReflect);
    SetCollisionActorState(mgr, kCJT_Normal, kCF_RemoveTargetMaterials | kCF_SetVulnerability);
    SetCollisionActorState(mgr, kCJT_Tongue, kCF_AddTargetMaterials | kCF_SetVulnerability);
    break;
  case kStateMsg_Update:
    if (mTongue.mState != kTS_Retracted) {
      UpdateTongueExtend(mgr, dt);
      UpdateTongueTip(mgr, true, 1.f);
    }
    FadeInAdditive(dt);
    UpdateTongueAdditive(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

CVector3f CSwampBossStage1::GetAdditiveAimPoint(CStateManager& mgr, float dt) {
  const CVector3f aimPoint = GetPlayerAimPoint(mgr);
  if (mTongue.mState != kTS_Retracted) {
    mAdditive.mTimer = 0.f;
    if (mAdditive.mStartPending) {
      mAdditive.mStart = aimPoint;
      mAdditive.mStartPending = false;
    }
    return mAdditive.mStart;
  }

  mAdditive.mStartPending = true;
  mAdditive.mTimer += dt;
  if (mAdditive.mTimer > 1.f) {
    return aimPoint;
  }
  return mAdditive.mStart * (1.f - mAdditive.mTimer) + aimPoint * mAdditive.mTimer;
}

void CSwampBossStage1::UpdateTongueAdditive(CStateManager& mgr, float dt) {
  if (mAdditive.mBlend <= 0.f) {
    AddAdditiveAnimation(mgr, 0, 0.f);
    AddAdditiveAnimation(mgr, 1, 0.f);
  } else {
    CVector3f toBoss = GetTranslation() - mTongue.mRootPosition;
    toBoss.SetZ(0.f);
    if (toBoss.CanBeNormalized()) {
      const CVector3f aimPoint = GetAdditiveAimPoint(mgr, dt);
      toBoss.Normalize();
      CVector3f toAim = aimPoint - mTongue.mRootPosition;
      toAim.SetZ(0.f);
      if (toAim.CanBeNormalized()) {
        toAim.Normalize();
        float angle = CVector3f::GetAngleDiff(toBoss, toAim) / (25.f * (M_PIF / 180.f));
        angle = CMath::Min(angle, 1.f);
        const CVector3f cross = CVector3f::Cross(toBoss, toAim);
        int primary;
        int secondary;
        if (cross.GetZ() > 0.f) {
          primary = 0;
          secondary = 1;
        } else {
          primary = 1;
          secondary = 0;
        }
        AddAdditiveAnimation(mgr, primary,
                             angle * CMath::EaseInOut(mAdditive.mBlend, CMath::kET_Sinusoidal,
                                                      0.25f, 0.75f, 0.f, 1.f, 2.f));
        AddAdditiveAnimation(mgr, secondary, 0.f);
      }
    }
  }
}

void CSwampBossStage1::AddAdditiveAnimation(CStateManager& mgr, int index, float weight) {
  if (mgr.IsRandomAvailable()) {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(index),
                                 CPASAnimParm::FromEnum(0));
    const rstl::pair< float, int > best =
        BodyController()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    AnimationData()->AddAdditiveAnimation(best.second, weight, false, false);
  }
}

void CSwampBossStage1::LaunchPlayer(CStateManager& mgr, const CVector3f& direction) {
  CPlayer* player = mgr.GetPlayer(0);
  player->Stop();
  player->ApplyImpulseWR(player->GetMass() * direction, CAxisAngle::Identity());
  player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  mgr.RumbleManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
      ->Rumble(mgr, kRFX_TwentyThree, 1.f, kRP_Two);
}

void CSwampBossStage1::ChooseTargetIndex(CStateManager& mgr) {
  if (!mgr.IsRandomAvailable()) {
    mTargetIndex = 0;
  } else {
    const int indices[4] = {0, 1, 2, 3};
    mTargetIndex = indices[mgr.Random()->Range(0, 3)];
  }
}

void CSwampBossStage1::RotateToIndex() {
  float angle = 0.f;
  switch (mTargetIndex) {
  case 1:
    angle = 180.f;
    break;
  case 2:
    angle = 90.f;
    break;
  case 3:
    angle = 270.f;
    break;
  case 0:
    break;
  }
  SetTransform(
      CQuaternion::ZRotation(CRelAngle::FromDegrees(angle)).BuildTransform4f(GetTranslation()));
}

EScriptObjectState CSwampBossStage1::GetGrabState(int index) const {
  switch (index) {
  case 0:
    return kSS_InternalState1;
  case 1:
    return kSS_InternalState0;
  case 2:
    return kSS_InternalState2;
  case 3:
    return kSS_InternalState3;
  default:
    return kSS_InternalState0;
  }
}

EScriptObjectState CSwampBossStage1::GetReleaseState(int index) const {
  switch (index) {
  case 0:
    return kSS_InternalState5;
  case 1:
    return kSS_InternalState4;
  case 2:
    return kSS_InternalState6;
  case 3:
    return kSS_InternalState7;
  default:
    return kSS_InternalState0;
  }
}

void CSwampBossStage1::GrabPlayer(CStateManager& mgr) {
  if (mConnectedIndex == -1) {
    const TUniqueId id = CheckConnectedObject(mgr, GetGrabState(mTargetIndex), kSM_None);
    if (id != kInvalidUniqueId) {
      mgr.SendScriptMsg(CScriptMsg(GetUniqueId(), id, kSM_SetToZero));
      SendScriptMsgs(kSS_InternalState12, mgr);
      mConnectedIndex = mTargetIndex;
      LaunchPlayer(mgr, CVector3f(0.f, 0.f, 30.f));
    }
  }
}

void CSwampBossStage1::ReleasePlayer(CStateManager& mgr) {
  if (mConnectedIndex != -1) {
    const TUniqueId id = CheckConnectedObject(mgr, GetReleaseState(mConnectedIndex), kSM_None);
    if (id != kInvalidUniqueId) {
      mgr.SendScriptMsg(CScriptMsg(GetUniqueId(), id, kSM_SetToZero));
      SendScriptMsgs(kSS_InternalState13, mgr);
      mConnectedIndex = -1;
      LaunchPlayer(mgr, CVector3f(0.f, 0.f, 30.f));
    }
  }
}

void CSwampBossStage1::IncrementMisses(CStateManager& mgr, float dt) {
  ++mMissCount;
  mTongue.mState = kTS_Missed;
}

void CSwampBossStage1::ShreddersOn(CStateManager& mgr, float dt) {
  if (mShredderState != 0) {
    SendScriptMsgs(kSS_AILogicState1, mgr);
    mShredderState = 0;
  }
}

void CSwampBossStage1::ShreddersOff(CStateManager& mgr, float dt) {
  if (mShredderState != 1) {
    SendScriptMsgs(kSS_AILogicState2, mgr);
    mShredderState = 1;
  }
}

void CSwampBossStage1::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    DecrementAttachedActors(mgr);
    if (mSavedStepUpHeight >= 0.f) {
      mgr.GetPlayer(0)->SetStepUpHeight(mSavedStepUpHeight);
    }
    if (mWaterRing.mEffect.get() != nullptr) {
      mWaterRing.mEffect->SetParticleEmission(false);
      mWaterRing.mEffect = rstl::auto_ptr< CElementGen >();
    }
    if (mSplashAttack.mTelegraphEffect.get() != nullptr) {
      mSplashAttack.mTelegraphEffect->SetParticleEmission(false);
      mSplashAttack.mTelegraphEffect = rstl::auto_ptr< CParticleGen >();
    }
    if (mTelegraphSfx) {
      CSfxManager::RemoveEmitter(mTelegraphSfx);
      mTelegraphSfx = CSfxHandle();
    }
    if (mSplashAttack.mShockWaveSfx) {
      CSfxManager::RemoveEmitter(mSplashAttack.mShockWaveSfx);
      mSplashAttack.mShockWaveSfx = CSfxHandle();
    }
    StopTongueSounds();
    SendScriptMsgs(kSS_Dead, mgr, kInvalidUniqueId, kSM_None);
  }
  CPatterned::Dead(mgr, msg, dt);
}

void CSwampBossStage1::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
  if (GetAlive()) {
    if (mWaterRing.mEffect.get() != nullptr) {
      gpRender->AddParticleGen(*mWaterRing.mEffect);
    }
    if (mSplashAttack.mTelegraphEffect.get() != nullptr) {
      gpRender->AddParticleGen(*mSplashAttack.mTelegraphEffect);
    }
    if (mState == kBS_TongueLoop || mState == kBS_TonguePull) {
      EnsureRendered(mgr);
    }
  }
}

void CSwampBossStage1::OnScanStateChange(EScanState state, CStateManager& mgr) {
  if (state == kSS_Done) {
    mScanned = true;
  }
  CActor::OnScanStateChange(state, mgr);
}

CVector3f CSwampBossStage1::GetOrbitPosition(const CStateManager& mgr) const {
  return GetLctrTransform(rstl::string_l("head")).GetTranslation();
}

void CSwampBossStage1::TakeDamage(const CVector3f& direction, float magnitude) {
  mDamageCooldownTimer = skDamageHitTime;
  if (!IsDamageableState()) {
    if (GetHealthInfo()->GetKnockBackResistance() < 1.f) {
      HealthInfo()->SetKnockbackResistance(1.f);
    }
  }
}

bool CSwampBossStage1::IsPlayerInsideTongueHint(CStateManager& mgr) const {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetHintType() == CScriptAIHint::kHT_SwampBossTongue) {
      CVector3f offset = mgr.GetPlayer(0)->GetTranslation() - hint->GetTranslation();
      offset.SetZ(0.f);
      return offset.Magnitude() < hint->GetRadius();
    }
  }
  return true;
}

CSwampBossStage1::SWaterRing::SWaterRing(CAssetId effect)
: mEffect(effect != kInvalidAssetId
              ? rs_new CElementGen(gpSimplePool->GetObj(SObjectTag('PART', effect)),
                                   CElementGen::kMOT_Normal, CElementGen::kOSF_One)
              : nullptr)
, mWaterId(kInvalidUniqueId)
, mEnabled(false)
, mDirection(CVector3f::Zero())
, mSpeed(1.f) {}

CSwampBossStage1::SWaterRing::~SWaterRing() {}

CSwampBossStage1::SSplashAttack::SSplashAttack(const SLdrShockWaveInfo& shockWave,
                                               CAssetId telegraphEffect)
: mShockWave(shockWave)
, mTelegraphEffect()
, mSplashCount(0)
, mRotation(0.f)
, mTelegraphEffectId(telegraphEffect)
, mSplashOver(false)
, mShockWaveId(kInvalidUniqueId)
, mShockWaveSfx()
, mShockWavePosition(CVector3f::Zero())
, mStartPhase(0) {}

CSwampBossStage1::SSplashAttack::~SSplashAttack() {}

CSwampBossStage1::STongue::STongue(CAssetId segmentModel, CAssetId tipModel)
: mRootPosition(CVector3f::Zero())
, mTargetPosition(CVector3f::Zero())
, mTipPosition(CVector3f::Zero())
, mWaveAmplitude(1.f)
, mSegmentModel(CModelData(CStaticRes(segmentModel, CVector3f::One())))
, mTipModel(CModelData(CStaticRes(tipModel, CVector3f::One())))
, mOrbitHidden(false)
, mCollided(false)
, mRetractSfx()
, mPullSfx() {
  Reset();
}

void CSwampBossStage1::STongue::Reset() {
  mRetractDelay = 0.f;
  mAmplitudeScale = 0.f;
  mPullTime = 0.f;
  mExtension = 0.f;
  mState = kTS_Retracted;
  mTipPosition = CVector3f::Zero();
  mTargetPosition = mTipPosition;
}

CSwampBossStage1::STongue::~STongue() {}

CSwampBossStage1::SBarf::SBarf(CAssetId effect)
: mEffect(effect != kInvalidAssetId
              ? rstl::optional_object< TToken< CGenDescription > >(
                    TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', effect))))
              : rstl::optional_object< TToken< CGenDescription > >())
, mCount(0) {}

CSwampBossStage1::SBarf::~SBarf() {}

CSwampBossStage1::SSpit::SSpit(CAssetId projectile, const CDamageInfo& damage, CAssetId visorEffect)
: mProjectile(projectile, damage), mVisorEffect() {
  mProjectile.Token().Lock();
  if (visorEffect != kInvalidAssetId) {
    mVisorEffect =
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', visorEffect)));
  }
}

CEntity* LoadSwampBossStage1(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSwampBossStage1 sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage1.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CSwampBossStage1(
      TUniqueId(mgr.AllocateUniqueId()), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.patterned.stateMachine2,
      sldrThis.swampBossStage1Properties);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SSwampBossStage1_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadSwampBossStage1;
  SetSSwampBossStage1_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSwampBossStage1_FuncPtrs(nullptr); }
#endif

template < typename T >
void CSwampBossStage1::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBodyController->CommandMgr().DeliverCmd(cmd);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, state)) {
      mBodyController->CommandMgr().DeliverCmd(cmd);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}
