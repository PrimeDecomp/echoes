#ifndef _SLDREMPERORINGSTAGE1TENTACLEDATA
#define _SLDREMPERORINGSTAGE1TENTACLEDATA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrEmperorIngStage1TentacleData {
    SLdrEmperorIngStage1TentacleData();
    ~SLdrEmperorIngStage1TentacleData();

    SLdrHealthInfo health;
    SLdrDamageVulnerability normalVulnerability;
    SLdrDamageVulnerability warpAttackVulnerability;
    SLdrDamageVulnerability meleeAttackVulnerability;
    SLdrDamageVulnerability projectileAttackVulnerability;
    float stayRetractedTime;
    int tentacleDamagedSound;
};

void LoadTypedefSLdrEmperorIngStage1TentacleData(SLdrEmperorIngStage1TentacleData&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE1TENTACLEDATA
