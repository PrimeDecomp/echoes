#include "MetroidPrime/Weapons/CAuxWeapon.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CLightComboProjectile.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"
#include "Weapons/CWeaponDescription.hpp"

static const ushort skComboSoundIds[2][4] = {{238, 8163, 8162, 65535}, {9630, 9641, 9672, 9661}};
static const ushort skMissileSoundIds[2] = {196, 9650};
static const float skAnnihilatorDamageBases[3] = {150.f, 200.f, 300.f};
static const float skAnnihilatorDamageWeights[3] = {60.f, 30.f, 10.f};
static float skAnnihilatorDamageRanges[3][2] = {{0.f, 10.f}, {-10.f, 10.f}, {-10.f, 0.f}};
static const uint skSuppressFireEffectsAttribute = 1 << 23;

CAuxWeapon::CAuxWeapon(TUniqueId playerId)
: mMissile(gpSimplePool->GetObj("Missile"))
, mMuzzleFxGen(nullptr)
, mPlayerId(playerId)
, mFiringBeamId(CPlayerState::kBI_Invalid)
, mLoadBeamId(CPlayerState::kBI_Power)
, mComboSfx(CSfxHandle::NullHandle())
, mSoundVolume(0x4a)
, mIsLoaded(false) {
  InitComboData();
}

void CAuxWeapon::Fire(float dt, bool underwater, int currentBeam,
                      CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, EWeaponType type, TUniqueId homingId, uint attributes,
                      ushort soundId) {
  if (!mIsLoaded) {
    return;
  }

  const bool isCombo = chargeState != CPlayerState::kCS_Normal;
  if (isCombo) {
    attributes |= CGameProjectile::GetBeamAttribType(type) | CWeapon::kPA_ComboShot;
  }

  if (!isCombo) {
    FireProjectile(dt, type, underwater, false, false, currentBeam, attributes, xf, homingId,
                   soundId, mgr);
    return;
  }

  switch (currentBeam) {
  case CPlayerState::kBI_Power:
  case CPlayerState::kBI_Dark:
    FireProjectile(dt, type, underwater, true, false, currentBeam, attributes, xf, homingId,
                   soundId, mgr);
    break;
  case CPlayerState::kBI_Light:
    FireLightCombo(dt, underwater, currentBeam, attributes, xf, homingId, mgr);
    break;
  case CPlayerState::kBI_Annihilator:
    FireProjectile(dt, type, underwater, true, true, currentBeam, attributes, xf, homingId, soundId,
                   mgr);
    break;
  default:
    break;
  }
}

void CAuxWeapon::SetTargetId(TUniqueId) {}

TUniqueId CAuxWeapon::GetTargetId() const { return kInvalidUniqueId; }

void CAuxWeapon::RenderMuzzleFx() const {}

bool CAuxWeapon::UpdateComboFx(float, const CVector3f&, const CVector3f&, const CTransform4f&,
                               CStateManager&) {
  if (!mIsLoaded || mFiringBeamId == CPlayerState::kBI_Invalid) {
    return false;
  }

  if (!CSfxManager::IsPlaying(mComboSfx) && mComboSfx) {
    FreeComboVoiceId();
  }
  return false;
}

void CAuxWeapon::StopComboFx(CStateManager&, bool deactivate) {
  if (deactivate) {
    mFiringBeamId = CPlayerState::kBI_Invalid;
  }
}

bool CAuxWeapon::IsComboFxActive(const CStateManager&) const { return false; }

void CAuxWeapon::InitComboData() {
  static const char* const skComboNames[] = {"SuperMissile", "IceCombo", "WaveBuster",
                                             "FlameThrower"};
  for (int i = 0; i < 4; ++i) {
    mCombos.push_back(gpSimplePool->GetObj(skComboNames[i]));
  }
}

