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

    SLdrEditorProperties editorProperties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    bool unknown_0x5cdc877d;
    float chargeTime;
    float timeOpen;
    float platformTime;
    float unknown_0xf1c2d224;
    float unknown_0x3c6af2ac;
    float hitDetectionAngle;
    float shockWaveHeight;
    int sound;
    SLdrShockWaveInfo shockWaveInfo;
};

void LoadTypedefSLdrPuddleSpore(SLdrPuddleSpore&, CInputStream&);

#endif // _SLDRPUDDLESPORE
