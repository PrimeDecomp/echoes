#ifndef _SLDRTWEAKPLAYER_SHIELD
#define _SLDRTWEAKPLAYER_SHIELD

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Shield {
    SLdrTweakPlayer_Shield();
    ~SLdrTweakPlayer_Shield();

    float max_energy;
    float usage_rate;
    float recharge_rate;
    bool allows_motion;
};

void LoadTypedefSLdrTweakPlayer_Shield(SLdrTweakPlayer_Shield&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_SHIELD
