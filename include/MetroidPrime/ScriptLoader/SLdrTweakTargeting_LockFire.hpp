#ifndef _SLDRTWEAKTARGETING_LOCKFIRE
#define _SLDRTWEAKTARGETING_LOCKFIRE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_LockFire {
    SLdrTweakTargeting_LockFire();
    ~SLdrTweakTargeting_LockFire();

    float lockFireReticleScale;
    float lockFireAnimTime;
    CColor lockFireColor;
};

void LoadTypedefSLdrTweakTargeting_LockFire(SLdrTweakTargeting_LockFire&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_LOCKFIRE
