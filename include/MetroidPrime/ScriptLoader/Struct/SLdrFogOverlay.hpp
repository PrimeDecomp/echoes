#ifndef _SLDRFOGOVERLAY
#define _SLDRFOGOVERLAY

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFogOverlay {
    SLdrFogOverlay();
    ~SLdrFogOverlay();

    SLdrEditorProperties editorProperties;
    float fullAlpha;
    float fadeDownTime;
    float fadeUpTime;
    bool startFadedOut;
    CColor color;
    float ambientRadiusX;
    float ambientRadiusY;
    float ambientSpeed;
    float ambientSpeedTarget;
    float unknown_0x6a111b96;
    float unknown_0xff226ea3;
    CVector3f unknown_0x2190ab0a;
    float unknown_0x9f19f0af;
    float unknown_0x90c10fe7;
    float unknown_0xd8daff1d;
};

void LoadTypedefSLdrFogOverlay(SLdrFogOverlay&, CInputStream&);

#endif // _SLDRFOGOVERLAY
