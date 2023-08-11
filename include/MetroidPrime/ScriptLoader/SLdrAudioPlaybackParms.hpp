#ifndef _SLDRAUDIOPLAYBACKPARMS
#define _SLDRAUDIOPLAYBACKPARMS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrAudioPlaybackParms {
    SLdrAudioPlaybackParms();
    ~SLdrAudioPlaybackParms();

    float maximumDistance;
    float fallOff;
    int soundId;
    int maxVolume;
    int minVolume;
    bool useRoomAcoustics;
};

void LoadTypedefSLdrAudioPlaybackParms(SLdrAudioPlaybackParms&, CInputStream&);

#endif // _SLDRAUDIOPLAYBACKPARMS
