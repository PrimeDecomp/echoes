#ifndef _SLDRSOUND
#define _SLDRSOUND

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSurroundPan.hpp"

struct SLdrSound {
    SLdrSound();
    ~SLdrSound();

    SLdrEditorProperties editor_properties;
    int sound;
    float max_audible_distance;
    float drop_off;
    float delay_time;
    int min_volume;
    int max_volume;
    int priority;
    SLdrSurroundPan surround_pan;
    bool loop;
    bool ambient;
    bool unknown;
    bool auto_start;
    bool can_occlude;
    bool use_room_acoustics;
    bool persistent;
    bool play_always;
    bool all_area;
    bool sound_is_music;
    int pitch;
    int echo_visor_max_volume;
};

void LoadTypedefSLdrSound(SLdrSound&, CInputStream&);

#endif // _SLDRSOUND
