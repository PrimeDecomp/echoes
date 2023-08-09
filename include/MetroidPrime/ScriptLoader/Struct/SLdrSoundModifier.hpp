#ifndef _SLDRSOUNDMODIFIER
#define _SLDRSOUNDMODIFIER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSoundModifier {
    SLdrSoundModifier();
    ~SLdrSoundModifier();

    SLdrEditorProperties editor_properties;
    float time;
    bool auto_reset;
    bool auto_start;
    spline volume;
    spline pan;
    spline surround_pan;
    spline pitch;
};

void LoadTypedefSLdrSoundModifier(SLdrSoundModifier&, CInputStream&);

#endif // _SLDRSOUNDMODIFIER
