#include "MetroidPrime/ScriptObjects/CScriptPlayerTurret.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerTurret.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"
#include <math.h>

CEntity* LoadPlayerTurret(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

static EMaterialTypes skHullMaterial = kMT_CameraPassthrough;
static rstl::string skMuzzleLocator = rstl::string_l("muzzleend_LCTR");
static CColor skDamageFlashColor = CColor(0.5f, 0.f, 0.f, 1.f);

CScriptPlayerTurret::CScriptPlayerTurret(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    uint flags, float horizontalRotationLeft, float horizontalRotationRight,
    float verticalElevationUp, float verticalElevationDown, float damageAngle,
    float horizontalSpeed, float verticalSpeed, float fireRate, const CDamageInfo& weaponDamage,
    CAssetId weaponEffect, CAssetId weaponEffectMultiPlayer, ushort sfxRotation,
    ushort sfxSinglePlayerImpact, ushort sfxMultiPlayerImpact, ushort sfxSinglePlayerProjectile,
    ushort sfxMultiPlayerProjectile)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(skHullMaterial),
         CActorParameters::None(), kInvalidUniqueId)
, mBounds(GetTranslation() - CVector3f(0.5f, 0.5f, 0.5f),
          GetTranslation() + CVector3f(0.5f, 0.5f, 0.5f))
, mFlags(flags)
, mHorizRotationLeft(horizontalRotationLeft)
, mHorizRotationRight(horizontalRotationRight)
, mVertElevationUp(verticalElevationUp)
, mVertElevationDown(verticalElevationDown)
, mMaxAimAngle(damageAngle)
, mPlayerId(kInvalidUniqueId)
, mHorizSpeed(horizontalSpeed)
, mVertSpeed(verticalSpeed)
, mFireRate(fireRate)
, mBaseId(kInvalidUniqueId)
, mHullId(kInvalidUniqueId)
, mAimUpAnim(0)
, mAimDownAnim(0)
, mFireAnim(0)
, mDeathAnim(0)
, mTargetElevation(0.f)
, mElevation(0.f)
, mFireTimer(0.f)
, mRotationSfxTimer(0.f)
, mAlignment(0.f)
, mLastForward(xf.GetForward())
, mHorizInput(0.f)
, mBaseModelFlags(CModelFlags::Normal())
, mWeaponEffect(weaponEffect)
, mWeaponEffectMultiPlayer(weaponEffectMultiPlayer)
, mWeaponToken(nullptr)
, mDamageInfo(weaponDamage)
, mDamageFlashTimer(0.f)
, mTargetPosition(xf.GetTranslation())
, mRotationSfxHandle()
, mDamagerId(kInvalidUniqueId)
, mSfxRotation(sfxRotation)
, mSfxSinglePlayerImpact(sfxSinglePlayerImpact)
, mSfxMultiPlayerImpact(sfxMultiPlayerImpact)
, mSfxSinglePlayerProjectile(sfxSinglePlayerProjectile)
, mSfxMultiPlayerProjectile(sfxMultiPlayerProjectile) {
  mPlayerInTurret = false;
  x22c_25_ = false;
  mDamageInfo.SetDamageSfxId(0x2812);
}

CScriptPlayerTurret::~CScriptPlayerTurret() {}

rstl::optional_object< CAABox > CScriptPlayerTurret::GetTouchBounds() const {
  return rstl::optional_object< CAABox >(mBounds);
}

void CScriptPlayerTurret::PreRender(CStateManager& mgr) {
  if (CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId))) {
    CModelFlags flags(mBaseModelFlags);
    if (mPlayerId != kInvalidUniqueId &&
        mgr.GetCurrentRenderPlayerIndex() == mgr.MaskUIdNumPlayers(mPlayerId)) {
      const CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
      if (player != nullptr && player->GetTurretState() != CPlayer::kTS_Exiting) {
        flags = CModelFlags::AlphaBlended(0.25f).DepthCompareUpdate(true, false);
      }
      if (mDamageFlashTimer > 0.f) {
        flags = CModelFlags::AlphaBlended(CColor(1.f, 0.f, 0.f, 0.25f));
      }
    } else if (mDamageFlashTimer > 0.f) {
      const float t = CMath::Clamp(0.f, mDamageFlashTimer / 0.33f, 1.f);
      flags =
          CModelFlags(CModelFlags::kT_Two, CColor::Lerp(CColor::Black(), skDamageFlashColor, t));
    }
    base->SetModelFlags(flags);
    if (CScriptActor* scriptActor = TCastToPtr< CScriptActor >(base)) {
      scriptActor->SetRenderImmediately(flags == mBaseModelFlags);
    }
  }
}

