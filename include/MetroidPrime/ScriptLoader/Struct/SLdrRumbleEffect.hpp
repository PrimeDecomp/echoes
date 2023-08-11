#ifndef _SLDRRUMBLEEFFECT
#define _SLDRRUMBLEEFFECT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRumbleEffect {
    SLdrRumbleEffect();
    ~SLdrRumbleEffect();

    SLdrEditorProperties editorProperties;
    float radius;
    int effect;
    int flagsRumble;
};

void LoadTypedefSLdrRumbleEffect(SLdrRumbleEffect&, CInputStream&);

#endif // _SLDRRUMBLEEFFECT
