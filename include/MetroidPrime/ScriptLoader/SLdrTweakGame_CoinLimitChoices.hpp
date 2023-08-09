#ifndef _SLDRTWEAKGAME_COINLIMITCHOICES
#define _SLDRTWEAKGAME_COINLIMITCHOICES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGame_CoinLimitChoices {
    SLdrTweakGame_CoinLimitChoices();
    ~SLdrTweakGame_CoinLimitChoices();

    int coin_limit0;
    int coin_limit1;
    int coin_limit2;
    int coin_limit3;
    int coin_limit4;
};

void LoadTypedefSLdrTweakGame_CoinLimitChoices(SLdrTweakGame_CoinLimitChoices&, CInputStream&);

#endif // _SLDRTWEAKGAME_COINLIMITCHOICES
