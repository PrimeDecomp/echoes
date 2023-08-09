#ifndef _SLDRSAFEZONESTRUCTA
#define _SLDRSAFEZONESTRUCTA

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVector2f.hpp"

struct SLdrSafeZoneStructA {
    SLdrSafeZoneStructA();
    ~SLdrSafeZoneStructA();

    bool enabled;
    int mode;
    CColor color;
    SLdrVector2f near_far_plane;
    float color_rate;
    SLdrVector2f distance_rate;
};

void LoadTypedefSLdrSafeZoneStructA(SLdrSafeZoneStructA&, CInputStream&);

#endif // _SLDRSAFEZONESTRUCTA
