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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDigitalGuardianHeadData digital_guardian_head_data;
};

void LoadTypedefSLdrDigitalGuardianHead(SLdrDigitalGuardianHead&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANHEAD
