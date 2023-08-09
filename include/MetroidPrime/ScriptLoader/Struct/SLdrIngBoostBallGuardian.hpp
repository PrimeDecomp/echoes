#ifndef _SLDRINGBOOSTBALLGUARDIAN
#define _SLDRINGBOOSTBALLGUARDIAN

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct28.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct29.hpp"

struct SLdrIngBoostBallGuardian {
    SLdrIngBoostBallGuardian();
    ~SLdrIngBoostBallGuardian();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrUnknownStruct28 unknown_struct28;
    SLdrUnknownStruct29 unknown_struct29;
};

void LoadTypedefSLdrIngBoostBallGuardian(SLdrIngBoostBallGuardian&, CInputStream&);

#endif // _SLDRINGBOOSTBALLGUARDIAN
