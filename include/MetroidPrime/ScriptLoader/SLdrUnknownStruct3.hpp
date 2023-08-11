#ifndef _SLDRUNKNOWNSTRUCT3
#define _SLDRUNKNOWNSTRUCT3

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrUnknownStruct3 {
    SLdrUnknownStruct3();
    ~SLdrUnknownStruct3();

    float unknown_0x17cd8b2a;
    float unknown_0x1473dad2;
    float unknown_0x3650ce75;
    float unknown_0x78520e6e;
    float damageAngle;
    float horizSpeed;
    float vertSpeed;
    float fireRate;
    float unknown_0xf9bd253e;
    float maxAttackAngle;
    float maxAttackRange;
    float startAttackRange;
    float attackLeashTimer;
    SLdrDamageInfo weaponDamage;
    CAssetId weaponEffect;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    CAssetId stateMachine;
    CAssetId telegraphEffect;
};

void LoadTypedefSLdrUnknownStruct3(SLdrUnknownStruct3&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT3
