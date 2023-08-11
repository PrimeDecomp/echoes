#ifndef _SLDRTWEAKGAME_COINLIMITCHOICES
#define _SLDRTWEAKGAME_COINLIMITCHOICES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGame_CoinLimitChoices {
    SLdrTweakGame_CoinLimitChoices();
    ~SLdrTweakGame_CoinLimitChoices();

    int coinLimit0;
    int coinLimit1;
    int coinLimit2;
    int coinLimit3;
    int coinLimit4;
};

void LoadTypedefSLdrTweakGame_CoinLimitChoices(SLdrTweakGame_CoinLimitChoices&, CInputStream&);

#endif // _SLDRTWEAKGAME_COINLIMITCHOICES
