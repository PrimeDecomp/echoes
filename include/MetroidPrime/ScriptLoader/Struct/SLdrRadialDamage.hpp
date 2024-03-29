#ifndef _SLDRRADIALDAMAGE
#define _SLDRRADIALDAMAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRadialDamage {
    SLdrRadialDamage();
    ~SLdrRadialDamage();

    SLdrEditorProperties editorProperties;
    SLdrDamageInfo damage;
    float radius;
    bool autoAction;
    bool autoDelete;
    bool originator;
};

void LoadTypedefSLdrRadialDamage(SLdrRadialDamage&, CInputStream&);

#endif // _SLDRRADIALDAMAGE
