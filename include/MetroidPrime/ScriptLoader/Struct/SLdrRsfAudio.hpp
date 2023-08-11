#ifndef _SLDRRSFAUDIO
#define _SLDRRSFAUDIO

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrRsfAudio {
    SLdrRsfAudio();
    ~SLdrRsfAudio();

    SLdrEditorProperties editorProperties;
    rstl::string unknown;
    int loopStart;
    int loopEnd;
    float fadeInTime;
    float fadeOutTime;
    int volume;
};

void LoadTypedefSLdrRsfAudio(SLdrRsfAudio&, CInputStream&);

#endif // _SLDRRSFAUDIO
