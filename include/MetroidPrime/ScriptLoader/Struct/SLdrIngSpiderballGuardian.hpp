#ifndef _SLDRINGSPIDERBALLGUARDIAN
#define _SLDRINGSPIDERBALLGUARDIAN

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct31.hpp"

struct SLdrIngSpiderballGuardian {
    SLdrIngSpiderballGuardian();
    ~SLdrIngSpiderballGuardian();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrUnknownStruct31 unknownStruct31;
};

void LoadTypedefSLdrIngSpiderballGuardian(SLdrIngSpiderballGuardian&, CInputStream&);

#endif // _SLDRINGSPIDERBALLGUARDIAN
