#ifndef _SLDRTWEAKTARGETING_SCAN
#define _SLDRTWEAKTARGETING_SCAN

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_Scan {
    SLdrTweakTargeting_Scan();
    ~SLdrTweakTargeting_Scan();

    float scan_lock_scale;
    float scan_lock_transition_time;
    float scan_lock_translation;
    CColor unknown;
    CColor scan_lock_locked_color;
    CColor scan_lock_unlocked_color;
};

void LoadTypedefSLdrTweakTargeting_Scan(SLdrTweakTargeting_Scan&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_SCAN
