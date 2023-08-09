#ifndef _SLDRINGSPIDERBALLGUARDIANSTRUCT
#define _SLDRINGSPIDERBALLGUARDIANSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrIngSpiderballGuardianStruct {
    SLdrIngSpiderballGuardianStruct();
    ~SLdrIngSpiderballGuardianStruct();

    float min_patrol_speed;
    float max_patrol_speed;
    float linear_acceleration;
    float angular_speed;
    float unknown;
    float stunned_speed;
    float stunned_time;
    float max_charge_time;
};

void LoadTypedefSLdrIngSpiderballGuardianStruct(SLdrIngSpiderballGuardianStruct&, CInputStream&);

#endif // _SLDRINGSPIDERBALLGUARDIANSTRUCT
