#include "MetroidPrime/Weapons/CLightBeam.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/SFX/Weapons3.h"
#include "MetroidPrime/SFX/Weapons3_MP.h"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

// Reconstructed names for Echoes-specific projectile attribute bits.
static const uint skPiercingAttribute = 1 << 21;
static const uint skSuppressRecoilAttribute = 1 << 23;
static const uint skSuppressShootAnimAttribute = 1 << 24;

static const ushort kChargeSoundIds[2][2] = {
    {SFXsam_a_litfire_00_oneshot, SFXsam_a_litchfire_00_oneshot},
    {SFXsa2_a_litfire_00_oneshot, SFXsa2_a_litchfire_00_oneshot},
};

CLightBeam::CLightBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Light, playerId, scale, flags) {}

CLightBeam::~CLightBeam() {}

void CLightBeam::ReInitVariables() {
  mSecondaryGenerator = nullptr;
  mEnabledSecondaryEffect = kSFT_None;
}

void CLightBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mEnabledSecondaryEffect != kSFT_None && mSecondaryGenerator.get() != nullptr) {
    mSecondaryGenerator->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CLightBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  if (mSecondaryGenerator.get() != nullptr && mEnabledSecondaryEffect != kSFT_None) {
    if (mSecondaryGenerator->IsSystemDeletable()) {
      mEnabledSecondaryEffect = kSFT_None;
    }
    mSecondaryGenerator->SetTranslation(xf.GetTranslation());
    mSecondaryGenerator->SetOrientation(xf.GetRotation());
    mSecondaryGenerator->Update(dt);
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CLightBeam::Update(float dt, CStateManager& mgr) { CGunWeapon::Update(dt, mgr); }

void CLightBeam::Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater,
                      float dt, CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                      ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                      float chargeFactor1, float chargeFactor2) {
  const bool charged = chargeState != CPlayerState::kCS_Normal;
  if (soundId == CSfxManager::kInternalInvalidSfxId) {
    soundId = kChargeSoundIds[mgr.IsMultiplayer() ? 1 : 0][chargeState];
  }

  if (charged) {
    CPlayer* player = GetPlayerFromAll(mgr);
    float shotCount = chargeFactor1 * (mgr.IsMultiplayer() ? 5 : 10);
    float minShotCount = mgr.IsMultiplayer() ? 3 : 3;
    if (minShotCount >= shotCount) {
      shotCount = minShotCount;
    }
    const int pelletCount = static_cast< int >(shotCount);

    TUniqueId aimTarget = GetPlayer(mgr)->GetPlayerGun()->GetTargetId(mgr);
    const float aimOffsetZ = aimTarget != kInvalidUniqueId ? 0.f : 0.5f;
    CTransform4f cameraXf = mgr.GetCameraManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
                                ->GetBallCamera()
                                ->GetTransform();

    const CMaterialFilter rayFilter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    if (aimTarget == kInvalidUniqueId) {
      CAABox box(CVector3f(-51.3f, 0.f, -27.f), CVector3f(51.3f, 246.f, 27.f));
      box = box.GetTransformedAABox(cameraXf);
      mgr.BuildNearList(
          nearList, box,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid, kMT_Target),
                                              CMaterialList(kMT_ProjectilePassthrough)),
          player);
    }

    int searchStart = 0;
    int collisionDelay = 0;
    for (int i = 0; i < pelletCount; ++i) {
      const TToken< CWeaponDescription >& pelletToken = *mChargedProjectiles[i % 3];
      const float angle = (M_PIF / 180.f) * (360.f * mgr.Random()->Float());
      const float radius = mgr.Random()->Float();
      CVector3f dir(radius * (1.9f * CMath::FastSinR(angle)), 8.f,
                    radius * CMath::FastCosR(angle) + aimOffsetZ);
      dir.Normalize();
      CTransform4f shotXf = xf * CTransform4f::LookAt(CVector3f::Zero(), dir, CVector3f::Up());

      if (nearList.size() != 0) {
        const CVector3f shotDir = shotXf.GetColumn(kDY);
        aimTarget = kInvalidUniqueId;
        int index = searchStart;
        for (int tried = 0; tried < nearList.size(); ++tried) {
          bool usable = false;
          CActor* actor = TCastToPtr< CPatterned >(mgr.ObjectById(nearList[index]));
          if (actor == nullptr) {
            actor = TCastToPtr< CPlayer >(mgr.ObjectById(nearList[index]));
          }
          if (actor != nullptr) {
            usable = true;
            CVector3f toTarget = actor->GetTranslation() - cameraXf.GetTranslation();
            toTarget.SetZ(toTarget.GetZ() * 1.9f);
            if (toTarget.CanBeNormalized()) {
              const float angle = acos(CVector3f::Dot(shotDir, toTarget.AsNormalized()));
              if (angle > (10.f * (M_PIF / 180.f))) {
                usable = false;
              }
            }
            if (usable) {
              CVector3f toTargetFlat = actor->GetTranslation() - cameraXf.GetTranslation();
              if (toTargetFlat.CanBeNormalized()) {
                TUniqueId hitId = kInvalidUniqueId;
                CRayCastResult result = mgr.RayWorldIntersection(
                    hitId, cameraXf.GetTranslation(), toTargetFlat.AsNormalized(),
                    toTargetFlat.Magnitude(), rayFilter, nearList);
                if (!result.IsValid() || hitId != nearList[index]) {
                  usable = false;
                }
              }
            }
          }
          const int current = index;
          index = (index + 1) % nearList.size();
          if (usable) {
            aimTarget = nearList[current];
            break;
          }
        }
        searchStart = index;
      }

      const uint attributes =
          projectileAttributes |
          (i == 0 ? 0 : skSuppressRecoilAttribute | skSuppressShootAnimAttribute);
      TUniqueId firedId = kInvalidUniqueId;
      CGunWeapon::Fire(pelletToken, underwater, dt, chargeState, shotXf, mgr, aimTarget, attributes,
                       i == 0 ? soundId : CSfxManager::kInternalInvalidSfxId, &firedId, soundHandle,
                       1.f, 1.f);
      if (firedId != kInvalidUniqueId) {
        if (CGameProjectile* fired = TCastToPtr< CGameProjectile >(mgr.ObjectById(firedId))) {
          fired->Projectile().SetCollisionResponseDelay(collisionDelay);
        }
      }
      if (projectileId != nullptr) {
        *projectileId = firedId;
      }
      collisionDelay += 2;
    }
  } else {
    ActivateCharge(false, true);
    CGunWeapon::Fire(projectile, underwater, dt, chargeState, xf, mgr, homingTarget,
                     projectileAttributes | skPiercingAttribute, soundId, projectileId, soundHandle,
                     chargeFactor1, chargeFactor2);
  }
}

void CLightBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mSecondaryEffect->Lock();
}

void CLightBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  if (!mgr.IsMultiplayer()) {
    mSecondaryEffect->Unlock();
  }
  ReInitVariables();
}

void CLightBeam::ReleaseResources(CStateManager& mgr) {
  CGunWeapon::ReleaseResources(mgr);
  if (!mgr.IsMultiplayer()) {
    mSecondaryEffect->Unlock();
  }
  mSecondaryGenerator = nullptr;
  mEnabledSecondaryEffect = kSFT_None;
}

bool CLightBeam::IsLoaded() const { return CGunWeapon::IsLoaded(); }

void CLightBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case kSFT_None:
  case kSFT_ToCombo:
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect != kSFT_None && mSecondaryGenerator.get() != nullptr) {
      mSecondaryGenerator->SetParticleEmission(false);
    }
    mEnabledSecondaryEffect = kSFT_None;
    break;
  case kSFT_Charge:
    mSecondaryGenerator = rs_new CElementGen(*mSecondaryEffect);
    mSecondaryGenerator->SetGlobalScale(mScale);
    mEnabledSecondaryEffect = type;
    break;
  default:
    break;
  }
}

void CLightBeam::InitializeResources(CStateManager& mgr) {
  if (!mResourcesAllocated) {
    CGunWeapon::InitializeResources(mgr);
    static const char* const skChargedProjectileNames[3] = {"WaveBall_1", "WaveBall_2",
                                                            "WaveBall_3"};
    for (int i = 0; i < 3; ++i) {
      mChargedProjectiles[i] = gpSimplePool->GetObj(skChargedProjectileNames[i]);
    }
    mSecondaryEffect = gpSimplePool->GetObj("Wave2nd");
  }
}
