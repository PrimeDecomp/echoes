#ifndef _SLDRRIPPLE
#define _SLDRRIPPLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRipple {
    SLdrRipple();
    ~SLdrRipple();

    SLdrEditorProperties editorProperties;
    float energy;
};

void LoadTypedefSLdrRipple(SLdrRipple&, CInputStream&);

#endif // _SLDRRIPPLE
