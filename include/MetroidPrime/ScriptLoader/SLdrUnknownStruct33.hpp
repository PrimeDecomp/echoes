#ifndef _SLDRUNKNOWNSTRUCT33
#define _SLDRUNKNOWNSTRUCT33

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"

struct SLdrUnknownStruct33 {
    SLdrUnknownStruct33();
    ~SLdrUnknownStruct33();

    SLdrDamageInfo damage;
    float bombStunDuration;
    float unknown_0x46aaced3;
    float maxSpeed;
    float maxWallSpeed;
    float ballPursuitSpeed;
    float speedModifier;
    float turnSpeed;
    CAssetId blobEffect;
    CAssetId hitNormalDamage;
    CAssetId hitHeavyDamage;
    CAssetId death;
    int soundIdle;
    int soundMove;
    int sound_0xb392943a;
    int sound_0x24ecc1e9;
    int soundDeath;
    float unknown_0x7569fdba;
    float unknown_0xd55938d2;
    SLdrDamageVulnerability vulnerability;
};

void LoadTypedefSLdrUnknownStruct33(SLdrUnknownStruct33&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT33
