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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrUnknownStruct31 unknown_struct31;
};

void LoadTypedefSLdrIngSpiderballGuardian(SLdrIngSpiderballGuardian&, CInputStream&);

#endif // _SLDRINGSPIDERBALLGUARDIAN
