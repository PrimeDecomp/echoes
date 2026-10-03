#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame.hpp"

rstl::string CTweakGame::GetPakFile() const { return mData->pakFile; }

float CTweakGame::GetFieldOfView() const { return mData->fieldofView; }

float CTweakGame::GetTwoPlayerFieldOfView() const { return mData->fieldofView2Player; }

float CTweakGame::GetHardModeDamageMultiplier() const { return mData->hardModeDamageMultiplier; }

float CTweakGame::GetHardModeWeaponMultiplier() const { return mData->hardModeWeaponMultiplier; }

int CTweakGame::GetTotalPercentage() { return mData->maxPercentageInventoryItems; }

int CTweakGame::GetDeathMatchFragLimit(int index) const {
  return mData->unknown_0x1d627808.fragLimits[index];
}

float CTweakGame::GetDeathMatchTimeLimit(int index) const {
  return mData->unknown_0xb2e8828d.timeLimits[index];
}

int CTweakGame::GetCoinGameCoinLimit(int index) const {
  return mData->unknown_0x06af87bd.coinLimits[index];
}

float CTweakGame::GetCoinGameTimeLimit(int index) const {
  return mData->unknown_0x1533ea4e.timeLimits[index];
}

CMayaSpline& CTweakGame::GetMusicVolumeSpline() {
  return const_cast< CMayaSpline& >(mData->unknown_0x40818220);
}
