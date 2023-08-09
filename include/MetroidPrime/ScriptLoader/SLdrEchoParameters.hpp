#ifndef _SLDRECHOPARAMETERS
#define _SLDRECHOPARAMETERS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrEchoParameters {
    SLdrEchoParameters();
    ~SLdrEchoParameters();

    bool is_echo_emitter;
    bool only_emit_damage;
    int num_sound_waves;
    float space_between_waves;
    float wave_line_size;
    float forced_minimum_vis;
};

void LoadTypedefSLdrEchoParameters(SLdrEchoParameters&, CInputStream&);

#endif // _SLDRECHOPARAMETERS
