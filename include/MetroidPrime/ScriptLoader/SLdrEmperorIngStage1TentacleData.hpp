#ifndef _SLDREMPERORINGSTAGE1TENTACLEDATA
#define _SLDREMPERORINGSTAGE1TENTACLEDATA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrEmperorIngStage1TentacleData {
    SLdrEmperorIngStage1TentacleData();
    ~SLdrEmperorIngStage1TentacleData();

    SLdrHealthInfo health;
    SLdrDamageVulnerability normal_vulnerability;
    SLdrDamageVulnerability warp_attack_vulnerability;
    SLdrDamageVulnerability melee_attack_vulnerability;
    SLdrDamageVulnerability projectile_attack_vulnerability;
    float stay_retracted_time;
    int tentacle_damaged_sound;
};

void LoadTypedefSLdrEmperorIngStage1TentacleData(SLdrEmperorIngStage1TentacleData&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE1TENTACLEDATA
