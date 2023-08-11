#ifndef _SLDRTWEAKPLAYER_DARKWORLD
#define _SLDRTWEAKPLAYER_DARKWORLD

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrTweakPlayer_DarkWorld {
    SLdrTweakPlayer_DarkWorld();
    ~SLdrTweakPlayer_DarkWorld();

    float damageGracePeriod;
    float unknown_0xa4e33ef0;
    SLdrDamageInfo damagePerSecond;
    float darkSuitDamageReduction;
    float unknown_0xee452490;
    float unknown_0x19275a97;
};

void LoadTypedefSLdrTweakPlayer_DarkWorld(SLdrTweakPlayer_DarkWorld&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_DARKWORLD
