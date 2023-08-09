#ifndef _SLDRTWEAKPLAYER_FROZEN
#define _SLDRTWEAKPLAYER_FROZEN

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Frozen {
    SLdrTweakPlayer_Frozen();
    ~SLdrTweakPlayer_Frozen();

    float frozen_timer;
    int frozen_jump_counter;
    float frozen_damage_threshold;
};

void LoadTypedefSLdrTweakPlayer_Frozen(SLdrTweakPlayer_Frozen&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_FROZEN
