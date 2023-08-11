#ifndef _SLDRUNKNOWNSTRUCT31
#define _SLDRUNKNOWNSTRUCT31

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSpiderballGuardianStruct.hpp"

struct SLdrUnknownStruct31 {
    SLdrUnknownStruct31();
    ~SLdrUnknownStruct31();

    SLdrIngSpiderballGuardianStruct ingSpiderballGuardianStruct_0x152db484;
    SLdrIngSpiderballGuardianStruct ingSpiderballGuardianStruct_0x2d163ff7;
    SLdrIngSpiderballGuardianStruct ingSpiderballGuardianStruct_0x8c2fbb19;
    SLdrIngSpiderballGuardianStruct ingSpiderballGuardianStruct_0x5d612911;
    SLdrIngSpiderballGuardianStruct ingSpiderballGuardianStruct_0xfc58adff;
    SLdrIngSpiderballGuardianStruct ingSpiderballGuardianStruct_0xc463268c;
    float damageRadius;
    SLdrDamageInfo proximityDamage;
    float unknown;
    SLdrAudioPlaybackParms audioPlaybackParms_0xaed23abc;
    SLdrAudioPlaybackParms soundSpiderballRolling;
    SLdrAudioPlaybackParms audioPlaybackParms_0xcee38f10;
    SLdrAudioPlaybackParms audioPlaybackParms_0x796fa303;
    SLdrAudioPlaybackParms soundEnterStunned;
    SLdrAudioPlaybackParms audioPlaybackParms_0x44c1f241;
};

void LoadTypedefSLdrUnknownStruct31(SLdrUnknownStruct31&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT31
