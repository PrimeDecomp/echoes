#ifndef _SLDRTWEAKGUI_ECHOVISOR
#define _SLDRTWEAKGUI_ECHOVISOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_EchoVisor {
    SLdrTweakGui_EchoVisor();
    ~SLdrTweakGui_EchoVisor();

    CColor echo_base_color;
    CColor echo_outline_color;
    CColor echo_damage_color;
    CColor unknown_0xa288fe51;
    int unknown_0xf316c617;
    float unknown_0xb56d2286;
    float unknown_0xc5c2d8fc;
    float unknown_0x47d948b1;
    float unknown_0x5708b903;
    float unknown_0xafc8c571;
    float unknown_0x28bffdc5;
    CColor echo_ring_color;
    float unknown_0x99a65ca7;
    float unknown_0x51f19dea;
};

void LoadTypedefSLdrTweakGui_EchoVisor(SLdrTweakGui_EchoVisor&, CInputStream&);

#endif // _SLDRTWEAKGUI_ECHOVISOR
