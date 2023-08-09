#ifndef _SLDRPLAYERHINT
#define _SLDRPLAYERHINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlayerHint {
    SLdrPlayerHint();
    ~SLdrPlayerHint();

    SLdrEditorProperties editor_properties;
    int priority;
    float timer;
    float unknown;
    int flags_player_hint;
};

void LoadTypedefSLdrPlayerHint(SLdrPlayerHint&, CInputStream&);

#endif // _SLDRPLAYERHINT