void CScriptPlayerTurret::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const float horizInput = mHorizInput;
  if (mDamageFlashTimer > 0.f) {
    mDamageFlashTimer -= dt;
  }
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
  CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId));
  if (base != nullptr) {
    const CQuaternion rotation =
        CQuaternion::ZRotation(CRelAngle::FromRadians(dt * (mHorizInput * mHorizSpeed)));
    const CVector2f baseForward2d = base->GetTransform().GetForward().ToVec2f().AsNormalized();
    const CVector3f flatBaseForward(baseForward2d, 0.f);
    CVector3f heading = flatBaseForward;
    if (!close_enough(mHorizInput, 0.f)) {
      heading = rotation.Transform(flatBaseForward);
    }
    const CVector2f forward2d = GetTransform().GetForward().ToVec2f().AsNormalized();
    const CVector3f flatForward(forward2d, 0.f);
    if (player != nullptr && player->GetTurretState() != CPlayer::kTS_Active) {
      const CRelAngle step = CRelAngle::FromRadians(4.f * (dt * mHorizSpeed));
      const CQuaternion snap =
          CQuaternion::LookAt(CUnitVector3f(flatBaseForward), CUnitVector3f(flatForward), step);
      heading = snap.Transform(flatBaseForward);
    }
    const float dot = CVector3f::Dot(heading, flatForward);
    if (CMath::AbsF(dot) >= 0.0001f) {
      if (CVector3f::Cross(heading, flatForward).GetZ() >= 0.f) {
        if (acos(dot) > mHorizRotationRight) {
          heading = base->GetTransform().GetForward();
        }
      } else {
        if (acos(dot) > mHorizRotationLeft) {
          heading = base->GetTransform().GetForward();
        }
      }
      const CVector3f position = base->GetTranslation();
      if (heading.IsMagnitudeSafe()) {
        base->SetTransform(CTransform4f::LookAt(position, position + heading, CVector3f::Up()));
      }
    }
    mHorizInput = 0.f;
  }

  float vertStep = dt * mVertSpeed;
  bool blend = true;
  if (player != nullptr && player->GetTurretState() != CPlayer::kTS_Active) {
    blend = false;
    vertStep *= 4.f;
    mTargetElevation = 0.f;
  }
  const float diff = mTargetElevation - mElevation;
  if (CMath::AbsF(diff) >= 0.0017453292f) {
    float scale =
        1.f - CMath::PhongBlob(CMath::Clamp(0.f, CMath::AbsF(diff / 1.5707964f), 1.f), 8.f);
    if (!blend) {
      scale = 1.f;
    }
    const float scaledStep = vertStep * scale;
    if (diff > 0.f) {
      mElevation = CMath::Clamp(-3.1415927f, mElevation + scaledStep, mTargetElevation);
    } else {
      mElevation = CMath::Clamp(mTargetElevation, mElevation - scaledStep, 3.1415927f);
    }
  } else {
    mElevation = mTargetElevation;
  }
  mElevation = CMath::Clamp(-mVertElevationDown, mElevation, mVertElevationUp);
  if (base != nullptr && base->GetAnimationData() != nullptr) {
    if (mElevation >= 0.f) {
      base->AnimationData()->AddAdditiveAnimation(
          mAimUpAnim, CMath::Clamp(0.f, mElevation / 1.5707964f, 1.f), false, false);
      base->AnimationData()->AddAdditiveAnimation(mAimDownAnim, 0.f, false, false);
    } else {
      base->AnimationData()->AddAdditiveAnimation(
          mAimDownAnim, CMath::Clamp(0.f, -mElevation / 1.5707964f, 1.f), false, false);
      base->AnimationData()->AddAdditiveAnimation(mAimUpAnim, 0.f, false, false);
    }
  }

  CActor* hull = TCastToPtr< CActor >(mgr.ObjectById(mHullId));
  if (base != nullptr && hull != nullptr) {
    hull->SetTransform(base->GetTransform() * base->GetModelData()->GetLocatorTransform(
                                                  rstl::string_l("upwardballsnap_LCTR")));
  }

  if (player != nullptr && player->GetTurretState() == CPlayer::kTS_Active) {
    const float threshold = 1.f - 60.f * (0.00001001358f * dt);
    if (hull != nullptr) {
      const float previousAlignment = mAlignment;
      const CVector3f forward = hull->GetTransform().GetForward();
      mAlignment = CMath::Limit(CVector3f::Dot(forward, mLastForward), 1.f);
      if (close_enough(mTargetElevation, 0.f, 0.05f) && CMath::AbsF(horizInput) <= dt) {
        mAlignment = 1.f;
      }
      mLastForward = forward;
      const bool crossed = (previousAlignment >= threshold && mAlignment < threshold) ||
                           (previousAlignment < threshold && mAlignment >= threshold);
      if (crossed) {
        mRotationSfxTimer = 0.f;
      } else if (mRotationSfxTimer < 0.05f) {
        mRotationSfxTimer += dt;
        if (mRotationSfxTimer >= 0.05f) {
          mRotationSfxTimer = 0.05f;
        }
        if (mRotationSfxTimer == 0.05f) {
          if (mAlignment >= threshold) {
            CSfxManager::SfxStop(mRotationSfxHandle);
            mRotationSfxHandle = CSfxHandle();
          } else if (!mRotationSfxHandle) {
            mRotationSfxHandle = CSfxManager::SfxStart(
                mSfxRotation, 127, player->GetSoundPan(CPlayer::kMSP_Player),
                GetCurrentAreaId().Value(), true, true, CSfxManager::kMedPriority);
          }
        }
      }
      if (mRotationSfxHandle) {
        CSfxManager::PitchBend(mRotationSfxHandle,
                               static_cast< int >(4000.f * (mElevation / 1.5707964f) + 8192.f));
      }
    }
  } else if (mRotationSfxHandle) {
    CSfxManager::SfxStop(mRotationSfxHandle);
  }
}

