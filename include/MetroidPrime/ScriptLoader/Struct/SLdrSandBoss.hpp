#ifndef _SLDRSANDBOSS
#define _SLDRSANDBOSS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSandBossData.hpp"

struct SLdrSandBoss {
    SLdrSandBoss();
    ~SLdrSandBoss();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrSandBossData sand_boss_data;
};

void LoadTypedefSLdrSandBoss(SLdrSandBoss&, CInputStream&);

#endif // _SLDRSANDBOSS
