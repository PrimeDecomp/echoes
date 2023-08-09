#ifndef _SLDRUNKNOWNSTRUCT36
#define _SLDRUNKNOWNSTRUCT36

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"

struct SLdrUnknownStruct36 {
    SLdrUnknownStruct36();
    ~SLdrUnknownStruct36();

    SLdrAudioPlaybackParms audio_playback_parms_0x4f904909;
    SLdrAudioPlaybackParms audio_playback_parms_0x82e108de;
    SLdrAudioPlaybackParms audio_playback_parms_0xdf090545;
    SLdrAudioPlaybackParms audio_playback_parms_0x3dd5b3cf;
    SLdrAudioPlaybackParms audio_playback_parms_0xf82231bb;
    SLdrAudioPlaybackParms audio_playback_parms_0x009e3658;
    SLdrAudioPlaybackParms audio_playback_parms_0x62bd75b1;
    float unknown;
    SLdrAudioPlaybackParms audio_playback_parms_0x32969cba;
    SLdrAudioPlaybackParms audio_playback_parms_0x597d2ac9;
};

void LoadTypedefSLdrUnknownStruct36(SLdrUnknownStruct36&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT36
