#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CGunWeapon.hpp"
#include "dolphin/types.h"

static const float kFactorMultiplierForBeamCombo =
    1.0f / CPlayerState::GetMissileComboChargeFactor();
static const float kChargeDtFactor = 1.0f / CPlayerState::GetMissileComboChargeFactor();
static const short kSomeSfxForCharge[] = {
    0x524,
    0x25A4,
};

void CPlayerGun::UpdateNormalShotCycle(float dt, CStateManager& mgr) {
  CPlayer* player = GetPlayerFromAll(mgr);
  CPlayerState* playerState = player->GetPlayerState();

  if (playerState->GetItemAmount(CPlayerState::kIT_BeamWeaponsDisabled, true) > 0) {
    return;
  }

  CPlayerState::EItemType beamAmmoTypeA = CPlayerState::kIT_Invalid;
  CPlayerState::EItemType beamAmmoTypeB = CPlayerState::kIT_Invalid;
  int outBeamAmmoCost = 0;

  bool outOfAmmo = IsOutOfAmmoToShoot(mgr);
  if (outOfAmmo && m_chargePhase != kCP_AnimAndSfx) {
    if (m_chargePhase != kCP_NotCharging ||
        !playerState->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
      GetPlayer(mgr)->PlaySfxForPlayer(kSomeSfxForCharge[m_0x77c], m_0x3ac, mgr.GetNextAreaId(),
                                       m_isUnderwater, 0);
    }
    ResetCharge(mgr, false);
    fn_801cdca0(mgr, player, false);
    PlayAnim(mgr, 0, 0);
    //
  } else if (outOfAmmo ||
             GetBeamAmmoTypeAndCosts(false, mgr, beamAmmoTypeA, beamAmmoTypeB, outBeamAmmoCost)) {
    CPlayerState::EChargeStage chargeState = outOfAmmo ? CPlayerState::kCS_Normal : m_chargeState;
    float chargeFactor1 = playerState->GetChargeBeamFactor();
    if (!outOfAmmo && m_absorbedPhazonShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
      outBeamAmmoCost = 0;
    }
    if (beamAmmoTypeA != CPlayerState::kIT_Invalid) {
      playerState->DecrementAmmoAndDisplayAlertIfOut(mgr, beamAmmoTypeA, outBeamAmmoCost);
      if (beamAmmoTypeB != CPlayerState::kIT_Invalid) {
        playerState->DecrementAmmoAndDisplayAlertIfOut(mgr, beamAmmoTypeB, outBeamAmmoCost);
      }
    }

    // bool chargeEffectVisible = false;
    // if (showChargeFx && x32c_chargePhase == kCP_NotCharging) {
    //   chargeEffectVisible = true;
    // }
    // x832_25_chargeEffectVisible = chargeEffectVisible;
    // x30c_rapidFireShots += 1;

    const bool targetHoming = m_currentBeam->GetVelocityInfo().GetTargetHoming(int(chargeState));

    CTransform4f xf = m_gunWorldXf;
    // CTransform4f xf(x833_29_pointBlankWorldSurface ? x448_elbowWorldXf
    //                                                : GetGunMotionTransform() * x418_beamLocalXf);
    // if (!x833_29_pointBlankWorldSurface && x364_gunStrikeCoolTimer <= 0.f) {
    //   const CVector3f fwd = xf.GetForward();
    //   xf = x478_assistAimXf;
    //   xf.SetTranslation(fwd);
    // }

    // xf.AddTranslation(mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
    // x38c_muzzleEffectVisTimer = 0.0625f;

    // TUniqueId homingTarget = targetHoming ? GetTargetId(mgr) : kInvalidUniqueId;
    // x72c_currentBeam->Fire(x834_27_underwater, dt, CPlayerState::EChargeStage(x330_chargeState),
    //                        xf, mgr, homingTarget, x340_chargeBeamFactor, x340_chargeBeamFactor);

    // mgr.InformListeners(GetGunMotionTransform().GetTranslation(), kLNT_PlayerFire);

    if (m_absorbedPhazonShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
      // fun!
      CToken phazonBallToken = gpSimplePool->GetObj("PhazonBall");

      TUniqueId homingTarget = targetHoming ? GetTargetId(mgr) : kInvalidUniqueId;
      m_currentBeam->Fire(phazonBallToken, m_isUnderwater, dt, chargeState, xf, mgr, homingTarget,
                          0, 0x1c4, kInvalidUniqueId, CSfxHandle::NullHandle(), chargeFactor1, chargeFactor1);

    } else {
      // more fun!
    }

    mgr.fn_8003C4B8(m_gunWorldXf.GetTranslation(), 0); // something with object lists

    bool resetCharge = false;
    m_cooldown = m_currentBeam->GetWeaponInfo().m_coolDown;
    if (m_chargePhase == kCP_Phase_3 || m_chargePhase == kCP_AnimAndSfx) {
      resetCharge = true;
    }
    if (!resetCharge && mgr.fn_80036F10()) {
      GetPlayerFromAll(mgr)->fn_8000BC44(mgr);
    }
    if (resetCharge) {
      ResetCharge(mgr, false);
    }

    if (playerState->GetItemAmount(CPlayerState::kIT_DoubleDamage, true)) {
      GetPlayer(mgr)->PlaySfxForPlayer(0x2612, m_0x3ac, mgr.GetNextAreaId(), m_isUnderwater, 0);
    }
    // TODO
  } else {
    // can't shoot
    GetPlayer(mgr)->PlaySfxForPlayer(kSomeSfxForCharge[m_0x77c], m_0x3ac, mgr.GetNextAreaId(),
                                     m_isUnderwater, 0);
  }
}
void CPlayerGun::UpdateChargeState(float dt, CStateManager& mgr) {

  CPlayerState* playerState = GetPlayerFromAll(mgr)->GetPlayerState();

  switch (m_chargePhase) {
  case kCP_AnimAndSfx:
    m_maybeChargeAnim += dt;
    if (m_maybeChargeAnim >= 5.0f) {
      m_maybeChargeAnim = 0.0f;
      CRumbleManager* rumbleMgr = mgr.RumbleManager(mgr.MaskUIdNumPlayers(GetPlayerUniqueId()));
      if (m_chargeRumbleHandle == -1) {
        rumbleMgr->HardStopAll();
        m_chargeRumbleHandle = -1;
      }
      m_chargeRumbleHandle = rumbleMgr->Rumble(mgr, kRFX_PlayerGunCharge, 1.f, kRP_Three);
    }
    break;
  default:
    m_maybeChargeAnim = 0.0f;
  }

  // TODO: probably some sort of weird switch case instead
  if (m_chargePhase != kCP_NotCharging) {
    if (m_chargePhase == kCP_Phase_1 &&
        playerState->GetChargeAnimStart() > playerState->GetChargeBeamFactor()) {
      m_chargePhase = kCP_Phase_2;
    }
    if (m_chargeSfx && m_seekerChargeState != kSCS_FullyCharged) {
      CSfxManager::PitchBend(m_chargeSfx, m_isUnderwater ? 0 : 0x2000);
    }
    if (0 < m_chargePhase && m_chargePhase < 4) {
      playerState->IncrementChargeBeamFactor(kChargeDtFactor * dt);
    }
  } else {
    if (playerState->GetChargeBeamFactor() > 0.0f) {
      playerState->IncrementChargeBeamFactor(-dt);
    }
  }
}

