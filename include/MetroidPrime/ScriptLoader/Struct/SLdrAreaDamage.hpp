#ifndef _SLDRAREADAMAGE
#define _SLDRAREADAMAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAreaDamage {
    SLdrAreaDamage();
    ~SLdrAreaDamage();

    SLdrEditorProperties editor_properties;
    SLdrDamageInfo damage;
    float pulse_time;
    float grace_time;
};

void LoadTypedefSLdrAreaDamage(SLdrAreaDamage&, CInputStream&);

#endif // _SLDRAREADAMAGE
