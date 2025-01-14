#ifndef _SLDREFFECT
#define _SLDREFFECT

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrLightParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrEffect {
    SLdrEffect();
    ~SLdrEffect();

    SLdrEditorProperties editorProperties;
    CAssetId particleEffect;
    bool unknown_0x3df5a489;
    bool restartOnActivate;
    bool unknown_0xee538174;
    float unknown_0xa94b0efd;
    float unknown_0x93756968;
    float unknown_0x0b94597d;
    float unknown_0xd0e8a496;
    bool unknown_0xa8bb6c61;
    float unknown_0x7589d549;
    float unknown_0xa7d7d767;
    float unknown_0xfe69615c;
    bool unknown_0x88d914a6;
    bool visibleInDark;
    bool visibleInEcho;
    bool unknown_0x6714021c;
    bool unknown_0xbe931927;
    int renderOrder;
    SLdrLightParameters lighting;
    bool motionSplinePathLoops;
    SLdrSplineType motionSplineType;
    SLdrSpline motionControlSpline;
    float motionSplineDuration;
    bool unknown_0x73e63382;
    bool unknown_0x608ecac5;
};

void LoadTypedefSLdrEffect(SLdrEffect&, CInputStream&);

#endif // _SLDREFFECT
