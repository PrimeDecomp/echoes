#ifndef _SLDRFISHCLOUD
#define _SLDRFISHCLOUD

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFishCloud {
    SLdrFishCloud();
    ~SLdrFishCloud();

    SLdrEditorProperties editor_properties;
    bool active;
    CAssetId fish_model;
    SLdrAnimationParameters animation_information;
    float fish_count;
    float speed;
    float influence_distance;
    float unknown_0x61959f0d;
    float alignment_priority;
    float separation_priority;
    float projectile_priority;
    float player_priority;
    float containment_priority;
    float wander_priority;
    float wander_amount;
    float player_ball_priority;
    float player_ball_distance;
    float projectile_decay_rate;
    float player_decay_rate;
    float look_ahead_time;
    int update_frame;
    CColor material_color;
    bool can_be_killed;
    float collision_radius;
    CAssetId death_effect0;
    int death_effect0_count;
    CAssetId death_effect1;
    int death_effect1_count;
    CAssetId death_effect2;
    int death_effect2_count;
    CAssetId death_effect3;
    int death_effect3_count;
    int death_sound;
    bool unknown_0xc320a050;
    bool unknown_0xcd4c81a1;
};

void LoadTypedefSLdrFishCloud(SLdrFishCloud&, CInputStream&);

#endif // _SLDRFISHCLOUD
