#ifndef _SLDRTWEAKPLAYERGUN_RICOCHETDAMAGE_FACTOR
#define _SLDRTWEAKPLAYERGUN_RICOCHETDAMAGE_FACTOR

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_RicochetDamage_Factor {
    SLdrTweakPlayerGun_RicochetDamage_Factor();
    ~SLdrTweakPlayerGun_RicochetDamage_Factor();

    float powerBeam;
    float darkBeam;
    float lightBeam;
    float annihilatorBeam;
    float phazonBeam;
    float missile;
};

void LoadTypedefSLdrTweakPlayerGun_RicochetDamage_Factor(SLdrTweakPlayerGun_RicochetDamage_Factor&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_RICOCHETDAMAGE_FACTOR