void CAuxWeapon::FireProjectile(float dt, EWeaponType type, bool underwater, bool isCombo,
                                bool adjustSpawn, int comboId, uint attributes,
                                const CTransform4f& xf, TUniqueId homingId, ushort soundId,
                                CStateManager& mgr) {
  const CToken& missile = mMissile;
  TToken< CWeaponDescription > description(isCombo ? mCombos[comboId].GetToken() : missile);
  CDamageInfo damage = isCombo ? gpTweakPlayerGun->GetComboDamage(CPlayerState::EBeamId(comboId))
                               : gpTweakPlayerGun->GetMissileDamage();
  const int multiplayer = mgr.IsMultiplayer() ? 1 : 0;
  const ushort sfx = soundId != CSfxManager::kInternalInvalidSfxId ? soundId
                     : isCombo ? skComboSoundIds[multiplayer][comboId]
                               : skMissileSoundIds[multiplayer];
  CTransform4f spawnTransform(xf);
  CPlayer* player = GetPlayerFromAll(mgr);
  if (adjustSpawn) {
    const CVector3f position = spawnTransform.GetTranslation();
    const CVector3f direction = spawnTransform.GetForward();
    TUniqueId hitId = kInvalidUniqueId;
    const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Unknown59), CMaterialList(kMT_NoPlatformCollision));
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, position, direction, 100.f, filter,
                      TCastToPtr< CActor >(mgr.ObjectById(mPlayerId)));
    const CRayCastResult result =
        mgr.RayWorldIntersection(hitId, position, direction, 100.f, filter, nearList);
    float distance = 100.f;
    if (result.IsValid()) {
      distance = result.GetTime();
    }
    CVector3f velocity = CVector3f::Forward();
    description->mIVEC->GetValue(0, velocity);
    spawnTransform.SetTranslation(spawnTransform.GetTranslation() +
                                  (distance - velocity.Magnitude() - 0.01f) * direction);
  }

  if (type == kWT_Annihilator && isCombo) {
    const float selection = mgr.Random()->Range(0.f, 100.f);
    float accumulatedWeight = 0.f;
    for (int i = 0; i < 3; ++i) {
      accumulatedWeight += skAnnihilatorDamageWeights[i];
      if (selection <= accumulatedWeight) {
        damage.SetDamage(
            skAnnihilatorDamageBases[i] +
            mgr.Random()->Range(skAnnihilatorDamageRanges[i][0], skAnnihilatorDamageRanges[i][1]));
        break;
      }
    }
  }

  CEnergyProjectile* projectile = rs_new CEnergyProjectile(
      true, description, isCombo ? type : kWT_Missile, spawnTransform, kMT_NoPlatformCollision,
      damage.ApplyDoubleDamage(*FindPlayer(mgr)->GetPlayerState()), mgr.AllocateUniqueId(),
      kInvalidAreaId, mPlayerId, description->mHOMG ? homingId : kInvalidUniqueId, attributes,
      underwater, CVector3f::One(), CImpactVisorEffect(), false, true, type == kWT_Annihilator, 1.f,
      type == kWT_Annihilator ? 20.f : 4.f, type == kWT_Annihilator ? 20.f : 4.f);
  if (projectile) {
    mgr.AddObject(projectile);
    projectile->InitializeMuzzleOffset(0.5f, mgr);
    projectile->SetFluidList(player->CameraManager()->GetFirstPersonCamera()->GetFluidList());
    projectile->Think(dt, mgr);
  }

  if (!(attributes & skSuppressFireEffectsAttribute)) {
    player->CameraManager()->CameraShakerManager()->AddCameraShaker(
        gpTweakPlayerGunSingle->GetProjectileRecoilCameraShakerData(), mgr, false, false);
    if (isCombo) {
      projectile->SetCameraShakerData(gpTweakPlayerGun->GetProjectileImpactCameraShakerData());
    } else {
      mgr.RumbleManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
          ->Rumble(mgr, kRFX_PlayerMissileFire, 0.5f, kRP_One);
    }
    mComboSfx = PlaySfxForPlayer(FindPlayer(mgr), sfx, mSoundVolume, mgr.GetNextAreaId().Value(),
                                 underwater, false);
  }
}

void CAuxWeapon::FireLightCombo(float dt, bool underwater, int comboId, uint attributes,
                                const CTransform4f& xf, TUniqueId homingId, CStateManager& mgr) {
  TCachedToken< CWeaponDescription >& description = mCombos[comboId];
  const CDamageInfo& damage = gpTweakPlayerGun->GetComboDamage(CPlayerState::EBeamId(comboId));
  const ushort sfx = skComboSoundIds[mgr.IsMultiplayer() ? 1 : 0][comboId];
  CPlayer* player = GetPlayerFromAll(mgr);
  CLightComboProjectile* projectile = rs_new CLightComboProjectile(
      description, kWT_Light, xf, kMT_NoPlatformCollision,
      damage.ApplyDoubleDamage(*FindPlayer(mgr)->GetPlayerState()), mgr.AllocateUniqueId(),
      kInvalidAreaId, mPlayerId, description.GetObject()->mHOMG ? homingId : kInvalidUniqueId,
      underwater, attributes, 10.f);
  if (projectile) {
    mgr.AddObject(projectile);
    projectile->InitializeMuzzleOffset(0.5f, mgr);
    projectile->SetFluidList(player->CameraManager()->GetFirstPersonCamera()->GetFluidList());
    projectile->Think(dt, mgr);
  }

  player->CameraManager()->CameraShakerManager()->AddCameraShaker(
      gpTweakPlayerGun->GetProjectileRecoilCameraShakerData(), mgr, false, false);
  projectile->SetCameraShakerData(gpTweakPlayerGun->GetProjectileImpactCameraShakerData());
  mComboSfx = PlaySfxForPlayer(FindPlayer(mgr), sfx, mSoundVolume, mgr.GetNextAreaId().Value(),
                               underwater, false);
}

void CAuxWeapon::AcceptScriptMsg(CStateManager&, const CScriptMsg&) {}

void CAuxWeapon::Load(int beam, CStateManager& mgr) {
  mIsLoaded = false;
  if (!mgr.IsMultiplayer()) {
    mCombos[mLoadBeamId].Unlock();
  }
  mCombos[beam].Lock();
  mLoadBeamId = beam;
  LoadIdle();
}

void CAuxWeapon::LoadIdle() { mIsLoaded = mCombos[mLoadBeamId].IsLoaded(); }

void CAuxWeapon::FreeComboVoiceId() {
  CSfxManager::SfxStop(mComboSfx);
  mComboSfx.Clear();
}

bool CAuxWeapon::HasChargeCombo(int beam, CStateManager& mgr) const {
  static const CPlayerState::EItemType skChargeCombos[] = {
      CPlayerState::kIT_SuperMissile, CPlayerState::kIT_Darkburst, CPlayerState::kIT_Sunburst,
      CPlayerState::kIT_SonicBoom};
  const CPlayerState::EItemType item = skChargeCombos[beam];
  return item != CPlayerState::kIT_Invalid && FindPlayer(mgr)->GetPlayerState()->HasPowerUp(item);
}

CPlayer* CAuxWeapon::FindPlayer(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(const_cast< CEntity* >(mgr.GetObjectById(mPlayerId)));
}

CPlayer* CAuxWeapon::GetPlayerFromAll(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
}
