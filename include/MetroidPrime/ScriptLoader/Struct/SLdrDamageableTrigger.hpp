#ifndef _SLDRDAMAGEABLETRIGGER
#define _SLDRDAMAGEABLETRIGGER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVisorParameters.hpp"

struct SLdrDamageableTrigger {
    SLdrDamageableTrigger();
    ~SLdrDamageableTrigger();

    SLdrEditorProperties editor_properties;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    bool orbitable;
    bool enable_seeker_lock_on;
    bool invulnerable;
    SLdrVisorParameters visor;
};

void LoadTypedefSLdrDamageableTrigger(SLdrDamageableTrigger&, CInputStream&);

#endif // _SLDRDAMAGEABLETRIGGER
