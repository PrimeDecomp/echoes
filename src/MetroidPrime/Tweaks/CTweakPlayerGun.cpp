#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun.hpp"

void CTweakPlayerGun::BuildCache() {
  mBeamInfo.clear();
  mBeamInfo.push_back(
      SWeaponInfo(mData->weapons.power_Beam.delayBetweenShots,
                  LdrToDamageInfo(mData->weapons.power_Beam.damageInfo.normal),
                  LdrToDamageInfo(mData->weapons.power_Beam.damageInfo.charged, true)));
  mBeamInfo.push_back(
      SWeaponInfo(mData->weapons.dark_Beam.delayBetweenShots,
                  LdrToDamageInfo(mData->weapons.dark_Beam.damageInfo.normal),
                  LdrToDamageInfo(mData->weapons.dark_Beam.damageInfo.charged, true)));
  mBeamInfo.push_back(
      SWeaponInfo(mData->weapons.light_Beam.delayBetweenShots,
                  LdrToDamageInfo(mData->weapons.light_Beam.damageInfo.normal),
                  LdrToDamageInfo(mData->weapons.light_Beam.damageInfo.charged, true)));
  mBeamInfo.push_back(
      SWeaponInfo(mData->weapons.annihilator_Beam.delayBetweenShots,
                  LdrToDamageInfo(mData->weapons.annihilator_Beam.damageInfo.normal),
                  LdrToDamageInfo(mData->weapons.annihilator_Beam.damageInfo.charged, true)));
}

const SWeaponInfo& CTweakPlayerGun::GetBeamInfo(CPlayerState::EBeamId beam) const {
  return mBeamInfo[beam];
}

CDamageInfo CTweakPlayerGun::GetDarkBeamBlobDamage() const {
  return LdrToDamageInfo(mData->weapons.dark_Beam_Blob);
}

SWeaponInfo::SWeaponInfo(float coolDown, const CDamageInfo& normal, const CDamageInfo& charged)
: mCoolDown(coolDown), mNormal(normal), mCharged(charged) {}

SWeaponInfo CTweakPlayerGun::GetPhazonBeamInfo() const {
  return SWeaponInfo(mData->weapons.phazon_Beam.delayBetweenShots,
                     LdrToDamageInfo(mData->weapons.phazon_Beam.damageInfo.normal),
                     LdrToDamageInfo(mData->weapons.phazon_Beam.damageInfo.charged, true));
}

CDamageInfo CTweakPlayerGun::GetMissileDamage() const {
  return LdrToDamageInfo(mData->weapons.missile);
}

CDamageInfo CTweakPlayerGun::GetBombInfo() const { return LdrToDamageInfo(mData->weapons.bomb); }

CDamageInfo CTweakPlayerGun::GetPowerBombInfo() const {
  return LdrToDamageInfo(mData->weapons.power_Bomb);
}

CDamageInfo CTweakPlayerGun::GetBlackHoleDamage() const {
  return LdrToDamageInfo(mData->beam_Misc.blackhole_Dark, false, true, true);
}

CDamageInfo CTweakPlayerGun::GetSunBurstRaysDamage() const {
  return LdrToDamageInfo(mData->beam_Misc.sunBurstRays_Light, false, true, true, true);
}

CDamageInfo CTweakPlayerGun::GetImploderDamage() const {
  return LdrToDamageInfo(mData->beam_Misc.imploder_Annihilator, false, true, true, true);
}

float CTweakPlayerGun::GetAIBurnDamage() const { return mData->beam_Misc.aIBurnDamage; }

float CTweakPlayerGun::GetPlayerBurnDamage() const { return mData->beam_Misc.playerBurnDamage; }

int CTweakPlayerGun::GetMaxAbsorbedPhazonShots() const {
  return mData->beam_Misc.maxAbsorbedPhazonShots;
}

float CTweakPlayerGun::GetPhazonShotAbsorbRadius() const {
  return mData->beam_Misc.phazonShotAbsorbRadius;
}

float CTweakPlayerGun::GetGunExtendDistance() const { return mData->position.unknown_0x1547d77b; }

CVector3f CTweakPlayerGun::GetGunPosition() const {
  return CVector3f(mData->position.x, mData->position.y, mData->position.z);
}

CVector3f CTweakPlayerGun::GetGrapplingArmPosition() const { return mData->arm_Position.grappling; }

float CTweakPlayerGun::GetGunHolsterTime() const { return mData->holstering.gunHolsterTime; }

float CTweakPlayerGun::GetGunNotFiringTime() const { return mData->holstering.gunNotFiringTime; }

float CTweakPlayerGun::GetFixedVerticalAim() const {
  return CRelAngle::FromDegrees(mData->holstering.gunHolsteredAngle).AsRadians();
}

float CTweakPlayerGun::GetBombTriggerRadius() const { return mData->weapons.unknown_0xe8907530; }

float CTweakPlayerGun::GetBombDropDelayTime() const { return mData->weapons.bombDropDelayTime; }

float CTweakPlayerGun::GetHoloHoldTime() const { return mData->misc.hologramDisplayTime; }

float CTweakPlayerGun::GetGunTransformTime() const { return mData->misc.gunTransformTime; }

CDamageInfo CTweakPlayerGun::GetComboDamage(CPlayerState::EBeamId beam) const {
  switch (beam) {
  default:
  case CPlayerState::kBI_Power:
    return LdrToDamageInfo(mData->beam_Combo.superMissile_Power, false, true);
  case CPlayerState::kBI_Dark:
    return LdrToDamageInfo(mData->beam_Combo.darkCombo_Dark, false, true);
  case CPlayerState::kBI_Light:
    return LdrToDamageInfo(mData->beam_Combo.lightCombo_Light, false, true);
  case CPlayerState::kBI_Annihilator:
    return LdrToDamageInfo(mData->beam_Combo.annihilatorCombo_Annihilator, false, true);
  }
}

CCameraShakerData CTweakPlayerGun::GetRecoilCameraShakerData() const {
  const SLdrCameraShakerData& shaker = mData->recoil;
  // All three native presets use the recoil record's audio effect.
  return CCameraShakerData(shaker.attenuationDistance, shaker.duration, shaker.flagsCameraShaker,
                           CVector3f::Zero(), shaker.horizontalMotion, shaker.verticalMotion,
                           shaker.forwardMotion, mData->recoil.audioEffect);
}

CCameraShakerData CTweakPlayerGun::GetProjectileRecoilCameraShakerData() const {
  const SLdrCameraShakerData& shaker = mData->projectileRecoil;
  // All three native presets use the recoil record's audio effect.
  return CCameraShakerData(shaker.attenuationDistance, shaker.duration, shaker.flagsCameraShaker,
                           CVector3f::Zero(), shaker.horizontalMotion, shaker.verticalMotion,
                           shaker.forwardMotion, mData->recoil.audioEffect);
}

CCameraShakerData CTweakPlayerGun::GetProjectileImpactCameraShakerData() const {
  const SLdrCameraShakerData& shaker = mData->projectileImpact;
  // All three native presets use the recoil record's audio effect.
  return CCameraShakerData(shaker.attenuationDistance, shaker.duration, shaker.flagsCameraShaker,
                           CVector3f::Zero(), shaker.horizontalMotion, shaker.verticalMotion,
                           shaker.forwardMotion, mData->recoil.audioEffect);
}
