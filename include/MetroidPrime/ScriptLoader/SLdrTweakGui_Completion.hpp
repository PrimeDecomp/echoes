#ifndef _SLDRTWEAKGUI_COMPLETION
#define _SLDRTWEAKGUI_COMPLETION

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGui_Completion {
    SLdrTweakGui_Completion();
    ~SLdrTweakGui_Completion();

    rstl::string unknown_0x81fc78c2;
    rstl::string main_font;
    rstl::string secondary_font;
    CColor main_font_color;
    CColor main_font_outline_color;
    CColor stats_font_color;
    CColor stats_font_outline_color;
    CColor unlock_font_color;
    CColor unlock_font_outline_color;
    float unknown_0xb6fe7398;
    float unknown_0x6af2871b;
    float text_start_delay;
};

void LoadTypedefSLdrTweakGui_Completion(SLdrTweakGui_Completion&, CInputStream&);

#endif // _SLDRTWEAKGUI_COMPLETION
