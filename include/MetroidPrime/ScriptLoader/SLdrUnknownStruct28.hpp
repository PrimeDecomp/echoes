#ifndef _SLDRUNKNOWNSTRUCT28
#define _SLDRUNKNOWNSTRUCT28

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrUnknownStruct28 {
    SLdrUnknownStruct28();
    ~SLdrUnknownStruct28();

    int unknown_0x01789abd;
    float hearing_radius;
    float ing_spot_max_speed;
    float unknown_0x8d42a8d5;
    float unknown_0x84586bfd;
    float unknown_0x50398a06;
    float ing_spot_turn_speed;
    CAssetId ing_spot_blob_effect;
    CAssetId ing_spot_hit_normal_damage;
    CAssetId ing_spot_hit_heavy_damage;
    CAssetId ing_spot_death;
    int sound_ing_spot_idle;
    int sound_ing_spot_move;
    int sound_0xb392943a;
    int sound_0x24ecc1e9;
    int sound_ing_spot_death;
    CAssetId part_0x3c2d681e;
    CAssetId srsc;
    CAssetId part_0x3da219c7;
    float unknown_0x23271976;
    CAssetId part_0x081e9e6c;
    float unknown_0xcb39eccb;
    float unknown_0x587ca175;
    float unknown_0x0bd7d5a9;
    int sound_swarm_move;
    int sound_0x5650366a;
    int sound_body_projectile_blaster_middle;
    int sound_0x0c13c5a8;
    int sound_0x148b81e4;
    float unknown_0x5d0d2c40;
    float unknown_0xc620183a;
    float frustration_time;
    float taunt_chance;
    float aggressiveness;
    SLdrDamageInfo arm_swipe_damage;
    SLdrDamageInfo body_projectile_contact_damage;
    float unknown_0xa0d63374;
    float body_projectile_suck_time;
    CAssetId body_projectile_splat_effect;
    float body_projectile_speed;
    float body_projectile_drop_time;
    float unknown_0xe6ddb662;
    float unknown_0xb57bae86;
    int sound_body_projectile;
    int sound_body_projectile_splat_wall;
    float body_projectile_odds;
    float unknown_0xfa6edeb5;
    float unknown_0xa9c8c651;
    CAssetId mini_portal_effect;
    int sound_mini_portal;
    SLdrDamageInfo mini_portal_projectile_damage;
    SLdrPlasmaBeamInfo mini_portal_beam_info;
    CColor light_color;
    float light_attenuation;
    SLdrDamageVulnerability ing_spot_vulnerability;
};

void LoadTypedefSLdrUnknownStruct28(SLdrUnknownStruct28&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT28