void CPlayerGun::Charging(CStateManager& mgr, int param) {
  switch (param) {
  case 0:
    PlayAnim(mgr, 1, 0);
    StopChargeSound(mgr, true);
    break;

  case 1: {
    CPlayerState* playerState = GetPlayer(mgr)->GetPlayerState();
    float factor = IsOutOfAmmoToShoot(mgr) ? 0.5f : 1.f;
    switch (m_chargePhase) {
    case kCP_Phase_2:
      if (playerState->GetChargeBeamFactor() >= kFactorMultiplierForBeamCombo * factor) {
        m_0x810_b5 = true;
        m_chargePhase = kCP_Phase_3;
        m_chargeState = CPlayerState::kCS_Charged;
        EnableChargeFx(mgr, true);
        PlayAnim(mgr, 2, 1);
      }
      break;
    case kCP_Phase_3:
      if (playerState->GetChargeBeamFactor() >= factor) {
        m_chargePhase = kCP_AnimAndSfx;
        break;
      }
    }
    break;
  }

  case 2:
    if (m_0x810_b3) {
      ResetCharge(mgr, false);
      if (m_0x770 == 0) {
        PlayAnim(mgr, 0, 0);
      } else {
        m_0x810_b2 = true;
      }
    }
    break;
  }
}
