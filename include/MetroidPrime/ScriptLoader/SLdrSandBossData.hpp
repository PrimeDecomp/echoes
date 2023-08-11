#ifndef _SLDRSANDBOSSDATA
#define _SLDRSANDBOSSDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSandBossStructA.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct40.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct41.hpp"

struct SLdrSandBossData {
    SLdrSandBossData();
    ~SLdrSandBossData();

    CAssetId scannableInfo1;
    int commandIndex;
    CAssetId crackedSphere1;
    CAssetId crackedSphere2;
    CAssetId crackedSphere3;
    SLdrDamageInfo snapJawDamage;
    SLdrDamageInfo spitOutDamage;
    float unknown_0xbf88fe4f;
    float unknown_0x74c702b3;
    CAssetId darkBeamProjectile;
    SLdrDamageInfo darkBeamDamage;
    float unknown_0x2b42dddf;
    float unknown_0x1562e0d6;
    float unknown_0xd0db2574;
    float suckAirTime;
    float suckMorphballRange;
    float spitMorphballTime;
    CAssetId part;
    SLdrUnknownStruct40 unknownStruct40;
    SLdrUnknownStruct41 unknownStruct41;
    SLdrSandBossStructA sandBossStructA_0x8b452a19;
    SLdrSandBossStructA sandBossStructA_0x0cf8c54c;
    CAssetId modelWithTailArmor;
    CAssetId skinForArmoredTail;
    SLdrDamageVulnerability damageVulnerability;
    SLdrDamageVulnerability stampedeVulnerability;
    SLdrDamageVulnerability suckAirVulnerability;
};

void LoadTypedefSLdrSandBossData(SLdrSandBossData&, CInputStream&);

#endif // _SLDRSANDBOSSDATA
