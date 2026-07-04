#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

static const float kFactorMultiplierForBeamCombo = 1.0f / CPlayerState::GetMissileComboChargeFactor();

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
