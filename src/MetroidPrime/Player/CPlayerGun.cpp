#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"

static const float kFactorMultiplierForBeamCombo = 1.0f / CPlayerState::GetMissileComboChargeFactor();
static const float kChargeDtFactor = 1.0f / CPlayerState::GetMissileComboChargeFactor();

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
    if (m_chargePhase == kCP_Phase_1 && playerState->GetChargeAnimStart() > playerState->GetChargeBeamFactor()) {
      m_chargePhase = kCP_Phase_2;
    }
    if (m_chargeSfx && m_seekerChargeState != kSCS_FullyCharged) {
      CSfxManager::PitchBend(m_chargeSfx, m_0x3ae_b0 ? 0 : 0x2000);
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
            m_chargeState = kCS_Charged;
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
