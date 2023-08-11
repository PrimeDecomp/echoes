#ifndef _SLDRTWEAKGUI_DARKVISOR
#define _SLDRTWEAKGUI_DARKVISOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_DarkVisor {
    SLdrTweakGui_DarkVisor();
    ~SLdrTweakGui_DarkVisor();

    CColor darkWorldBaseColor;
    CColor darkVisorStaticColor;
    CColor darkVisorPaletteModulate;
    float darkVisorBlurSpeed;
    int darkVisorFrameTop;
    int darkVisorFrameHeight;
    int darkVisorFrameLeft;
    int darkVisorFrameWidth;
};

void LoadTypedefSLdrTweakGui_DarkVisor(SLdrTweakGui_DarkVisor&, CInputStream&);

#endif // _SLDRTWEAKGUI_DARKVISOR
