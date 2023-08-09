#ifndef _SLDRRSFAUDIO
#define _SLDRRSFAUDIO

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrRsfAudio {
    SLdrRsfAudio();
    ~SLdrRsfAudio();

    SLdrEditorProperties editor_properties;
    rstl::string unknown;
    int loop_start;
    int loop_end;
    float fade_in_time;
    float fade_out_time;
    int volume;
};

void LoadTypedefSLdrRsfAudio(SLdrRsfAudio&, CInputStream&);

#endif // _SLDRRSFAUDIO
