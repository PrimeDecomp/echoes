#ifndef _SLDREMPERORINGSTAGE2TENTACLEDATA
#define _SLDREMPERORINGSTAGE2TENTACLEDATA

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrEmperorIngStage2TentacleData {
    SLdrEmperorIngStage2TentacleData();
    ~SLdrEmperorIngStage2TentacleData();

    float detection_time;
    float forget_time;
};

void LoadTypedefSLdrEmperorIngStage2TentacleData(SLdrEmperorIngStage2TentacleData&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE2TENTACLEDATA
