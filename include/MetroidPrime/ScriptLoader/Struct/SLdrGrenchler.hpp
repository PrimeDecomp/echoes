#ifndef _SLDRGRENCHLER
#define _SLDRGRENCHLER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGrenchler {
    SLdrGrenchler();
    ~SLdrGrenchler();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float unknown_0x04d51e3a;
    bool is_grapple_guardian;
    bool has_health_bar;
    SLdrDamageVulnerability damage_vulnerability;
    SLdrAnimationParameters tail;
    SLdrAnimationParameters unknown_0x0abef809;
    CAssetId tailless_model;
    CAssetId tailless_skin_rules;
    SLdrAnimationParameters unknown_0x9b193ae8;
    SLdrAnimationParameters unknown_0xc24cf580;
    CAssetId cmdl;
    CAssetId cskr;
    int tail_hit_sound;
    int tail_destroyed_sound;
    float unknown_0x5d8f2bee;
    float unknown_0x7bd1a35f;
    float unknown_0xea4b88c8;
    float unknown_0xaa04f0be;
    float unknown_0x98d5d373;
    float unknown_0xd89aab05;
    float unknown_0x2e6096ee;
    float unknown_0x6e2fee98;
    float unknown_0x49632b31;
    SLdrDamageInfo bite_damage;
    float unknown_0x262b2508;
    float unknown_0x66645d7e;
    float unknown_0x909e6095;
    float unknown_0xd0d118e3;
    CAssetId electric_effect;
    SLdrDamageInfo beam_damage;
    float unknown_0x680ce795;
    SLdrAudioPlaybackParms audio_playback_parms_0xad47febe;
    float unknown_0x06f7ceed;
    float unknown_0x46b8b69b;
    float unknown_0xb0428b70;
    float unknown_0xf00df306;
    float unknown_0xb00775e6;
    CAssetId burst_projectile;
    SLdrDamageInfo burst_damage;
    CAssetId surface_rings_effect;
    CAssetId part_0xbf4daae6;
    CAssetId part_always_ff;
    CAssetId part_0xffcee1a9;
    CAssetId grapple_swoosh;
    CAssetId grapple_beam_part;
    CAssetId grapple_hit_fx;
    SLdrDamageInfo grapple_damage;
    SLdrAudioPlaybackParms audio_playback_parms_0xb6b9074b;
    CAssetId beam_effect;
    int unknown_0xd4753ff4;
    float unknown_0x05fc6001;
    float unknown_0x13e5b580;
    float unknown_0xfc6f199d;
    CAssetId grapple_visor_effect;
    SLdrDamageInfo damage_info;
    CAssetId part_0x54b6bfa1;
    SLdrAudioPlaybackParms audio_playback_parms_0x5cf705f2;
    CAssetId part_0xb9f9f4f2;
    CAssetId alternate_scannable_info;
    SLdrIngPossessionData ing_possession_data;
};

void LoadTypedefSLdrGrenchler(SLdrGrenchler&, CInputStream&);

#endif // _SLDRGRENCHLER
