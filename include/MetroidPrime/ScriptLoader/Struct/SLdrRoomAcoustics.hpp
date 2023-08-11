#ifndef _SLDRROOMACOUSTICS
#define _SLDRROOMACOUSTICS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRoomAcoustics {
    SLdrRoomAcoustics();
    ~SLdrRoomAcoustics();

    SLdrEditorProperties editorProperties;
    int roomVolume;
    int priority;
    bool reverbHiEnabled;
    bool unknown_0x3263c26e;
    float reverbHiTime;
    float reverbHiPreDelay;
    float reverbHiDamping;
    float reverbHiColoration;
    float reverbHiCrossTalk;
    float reverbHiMix;
    bool chorusEnabled;
    float chorusBaseDelay;
    float chorusVariation;
    float chorusPeriod;
    bool reverbStdEnabled;
    bool unknown_0x4a5bbf90;
    float reverbStdTime;
    float reverbStdPreDelay;
    float reverbStdDamping;
    float reverbStdColoration;
    float reverbStdMix;
    bool delayEnabled;
    int delay0;
    int delay1;
    int delay2;
    int delayFeedback0;
    int delayFeedback1;
    int delayFeedback2;
    int delayOutput0;
    int delayOutput1;
    int delayOutput2;
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
    bool bitcrusherEnabled;
    int unknown_0xf51a1d6a;
    float bitcrusherGain;
    int bitcrusherBitDepth;
    float unknown_0x58096e0b;
    bool phaserEnabled;
    float phaserFrequency;
    float phaserFeedback;
    float phaserInvert;
    float phaserMix;
    float phaserSweep;
};

void LoadTypedefSLdrRoomAcoustics(SLdrRoomAcoustics&, CInputStream&);

#endif // _SLDRROOMACOUSTICS
