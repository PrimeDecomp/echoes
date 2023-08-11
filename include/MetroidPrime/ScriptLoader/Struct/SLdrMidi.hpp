#ifndef _SLDRMIDI
#define _SLDRMIDI

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrMidi {
    SLdrMidi();
    ~SLdrMidi();

    SLdrEditorProperties editorProperties;
    CAssetId songFile;
    float fadeInTime;
    float fadeOutTime;
    int volume;
};

void LoadTypedefSLdrMidi(SLdrMidi&, CInputStream&);

#endif // _SLDRMIDI