void CScriptPlayerTurret::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  const TUniqueId originator = msg.GetOriginator();
  switch (msg.GetMessage()) {
  case kSM_Create:
    if (mgr.IsMultiplayer()) {
      mWeaponToken = gpSimplePool->GetObj(
          SObjectTag(gpResourceFactory->GetResourceTypeById(mWeaponEffectMultiPlayer),
                     mWeaponEffectMultiPlayer));
    } else {
      mWeaponToken = gpSimplePool->GetObj(
          SObjectTag(gpResourceFactory->GetResourceTypeById(mWeaponEffect), mWeaponEffect));
    }
    break;
  case kSM_Activate: {
    mPlayerId = originator;
    if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId))) {
      if (player->GetHealthInfo()->GetHP() <= 0.f) {
        return;
      }
      player->StartTurret(GetUniqueId(), mgr);
      mPlayerInTurret = true;
    }
    mElevation = 0.f;
    mTargetElevation = 0.f;
    CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId));
    CActor* hull = TCastToPtr< CActor >(mgr.ObjectById(mHullId));
    if (base != nullptr && base->GetAnimationData() != nullptr && hull != nullptr) {
      const CVector3f position = base->GetTranslation();
      base->SetTransform(GetTransform());
      base->SetTranslation(position);
      base->AnimationData()->AddAdditiveAnimation(mAimUpAnim, 0.f, false, false);
      base->AnimationData()->AddAdditiveAnimation(mAimDownAnim, 0.f, false, false);
      hull->SetTransform(base->GetTransform() * base->GetModelData()->GetLocatorTransform(
                                                    rstl::string_l("upwardballsnap_LCTR")));
    }
    mDamageFlashTimer = 0.f;
    break;
  }
  case kSM_Deactivate:
    if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId))) {
      player->SetTurretState(CPlayer::kTS_None, mgr);
    }
    mPlayerId = kInvalidUniqueId;
    SetActive(false);
    break;
  case kSM_Start:
    if (GetActive()) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId))) {
        player->SetTurretState(CPlayer::kTS_Active, mgr);
      }
      EnableHullCollision(mgr);
      AddMaterial(kMT_Orbit, mgr);
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      RemoveMaterial(kMT_Orbit, mgr);
      if (CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId))) {
        base->SetModelFlags(mBaseModelFlags);
      }
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId))) {
        player->EjectFromTurret(GetUniqueId(), mgr);
        mgr.KillPlayer(player->GetPlayerState()->GetHealthInfo().GetHP(), mPlayerId, mDamagerId);
        mPlayerInTurret = false;
        mPlayerId = kInvalidUniqueId;
        CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId));
        CActor* hull = TCastToPtr< CActor >(mgr.ObjectById(mHullId));
        mElevation = 0.f;
        mTargetElevation = 0.f;
        if (base != nullptr && hull != nullptr) {
          const CVector3f position = base->GetTranslation();
          base->SetTransform(GetTransform());
          base->SetTranslation(position);
          base->AnimationData()->AddAdditiveAnimation(mAimUpAnim, 0.f, false, false);
          base->AnimationData()->AddAdditiveAnimation(mAimDownAnim, 0.f, false, false);
          base->AnimationData()->EnableLooping(false);
          base->AnimationData()->SetAnimation(CAnimPlaybackParms(mIdleAnim, -1, 1.f, true), true);
        }
      }
      SetActive(false);
    }
    break;
  case kSM_AreaLoaded:
    AttachToActors(mgr);
    break;
  case kSM_Damage:
    mDamageFlashTimer = 0.15f;
    if (mgr.IsMultiplayer()) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId))) {
        CSfxManager::SfxStart(mSfxMultiPlayerImpact, 127, player->GetSoundPan(CPlayer::kMSP_Player),
                              CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
      }
    } else {
      CSfxManager::AddEmitter(mSfxSinglePlayerImpact, mTargetPosition, GetCurrentAreaId().Value(),
                              true, false, CSfxManager::kMedPriority);
    }
    if (const CActor* damager = TCastToConstPtr< CActor >(mgr.GetObjectById(sender))) {
      if (damager->GetHealthInfo() != nullptr) {
        mDamagerId = damager->GetHealthInfo()->GetLastDamageOwner();
      } else {
        mDamagerId = originator;
      }
    }
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptPlayerTurret::ExitTurret(CStateManager& mgr) {
  mPlayerInTurret = false;
  SendScriptMsgs(kSS_Exited, mgr);
  DisableHullCollision(mgr);
}

