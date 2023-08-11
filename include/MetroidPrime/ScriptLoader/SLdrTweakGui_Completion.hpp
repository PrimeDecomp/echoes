#ifndef _SLDRTWEAKGUI_COMPLETION
#define _SLDRTWEAKGUI_COMPLETION

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGui_Completion {
    SLdrTweakGui_Completion();
    ~SLdrTweakGui_Completion();

    rstl::string unknown_0x81fc78c2;
    rstl::string mainFont;
    rstl::string secondaryFont;
    CColor mainFontColor;
    CColor mainFontOutlineColor;
    CColor statsFontColor;
    CColor statsFontOutlineColor;
    CColor unlockFontColor;
    CColor unlockFontOutlineColor;
    float unknown_0xb6fe7398;
    float unknown_0x6af2871b;
    float textStartDelay;
};

void LoadTypedefSLdrTweakGui_Completion(SLdrTweakGui_Completion&, CInputStream&);

#endif // _SLDRTWEAKGUI_COMPLETION
