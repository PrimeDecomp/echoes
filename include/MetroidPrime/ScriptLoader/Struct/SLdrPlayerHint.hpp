#ifndef _SLDRPLAYERHINT
#define _SLDRPLAYERHINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlayerHint {
    SLdrPlayerHint();
    ~SLdrPlayerHint();

    SLdrEditorProperties editorProperties;
    int priority;
    float timer;
    float unknown;
    int flagsPlayerHint;
};

void LoadTypedefSLdrPlayerHint(SLdrPlayerHint&, CInputStream&);

#endif // _SLDRPLAYERHINT
