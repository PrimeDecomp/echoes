#ifndef _SLDRSOUNDMODIFIER
#define _SLDRSOUNDMODIFIER

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSoundModifier {
    SLdrSoundModifier();
    ~SLdrSoundModifier();

    SLdrEditorProperties editorProperties;
    float time;
    bool autoReset;
    bool autoStart;
    SLdrSpline volume;
    SLdrSpline pan;
    SLdrSpline surroundPan;
    SLdrSpline pitch;
};

void LoadTypedefSLdrSoundModifier(SLdrSoundModifier&, CInputStream&);

#endif // _SLDRSOUNDMODIFIER
