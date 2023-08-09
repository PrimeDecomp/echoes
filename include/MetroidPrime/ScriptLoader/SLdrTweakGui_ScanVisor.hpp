#ifndef _SLDRTWEAKGUI_SCANVISOR
#define _SLDRTWEAKGUI_SCANVISOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_ScanVisor {
    SLdrTweakGui_ScanVisor();
    ~SLdrTweakGui_ScanVisor();

    CColor inactive_color;
    CColor inactive_external_color;
    CColor non_critical_color;
    CColor critical_color;
    CColor burn_in_color;
    CColor highlight_color;
    CColor critical_highlight_color;
    CColor unknown_0xe8f5018b;
    CColor unknown_0xba1ae1e5;
    CColor unknown_0xb39d450e;
    CColor unknown_0x1042455b;
    CColor unknown_0xd72435ad;
    CColor unknown_0x75cdc913;
    CColor sweep_bar_color;
    float burn_in_time;
    float fade_out_time;
};

void LoadTypedefSLdrTweakGui_ScanVisor(SLdrTweakGui_ScanVisor&, CInputStream&);

#endif // _SLDRTWEAKGUI_SCANVISOR
