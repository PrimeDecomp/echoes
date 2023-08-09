#ifndef _SLDRTWEAKGUI_VISORCOLORSCHEMETYPEDEF
#define _SLDRTWEAKGUI_VISORCOLORSCHEMETYPEDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_VisorColorSchemeTypedef {
    SLdrTweakGui_VisorColorSchemeTypedef();
    ~SLdrTweakGui_VisorColorSchemeTypedef();

    CColor hud_hue;
    CColor unknown;
};

void LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(SLdrTweakGui_VisorColorSchemeTypedef&, CInputStream&);

#endif // _SLDRTWEAKGUI_VISORCOLORSCHEMETYPEDEF
