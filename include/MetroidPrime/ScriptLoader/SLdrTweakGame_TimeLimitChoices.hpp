#ifndef _SLDRTWEAKGAME_TIMELIMITCHOICES
#define _SLDRTWEAKGAME_TIMELIMITCHOICES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGame_TimeLimitChoices {
    SLdrTweakGame_TimeLimitChoices();
    ~SLdrTweakGame_TimeLimitChoices();

    float timeLimit0;
    float timeLimit1;
    float timeLimit2;
    float timeLimit3;
    float timeLimit4;
};

void LoadTypedefSLdrTweakGame_TimeLimitChoices(SLdrTweakGame_TimeLimitChoices&, CInputStream&);

#endif // _SLDRTWEAKGAME_TIMELIMITCHOICES
