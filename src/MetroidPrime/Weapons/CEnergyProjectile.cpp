#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Weapons/CCollisionResponseData.hpp"

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

  // TODO: piercing-projectile checks also track the last hit actor and collision-actor
  // owner. Keep that path distinct from the ordinary impact handled below.
  const CVector3f offsetPosition = position + 0.01f * normal;
  const CWeaponTypeVulnerability response =
      vulnerability.GetVulnerability(GetCurrentDamageInfo().GetWeaponMode());
  const bool hurts = !close_enough(response.mDamageMultiplier, 0.f) &&
                     response.mEffect != CWeaponTypeVulnerability::kE_Immune;
  const bool deflected = hurts ? type == kWCR_Unknown15 || type == kWCR_EnemyShielded ||
                                     (type >= kWCR_Unknown79 && type <= kWCR_Unknown107)
                               : response.mEffect == CWeaponTypeVulnerability::kE_Reflect;

  SetTranslation(offsetPosition);
  if (deflected) {
    mHomingTargetId = kInvalidUniqueId;
    mHasExploded = false;
    mCollisionCooldowns.Add(hitActor);
  } else {
    mHasExploded = true;
    StopProjectile(mgr);
    // TODO: place the configured camera shaker at the impact and submit it to each player.
  }

  PlayImpactSound(position, type);
  // TODO: notify AI listeners, preserve piercing motion, and create the impact entity,
  // decal/platform attachment, Dark homing-blob/black-hole or Annihilator implosion.
  rstl::optional_object< TLockedToken< CGenDescription > > particle = GetImpactParticle(mgr);
  if (!particle.valid()) {
    particle = mProjectile.CollisionOccured(type, deflected, false, false, offsetPosition, normal,
                                            CVector3f::Zero());
  }
  return !deflected;
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
    if (mgr.MaskUIdNumPlayers(GetOwnerId()) != mgr.GetCurrentRenderPlayerIndex()) {
      if (!mMuzzleOffsetApplied) {
        mProjectile.SetParticleTranslationOffset((mMuzzleOffsetTime * mMuzzleOffset) /
                                                 mMuzzleOffsetDuration);
        mMuzzleOffsetApplied = true;
      }
    } else if (mMuzzleOffsetApplied) {
      mProjectile.SetParticleTranslationOffset(CVector3f::Zero());
      mMuzzleOffsetApplied = false;
    }
  }

  SetPreRenderClipped(!mgr.fn_800366e4(this));
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
    const float warpTime = 1.f - float(mProjectile.GameTime());
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat && warpTime > 0.f) {
      mgr.DrawSpaceWarp(GetTranslation(), 0.75f * warpTime);
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
  if (result.IsValid()) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(hitActor))) {
      ResolveCollisionWithActor(result, *actor, mgr);
    } else {
      ResolveCollisionWithWorld(result, mgr);
    }
  } else if (mActive && mProjectile.GetWeaponDescription()->mEELT &&
             mProjectile.GetCurrentFrame() >= mProjectile.GetLifetime()) {
    mSuppressDecal = true;
    if (Explode(GetTranslation(), -GetTransform().GetForward(), kWCR_Default, mgr,
                CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId)) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, GetTranslation(), GetCurrentDamageInfo(),
                             GetFilter());
    }
    mLastResolvedObj = kInvalidUniqueId;
  }
  mProjectile.UpdateParticleFX();
  if (mActive && mExplodePending) {
    Explode(GetTranslation(), GetExplosionNormal(), kWCR_Default, mgr,
            CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId);
  }

  if (mProjectileLight != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mProjectileLight))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(GetTranslation());
      CElementGen* particles = mProjectile.GetAttachedPS1();
      if (particles != nullptr && particles->SystemHasLight()) {
        light->SetLight(particles->GetLight());
      }
    }
  }

  mUseCombatVisorVolume = mEchoVisorMaxVolume == 0 || mgr.fn_80036F10() ||
                          mgr.GetPlayerState(0)->GetActiveVisor(mgr) != CPlayerState::kPV_Echo;
  if (mSfx) {
    CSfxManager::UpdateEmitter(mSfx, mProjectile.GetTranslation(), mProjectile.GetVelocity(),
                               mUseCombatVisorVolume ? mCombatVisorMaxVolume : mEchoVisorMaxVolume);
    CSfxManager::PitchBend(mSfx, mWaterUpdate ? 0 : 8192);
  }

  mLifetime += dt;
  if (mLifetime > 45.f || mProjectile.IsSystemDeletable() || mDead) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CEnergyProjectile::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                  CStateManager& mgr) {
  mLastResolvedObj = actor.GetUniqueId();
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
    mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XHIT, kInvalidUniqueId);
    mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XXDG, kInvalidUniqueId);
    actor.SendScriptMsgs(kSS_ReflectedDamage, mgr, kInvalidUniqueId, kSM_None);
  }

  if (CEnergyProjectile* projectile = TCastToPtr< CEnergyProjectile >(actor)) {
    projectile->mHitProjectileOwner = GetOwnerId();
    projectile->Explode(GetTranslation(), GetTransform().GetForward(), kWCR_OtherProjectile, mgr,
                        *GetDamageVulnerability(), GetUniqueId());
  }
}

