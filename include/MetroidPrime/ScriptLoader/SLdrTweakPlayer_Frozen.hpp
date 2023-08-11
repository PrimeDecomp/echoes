#ifndef _SLDRTWEAKPLAYER_FROZEN
#define _SLDRTWEAKPLAYER_FROZEN

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Frozen {
    SLdrTweakPlayer_Frozen();
    ~SLdrTweakPlayer_Frozen();

    float frozenTimer;
    int frozenJumpCounter;
    float frozenDamageThreshold;
};

void LoadTypedefSLdrTweakPlayer_Frozen(SLdrTweakPlayer_Frozen&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_FROZEN