void CScriptPlayerTurret::AttachToActors(CStateManager& mgr) {
  mBaseId = FindConnectedObject(mgr, kSS_AttachedAnimatedObject, kSM_Attach);
  if (CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId))) {
    if (base->GetAnimationData() != nullptr) {
      const CPASDatabase& pasDatabase = base->AnimationData()->GetPASDatabase();
      mAimUpAnim =
          pasDatabase
              .FindBestAnimation(CPASAnimParmData(pas::kAS_Getup, CPASAnimParm::FromEnum(0)), -1)
              .second;
      mAimDownAnim =
          pasDatabase
              .FindBestAnimation(CPASAnimParmData(pas::kAS_Getup, CPASAnimParm::FromEnum(1)), -1)
              .second;
      mFireAnim = pasDatabase.FindBestAnimation(CPASAnimParmData(pas::kAS_Step), -1).second;
      mDeathAnim = pasDatabase.FindBestAnimation(CPASAnimParmData(pas::kAS_Death), -1).second;
      mIdleAnim = pasDatabase.FindBestAnimation(CPASAnimParmData(pas::kAS_Locomotion), -1).second;
      mBaseModelFlags = base->GetModelFlags();
    }
  }
  mHullId = FindConnectedObject(mgr, kSS_AttachedCollisionObject, kSM_Attach);
}

