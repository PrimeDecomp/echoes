#include "MetroidPrime/CDamageInfo.hpp"

#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

CDamageInfo CDamageInfo::ApplyDoubleDamage(const CPlayerState& state) const {
  if (state.GetItemAmount(CPlayerState::kIT_DoubleDamage, true) != 0) {
    CDamageInfo result = *this;
    result.MultiplyDamage(2.f);
    return result;
  }

  return *this;
}

CDamageInfo NGunUtils::DifficultyModifyDamageInfo(const CDamageInfo& damage) {
  CDamageInfo result = damage;
  result.MultiplyDamage(gpGameState->GetHardModeWeaponMultiplier());
  return result;
}
