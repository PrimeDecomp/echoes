#ifndef _SLDRSHADOWPROJECTOR
#define _SLDRSHADOWPROJECTOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrShadowProjector {
    SLdrShadowProjector();
    ~SLdrShadowProjector();

    SLdrEditorProperties editorProperties;
    float shadowScale;
    CVector3f shadowOffset;
    float shadowHeight;
    float shadowAlpha;
    float shadowFadeTime;
    bool unknown_0xbca8b742;
    int unknown_0x606e341c;
};

void LoadTypedefSLdrShadowProjector(SLdrShadowProjector&, CInputStream&);

#endif // _SLDRSHADOWPROJECTOR
