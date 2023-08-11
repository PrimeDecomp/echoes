#ifndef _SLDRTWEAKPLAYERGUN_WEAPONS
#define _SLDRTWEAKPLAYERGUN_WEAPONS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakPlayerGun_Weapons {
    SLdrTweakPlayerGun_Weapons();
    ~SLdrTweakPlayerGun_Weapons();

    SLdrTDamageInfo bomb;
    float unknown_0xe8907530;
    float unknown_0x0a9186cb;
    SLdrTDamageInfo powerBomb;
    SLdrTDamageInfo missile;
    SLdrTBeamInfo powerBeam;
    SLdrTBeamInfo darkBeam;
    SLdrTDamageInfo darkBeamBlob;
    SLdrTBeamInfo lightBeam;
    SLdrTBeamInfo annihilatorBeam;
    SLdrTBeamInfo phazonBeam;
};

void LoadTypedefSLdrTweakPlayerGun_Weapons(SLdrTweakPlayerGun_Weapons&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_WEAPONS
