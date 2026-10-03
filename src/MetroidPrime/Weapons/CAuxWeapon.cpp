#include "MetroidPrime/Weapons/CAuxWeapon.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

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

void CAuxWeapon::Fire(bool underwater, int currentBeam, CPlayerState::EChargeStage chargeState,
                      const CTransform4f& xf, CStateManager& mgr, EWeaponType type,
                      TUniqueId homingId, uint attributes, ushort soundId) {
  if (!mIsLoaded) {
    return;
  }

  const bool isCombo = chargeState != CPlayerState::kCS_Normal;
  if (isCombo) {
    attributes |= CGameProjectile::GetBeamAttribType(type) | CWeapon::kPA_ComboShot;
  }

  if (!isCombo) {
    FireProjectile(type, underwater, false, false, currentBeam, attributes, xf, homingId, soundId,
                   mgr);
    return;
  }

  switch (currentBeam) {
  case CPlayerState::kBI_Power:
  case CPlayerState::kBI_Dark:
    FireProjectile(type, underwater, true, false, currentBeam, attributes, xf, homingId, soundId,
                   mgr);
    break;
  case CPlayerState::kBI_Light:
    FireLightCombo(underwater, currentBeam, attributes, xf, homingId, mgr);
    break;
  case CPlayerState::kBI_Annihilator:
    FireProjectile(type, underwater, true, true, currentBeam, attributes, xf, homingId, soundId,
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

void CAuxWeapon::FireProjectile(EWeaponType, bool, bool, bool, int, uint, const CTransform4f&,
                                TUniqueId, ushort, CStateManager&) {
  // TODO: Construct the Echoes projectile and apply its per-player effects.
}

void CAuxWeapon::FireLightCombo(bool, int, uint, const CTransform4f&, TUniqueId, CStateManager&) {
  // TODO: Construct the special Light-beam combo projectile.
}

void CAuxWeapon::AcceptScriptMsg(CStateManager&, const CScriptMsg&) {}

void CAuxWeapon::Load(int beam, CStateManager& mgr) {}

void CAuxWeapon::LoadIdle() {}

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
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
}

CPlayer* CAuxWeapon::GetPlayerFromAll(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.GetObjectByIdFromListAll(mPlayerId));
}
