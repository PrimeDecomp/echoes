#ifndef _SLDRTWEAKGUI_SCANVISOR
#define _SLDRTWEAKGUI_SCANVISOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_ScanVisor {
    SLdrTweakGui_ScanVisor();
    ~SLdrTweakGui_ScanVisor();

    CColor inactiveColor;
    CColor inactiveExternalColor;
    CColor nonCriticalColor;
    CColor criticalColor;
    CColor burnInColor;
    CColor highlightColor;
    CColor criticalHighlightColor;
    CColor unknown_0xe8f5018b;
    CColor unknown_0xba1ae1e5;
    CColor unknown_0xb39d450e;
    CColor unknown_0x1042455b;
    CColor unknown_0xd72435ad;
    CColor unknown_0x75cdc913;
    CColor sweepBarColor;
    float burnInTime;
    float fadeOutTime;
};

void LoadTypedefSLdrTweakGui_ScanVisor(SLdrTweakGui_ScanVisor&, CInputStream&);

#endif // _SLDRTWEAKGUI_SCANVISOR
