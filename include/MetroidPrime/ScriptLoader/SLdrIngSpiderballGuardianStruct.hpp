#ifndef _SLDRINGSPIDERBALLGUARDIANSTRUCT
#define _SLDRINGSPIDERBALLGUARDIANSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrIngSpiderballGuardianStruct {
    SLdrIngSpiderballGuardianStruct();
    ~SLdrIngSpiderballGuardianStruct();

    float minPatrolSpeed;
    float maxPatrolSpeed;
    float linearAcceleration;
    float angularSpeed;
    float unknown;
    float stunnedSpeed;
    float stunnedTime;
    float maxChargeTime;
};

void LoadTypedefSLdrIngSpiderballGuardianStruct(SLdrIngSpiderballGuardianStruct&, CInputStream&);

#endif // _SLDRINGSPIDERBALLGUARDIANSTRUCT
