#ifndef _SLDRUNKNOWNSTRUCT29
#define _SLDRUNKNOWNSTRUCT29

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngBoostBallGuardianStruct.hpp"

struct SLdrUnknownStruct29 {
    SLdrUnknownStruct29();
    ~SLdrUnknownStruct29();

    CVector3f boost_ball_scale;
    float boost_ball_mass;
    float unknown_0xbea96fb7;
    spline boost_ball_speed;
    SLdrDamageInfo damage_info_0x0e1a78bd;
    SLdrDamageInfo damage_info_0x19c3d263;
    float unknown_0xd8047cba;
    float unknown_0x7b21e31a;
    CAssetId boost_ball_model;
    CAssetId part_0x15534429;
    CAssetId boost_ball_shield_effect;
    CAssetId spsc;
    int sound_bounce;
    int sound_into_ball;
    int sound_outof_ball;
    int sound;
    int sound_boost;
    int sound_rolling;
    SLdrDamageVulnerability boost_ball_vulnerability;
    int unknown_0xee69b993;
    int unknown_0x4b2de673;
    float search_cone_angle;
    float unknown_0xb0e85d53;
    SLdrDamageInfo damage_info_0x5616d5f1;
    SLdrDamageInfo damage_info_0xed685533;
    CAssetId part_0xd771ec43;
    CAssetId part_0x2009a977;
    CAssetId part_0x62ab33a2;
    SLdrIngBoostBallGuardianStruct ing_boost_ball_guardian_struct_0xbab98497;
    SLdrIngBoostBallGuardianStruct ing_boost_ball_guardian_struct_0xfe18a18f;
    SLdrIngBoostBallGuardianStruct ing_boost_ball_guardian_struct_0xc2784287;
};

void LoadTypedefSLdrUnknownStruct29(SLdrUnknownStruct29&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT29
