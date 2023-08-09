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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDigitalGuardianData digital_guardian_data;
};

void LoadTypedefSLdrDigitalGuardian(SLdrDigitalGuardian&, CInputStream&);

#endif // _SLDRDIGITALGUARDIAN
