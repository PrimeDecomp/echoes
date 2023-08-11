#ifndef _SLDRUNKNOWNSTRUCT43
#define _SLDRUNKNOWNSTRUCT43

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct42.hpp"

struct SLdrUnknownStruct43 {
    SLdrUnknownStruct43();
    ~SLdrUnknownStruct43();

    int unknown_0xbd80fd94;
    float maxLinearVelocity;
    float maxTurnSpeed;
    float scanningTurnSpeed;
    float unknown_0xe32fcae9;
    float unknown_0xc5e0b92c;
    float unknown_0xc17a8806;
    int unknown_0xe75bae9e;
    CAssetId laserPulseProjectile;
    SLdrDamageInfo laserPulseDamage;
    int unknown_0xeda45014;
    int unknown_0x7dd740fe;
    float dodgeChance;
    float resetShieldTime;
    float splitDestroyedPriority;
    float laserSweepTurnSpeed;
    SLdrDamageInfo laserSweepDamage;
    SLdrPlasmaBeamInfo laserSweepBeamInfo;
    SLdrUnknownStruct42 unknownStruct42;
    int soundLaserSweep;
    int soundLaserChargeUp;
    int soundDocking;
    int soundScanning;
    int soundLightShield;
    int soundDarkShield;
    int soundShieldOn;
    SLdrIngPossessionData ingPossessionData;
    SLdrDamageVulnerability lightShieldVulnerability;
    SLdrDamageVulnerability darkShieldVulnerability;
};

void LoadTypedefSLdrUnknownStruct43(SLdrUnknownStruct43&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT43
