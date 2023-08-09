#ifndef _SLDRVISORFLARE
#define _SLDRVISORFLARE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFlareDef.hpp"

struct SLdrVisorFlare {
    SLdrVisorFlare();
    ~SLdrVisorFlare();

    SLdrEditorProperties editor_properties;
    int blend_mode;
    bool constant_scale;
    float fade_time;
    float fade_factor;
    float rotate_factor;
    int combat_visor_mode;
    bool unknown;
    bool no_occlusion_test;
    SLdrFlareDef flare1;
    SLdrFlareDef flare2;
    SLdrFlareDef flare3;
    SLdrFlareDef flare4;
    SLdrFlareDef flare5;
};

void LoadTypedefSLdrVisorFlare(SLdrVisorFlare&, CInputStream&);

#endif // _SLDRVISORFLARE
