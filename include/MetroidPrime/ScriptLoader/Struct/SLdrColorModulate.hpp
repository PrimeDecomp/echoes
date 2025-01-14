#ifndef _SLDRCOLORMODULATE
#define _SLDRCOLORMODULATE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrColorModulate {
    SLdrColorModulate();
    ~SLdrColorModulate();

    SLdrEditorProperties editorProperties;
    CColor colorA;
    CColor colorB;
    int blendMode;
    float timeA2B;
    float timeB2A;
    bool doReverse;
    bool resetTargetWhenDone;
    bool depthCompare;
    bool depthUpdate;
    bool depthBackwards;
    bool autoStart;
    bool updateTime;
    bool loopForever;
    bool externalTime;
    bool copyModelColorToColorA;
    SLdrSpline controlSpline;
};

void LoadTypedefSLdrColorModulate(SLdrColorModulate&, CInputStream&);

#endif // _SLDRCOLORMODULATE
