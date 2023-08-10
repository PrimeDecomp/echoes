#ifndef _SLDRRUMBLEEFFECT
#define _SLDRRUMBLEEFFECT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRumbleEffect {
    SLdrRumbleEffect();
    ~SLdrRumbleEffect();

    SLdrEditorProperties editor_properties;
    float radius;
    int effect;
    int flags_rumble;
};

void LoadTypedefSLdrRumbleEffect(SLdrRumbleEffect&, CInputStream&);

#endif // _SLDRRUMBLEEFFECT
