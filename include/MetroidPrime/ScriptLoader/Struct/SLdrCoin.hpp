#ifndef _SLDRCOIN
#define _SLDRCOIN

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCoin {
    SLdrCoin();
    ~SLdrCoin();

    SLdrEditorProperties editor_properties;
    float cone_spread;
    float minimum_speed;
    float maximum_speed;
    float minimum_spin_speed;
    float maximum_spin_speed;
    float minimum_life_time;
    float maximum_life_time;
    float disable_collision_time;
    float fade_in_end_percentage;
    float fade_out_start_percentage;
    CColor start_color;
    CColor end_color;
    float scale_start_percentage;
    CVector3f final_scale;
    float unknown_0x417f4a91;
    float gravity;
    CVector3f position_offset;
    CAssetId model;
    SLdrActorParameters actor_information;
    CAssetId particle1;
    int bounce_sound;
    int max_bounce_sounds;
    float unknown_0x76c79503;
    float unknown_0x310dfac8;
    CVector3f particle_system1_scale;
    bool particle_system1_uses_global_translation;
    bool particle_system1_wait_for_particles_to_die;
    int particle_system1_orientation;
    CAssetId particle2;
    CVector3f particle_system2_scale;
    bool particle_system2_uses_global_translation;
    bool particle_system2_wait_for_particles_to_die;
    int particle_system2_orientation;
    CAssetId death_particle;
    CVector3f death_particle_system_scale;
    int death_particle_system_orientation;
    bool is_collider;
    bool is_shootable;
    bool die_on_collision;
    bool unknown_0xdcaa0f22;
    bool unknown_0xbfd82a19;
    bool unknown_0x723d42d6;
    float disable_physics_threshold;
};

void LoadTypedefSLdrCoin(SLdrCoin&, CInputStream&);

#endif // _SLDRCOIN
