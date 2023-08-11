#ifndef _SLDRDAMAGEABLETRIGGERORIENTATED
#define _SLDRDAMAGEABLETRIGGERORIENTATED

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVisorParameters.hpp"

struct SLdrDamageableTriggerOrientated {
    SLdrDamageableTriggerOrientated();
    ~SLdrDamageableTriggerOrientated();

    SLdrEditorProperties editorProperties;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    bool orbitable;
    bool enableSeekerLockOn;
    bool invulnerable;
    SLdrVisorParameters visor;
};

void LoadTypedefSLdrDamageableTriggerOrientated(SLdrDamageableTriggerOrientated&, CInputStream&);

#endif // _SLDRDAMAGEABLETRIGGERORIENTATED
