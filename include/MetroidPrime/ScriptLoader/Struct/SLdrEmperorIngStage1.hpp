#ifndef _SLDREMPERORINGSTAGE1
#define _SLDREMPERORINGSTAGE1

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage1Data.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrEmperorIngStage1 {
    SLdrEmperorIngStage1();
    ~SLdrEmperorIngStage1();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrEmperorIngStage1Data data;
};

void LoadTypedefSLdrEmperorIngStage1(SLdrEmperorIngStage1&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE1
