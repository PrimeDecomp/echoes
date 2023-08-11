#ifndef _SLDRSOUNDMODIFIER
#define _SLDRSOUNDMODIFIER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSoundModifier {
    SLdrSoundModifier();
    ~SLdrSoundModifier();

    SLdrEditorProperties editorProperties;
    float time;
    bool autoReset;
    bool autoStart;
    spline volume;
    spline pan;
    spline surroundPan;
    spline pitch;
};

void LoadTypedefSLdrSoundModifier(SLdrSoundModifier&, CInputStream&);

#endif // _SLDRSOUNDMODIFIER
