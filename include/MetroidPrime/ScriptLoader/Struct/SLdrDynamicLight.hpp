#ifndef _SLDRDYNAMICLIGHT
#define _SLDRDYNAMICLIGHT

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDynamicLightFalloff.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDynamicLightIntensity.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDynamicLightMotionSpline.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDynamicLightParent.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDynamicLightSpotlight.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrDynamicLight {
    SLdrDynamicLight();
    ~SLdrDynamicLight();

    SLdrEditorProperties editorProperties;
    int lightType;
    int lightSet;
    CColor color;
    SLdrDynamicLightIntensity intensity;
    SLdrDynamicLightFalloff falloff;
    SLdrDynamicLightSpotlight spotlight;
    SLdrDynamicLightMotionSpline motionSpline;
    SLdrDynamicLightParent parent;
};

void LoadTypedefSLdrDynamicLight(SLdrDynamicLight&, CInputStream&);

#endif // _SLDRDYNAMICLIGHT
