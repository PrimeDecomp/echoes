#ifndef _SLDRTWEAKTARGETING_CHARGE_GAUGE
#define _SLDRTWEAKTARGETING_CHARGE_GAUGE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_Charge_Gauge {
    SLdrTweakTargeting_Charge_Gauge();
    ~SLdrTweakTargeting_Charge_Gauge();

    float unknown_0xd032c2a1;
    float unknown_0xa118e250;
    float unknown_0xdb1ac8ee;
    float unknown_0xecd100f8;
    float chargeGaugeScale;
    CColor chargeGaugeColor;
    int unknown_0xed78e6eb;
    float unknown_0x2c3d9e27;
};

void LoadTypedefSLdrTweakTargeting_Charge_Gauge(SLdrTweakTargeting_Charge_Gauge&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_CHARGE_GAUGE
