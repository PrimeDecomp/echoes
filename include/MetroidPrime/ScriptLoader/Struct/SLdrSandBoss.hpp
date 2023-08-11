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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrSandBossData sandBossData;
};

void LoadTypedefSLdrSandBoss(SLdrSandBoss&, CInputStream&);

#endif // _SLDRSANDBOSS
