#ifndef _SLDRTWEAKPLAYER_SHIELD
#define _SLDRTWEAKPLAYER_SHIELD

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Shield {
    SLdrTweakPlayer_Shield();
    ~SLdrTweakPlayer_Shield();

    float maxEnergy;
    float usageRate;
    float rechargeRate;
    bool allowsMotion;
};

void LoadTypedefSLdrTweakPlayer_Shield(SLdrTweakPlayer_Shield&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_SHIELD
