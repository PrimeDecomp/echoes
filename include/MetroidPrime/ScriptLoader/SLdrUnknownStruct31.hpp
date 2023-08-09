#ifndef _SLDRUNKNOWNSTRUCT31
#define _SLDRUNKNOWNSTRUCT31

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSpiderballGuardianStruct.hpp"

struct SLdrUnknownStruct31 {
    SLdrUnknownStruct31();
    ~SLdrUnknownStruct31();

    SLdrIngSpiderballGuardianStruct ing_spiderball_guardian_struct_0x152db484;
    SLdrIngSpiderballGuardianStruct ing_spiderball_guardian_struct_0x2d163ff7;
    SLdrIngSpiderballGuardianStruct ing_spiderball_guardian_struct_0x8c2fbb19;
    SLdrIngSpiderballGuardianStruct ing_spiderball_guardian_struct_0x5d612911;
    SLdrIngSpiderballGuardianStruct ing_spiderball_guardian_struct_0xfc58adff;
    SLdrIngSpiderballGuardianStruct ing_spiderball_guardian_struct_0xc463268c;
    float damage_radius;
    SLdrDamageInfo proximity_damage;
    float unknown;
    SLdrAudioPlaybackParms audio_playback_parms_0xaed23abc;
    SLdrAudioPlaybackParms sound_spiderball_rolling;
    SLdrAudioPlaybackParms audio_playback_parms_0xcee38f10;
    SLdrAudioPlaybackParms audio_playback_parms_0x796fa303;
    SLdrAudioPlaybackParms sound_enter_stunned;
    SLdrAudioPlaybackParms audio_playback_parms_0x44c1f241;
};

void LoadTypedefSLdrUnknownStruct31(SLdrUnknownStruct31&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT31