CTransform4f CScriptPlayerTurret::GetMuzzleTransform(CStateManager& mgr) const {
  CTransform4f xf = GetTransform();
  if (const CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId))) {
    xf = base->GetTransform() * base->GetModelData()->GetScaledLocatorTransform(skMuzzleLocator);
  }
  return xf;
}

void CScriptPlayerTurret::EnableHullCollision(CStateManager& mgr) {
  if (CActor* hull = TCastToPtr< CActor >(mgr.ObjectById(mHullId))) {
    hull->AddMaterial(kMT_Solid, mgr);
  }
}

void CScriptPlayerTurret::DisableHullCollision(CStateManager& mgr) {
  if (CActor* hull = TCastToPtr< CActor >(mgr.ObjectById(mHullId))) {
    hull->RemoveMaterial(kMT_Solid, mgr);
  }
}

void CScriptPlayerTurret::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
  if (player == nullptr) {
    return;
  }
  CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId));
  if (base == nullptr) {
    return;
  }
  const float dt = input.DeltaTime();
  if (player->GetTurretState() == CPlayer::kTS_Active) {
    if (mFireTimer > 0.f) {
      mFireTimer -= dt;
      if (base->GetAnimationData() != nullptr) {
        if (base->AnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")) <= 0.f) {
          base->AnimationData()->EnableLooping(true);
          base->AnimationData()->SetAnimation(CAnimPlaybackParms(mDeathAnim, -1, 1.f, true), false);
        }
      }
    }
    if (player->FireBeamPressed(input) && mFireTimer <= 0.f) {
      Fire(mgr);
      mFireTimer = mFireRate;
    }
    const float turnRight =
        player->GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnRight, input);
    const float turnLeft =
        player->GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnLeft, input);
    mHorizInput = CMath::Limit(turnLeft - turnRight, 1.f);
  }
  const float forward =
      player->GetControlMapper().GetAnalogInput(CControlMapper::kC_Forward, input);
  const float backward =
      player->GetControlMapper().GetAnalogInput(CControlMapper::kC_Backward, input);
  float vertInput = CMath::Limit(backward - forward, 1.f);
  if (gpGameState->GameOptions().GetInvertYAxis()) {
    const float invertedBackward =
        player->GetControlMapper().GetAnalogInput(CControlMapper::kC_Backward, input);
    const float invertedForward =
        player->GetControlMapper().GetAnalogInput(CControlMapper::kC_Forward, input);
    vertInput = CMath::Limit(invertedForward - invertedBackward, 1.f);
  }
  if (vertInput < 0.f) {
    mTargetElevation = vertInput * mVertElevationUp;
  } else {
    mTargetElevation = vertInput * mVertElevationDown;
  }
}

CTransform4f CScriptPlayerTurret::GetCameraTransform(CStateManager& mgr) {
  const CTransform4f xf = GetTurretTransform(mgr);
  return CTransform4f::LookAt(xf.GetTranslation(),
                              xf.GetTranslation() + CVector3f(xf.GetForward().ToVec2f(), 0.f),
                              CVector3f::Up());
}

CTransform4f CScriptPlayerTurret::GetTurretTransform(CStateManager& mgr) {
  CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId));
  if (base == nullptr) {
    return GetTransform();
  }
  const CTransform4f locatorXf =
      base->GetModelData()->GetLocatorTransform(rstl::string_l("upwardballsnap_LCTR"));
  return base->GetTransform() * locatorXf;
}

