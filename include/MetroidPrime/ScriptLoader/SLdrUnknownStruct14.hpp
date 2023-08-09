#ifndef _SLDRUNKNOWNSTRUCT14
#define _SLDRUNKNOWNSTRUCT14

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"

struct SLdrUnknownStruct14 {
    SLdrUnknownStruct14();
    ~SLdrUnknownStruct14();

    float unknown_0xa0d037ee;
    float unknown_0x4f522994;
    float shadow_dash_speed;
    float unknown_0x5d02f384;
    CAssetId part;
    SLdrAudioPlaybackParms audio_playback_parms;
    SLdrAudioPlaybackParms sound_cloak;
    SLdrAudioPlaybackParms sound_de_cloak;
    SLdrDamageVulnerability shadow_decoy_vulnerability;
    SLdrDamageVulnerability shadow_dash_vulnerability;
};

void LoadTypedefSLdrUnknownStruct14(SLdrUnknownStruct14&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT14
