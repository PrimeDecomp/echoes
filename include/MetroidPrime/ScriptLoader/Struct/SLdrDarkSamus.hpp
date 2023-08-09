#ifndef _SLDRDARKSAMUS
#define _SLDRDARKSAMUS

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrDarkSamus {
    SLdrDarkSamus();
    ~SLdrDarkSamus();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float unknown_0x72edeb7d;
    float unknown_0x74fa22f0;
    int glide_sound;
    int missile_ricochet_sound;
    bool unknown_0x6689925b;
    CAssetId txtr_0x3863160b;
    float unknown_0x2c6a3344;
    SLdrDamageInfo melee_attack_damage;
    CAssetId part_0x6d40aa56;
    CAssetId part_0x9603a544;
    SLdrDamageInfo dive_attack_damage;
    float unknown_0x4aa5dd62;
    float unknown_0x1ad7dc21;
    CAssetId dive_attack_effect;
    CAssetId scatter_shot_projectile;
    CAssetId scatter_shot_projectile2;
    SLdrDamageInfo scatter_shot_damage;
    float unknown_0x378285b9;
    float unknown_0xd242e25f;
    CAssetId normal_missile_projectile;
    SLdrDamageInfo normal_missile_damage;
    int unknown_0x1f1ef7a9;
    int unknown_0xc4a1a44e;
    CAssetId super_missile_projectile;
    SLdrDamageInfo super_missile_damage;
    CAssetId freeze_beam_projectile;
    SLdrDamageInfo freeze_beam_damage;
    CAssetId txtr_0x7ffeb33d;
    float damage_interrupt_threshold;
    float unknown_0xf317f4d5;
    CAssetId sweep_swoosh;
    SLdrDamageInfo sweep_beam_damage;
    CAssetId crsc;
    int sweep_beam_sound;
    int unknown_0x0ef8dc15;
    CAssetId invulnerable_model;
    CAssetId invulnerable_skin_rules;
    SLdrAnimationParameters boost_ball_model;
    SLdrDamageInfo boost_ball_damage;
    CAssetId boost_ball_glow;
    CAssetId swhc_0x449aa4aa;
    CAssetId swhc_0x0345fa17;
    int sound_0x2c72576b;
    int boost_ball_hit_player_sound;
    CAssetId boost_ball_collision;
    SLdrAudioPlaybackParms audio_playback_parms;
    CAssetId part_0xa6c42023;
    int ice_spread_sound;
    CAssetId part_0x908b06e9;
    CAssetId part_0x494de4a4;
    int sound_0xa861649f;
    SLdrDamageInfo damage_info_0x18402aa9;
    CAssetId part_0xe701daea;
    CAssetId phazon_projectile;
    CAssetId wpsc;
    SLdrDamageInfo damage_info_0x58769eb2;
    SLdrDamageInfo phazon_projectile_damage;
    CAssetId phazon_enrage_sphere;
    SLdrDamageInfo damage_info_0x8f3af226;
    CAssetId alternate_scannable_info;
};

void LoadTypedefSLdrDarkSamus(SLdrDarkSamus&, CInputStream&);

#endif // _SLDRDARKSAMUS
