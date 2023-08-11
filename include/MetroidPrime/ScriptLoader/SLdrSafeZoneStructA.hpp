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
    SLdrVector2f nearFarPlane;
    float colorRate;
    SLdrVector2f distanceRate;
};

void LoadTypedefSLdrSafeZoneStructA(SLdrSafeZoneStructA&, CInputStream&);

#endif // _SLDRSAFEZONESTRUCTA
