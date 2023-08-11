#ifndef _SLDRSWAMPBOSSSTAGE2
#define _SLDRSWAMPBOSSSTAGE2

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage2Data.hpp"

struct SLdrSwampBossStage2 {
    SLdrSwampBossStage2();
    ~SLdrSwampBossStage2();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrIngPossessionData ingPossessionData;
    SLdrSwampBossStage2Data swampBossStage2Data;
};

void LoadTypedefSLdrSwampBossStage2(SLdrSwampBossStage2&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE2
