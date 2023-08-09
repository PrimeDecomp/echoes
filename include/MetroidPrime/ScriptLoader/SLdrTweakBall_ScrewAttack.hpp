#ifndef _SLDRTWEAKBALL_SCREWATTACK
#define _SLDRTWEAKBALL_SCREWATTACK

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_ScrewAttack {
    SLdrTweakBall_ScrewAttack();
    ~SLdrTweakBall_ScrewAttack();

    float screw_attack_gravity;
    float unknown_0xcb77fb28;
    float unknown_0x3fdeb046;
    int unknown_0x691b244d;
    float screw_attack_vertical_jump_velocity;
    float screw_attack_horizontal_jump_velocity;
    float unknown_0x3d03d8a6;
    float unknown_0xf1f2498f;
    float unknown_0x4b0aba1c;
    float screw_attack_wall_jump_max_time;
    float screw_attack_wall_jump_vertical_velocity;
    float screw_attack_wall_jump_horizontal_velocity;
    float screw_attack_wall_jump_gravity;
    SLdrTDamageInfo screw_attack_damage;
};

void LoadTypedefSLdrTweakBall_ScrewAttack(SLdrTweakBall_ScrewAttack&, CInputStream&);

#endif // _SLDRTWEAKBALL_SCREWATTACK
