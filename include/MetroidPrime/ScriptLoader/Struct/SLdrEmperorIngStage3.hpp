#ifndef _SLDREMPERORINGSTAGE3
#define _SLDREMPERORINGSTAGE3

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage3Data.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrEmperorIngStage3 {
    SLdrEmperorIngStage3();
    ~SLdrEmperorIngStage3();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrEmperorIngStage3Data data;
};

void LoadTypedefSLdrEmperorIngStage3(SLdrEmperorIngStage3&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE3