void CEnergyProjectile::ResolveCollisionWithWorld(const CRayCastResult& result,
                                                  CStateManager& mgr) {
  mLastResolvedObj = kInvalidUniqueId;
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
  case kSM_XDelete:
    if (mActive) {
      mgr.RemoveWeaponId(GetOwnerId(), GetType());
    }
    if (mSfx) {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
    break;
  case kSM_XCRT: {
    CElementGen* particles = mProjectile.GetAttachedPS1();
    if (particles != nullptr && particles->SystemHasLight()) {
      CreateProjectileLight(rstl::string_l("ProjectileLight_GameProjectile"), particles->GetLight(),
                            mgr);
    }
    const TLockedToken< CWeaponDescription > description = mProjectile.GetWeaponDescription();
    if (description->mPJFX >= 0) {
      float range = 50.f;
      float falloff = 0.2f;
      if (description->mRNGE != nullptr) {
        description->mRNGE->GetValue(0, range);
      }
      if (description->mFOFF != nullptr) {
        description->mFOFF->GetValue(0, falloff);
      }
      CAudioSys::C3DEmitterParmData params(
          range, falloff, 9, mUseCombatVisorVolume ? mCombatVisorMaxVolume : mEchoVisorMaxVolume,
          20);
      params.mPos = mProjectile.GetTranslation();
      params.mDir = mProjectile.GetVelocity();
      params.mSfxId = description->mPJFX;
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
  const int sound = mProjectile.GetSoundIdForCollision(type);
  if (sound < 0) {
    return;
  }
  CAudioSys::C3DEmitterParmData params(
      mProjectile.GetAudibleRange(), mProjectile.GetAudibleFallOff(), 1,
      mUseCombatVisorVolume ? mCombatVisorMaxVolume : mEchoVisorMaxVolume, 20);
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
  if (mgr.fn_80036F10()) {
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetOwnerId()))) {
      const CTransform4f muzzle = player->GetTransform() * player->GetScaledLocatorTransform(
                                                               player->GetGunParticleLocator());
      mHasMuzzleOffset = true;
      mMuzzleOffset = muzzle.GetTranslation() - GetTranslation();
      mMuzzleOffsetTime = mMuzzleOffsetDuration = duration;
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
  for (rstl::list< rstl::pair< TUniqueId, float > >::const_iterator it = mEntries.begin();
       it != mEntries.end(); ++it) {
    if (it->first == id) {
      return true;
    }
  }
  return false;
}

void CEnergyProjectile::CCollisionCooldowns::Add(TUniqueId id) { Add(id, mDefaultDuration); }

void CEnergyProjectile::CCollisionCooldowns::Add(TUniqueId id, float duration) {
  rstl::list< rstl::pair< TUniqueId, float > >::iterator it = mEntries.begin();
  for (; it != mEntries.end() && it->first < id; ++it) {
  }
  if (it != mEntries.end() && it->first == id) {
    it->second = duration;
  } else {
    mEntries.insert(it, rstl::pair< TUniqueId, float >(id, duration));
  }
}

void CEnergyProjectile::CCollisionCooldowns::Update(float dt) {
  for (rstl::list< rstl::pair< TUniqueId, float > >::iterator it = mEntries.begin();
       it != mEntries.end(); ++it) {
    it->second -= dt;
    if (it->second <= 0.f) {
      it = mEntries.erase(it);
    }
  }
}
