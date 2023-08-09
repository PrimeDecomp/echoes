#ifndef _SLDRSHADOWPROJECTOR
#define _SLDRSHADOWPROJECTOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrShadowProjector {
    SLdrShadowProjector();
    ~SLdrShadowProjector();

    SLdrEditorProperties editor_properties;
    float shadow_scale;
    CVector3f shadow_offset;
    float shadow_height;
    float shadow_alpha;
    float shadow_fade_time;
    bool unknown_0xbca8b742;
    int unknown_0x606e341c;
};

void LoadTypedefSLdrShadowProjector(SLdrShadowProjector&, CInputStream&);

#endif // _SLDRSHADOWPROJECTOR
