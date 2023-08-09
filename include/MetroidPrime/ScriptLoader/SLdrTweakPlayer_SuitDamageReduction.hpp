#ifndef _SLDRTWEAKPLAYER_SUITDAMAGEREDUCTION
#define _SLDRTWEAKPLAYER_SUITDAMAGEREDUCTION

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_SuitDamageReduction {
    SLdrTweakPlayer_SuitDamageReduction();
    ~SLdrTweakPlayer_SuitDamageReduction();

    float varia;
    float dark;
    float light;
};

void LoadTypedefSLdrTweakPlayer_SuitDamageReduction(SLdrTweakPlayer_SuitDamageReduction&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_SUITDAMAGEREDUCTION
