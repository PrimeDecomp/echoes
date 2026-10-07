#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CBlackHole.hpp"
#include "MetroidPrime/Weapons/CHomingBlob.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Weapons/CCollisionResponseData.hpp"
#include "rstl/algorithm.hpp"

// Reconstructed names for Echoes-specific impact controls.
static const uint skPiercingAttribute = 1 << 21;
static const uint skForceImpactEffectsAttribute = 1 << 5;
static ushort skImpactVisibilityFrameWindow = 8;

#define MATERIAL_FLAG(material) (u64(1) << material)
const CMaterialList CEnergyProjectile::kCheckMaterial(
    MATERIAL_FLAG(kMT_Stone) | MATERIAL_FLAG(kMT_Metal) | MATERIAL_FLAG(kMT_Grass) |
    MATERIAL_FLAG(kMT_Ice) | MATERIAL_FLAG(kMT_Pillar) | MATERIAL_FLAG(kMT_MetalGrating) |
    MATERIAL_FLAG(kMT_Phazon) | MATERIAL_FLAG(kMT_Dirt) | MATERIAL_FLAG(kMT_Lava) |
    MATERIAL_FLAG(kMT_LavaStone) | MATERIAL_FLAG(kMT_Snow) | MATERIAL_FLAG(kMT_MudSlow) |
    MATERIAL_FLAG(kMT_HalfPipe) | MATERIAL_FLAG(kMT_Mud) | MATERIAL_FLAG(kMT_Glass) |
    MATERIAL_FLAG(kMT_Shield) | MATERIAL_FLAG(kMT_Sand) | MATERIAL_FLAG(kMT_CameraPassthrough) |
    MATERIAL_FLAG(kMT_Wood) | MATERIAL_FLAG(kMT_Organic));
#undef MATERIAL_FLAG

CEnergyProjectile::CEnergyProjectile(bool active, const TToken< CWeaponDescription >& description,
                                     EWeaponType type, const CTransform4f& xf,
                                     EMaterialTypes excludeMaterial, const CDamageInfo& damage,
                                     TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                     TUniqueId homingTarget, uint attribs, bool underwater,
                                     const CVector3f& scale, const CImpactVisorEffect& visorEffect,
                                     bool unused, bool playImpactSound, bool orientImpactToOwner,
                                     float chargeFactor, float closeImpactDistance,
                                     float impactScaleDistance)
: CGameProjectile(active, description, rstl::string_l("GameProjectile"), type, xf, excludeMaterial,
                  damage, uid, areaId, owner, homingTarget, attribs | 0x02000000, underwater, scale,
                  visorEffect)
, mInitialDirection(xf.GetForward())
, mInitialDirectionMagnitude(mInitialDirection.Magnitude())
, mLifetime(0.f)
, mChargeFactor(chargeFactor)
, mCameraShaker(gpTweakPlayerGun->GetProjectileImpactCameraShakerData())
, mCollisionCooldowns(0.08f)
, mMuzzleOffset(CVector3f::Zero())
, mMuzzleOffsetDuration(0.f)
, mMuzzleOffsetTime(0.f)
, mCloseImpactDistance(closeImpactDistance)
, mImpactScaleDistance(impactScaleDistance)
, mCombatVisorMaxVolume(CAudioSys::kMaxVolume)
, mEchoVisorMaxVolume(0)
, mUseCombatVisorVolume(true)
, mDead(false)
, mHasExploded(false)
, mExplodePending(false)
, mCameraShakerDirty(false)
, mSuppressDecal(false)
, mPlayImpactSound(playImpactSound)
, mHasMuzzleOffset(false)
, mMuzzleOffsetApplied(false)
, mOrientImpactToOwner(orientImpactToOwner)
, mLastVisibleFrame(0) {}

void CEnergyProjectile::StopProjectile(CStateManager& mgr) {
  CGameProjectile::StopProjectile(mgr);
  if (mSfx) {
    CSfxManager::RemoveEmitter(mSfx);
    mSfx.Clear();
  }
}

