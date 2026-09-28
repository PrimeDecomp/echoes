#ifndef _CTWEAKPLAYER
#define _CTWEAKPLAYER

#include "rstl/single_ptr.hpp"

struct SLdrTweakPlayer;

class CTweakPlayer {
public:
  explicit CTweakPlayer(const SLdrTweakPlayer& data) : mData(&data) {}

  float GetBallRadius();
  float GetEyeOffset() const;
  float GetLeftAnalogMax();
  float GetRightAnalogMax();
  float GetVariaSuitDamageReduction();
  float GetDarkSuitDamageReduction();
  float GetLightSuitDamageReduction();
  float GetGrappleBeamSpeed() const;
  float GetGrappleBeamXWaveAmplitude() const;
  float GetGrappleBeamZWaveAmplitude() const;
  float GetGrappleBeamAnglePhaseDelta() const;

private:
  const SLdrTweakPlayer* mData;
};
CHECK_SIZEOF(CTweakPlayer, 0x4)

extern rstl::single_ptr< CTweakPlayer > gpTweakPlayerA;
extern rstl::single_ptr< CTweakPlayer > gpTweakPlayerB;

#endif // _CTWEAKPLAYER
