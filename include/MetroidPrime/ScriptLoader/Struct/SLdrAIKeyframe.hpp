#ifndef _SLDRAIKEYFRAME
#define _SLDRAIKEYFRAME

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAIKeyframe {
    SLdrAIKeyframe();
    ~SLdrAIKeyframe();

    SLdrEditorProperties editorProperties;
    int animationIndex;
    bool loop;
    float loopDuration;
    int unknown_0x58810503;
    int unknown_0x6d62ef74;
    float playbackRate;
};

void LoadTypedefSLdrAIKeyframe(SLdrAIKeyframe&, CInputStream&);

#endif // _SLDRAIKEYFRAME
