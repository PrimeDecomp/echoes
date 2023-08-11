#ifndef _SLDRTWEAKGUICOLORS_MULTIPLAYER
#define _SLDRTWEAKGUICOLORS_MULTIPLAYER

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGuiColors_Multiplayer {
    SLdrTweakGuiColors_Multiplayer();
    ~SLdrTweakGuiColors_Multiplayer();

    CColor scoreTextColor;
    CColor unknown_0xa09caefe;
    CColor timerTextColor;
    CColor timerTextBlinkColor;
    CColor unknown_0xec4197e3;
    CColor unknown_0x823e2fb3;
    CColor unknown_0x95cc4ed8;
    CColor unknown_0xdb2ca6ff;
    CColor lockonIndicatorOnColor;
    CColor lockonIndicatorOffColor;
};

void LoadTypedefSLdrTweakGuiColors_Multiplayer(SLdrTweakGuiColors_Multiplayer&, CInputStream&);

#endif // _SLDRTWEAKGUICOLORS_MULTIPLAYER
