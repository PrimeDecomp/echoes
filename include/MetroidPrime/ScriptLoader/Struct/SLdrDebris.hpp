#ifndef _SLDRDEBRIS
#define _SLDRDEBRIS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrDebris {
    SLdrDebris();
    ~SLdrDebris();

    SLdrEditorProperties editor_properties;
    float impulse;
    CVector3f impulse_variance;
    CColor fade_out_color;
    float mass;
    float unknown_0x417f4a91;
    float life_time;
    int scale_type;
    bool random_spin;
    CAssetId model;
    SLdrActorParameters actor_information;
    CAssetId particle;
    CVector3f particle_system_scale;
    bool is_collider;
    bool unknown_0x4edb1d0e;
};

void LoadTypedefSLdrDebris(SLdrDebris&, CInputStream&);

#endif // _SLDRDEBRIS