bool CEnergyProjectile::Explode(const CVector3f& position, const CVector3f& normal,
                                EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                const CDamageVulnerability& vulnerability, TUniqueId hitActor) {
  if (mCollisionCooldowns.Contains(hitActor)) {
    return false;
  }

  const TUniqueId lastHitActor = mLastResolvedObj;
  CPlayer* player = TCastToPtr< CPlayer >(const_cast< CEntity* >(mgr.GetObjectById(lastHitActor)));
  const int playerIndex = player != nullptr ? mgr.MaskUIdNumPlayers(lastHitActor) : -1;
  bool piercing = (GetAttribField() & skPiercingAttribute) == skPiercingAttribute &&
                  lastHitActor != kInvalidUniqueId;
  if (piercing) {
    if (mCollisionCooldowns.Contains(lastHitActor)) {
      return false;
    }
    if ((mgr.IsMultiplayer() && player != nullptr) ||
        TCastToPtr< CPatterned >(mgr.ObjectById(lastHitActor)) != nullptr) {
      mCollisionCooldowns.Add(lastHitActor);
    } else if (CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(
                   const_cast< CEntity* >(mgr.GetObjectById(lastHitActor)))) {
      mCollisionCooldowns.Add(lastHitActor);
      const TUniqueId owner = collisionActor->GetOwnerId();
      if (TCastToPtr< CPatterned >(mgr.ObjectById(owner)) != nullptr) {
        if (mCollisionCooldowns.Contains(owner)) {
          return false;
        }
        mCollisionCooldowns.Add(owner);
      }
    } else {
      piercing = false;
    }
  }

  const CVector3f offsetPosition = position + 0.01f * normal;
  bool done = true;
  const CWeaponTypeVulnerability response =
      vulnerability.GetVulnerability(GetCurrentDamageInfo().GetWeaponMode());
  const bool hurts = !close_enough(response.mDamageMultiplier, 0.f) &&
                     response.mEffect != CWeaponTypeVulnerability::kE_Immune;
  const bool deflected = hurts ? type == kWCR_Unknown15 || type == kWCR_EnemyShielded ||
                                     (type >= kWCR_Unknown79 && type <= kWCR_Unknown107)
                               : response.mEffect == CWeaponTypeVulnerability::kE_Reflect;

  SetTranslation(offsetPosition);
  if (deflected) {
    done = false;
    mHomingTargetId = kInvalidUniqueId;
    mHasExploded = false;
    mCollisionCooldowns.Add(hitActor);
  } else {
    mHasExploded = true;
    if (!piercing || lastHitActor == kInvalidUniqueId) {
      StopProjectile(mgr);
    }
    if (mCameraShakerDirty) {
      mCameraShaker.SetPosition(position);
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraManager(i)->CameraShakerManager()->AddCameraShaker(mCameraShaker, mgr, false,
                                                                     false);
      }
    }
  }

  PlayImpactSound(position, type);
  mgr.InformListeners(position, kLNT_ProjectileExplode);
  CProjectileWeapon& projectile = Projectile();
  rstl::optional_object< TLockedToken< CGenDescription > > particle = GetImpactParticle(mgr);
  if (!particle.valid()) {
    particle = projectile.CollisionOccured(type, !done, false, piercing, offsetPosition, normal,
                                           CVector3f::Zero());
  }
  if (particle.valid()) {
    CTransform4f particleXf = CTransform4f::LookAt(CVector3f::Zero(), normal);
    if (mOrientImpactToOwner) {
      if (CActor* owner = TCastToPtr< CActor >(mgr.ObjectById(GetOwnerId()))) {
        particleXf =
            CTransform4f::LookAt(CVector3f::Zero(), owner->GetTranslation() - GetTranslation());
      }
    }
    particleXf.SetTranslation(offsetPosition);
    const bool underwaterPower =
        (GetType() == kWT_Power && GetFilter().GetExcludeList().HasMaterial(kMT_Player)) &&
        mInWater;
    if (!underwaterPower) {
      if ((GetAttribField() & skForceImpactEffectsAttribute) == skForceImpactEffectsAttribute ||
          uint(mgr.GetRenderFrameIndex() - mLastVisibleFrame) < skImpactVisibilityFrameWindow ||
          uint(mgr.GetRenderFrameIndex()) == uint(mCreationRenderFrameIndex)) {
        if (!mSuppressDecal && mgr.GetNumPlayers() <= 2) {
          const rstl::optional_object< TLockedToken< CDecalDescription > > decal =
              projectile.GetDecalForCollision(type);
          if (decal.valid()) {
            CDecalManager::AddDecal(*decal, particleXf,
                                    CUnitVector3f(mInitialDirection, CUnitVector3f::kN_No), mgr);
          }
        }

        CVector3f scale = CVector3f::One();
        bool cameraClose = false;
        if (!mgr.IsMultiplayer() &&
            mgr.GetPlayer(0)->GetCameraState() == CPlayer::kCS_FirstPerson) {
          const CVector3f delta =
              particleXf.GetTranslation() -
              mgr.CameraManager(0)->GetCurrentCamera(mgr, true)->GetTranslation();
          const float distance = delta.Magnitude();
          if (distance < mImpactScaleDistance) {
            const float factor = 0.75f * (distance / mImpactScaleDistance) + 0.25f;
            scale = CVector3f(factor, factor, factor);
          }
          cameraClose = distance < mCloseImpactDistance;
        }
        if (!cameraClose && gpMain->GetAverageTickTime() + gpMain->GetAverageDrawTime() > 0.8f) {
          cameraClose = true;
        }
        uint flags = 2;
        if (cameraClose) {
          flags |= 1;
        }
        if (projectile.GetWeaponDescription()->mFC60) {
          flags |= 4;
        }
        if (mgr.IsMultiplayer()) {
          flags |= 8;
        }
        CEntity* explosion = rs_new CExplosion(
            *particle, mgr.AllocateUniqueId(),
            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
            rstl::string_l("Projectile collision response"), particleXf, flags, scale,
            CColor::White(), mgr.IsMultiplayer() ? playerIndex : -1);
        if ((GetAttribField() & (kPA_Dark | kPA_Charged)) == (kPA_Dark | kPA_Charged)) {
          if (CActor* next = TCastToPtr< CActor >(mgr.ObjectById(GetDrawParent()))) {
            next->SetNextDrawNode(explosion->GetUniqueId());
          }
        }
        mgr.AddObject(explosion);
        if (CActor* hit = TCastToPtr< CActor >(mgr.ObjectById(hitActor))) {
          bool hasPlatform = false;
          CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(hit);
          if (platform != nullptr) {
            hasPlatform = true;
          } else if (hit->GetMaterialList().HasMaterial(kMT_PlatformSlave)) {
            CObjectList& platforms = mgr.ObjectListById(kOL_Platform);
            for (int i = platforms.GetFirstObjectIndex(); i != -1;
                 i = platforms.GetNextObjectIndex(i)) {
              CScriptPlatform* other = static_cast< CScriptPlatform* >(platforms[i]);
              if (other->IsSlave(hitActor)) {
                platform = other;
                hasPlatform = true;
                break;
              }
            }
          }
          if (hasPlatform) {
            platform->AddSlave(explosion->GetUniqueId(), mgr, rstl::optional_object_null());
          }
        }
      }
    } else {
      mDead = true;
    }

    if ((GetAttribField() & (kPA_Dark | kPA_Charged)) == (kPA_Dark | kPA_Charged)) {
      const TLockedToken< CGenDescription > blobParticle =
          gpSimplePool->GetObj("HomingBlobSpread1");
      const float chargeFactor = CMath::Clamp(0.25f, mChargeFactor, 1.f);
      static const CAABox skChargedDarkImpactBounds(CVector3f(-7.f, -7.f, -7.f),
                                                    CVector3f(7.f, 7.f, 7.f));
      const int impactPlayer = mgr.IsMultiplayer() ? playerIndex : -1;
      const float homingAcceleration = mgr.IsMultiplayer() ? 0.095f : 0.06f;
      const float targetSearchRadius = mgr.IsMultiplayer() ? 11.f : 7.f;
      CActor* blob = rs_new CHomingBlob(
          blobParticle, mgr.AllocateUniqueId(), GetCurrentAreaId(), GetOwnerId(), true,
          skChargedDarkImpactBounds.GetTransformedAABox(particleXf),
          gpTweakPlayerGun->GetDarkBeamBlobDamage(), impactPlayer,
          rstl::string_l("CHomingBlobImpact"), particleXf, CHomingBlob::kMF_FollowPlayerArea,
          chargeFactor, 0.5f, 1.f, 9.f, targetSearchRadius, homingAcceleration);
      SetNextDrawNode(blob->GetUniqueId());
      mgr.AddObject(blob);
    } else if ((GetAttribField() & (kPA_Dark | kPA_ComboShot)) == (kPA_Dark | kPA_ComboShot)) {
      const rstl::optional_object< TToken< CGenDescription > > blackHoleParticle =
          TToken< CGenDescription >(gpSimplePool->GetObj("DarkBlackHole"));
      CBlackHole* blackHole = rs_new CBlackHole(
          blackHoleParticle, mgr.AllocateUniqueId(), kInvalidAreaId, GetOwnerId(),
          CTransform4f::Translate(position), gpTweakPlayerGun->GetBlackHoleDamage(),
          rstl::string_l("DarkBlackHole"), 0.f, 15.f,
          CBlackHole::kF_PullPlayers | CBlackHole::kF_CreationSound);
      mgr.AddObject(blackHole);
    } else if (HasAttrib(kPA_Dark)) {
      const TLockedToken< CGenDescription > blobParticle =
          gpSimplePool->GetObj("HomingBlobSpreadRegularBeam");
      static const CAABox skRegularDarkImpactBounds(CVector3f(-2.f, -2.f, -2.f),
                                                    CVector3f(2.f, 2.f, 2.f));
      const int impactPlayer = mgr.IsMultiplayer() ? playerIndex : -1;
      const float homingAcceleration = mgr.IsMultiplayer() ? 0.095f : 0.06f;
      CActor* blob = rs_new CHomingBlob(
          blobParticle, mgr.AllocateUniqueId(), GetCurrentAreaId(), GetOwnerId(), true,
          skRegularDarkImpactBounds.GetTransformedAABox(particleXf),
          gpTweakPlayerGun->GetDarkBeamBlobDamage(), impactPlayer,
          rstl::string_l("CHomingBlobImpact"), particleXf,
          CHomingBlob::kMF_FollowPlayerArea | CHomingBlob::kMF_SkipInitialTargets, 1.f, 0.2f, 1.f,
          1.f, 7.f, homingAcceleration);
      SetNextDrawNode(blob->GetUniqueId());
      mgr.AddObject(blob);
    } else if ((GetAttribField() & (kPA_Annihilator | kPA_ComboShot)) ==
               (kPA_Annihilator | kPA_ComboShot)) {
      CBlackHole* imploder = rs_new CBlackHole(
          rstl::optional_object_null(), mgr.AllocateUniqueId(), kInvalidAreaId, GetOwnerId(),
          CTransform4f::Translate(position), gpTweakPlayerGun->GetImploderDamage(),
          rstl::string_l("AnnihilatorImploder"), 20.f, 1.7f, 0);
      mgr.AddObject(imploder);
    }
  }
  return done;
}

