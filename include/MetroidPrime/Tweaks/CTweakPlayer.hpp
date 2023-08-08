#ifndef _CTWEAKPLAYER
#define _CTWEAKPLAYER

class CTweakPlayer {
public:
  float GetLeftAnalogMax();
  float GetRightAnalogMax();
};

extern CTweakPlayer* gpTweakPlayerA;
extern CTweakPlayer* gpTweakPlayerB;

#endif // _CTWEAKPLAYER
