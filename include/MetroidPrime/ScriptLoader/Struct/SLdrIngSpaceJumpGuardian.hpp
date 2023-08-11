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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrUnknownStruct32 unknownStruct32;
};

void LoadTypedefSLdrIngSpaceJumpGuardian(SLdrIngSpaceJumpGuardian&, CInputStream&);

#endif // _SLDRINGSPACEJUMPGUARDIAN
