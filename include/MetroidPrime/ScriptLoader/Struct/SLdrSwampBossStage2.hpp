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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrIngPossessionData ing_possession_data;
    SLdrSwampBossStage2Data swamp_boss_stage2_data;
};

void LoadTypedefSLdrSwampBossStage2(SLdrSwampBossStage2&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE2
