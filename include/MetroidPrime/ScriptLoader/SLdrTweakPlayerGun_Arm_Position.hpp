#ifndef _SLDRTWEAKPLAYERGUN_ARM_POSITION
#define _SLDRTWEAKPLAYERGUN_ARM_POSITION

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_Arm_Position {
    SLdrTweakPlayerGun_Arm_Position();
    ~SLdrTweakPlayerGun_Arm_Position();

    CVector3f normal;
    CVector3f grappling;
};

void LoadTypedefSLdrTweakPlayerGun_Arm_Position(SLdrTweakPlayerGun_Arm_Position&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_ARM_POSITION
