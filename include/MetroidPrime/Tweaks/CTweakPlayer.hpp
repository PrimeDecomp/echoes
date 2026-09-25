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
};

extern CTweakPlayer* gpTweakPlayerA;
extern CTweakPlayer* gpTweakPlayerB;

#endif // _CTWEAKPLAYER
