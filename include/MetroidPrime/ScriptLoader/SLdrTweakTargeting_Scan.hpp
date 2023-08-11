#ifndef _SLDRTWEAKTARGETING_SCAN
#define _SLDRTWEAKTARGETING_SCAN

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_Scan {
    SLdrTweakTargeting_Scan();
    ~SLdrTweakTargeting_Scan();

    float scanLockScale;
    float scanLockTransitionTime;
    float scanLockTranslation;
    CColor unknown;
    CColor scanLockLockedColor;
    CColor scanLockUnlockedColor;
};

void LoadTypedefSLdrTweakTargeting_Scan(SLdrTweakTargeting_Scan&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_SCAN
