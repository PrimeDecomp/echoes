#ifndef _SLDRTWEAKPLAYER_MISC
#define _SLDRTWEAKPLAYER_MISC

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Misc {
    SLdrTweakPlayer_Misc();
    ~SLdrTweakPlayer_Misc();

    float eyeOffset;
    float normalTurnFactor;
    float freeLookTurnFactor;
    float freeLookMaxX;
    float freeLookMaxZ;
    float freeLookSpeed;
    float freeLookSnapSpeed;
    float freeLookFadeAngle;
    float freeLookMinAngle;
    float freeLookCenteredTime;
    float freeLookDampenFactor;
    bool nullAnalogScales;
    float unknown;
    float leftAnalogMax;
    float rightAnalogMax;
};

void LoadTypedefSLdrTweakPlayer_Misc(SLdrTweakPlayer_Misc&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_MISC
