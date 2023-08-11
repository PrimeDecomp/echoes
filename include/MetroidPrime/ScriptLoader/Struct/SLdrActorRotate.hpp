#ifndef _SLDRACTORROTATE
#define _SLDRACTORROTATE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRotationSplines.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScaleSplines.hpp"

struct SLdrActorRotate {
    SLdrActorRotate();
    ~SLdrActorRotate();

    SLdrEditorProperties editorProperties;
    int flagsActorRotate;
    float duration;
    SLdrRotationSplines rotationControls;
    SLdrScaleSplines scaleControls;
};

void LoadTypedefSLdrActorRotate(SLdrActorRotate&, CInputStream&);

#endif // _SLDRACTORROTATE