void CEnergyProjectile::PreRenderAllViewports(CStateManager& mgr) {
  const rstl::optional_object< CAABox > bounds = mProjectile.GetBounds();
  if (bounds.valid()) {
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
    UpdatePortalSystemState(mgr);
  } else {
    CActor::PreRenderAllViewports(mgr);
  }
}

void CEnergyProjectile::PreRender(CStateManager& mgr) {
  if (mHasMuzzleOffset) {
    const bool remote = mgr.GetCurrentRenderPlayerIndex() != mgr.MaskUIdNumPlayers(GetOwnerId());
    if (remote) {
      if (!mMuzzleOffsetApplied) {
        const CVector3f scaled = mMuzzleOffsetTime * mMuzzleOffset;
        mProjectile.SetParticleTranslationOffset((1.f / mMuzzleOffsetDuration) * scaled);
        mMuzzleOffsetApplied = true;
      }
    } else if (mMuzzleOffsetApplied) {
      mProjectile.SetParticleTranslationOffset(CVector3f::Zero());
      mMuzzleOffsetApplied = false;
    }
  }

  SetPreRenderClipped(!mgr.IsActorVisible(*this));
  if (!GetPreRenderClipped()) {
    mLastVisibleFrame = mgr.GetRenderFrameIndex();
  }
}

