#ifndef _SLDRSPANKWEED
#define _SLDRSPANKWEED

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrSpankWeed {
    SLdrSpankWeed();
    ~SLdrSpankWeed();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    bool unknown;
    float wake_up_radius;
    float search_radius;
    float attack_radius;
    float hurt_sleep_delay;
};

void LoadTypedefSLdrSpankWeed(SLdrSpankWeed&, CInputStream&);

#endif // _SLDRSPANKWEED
