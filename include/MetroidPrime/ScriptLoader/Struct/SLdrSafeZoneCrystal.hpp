#ifndef _SLDRSAFEZONECRYSTAL
#define _SLDRSAFEZONECRYSTAL

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSafeZoneCrystal {
    SLdrSafeZoneCrystal();
    ~SLdrSafeZoneCrystal();

    SLdrEditorProperties editor_properties;
    SLdrActorParameters actor_parameters;
    CAssetId scannable_info_collapsed;
    CAssetId scannable_info_entangled;
    CAssetId scannable_info_light;
    CAssetId scannable_info_annihilator;
    int safezone_type;
    bool initially_entangled;
    CAssetId collapsed_effect;
    CAssetId expanded_effect;
    CAssetId entangled_effect;
    CAssetId part;
    CAssetId echo_effect;
    CAssetId normal_crystal;
    CAssetId entangled_crystal;
    CAssetId energized_model;
    CAssetId echo_crystal;
    float max_time_expanded;
    float max_time_entangled;
    float unknown_0xf0a45c32;
    float unknown_0xd8116003;
    float unknown_0x415046ed;
    float unknown_0xec9c01b2;
    float unknown_0x545540e5;
    CAssetId power_beam_refresh_effect;
    CVector3f hit_radius;
    CVector3f hit_offset;
    CVector3f effect_offset;
    spline unknown_0xbbbee60b;
};

void LoadTypedefSLdrSafeZoneCrystal(SLdrSafeZoneCrystal&, CInputStream&);

#endif // _SLDRSAFEZONECRYSTAL
