#ifndef _SLDRCONDITIONALRELAY
#define _SLDRCONDITIONALRELAY

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrConditionalTest.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrConditionalRelay {
    SLdrConditionalRelay();
    ~SLdrConditionalRelay();

    SLdrEditorProperties editorProperties;
    bool triggerOnFirstThink;
    int multiplayerMaskAndNegate;
    SLdrConditionalTest conditional1;
    SLdrConditionalTest conditional2;
    SLdrConditionalTest conditional3;
    SLdrConditionalTest conditional4;
};

void LoadTypedefSLdrConditionalRelay(SLdrConditionalRelay&, CInputStream&);

#endif // _SLDRCONDITIONALRELAY
