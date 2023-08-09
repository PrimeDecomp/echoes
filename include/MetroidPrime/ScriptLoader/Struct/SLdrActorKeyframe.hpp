#ifndef _SLDRACTORKEYFRAME
#define _SLDRACTORKEYFRAME

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrActorKeyframe {
    SLdrActorKeyframe();
    ~SLdrActorKeyframe();

    SLdrEditorProperties editor_properties;
    int animation_index;
    bool loop;
    float loop_duration;
    int unknown;
    float playback_rate;
};

void LoadTypedefSLdrActorKeyframe(SLdrActorKeyframe&, CInputStream&);

#endif // _SLDRACTORKEYFRAME
