#ifndef _SLDRUNKNOWNSTRUCT33
#define _SLDRUNKNOWNSTRUCT33

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"

struct SLdrUnknownStruct33 {
    SLdrUnknownStruct33();
    ~SLdrUnknownStruct33();

    SLdrDamageInfo damage;
    float bomb_stun_duration;
    float unknown_0x46aaced3;
    float max_speed;
    float max_wall_speed;
    float ball_pursuit_speed;
    float speed_modifier;
    float turn_speed;
    CAssetId blob_effect;
    CAssetId hit_normal_damage;
    CAssetId hit_heavy_damage;
    CAssetId death;
    int sound_idle;
    int sound_move;
    int sound_0xb392943a;
    int sound_0x24ecc1e9;
    int sound_death;
    float unknown_0x7569fdba;
    float unknown_0xd55938d2;
    SLdrDamageVulnerability vulnerability;
};

void LoadTypedefSLdrUnknownStruct33(SLdrUnknownStruct33&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT33
