#ifndef _CTWEAKPLAYER
#define _CTWEAKPLAYER

class CTweakPlayer {
public:
  float GetLeftAnalogMax();
  float GetRightAnalogMax();
  float GetVariaSuitDamageReduction();
  float GetDarkSuitDamageReduction();
  float GetLightSuitDamageReduction();
};

extern CTweakPlayer* gpTweakPlayerA;
extern CTweakPlayer* gpTweakPlayerB;

#endif // _CTWEAKPLAYER
