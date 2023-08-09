#ifndef _SLDRTWEAKPLAYER_DARKWORLD
#define _SLDRTWEAKPLAYER_DARKWORLD

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrTweakPlayer_DarkWorld {
    SLdrTweakPlayer_DarkWorld();
    ~SLdrTweakPlayer_DarkWorld();

    float damage_grace_period;
    float unknown_0xa4e33ef0;
    SLdrDamageInfo damage_per_second;
    float dark_suit_damage_reduction;
    float unknown_0xee452490;
    float unknown_0x19275a97;
};

void LoadTypedefSLdrTweakPlayer_DarkWorld(SLdrTweakPlayer_DarkWorld&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_DARKWORLD
