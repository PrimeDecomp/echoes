#ifndef _SLDRSOUND
#define _SLDRSOUND

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSurroundPan.hpp"

struct SLdrSound {
    SLdrSound();
    ~SLdrSound();

    SLdrEditorProperties editorProperties;
    int sound;
    float maxAudibleDistance;
    float dropOff;
    float delayTime;
    int minVolume;
    int maxVolume;
    int priority;
    SLdrSurroundPan surroundPan;
    bool loop;
    bool ambient;
    bool unknown;
    bool autoStart;
    bool canOcclude;
    bool useRoomAcoustics;
    bool persistent;
    bool playAlways;
    bool allArea;
    bool soundIsMusic;
    int pitch;
    int echoVisorMaxVolume;
};

void LoadTypedefSLdrSound(SLdrSound&, CInputStream&);

#endif // _SLDRSOUND
