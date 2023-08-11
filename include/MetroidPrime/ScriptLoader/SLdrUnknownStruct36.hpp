#ifndef _SLDRUNKNOWNSTRUCT36
#define _SLDRUNKNOWNSTRUCT36

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"

struct SLdrUnknownStruct36 {
    SLdrUnknownStruct36();
    ~SLdrUnknownStruct36();

    SLdrAudioPlaybackParms audioPlaybackParms_0x4f904909;
    SLdrAudioPlaybackParms audioPlaybackParms_0x82e108de;
    SLdrAudioPlaybackParms audioPlaybackParms_0xdf090545;
    SLdrAudioPlaybackParms audioPlaybackParms_0x3dd5b3cf;
    SLdrAudioPlaybackParms audioPlaybackParms_0xf82231bb;
    SLdrAudioPlaybackParms audioPlaybackParms_0x009e3658;
    SLdrAudioPlaybackParms audioPlaybackParms_0x62bd75b1;
    float unknown;
    SLdrAudioPlaybackParms audioPlaybackParms_0x32969cba;
    SLdrAudioPlaybackParms audioPlaybackParms_0x597d2ac9;
};

void LoadTypedefSLdrUnknownStruct36(SLdrUnknownStruct36&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT36
