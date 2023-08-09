#ifndef _SLDRRIPPLE
#define _SLDRRIPPLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRipple {
    SLdrRipple();
    ~SLdrRipple();

    SLdrEditorProperties editor_properties;
    float energy;
};

void LoadTypedefSLdrRipple(SLdrRipple&, CInputStream&);

#endif // _SLDRRIPPLE
