#ifndef _SLDREMPERORINGSTAGE2TENTACLE
#define _SLDREMPERORINGSTAGE2TENTACLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage2TentacleData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrEmperorIngStage2Tentacle {
    SLdrEmperorIngStage2Tentacle();
    ~SLdrEmperorIngStage2Tentacle();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrEmperorIngStage2TentacleData data;
};

void LoadTypedefSLdrEmperorIngStage2Tentacle(SLdrEmperorIngStage2Tentacle&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE2TENTACLE
