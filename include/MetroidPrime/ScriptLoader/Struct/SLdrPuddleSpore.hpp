#ifndef _SLDRPUDDLESPORE
#define _SLDRPUDDLESPORE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrPuddleSpore {
    SLdrPuddleSpore();
    ~SLdrPuddleSpore();

    SLdrEditorProperties editor_properties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    bool unknown_0x5cdc877d;
    float charge_time;
    float time_open;
    float platform_time;
    float unknown_0xf1c2d224;
    float unknown_0x3c6af2ac;
    float hit_detection_angle;
    float shock_wave_height;
    int sound;
    SLdrShockWaveInfo shock_wave_info;
};

void LoadTypedefSLdrPuddleSpore(SLdrPuddleSpore&, CInputStream&);

#endif // _SLDRPUDDLESPORE
