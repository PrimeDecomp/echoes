#ifndef _SLDRAREADAMAGE
#define _SLDRAREADAMAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAreaDamage {
    SLdrAreaDamage();
    ~SLdrAreaDamage();

    SLdrEditorProperties editorProperties;
    SLdrDamageInfo damage;
    float pulseTime;
    float graceTime;
};

void LoadTypedefSLdrAreaDamage(SLdrAreaDamage&, CInputStream&);

#endif // _SLDRAREADAMAGE
