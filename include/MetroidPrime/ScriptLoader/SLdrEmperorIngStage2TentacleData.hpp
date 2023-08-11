#ifndef _SLDREMPERORINGSTAGE2TENTACLEDATA
#define _SLDREMPERORINGSTAGE2TENTACLEDATA

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrEmperorIngStage2TentacleData {
    SLdrEmperorIngStage2TentacleData();
    ~SLdrEmperorIngStage2TentacleData();

    float detectionTime;
    float forgetTime;
};

void LoadTypedefSLdrEmperorIngStage2TentacleData(SLdrEmperorIngStage2TentacleData&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE2TENTACLEDATA
