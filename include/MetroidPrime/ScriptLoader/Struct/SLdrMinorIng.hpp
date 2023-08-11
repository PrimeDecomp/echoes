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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDamageInfo projectileDamage;
    CAssetId projectile;
    float unknown_0xa03e450c;
    float attackAngleLimit;
    float unknown_0x99bb2559;
    float unknown_0xd6c8eac2;
    float unknown_0x2a5449ba;
    float hearingRadius;
    bool unknown_0x399d1eaa;
    bool unknown_0xbce16644;
    bool unknown_0x142433d3;
    bool unknown_0xb6cc0063;
    bool unknown_0xe601f7bd;
    bool unknown_0x09207f51;
    bool unknown_0x2107e4fb;
    SLdrUnknownStruct33 unknownStruct33;
    SLdrUnknownStruct34 unknownStruct34;
};

void LoadTypedefSLdrMinorIng(SLdrMinorIng&, CInputStream&);

#endif // _SLDRMINORING
