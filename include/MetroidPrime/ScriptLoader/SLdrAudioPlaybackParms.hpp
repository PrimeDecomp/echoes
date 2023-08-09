#ifndef _SLDRAUDIOPLAYBACKPARMS
#define _SLDRAUDIOPLAYBACKPARMS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrAudioPlaybackParms {
    SLdrAudioPlaybackParms();
    ~SLdrAudioPlaybackParms();

    float maximum_distance;
    float fall_off;
    int sound_id;
    int max_volume;
    int min_volume;
    bool use_room_acoustics;
};

void LoadTypedefSLdrAudioPlaybackParms(SLdrAudioPlaybackParms&, CInputStream&);

#endif // _SLDRAUDIOPLAYBACKPARMS
