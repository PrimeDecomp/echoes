#ifndef _SLDRINGSPACEJUMPGUARDIAN
#define _SLDRINGSPACEJUMPGUARDIAN

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct32.hpp"

struct SLdrIngSpaceJumpGuardian {
    SLdrIngSpaceJumpGuardian();
    ~SLdrIngSpaceJumpGuardian();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrUnknownStruct32 unknown_struct32;
};

void LoadTypedefSLdrIngSpaceJumpGuardian(SLdrIngSpaceJumpGuardian&, CInputStream&);

#endif // _SLDRINGSPACEJUMPGUARDIAN
