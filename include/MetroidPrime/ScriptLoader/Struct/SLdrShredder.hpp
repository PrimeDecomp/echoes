#ifndef _SLDRSHREDDER
#define _SLDRSHREDDER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct39.hpp"

struct SLdrShredder {
    SLdrShredder();
    ~SLdrShredder();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrUnknownStruct39 data;
};

void LoadTypedefSLdrShredder(SLdrShredder&, CInputStream&);

#endif // _SLDRSHREDDER
