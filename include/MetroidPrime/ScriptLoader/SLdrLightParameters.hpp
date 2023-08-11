#ifndef _SLDRLIGHTPARAMETERS
#define _SLDRLIGHTPARAMETERS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrLightParameters {
    SLdrLightParameters();
    ~SLdrLightParameters();

    bool castShadow;
    float shadowScale;
    int unknown_0xecda4163;
    float shadowAlpha;
    float maxShadowHeight;
    CColor ambientColor;
    bool unknown_0xa71810e9;
    int worldLightingOptions;
    int lightRecalculation;
    CVector3f lightingPositionOffset;
    int numDynamicLights;
    int numAreaLights;
    bool useOldLighting;
    bool ignoreAmbientLighting;
    int useLightSet;
};

void LoadTypedefSLdrLightParameters(SLdrLightParameters&, CInputStream&);

#endif // _SLDRLIGHTPARAMETERS
