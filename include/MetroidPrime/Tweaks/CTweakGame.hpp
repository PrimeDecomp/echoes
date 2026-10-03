#ifndef _CTWEAKGAME
#define _CTWEAKGAME

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CMayaSpline;
struct SLdrTweakGame;

class CTweakGame {
public:
  explicit CTweakGame(const SLdrTweakGame& data) : mData(&data) {}

  rstl::string GetPakFile() const;       // Guessed name.
  float GetFieldOfView() const;          // Guessed name.
  float GetTwoPlayerFieldOfView() const; // Guessed name.
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;
  int GetTotalPercentage(); // Guessed name.
  int GetDeathMatchFragLimit(int index) const;
  float GetDeathMatchTimeLimit(int index) const;
  int GetCoinGameCoinLimit(int index) const;
  float GetCoinGameTimeLimit(int index) const;
  CMayaSpline& GetMusicVolumeSpline(); // Guessed name.

private:
  const SLdrTweakGame* mData;
};
CHECK_SIZEOF(CTweakGame, 0x4)

extern rstl::single_ptr< CTweakGame > gpTweakGame;

#endif // _CTWEAKGAME
