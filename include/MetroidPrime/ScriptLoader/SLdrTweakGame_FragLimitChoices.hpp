#ifndef _SLDRTWEAKGAME_FRAGLIMITCHOICES
#define _SLDRTWEAKGAME_FRAGLIMITCHOICES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGame_FragLimitChoices {
    SLdrTweakGame_FragLimitChoices();
    ~SLdrTweakGame_FragLimitChoices();

    int frag_limit0;
    int frag_limit1;
    int frag_limit2;
    int frag_limit3;
    int frag_limit4;
};

void LoadTypedefSLdrTweakGame_FragLimitChoices(SLdrTweakGame_FragLimitChoices&, CInputStream&);

#endif // _SLDRTWEAKGAME_FRAGLIMITCHOICES
