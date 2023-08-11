#ifndef _SLDRTWEAKTARGETING_LOCKDAGGER
#define _SLDRTWEAKTARGETING_LOCKDAGGER

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_LockDagger {
    SLdrTweakTargeting_LockDagger();
    ~SLdrTweakTargeting_LockDagger();

    float lockDaggerNormalScale;
    float unknown;
    CColor lockDaggerColor;
    float lockDagger0Angle;
    float lockDagger1Angle;
    float lockDagger2Angle;
};

void LoadTypedefSLdrTweakTargeting_LockDagger(SLdrTweakTargeting_LockDagger&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_LOCKDAGGER
