#ifndef _SLDRTWEAKGAME_TIMELIMITCHOICES
#define _SLDRTWEAKGAME_TIMELIMITCHOICES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGame_TimeLimitChoices {
    SLdrTweakGame_TimeLimitChoices();
    ~SLdrTweakGame_TimeLimitChoices();

    float time_limit0;
    float time_limit1;
    float time_limit2;
    float time_limit3;
    float time_limit4;
};

void LoadTypedefSLdrTweakGame_TimeLimitChoices(SLdrTweakGame_TimeLimitChoices&, CInputStream&);

#endif // _SLDRTWEAKGAME_TIMELIMITCHOICES
