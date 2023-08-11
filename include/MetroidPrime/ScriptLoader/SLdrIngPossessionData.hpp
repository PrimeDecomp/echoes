#ifndef _SLDRINGPOSSESSIONDATA
#define _SLDRINGPOSSESSIONDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrIngPossessionData {
    SLdrIngPossessionData();
    ~SLdrIngPossessionData();

    bool isAnEncounter;
    bool unknown_0xb68c0aa3;
    CAssetId ingPossessedModel;
    CAssetId ingPossessedSkinRules;
    CAssetId darkScanInfo;
    SLdrHealthInfo ingPossessedHealth;
    float ingPossessedDamageMultiplier;
    int unknown_0x2befc1bf;
    SLdrDamageVulnerability ingVulnerability;
};

void LoadTypedefSLdrIngPossessionData(SLdrIngPossessionData&, CInputStream&);

#endif // _SLDRINGPOSSESSIONDATA
