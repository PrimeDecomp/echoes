#ifndef _SLDRSPANKWEED
#define _SLDRSPANKWEED

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrSpankWeed {
    SLdrSpankWeed();
    ~SLdrSpankWeed();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    bool unknown;
    float wakeUpRadius;
    float searchRadius;
    float attackRadius;
    float hurtSleepDelay;
};

void LoadTypedefSLdrSpankWeed(SLdrSpankWeed&, CInputStream&);

#endif // _SLDRSPANKWEED
