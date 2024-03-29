#ifndef _SLDRTWEAKGUI_CREDITS
#define _SLDRTWEAKGUI_CREDITS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGui_Credits {
    SLdrTweakGui_Credits();
    ~SLdrTweakGui_Credits();

    rstl::string unknown_0x81fc78c2;
    rstl::string unknown_0x2bcd300d;
    rstl::string alternateFont;
    CColor fontColor;
    CColor fontOutlineColor;
    float totalTime;
    float textFadeTime;
    float movieFadeTime;
};

void LoadTypedefSLdrTweakGui_Credits(SLdrTweakGui_Credits&, CInputStream&);

#endif // _SLDRTWEAKGUI_CREDITS