void CScriptPlayerTurret::Fire(CStateManager& mgr) {
  const CTransform4f xf = GetMuzzleTransform(mgr);
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
  if (player != nullptr && mWeaponToken.IsLoaded()) {
    CEnergyProjectile* projectile;
    projectile = rs_new CEnergyProjectile(
        true, mWeaponToken, static_cast< EWeaponType >(mDamageInfo.GetWeaponMode1()), xf,
        kMT_ProjectilePassthrough, mDamageInfo, mgr.AllocateUniqueId(), player->GetCurrentAreaId(),
        mPlayerId, kInvalidUniqueId, 0, false, CVector3f::One(), CImpactVisorEffect::None(), false,
        true, false, 1.f, 4.f, 4.f);
    if (projectile != nullptr) {
      projectile->AddCollisionCooldown(mHullId, 3.4028235e38f);
      mgr.AddObject(projectile);
      if (CActor* base = TCastToPtr< CActor >(mgr.ObjectById(mBaseId))) {
        if (base->GetAnimationData() != nullptr) {
          base->AnimationData()->EnableLooping(false);
          base->AnimationData()->SetAnimation(CAnimPlaybackParms(mFireAnim, -1, 1.f, true), false);
        }
      }
      if (mgr.IsMultiplayer()) {
        CSfxManager::SfxStart(mSfxMultiPlayerProjectile, 127,
                              player->GetSoundPan(CPlayer::kMSP_Player), CSfxManager::kAllAreas,
                              false, false, CSfxManager::kMedPriority);
      } else {
        CSfxManager::SfxStart(mSfxSinglePlayerProjectile, 127, 64, CSfxManager::kAllAreas, false,
                              false, CSfxManager::kMedPriority);
      }
    }
  }
}

TUniqueId CScriptPlayerTurret::GetHullActorId() { return mHullId; }

CEntity* LoadPlayerTurret(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPlayerTurret sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPlayerTurret.inc"

  return rs_new CScriptPlayerTurret(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.flagsPlayerTurret, 0.017453292f * sldrThis.maxHorizRotationLeft,
      0.017453292f * sldrThis.maxHorizRotationRight, 0.017453292f * sldrThis.maxVertElevationUp,
      0.017453292f * sldrThis.maxVertElevationDown, 0.017453292f * sldrThis.damageAngle,
      0.017453292f * sldrThis.horizSpeed, 0.017453292f * sldrThis.vertSpeed, sldrThis.fireRate,
      LdrToDamageInfo(sldrThis.weaponDamage), sldrThis.weaponEffect,
      sldrThis.weaponEffectMultiPlayer,
      sldrThis.sFXTurretRotation == -1 ? CSfxManager::kInternalInvalidSfxId
                                       : sldrThis.sFXTurretRotation,
      sldrThis.sFXSinglePlayerImpact == -1 ? CSfxManager::kInternalInvalidSfxId
                                           : sldrThis.sFXSinglePlayerImpact,
      sldrThis.sFXMultiPlayerImpact == -1 ? CSfxManager::kInternalInvalidSfxId
                                          : sldrThis.sFXMultiPlayerImpact,
      sldrThis.sFXSinglePlayerProjectile == -1 ? CSfxManager::kInternalInvalidSfxId
                                               : sldrThis.sFXSinglePlayerProjectile,
      sldrThis.sFXMultiPlayerProjectile == -1 ? CSfxManager::kInternalInvalidSfxId
                                              : sldrThis.sFXMultiPlayerProjectile);
}

static void SetFuncPtrs() {
  static SPlayerTurret_FuncPtrs funcPtrs;
  funcPtrs.mLoadPlayerTurret = &LoadPlayerTurret;
  funcPtrs.mGetCameraTransform = static_cast< CTransform4f (CEntity::*)(CStateManager&) >(
      &CScriptPlayerTurret::GetCameraTransform);
  funcPtrs.mGetTurretTransform = static_cast< CTransform4f (CEntity::*)(CStateManager&) >(
      &CScriptPlayerTurret::GetTurretTransform);
  funcPtrs.mExitTurret =
      static_cast< void (CEntity::*)(CStateManager&) >(&CScriptPlayerTurret::ExitTurret);
  funcPtrs.mProcessInput = static_cast< void (CEntity::*)(const CFinalInput&, CStateManager&) >(
      &CScriptPlayerTurret::ProcessInput);
  funcPtrs.mGetHullActorId =
      static_cast< TUniqueId (CEntity::*)() >(&CScriptPlayerTurret::GetHullActorId);
  SetSPlayerTurret_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPlayerTurret_FuncPtrs(nullptr); }
