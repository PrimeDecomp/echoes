#ifndef _SLDRLIGHTPARAMETERS
#define _SLDRLIGHTPARAMETERS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrLightParameters {
    SLdrLightParameters();
    ~SLdrLightParameters();

    bool cast_shadow;
    float shadow_scale;
    int unknown_0xecda4163;
    float shadow_alpha;
    float max_shadow_height;
    CColor ambient_color;
    bool unknown_0xa71810e9;
    int world_lighting_options;
    int light_recalculation;
    CVector3f lighting_position_offset;
    int num_dynamic_lights;
    int num_area_lights;
    bool use_old_lighting;
    bool ignore_ambient_lighting;
    int use_light_set;
};

void LoadTypedefSLdrLightParameters(SLdrLightParameters&, CInputStream&);

#endif // _SLDRLIGHTPARAMETERS
