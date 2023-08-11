#ifndef _SLDRACTORKEYFRAME
#define _SLDRACTORKEYFRAME

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrActorKeyframe {
    SLdrActorKeyframe();
    ~SLdrActorKeyframe();

    SLdrEditorProperties editorProperties;
    int animationIndex;
    bool loop;
    float loopDuration;
    int unknown;
    float playbackRate;
};

void LoadTypedefSLdrActorKeyframe(SLdrActorKeyframe&, CInputStream&);

#endif // _SLDRACTORKEYFRAME
