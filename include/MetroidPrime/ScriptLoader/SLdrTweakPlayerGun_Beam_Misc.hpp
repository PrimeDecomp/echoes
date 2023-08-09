#ifndef _SLDRTWEAKPLAYERGUN_BEAM_MISC
#define _SLDRTWEAKPLAYERGUN_BEAM_MISC

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakPlayerGun_Beam_Misc {
    SLdrTweakPlayerGun_Beam_Misc();
    ~SLdrTweakPlayerGun_Beam_Misc();

    SLdrTDamageInfo unknown_0x8aacfc27;
    SLdrTDamageInfo unknown_0xa054ff1c;
    SLdrTDamageInfo imploder_annihilator;
    float ai_burn_damage;
    float unknown_0x4848f444;
    int max_absorbed_phazon_shots;
    float unknown_0x3ae5d1fa;
};

void LoadTypedefSLdrTweakPlayerGun_Beam_Misc(SLdrTweakPlayerGun_Beam_Misc&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_BEAM_MISC
