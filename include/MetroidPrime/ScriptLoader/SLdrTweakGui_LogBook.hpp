#ifndef _SLDRTWEAKGUI_LOGBOOK
#define _SLDRTWEAKGUI_LOGBOOK

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_LogBook {
    SLdrTweakGui_LogBook();
    ~SLdrTweakGui_LogBook();

    CColor mainWindowBorderColor;
    CColor mainWindowTextColor;
    CColor mainWindowSelectedTextColor;
    CColor unknown_0xfe98f30e;
    CColor unknown_0x39e57ac0;
    CColor legendBackgroundColor;
    CColor nodeColor;
    CColor selectedNodeColor;
    CColor parentNodeColor;
    CColor unknown_0x56843943;
    CColor selectedTextCursorColor;
    float unknown_0x905ce2c0;
    float textScale;
    float selectedTextScale;
    float transitionTime;
    spline nodeCollapseMotion;
    spline selectedNodeCollapseMotion;
    spline nodeExpandMotion;
    float rotationSpeed;
    float nodeScale;
    float selectedNodeScale;
    float selectedTextCursorScale;
    float scanModelScale;
    spline unknown_0x925b9d4a;
    CColor unknown_0x075a0734;
    float unknown_0x8ef43309;
    float fogNear;
    float fogFar;
    CColor fogColor;
    float unknown_0xeef15783;
    float unknown_0x78055ab2;
    float unknown_0x09e3197f;
    float unknown_0xfaffce1f;
    float unknown_0xe5280e7c;
    CColor backgroundSweepColor;
    float backgroundSweepRadius;
    float backgroundSweepTime;
    CColor scanTextWindowBackgroundColor;
    CColor scanTextWindowBorderColor;
    CColor scanTextWindowFontColor;
    CColor legendWindowBackgroundColor;
    CColor legendWindowBorderColor;
    CColor legendWindowFontColor;
    float legendHideTime;
    CColor unknown_0x7f35c573;
    CColor unknown_0xe0d77cf1;
    CColor unknown_0x903d9b7e;
    CColor unknown_0x0fdf22fc;
    CColor unknown_0xbeaf9d6b;
    CColor unknown_0x7e8247d6;
    CColor unknown_0x8b3f7902;
    CColor unknown_0xdc1abb82;
    CColor unknown_0x306b2b24;
    CColor frameColor;
    CColor unknown_0x1a4b8068;
    CColor sliderBackgroundColor;
    CColor sliderSelectionColor;
    float sliderScale;
    float sliderSpeed;
    float unknown_0x58795865;
    float unknown_0x35e0bd31;
    CColor menuOptionColor;
    CColor menuOptionEnabledArrowColor;
    CColor menuOptionDisabledArrowColor;
    float menuOptionScale;
    float menuOptionArrowScale;
    float modelRotationSpeed;
    float unknown_0x60914b79;
    float unknown_0x1fd44d9b;
    CVector3f modelLight1Position;
    CColor modelLight1Color;
    CVector3f modelLight2Position;
    CColor modelLight2Color;
    CColor modelAmbientLightColor;
};

void LoadTypedefSLdrTweakGui_LogBook(SLdrTweakGui_LogBook&, CInputStream&);

#endif // _SLDRTWEAKGUI_LOGBOOK
