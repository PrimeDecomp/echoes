#ifndef _SLDRMINORING
#define _SLDRMINORING

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct33.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct34.hpp"

struct SLdrMinorIng {
    SLdrMinorIng();
    ~SLdrMinorIng();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDamageInfo projectile_damage;
    CAssetId projectile;
    float unknown_0xa03e450c;
    float attack_angle_limit;
    float unknown_0x99bb2559;
    float unknown_0xd6c8eac2;
    float unknown_0x2a5449ba;
    float hearing_radius;
    bool unknown_0x399d1eaa;
    bool unknown_0xbce16644;
    bool unknown_0x142433d3;
    bool unknown_0xb6cc0063;
    bool unknown_0xe601f7bd;
    bool unknown_0x09207f51;
    bool unknown_0x2107e4fb;
    SLdrUnknownStruct33 unknown_struct33;
    SLdrUnknownStruct34 unknown_struct34;
};

void LoadTypedefSLdrMinorIng(SLdrMinorIng&, CInputStream&);

#endif // _SLDRMINORING
