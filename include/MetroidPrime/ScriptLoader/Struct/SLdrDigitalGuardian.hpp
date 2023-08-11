#ifndef _SLDRDIGITALGUARDIAN
#define _SLDRDIGITALGUARDIAN

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDigitalGuardianData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrDigitalGuardian {
    SLdrDigitalGuardian();
    ~SLdrDigitalGuardian();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDigitalGuardianData digitalGuardianData;
};

void LoadTypedefSLdrDigitalGuardian(SLdrDigitalGuardian&, CInputStream&);

#endif // _SLDRDIGITALGUARDIAN
