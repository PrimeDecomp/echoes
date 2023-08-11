#ifndef _SLDRDIGITALGUARDIANHEAD
#define _SLDRDIGITALGUARDIANHEAD

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDigitalGuardianHeadData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrDigitalGuardianHead {
    SLdrDigitalGuardianHead();
    ~SLdrDigitalGuardianHead();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDigitalGuardianHeadData digitalGuardianHeadData;
};

void LoadTypedefSLdrDigitalGuardianHead(SLdrDigitalGuardianHead&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANHEAD
