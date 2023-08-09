#ifndef _SLDRRADIALDAMAGE
#define _SLDRRADIALDAMAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRadialDamage {
    SLdrRadialDamage();
    ~SLdrRadialDamage();

    SLdrEditorProperties editor_properties;
    SLdrDamageInfo damage;
    float radius;
    bool auto_action;
    bool auto_delete;
    bool originator;
};

void LoadTypedefSLdrRadialDamage(SLdrRadialDamage&, CInputStream&);

#endif // _SLDRRADIALDAMAGE