void CEnergyProjectile::AddToRenderer(const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }
  if (!mProjectile.GetWeaponDescription()->mRWPE) {
    mProjectile.AddToRenderer();
  }
  EnsureRendered(mgr);
}

void CEnergyProjectile::Render(const CStateManager& mgr) const {
  if (mProjectile.GetWeaponDescription()->mRWPE) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat) {
      const float warpTime = 1.f - float(mProjectile.GameTime());
      if (warpTime > 0.f) {
        mgr.DrawSpaceWarp(GetTranslation(), 0.75f * warpTime);
      }
    }
    mProjectile.RenderParticles();
  }
  CGameProjectile::Render(mgr);
}

void CEnergyProjectile::Think(float dt, CStateManager& mgr) {
  if (mHasMuzzleOffset) {
    if (mMuzzleOffsetApplied) {
      mProjectile.SetParticleTranslationOffset(CVector3f::Zero());
      mMuzzleOffsetApplied = false;
    }
    mMuzzleOffsetTime -= dt;
    if (mMuzzleOffsetTime <= 0.f) {
      mMuzzleOffsetTime = 0.f;
      mMuzzleOffset = CVector3f::Zero();
      mHasMuzzleOffset = false;
    }
  }

  CWeapon::Think(dt, mgr);
  if (mActive) {
    mCollisionCooldowns.Update(dt);
  }
  UpdateProjectileMovement(dt, mgr);
  TUniqueId hitActor = kInvalidUniqueId;
  const CRayCastResult result = DoCollisionCheck(hitActor, mgr);
  CProjectileWeapon& projectile = mProjectile;
  if (result.IsValid()) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(hitActor))) {
      ResolveCollisionWithActor(result, *actor, mgr);
    } else {
      ResolveCollisionWithWorld(result, mgr);
    }
  } else if (mActive && projectile.GetWeaponDescription()->mEELT &&
             projectile.GetCurrentFrame() >= projectile.GetLifetime()) {
    mSuppressDecal = true;
    if (Explode(GetTranslation(), -1.f * GetTransform().GetForward(), kWCR_Default, mgr,
                CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId)) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, GetTranslation(), GetCurrentDamageInfo(),
                             GetFilter());
    }
    SetLastResolvedObject(kInvalidUniqueId);
  }
  projectile.UpdateParticleFX();
  if (mActive && mExplodePending) {
    Explode(GetTranslation(), GetExplosionNormal(), kWCR_Default, mgr,
            CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId);
  }

  if (mProjectileLight != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mProjectileLight))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(GetTranslation());
      if (projectile.GetAttachedPS1() != nullptr && projectile.GetAttachedPS1()->SystemHasLight()) {
        light->SetLight(projectile.GetAttachedPS1()->GetLight());
      }
    }
  }

  mUseCombatVisorVolume = mEchoVisorMaxVolume == 0 || mgr.IsMultiplayer() ||
                          mgr.GetPlayerState(0)->GetActiveVisor(mgr) != CPlayerState::kPV_Echo;
  if (mSfx) {
    CSfxManager::UpdateEmitter(mSfx, projectile.GetTranslation(), projectile.GetVelocity(),
                               mUseCombatVisorVolume ? mCombatVisorMaxVolume : mEchoVisorMaxVolume);
    CSfxManager::PitchBend(mSfx, mWaterUpdate ? 0 : 8192);
  }

  mLifetime += dt;
  if (mLifetime > 45.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  } else if (projectile.IsSystemDeletable() || mDead) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CEnergyProjectile::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                  CStateManager& mgr) {
  SetLastResolvedObject(actor.GetUniqueId());
  const CDamageVulnerability vulnerability = *actor.GetDamageVulnerability(
      result.GetPoint(), GetTransform().GetForward(), GetCurrentDamageInfo());
  const EWeaponCollisionResponseTypes type =
      actor.GetCollisionResponseType(result.GetPoint(), GetTransform().GetForward().AsNormalized(),
                                     GetCurrentDamageInfo().GetWeaponMode(), GetAttribField());
  actor.Touch(*this, mgr);
  if (Explode(result.GetPoint(), result.GetPlane().GetNormal(), type, mgr, vulnerability,
              actor.GetUniqueId())) {
    CGameProjectile::ResolveCollisionWithActor(result, actor, mgr);
    ApplyDamageToActors(mgr, GetCurrentDamageInfo());
  } else {
    mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XHIT);
    mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XXDG);
    actor.SendScriptMsgs(kSS_ReflectedDamage, mgr);
  }

  if (CEnergyProjectile* projectile = TCastToPtr< CEnergyProjectile >(actor)) {
    projectile->mHitProjectileOwner = GetOwnerId();
    projectile->Explode(GetTranslation(), GetTransform().GetForward(), kWCR_OtherProjectile, mgr,
                        *GetDamageVulnerability(), GetUniqueId());
  }
}

