#ifndef _SLDRAIKEYFRAME
#define _SLDRAIKEYFRAME

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAIKeyframe {
    SLdrAIKeyframe();
    ~SLdrAIKeyframe();

    SLdrEditorProperties editor_properties;
    int animation_index;
    bool loop;
    float loop_duration;
    int unknown_0x58810503;
    int unknown_0x6d62ef74;
    float playback_rate;
};

void LoadTypedefSLdrAIKeyframe(SLdrAIKeyframe&, CInputStream&);

#endif // _SLDRAIKEYFRAME
