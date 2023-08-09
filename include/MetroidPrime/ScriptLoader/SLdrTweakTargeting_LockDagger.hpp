#ifndef _SLDRTWEAKTARGETING_LOCKDAGGER
#define _SLDRTWEAKTARGETING_LOCKDAGGER

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_LockDagger {
    SLdrTweakTargeting_LockDagger();
    ~SLdrTweakTargeting_LockDagger();

    float lock_dagger_normal_scale;
    float unknown;
    CColor lock_dagger_color;
    float lock_dagger0_angle;
    float lock_dagger1_angle;
    float lock_dagger2_angle;
};

void LoadTypedefSLdrTweakTargeting_LockDagger(SLdrTweakTargeting_LockDagger&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_LOCKDAGGER