void CEnergyProjectile::ResolveCollisionWithWorld(const CRayCastResult& result,
                                                  CStateManager& mgr) {
  SetLastResolvedObject(kInvalidUniqueId);
  const EWeaponCollisionResponseTypes type =
      CCollisionResponseData::GetWorldCollisionResponseType(CMaterialList::BitPosition(
          (kCheckMaterial.GetValue() & result.GetMaterial().GetValue()) & 0xffffffff));
  if (Explode(result.GetPoint(), result.GetPlane().GetNormal(), type, mgr,
              CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId)) {
    mgr.ApplyDamageToWorld(GetOwnerId(), *this, result.GetPoint(), GetCurrentDamageInfo(),
                           GetFilter());
  }
}

void CEnergyProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Delete:
    if (mActive) {
      mgr.RemoveWeaponId(GetOwnerId(), GetType());
    }
    if (mSfx) {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
    break;
  case kSM_Create: {
    CProjectileWeapon& projectile = mProjectile;
    if (projectile.GetAttachedPS1() && projectile.GetAttachedPS1()->SystemHasLight()) {
      CreateProjectileLight(rstl::string_l("ProjectileLight_GameProjectile"),
                            projectile.GetAttachedPS1()->GetLight(), mgr);
    }
    if (projectile.GetWeaponDescription()->mPJFX >= 0) {
      float range = 50.f;
      float falloff = 0.2f;
      uchar volume = mUseCombatVisorVolume ? mCombatVisorMaxVolume : mEchoVisorMaxVolume;
      if (projectile.GetWeaponDescription()->mRNGE) {
        projectile.GetWeaponDescription()->mRNGE->GetValue(0, range);
      }
      if (projectile.GetWeaponDescription()->mFOFF) {
        projectile.GetWeaponDescription()->mFOFF->GetValue(0, falloff);
      }
      CAudioSys::C3DEmitterParmData params(
          range, falloff, 9, volume,
          20);
      params.mPos = mProjectile.GetTranslation();
      params.mDir = mProjectile.GetVelocity();
      params.mSfxId = mProjectile.GetWeaponDescription()->mPJFX;
      mSfx = CSfxManager::AddEmitter(params, GetCurrentAreaId().Value(), true, true);
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    break;
  }
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(mgr, msg);
}

void CEnergyProjectile::PlayImpactSound(const CVector3f& position,
                                        EWeaponCollisionResponseTypes type) {
  if (!mPlayImpactSound) {
    return;
  }
  const CProjectileWeapon& projectile = mProjectile;
  const int sound = projectile.GetSoundIdForCollision(type);
  if (sound < 0) {
    return;
  }
  const float range = projectile.GetAudibleRange();
  const float fallOff = projectile.GetAudibleFallOff();
  CAudioSys::C3DEmitterParmData params(
      range, fallOff, 1, mUseCombatVisorVolume ? mCombatVisorMaxVolume : mEchoVisorMaxVolume, 20);
  params.mPos = position;
  params.mSfxId = sound;
  const CSfxHandle handle = CSfxManager::AddEmitter(params, GetCurrentAreaId().Value(), true);
  if (mWaterUpdate) {
    CSfxManager::PitchBend(handle, 0);
  }
}

void CEnergyProjectile::SetCameraShakerData(const CCameraShakerData& data) {
  mCameraShaker = data;
  mCameraShakerDirty = true;
}

void CEnergyProjectile::InitializeMuzzleOffset(float duration, CStateManager& mgr) {
  if (mgr.IsMultiplayer()) {
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetOwnerId()))) {
      const CTransform4f muzzle = player->GetTransform() * player->GetScaledLocatorTransform(
                                                               player->GetGunParticleLocator());
      const CVector3f offset = muzzle.GetTranslation() - GetTranslation();
      mHasMuzzleOffset = true;
      mMuzzleOffset = offset;
      mMuzzleOffsetDuration = duration;
      mMuzzleOffsetTime = mMuzzleOffsetDuration;
    }
  }
}

