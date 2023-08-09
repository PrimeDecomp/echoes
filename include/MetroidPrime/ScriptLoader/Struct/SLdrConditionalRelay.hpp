#ifndef _SLDRCONDITIONALRELAY
#define _SLDRCONDITIONALRELAY

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrConditionalTest.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrConditionalRelay {
    SLdrConditionalRelay();
    ~SLdrConditionalRelay();

    SLdrEditorProperties editor_properties;
    bool trigger_on_first_think;
    int multiplayer_mask_and_negate;
    SLdrConditionalTest conditional1;
    SLdrConditionalTest conditional2;
    SLdrConditionalTest conditional3;
    SLdrConditionalTest conditional4;
};

void LoadTypedefSLdrConditionalRelay(SLdrConditionalRelay&, CInputStream&);

#endif // _SLDRCONDITIONALRELAY
