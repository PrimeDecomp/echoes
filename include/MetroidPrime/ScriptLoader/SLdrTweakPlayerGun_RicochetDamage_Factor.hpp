#ifndef _SLDRTWEAKPLAYERGUN_RICOCHETDAMAGE_FACTOR
#define _SLDRTWEAKPLAYERGUN_RICOCHETDAMAGE_FACTOR

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_RicochetDamage_Factor {
    SLdrTweakPlayerGun_RicochetDamage_Factor();
    ~SLdrTweakPlayerGun_RicochetDamage_Factor();

    float power_beam;
    float dark_beam;
    float light_beam;
    float annihilator_beam;
    float phazon_beam;
    float missile;
};

void LoadTypedefSLdrTweakPlayerGun_RicochetDamage_Factor(SLdrTweakPlayerGun_RicochetDamage_Factor&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_RICOCHETDAMAGE_FACTOR