void CEnergyProjectile::SetCombatVisorMaxVolume(uchar volume) {
  mCombatVisorMaxVolume = rstl::min_val(volume, CAudioSys::kMaxVolume);
}

void CEnergyProjectile::SetEchoVisorMaxVolume(uchar volume) {
  mEchoVisorMaxVolume = rstl::min_val(volume, CAudioSys::kMaxVolume);
}

CAABox CEnergyProjectile::GetSortingBounds(const CStateManager& mgr) const {
  const CVector3f extent(0.5f, 0.5f, 0.5f);
  return CAABox(GetTranslation() - extent, GetTranslation() + extent);
}

bool CEnergyProjectile::CCollisionCooldowns::Contains(TUniqueId id) const {
  const rstl::list< rstl::pair< TUniqueId, float > >::const_iterator it = rstl::binary_find(
      mEntries.begin(), mEntries.end(), rstl::pair< TUniqueId, float >(id, mDefaultDuration),
      rstl::pair_sorter_finder< rstl::pair< TUniqueId, float >, rstl::less< TUniqueId > >(
          rstl::less< TUniqueId >()));
  return it != mEntries.end();
}

void CEnergyProjectile::CCollisionCooldowns::Add(TUniqueId id) { Add(id, mDefaultDuration); }

void CEnergyProjectile::CCollisionCooldowns::Add(TUniqueId id, float duration) {
  const rstl::pair< TUniqueId, float > entry(id, duration);
  const rstl::pair_sorter_finder< rstl::pair< TUniqueId, float >, rstl::less< TUniqueId > >
      compareIds((rstl::less< TUniqueId >()));
  rstl::list< rstl::pair< TUniqueId, float > >::iterator it = rstl::binary_find(
      mEntries.begin(), mEntries.end(), entry, compareIds);
  if (it != mEntries.end()) {
    it->second = duration;
  } else {
    mEntries.insert(rstl::lower_bound(mEntries.begin(), mEntries.end(), entry, compareIds), entry);
  }
}

CEnergyProjectile::CCollisionCooldowns::CCollisionCooldowns(float duration)
: mDefaultDuration(duration) {}

void CEnergyProjectile::CCollisionCooldowns::Update(float dt) {
  for (rstl::list< rstl::pair< TUniqueId, float > >::iterator it = mEntries.begin();
       it != mEntries.end(); ++it) {
    it->second -= dt;
    if (it->second <= 0.f) {
      it = mEntries.erase(it);
    }
  }
}
