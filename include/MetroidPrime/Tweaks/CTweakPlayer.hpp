#ifndef _CTWEAKPLAYER
#define _CTWEAKPLAYER

class CTweakPlayer {
public:
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
};

extern CTweakPlayer* gpTweakPlayerA;
extern CTweakPlayer* gpTweakPlayerB;

#endif // _CTWEAKPLAYER
