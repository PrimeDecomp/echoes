#ifndef _SLDRINGPOSSESSIONDATA
#define _SLDRINGPOSSESSIONDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrIngPossessionData {
    SLdrIngPossessionData();
    ~SLdrIngPossessionData();

    bool is_an_encounter;
    bool unknown_0xb68c0aa3;
    CAssetId ing_possessed_model;
    CAssetId ing_possessed_skin_rules;
    CAssetId dark_scan_info;
    SLdrHealthInfo ing_possessed_health;
    float ing_possessed_damage_multiplier;
    int unknown_0x2befc1bf;
    SLdrDamageVulnerability ing_vulnerability;
};

void LoadTypedefSLdrIngPossessionData(SLdrIngPossessionData&, CInputStream&);

#endif // _SLDRINGPOSSESSIONDATA
