#ifndef _SLDRACTORROTATE
#define _SLDRACTORROTATE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRotationSplines.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScaleSplines.hpp"

struct SLdrActorRotate {
    SLdrActorRotate();
    ~SLdrActorRotate();

    SLdrEditorProperties editor_properties;
    int flags_actor_rotate;
    float duration;
    SLdrRotationSplines rotation_controls;
    SLdrScaleSplines scale_controls;
};

void LoadTypedefSLdrActorRotate(SLdrActorRotate&, CInputStream&);

#endif // _SLDRACTORROTATE
