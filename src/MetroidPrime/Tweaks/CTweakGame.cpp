#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame.hpp"

rstl::string CTweakGame::GetPakFile() const { return mData->pakFile; }

float CTweakGame::GetFieldOfView() const { return mData->fieldofView; }

float CTweakGame::GetTwoPlayerFieldOfView() const { return mData->fieldofView2Player; }

float CTweakGame::GetHardModeDamageMultiplier() const { return mData->hardModeDamageMultiplier; }

float CTweakGame::GetHardModeWeaponMultiplier() const { return mData->hardModeWeaponMultiplier; }

int CTweakGame::GetTotalPercentage() { return mData->maxPercentageInventoryItems; }

int CTweakGame::GetDeathMatchFragLimit(int index) const {
  return reinterpret_cast< const int* >(&mData->unknown_0x1d627808)[index];
}

float CTweakGame::GetDeathMatchTimeLimit(int index) const {
  return reinterpret_cast< const float* >(&mData->unknown_0xb2e8828d)[index];
}

int CTweakGame::GetCoinGameCoinLimit(int index) const {
  return reinterpret_cast< const int* >(&mData->unknown_0x06af87bd)[index];
}

float CTweakGame::GetCoinGameTimeLimit(int index) const {
  return reinterpret_cast< const float* >(&mData->unknown_0x1533ea4e)[index];
}

CMayaSpline& CTweakGame::GetMusicVolumeSpline() {
  return const_cast< CMayaSpline& >(mData->unknown_0x40818220);
}
