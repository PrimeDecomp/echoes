#ifndef _SLDRSWAMPBOSSSTAGE1
#define _SLDRSWAMPBOSSSTAGE1

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct37.hpp"

struct SLdrSwampBossStage1 {
    SLdrSwampBossStage1();
    ~SLdrSwampBossStage1();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrUnknownStruct37 unknownStruct37;
};

void LoadTypedefSLdrSwampBossStage1(SLdrSwampBossStage1&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE1
