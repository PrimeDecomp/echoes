#ifndef _SLDRSPLITTERMAINCHASSISDATA
#define _SLDRSPLITTERMAINCHASSISDATA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"

struct SLdrSplitterMainChassisData {
    SLdrSplitterMainChassisData();
    ~SLdrSplitterMainChassisData();

    int unknown_0xcef5c2fe;
    float leg_stab_attack_interval;
    float unknown_0xf6047d40;
    float unknown_0x5130fd39;
    SLdrDamageInfo leg_stab_damage;
    float min_dodge_interval;
    float dodge_chance;
    float deployment_speed;
    float scan_duration;
    float laser_sweep_interval;
    float unknown_0xb3ea58f8;
    float unknown_0x14ded881;
    float unknown_0x35eedd1c;
    float unknown_0x2dde6bfb;
    float unknown_0x8ae1ee93;
    float unknown_0x5027d1aa;
    float spin_attack_interval;
    float unknown_0xf65e430f;
    float unknown_0x43722555;
    float unknown_0x8935377c;
    float unknown_0x2e01b705;
    int unknown_0xd5f34476;
    float unknown_0x21296bdc;
    SLdrDamageInfo spin_attack_damage;
    int sound_alerted;
    SLdrIngPossessionData ing_possession_data;
    SLdrDamageVulnerability spin_attack_vulnerability;
};

void LoadTypedefSLdrSplitterMainChassisData(SLdrSplitterMainChassisData&, CInputStream&);

#endif // _SLDRSPLITTERMAINCHASSISDATA
