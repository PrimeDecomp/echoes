#ifndef _SLDRROOMACOUSTICS
#define _SLDRROOMACOUSTICS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRoomAcoustics {
    SLdrRoomAcoustics();
    ~SLdrRoomAcoustics();

    SLdrEditorProperties editor_properties;
    int room_volume;
    int priority;
    bool reverb_hi_enabled;
    bool unknown_0x3263c26e;
    float reverb_hi_time;
    float reverb_hi_pre_delay;
    float reverb_hi_damping;
    float reverb_hi_coloration;
    float reverb_hi_cross_talk;
    float reverb_hi_mix;
    bool chorus_enabled;
    float chorus_base_delay;
    float chorus_variation;
    float chorus_period;
    bool reverb_std_enabled;
    bool unknown_0x4a5bbf90;
    float reverb_std_time;
    float reverb_std_pre_delay;
    float reverb_std_damping;
    float reverb_std_coloration;
    float reverb_std_mix;
    bool delay_enabled;
    int delay0;
    int delay1;
    int delay2;
    int delay_feedback0;
    int delay_feedback1;
    int delay_feedback2;
    int delay_output0;
    int delay_output1;
    int delay_output2;
    int unknown_0xcf45711c;
    int unknown_0x626d1e9b;
    bool unknown_0x7d8ee273;
    float unknown_0x83378e6a;
    float unknown_0xf549d269;
    float unknown_0x11fdae16;
    float unknown_0xa3c439c1;
    float unknown_0x2dc70efe;
    float unknown_0xe2bd3706;
    float unknown_0x5e29cef8;
    float unknown_0x35ae9f10;
    bool bitcrusher_enabled;
    int unknown_0xf51a1d6a;
    float bitcrusher_gain;
    int bitcrusher_bit_depth;
    float unknown_0x58096e0b;
    bool phaser_enabled;
    float phaser_frequency;
    float phaser_feedback;
    float phaser_invert;
    float phaser_mix;
    float phaser_sweep;
};

void LoadTypedefSLdrRoomAcoustics(SLdrRoomAcoustics&, CInputStream&);

#endif // _SLDRROOMACOUSTICS
