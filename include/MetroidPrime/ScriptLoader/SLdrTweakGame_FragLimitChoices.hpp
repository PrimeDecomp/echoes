#ifndef _SLDRTWEAKGAME_FRAGLIMITCHOICES
#define _SLDRTWEAKGAME_FRAGLIMITCHOICES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGame_FragLimitChoices {
    SLdrTweakGame_FragLimitChoices();
    ~SLdrTweakGame_FragLimitChoices();

    int fragLimit0;
    int fragLimit1;
    int fragLimit2;
    int fragLimit3;
    int fragLimit4;
};

void LoadTypedefSLdrTweakGame_FragLimitChoices(SLdrTweakGame_FragLimitChoices&, CInputStream&);

#endif // _SLDRTWEAKGAME_FRAGLIMITCHOICES
