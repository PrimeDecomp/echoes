#ifndef _SLDRTWEAKBALL_SCREWATTACK
#define _SLDRTWEAKBALL_SCREWATTACK

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_ScrewAttack {
    SLdrTweakBall_ScrewAttack();
    ~SLdrTweakBall_ScrewAttack();

    float screwAttackGravity;
    float unknown_0xcb77fb28;
    float unknown_0x3fdeb046;
    int unknown_0x691b244d;
    float screwAttackVerticalJumpVelocity;
    float screwAttackHorizontalJumpVelocity;
    float unknown_0x3d03d8a6;
    float unknown_0xf1f2498f;
    float unknown_0x4b0aba1c;
    float screwAttackWallJumpMaxTime;
    float screwAttackWallJumpVerticalVelocity;
    float screwAttackWallJumpHorizontalVelocity;
    float screwAttackWallJumpGravity;
    SLdrTDamageInfo screwAttackDamage;
};

void LoadTypedefSLdrTweakBall_ScrewAttack(SLdrTweakBall_ScrewAttack&, CInputStream&);

#endif // _SLDRTWEAKBALL_SCREWATTACK
