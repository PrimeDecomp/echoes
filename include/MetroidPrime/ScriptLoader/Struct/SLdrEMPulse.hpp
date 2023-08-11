#ifndef _SLDREMPULSE
#define _SLDREMPULSE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrEMPulse {
    SLdrEMPulse();
    ~SLdrEMPulse();

    SLdrEditorProperties editorProperties;
    float initialSize;
    float finalSize;
    float duration;
    float unknown_0x96bd6426;
    float unknown_0xd7aa5ba0;
    float backwardForwardSweepChance;
    float unknown_0xce54e50e;
    CAssetId explosion;
};

void LoadTypedefSLdrEMPulse(SLdrEMPulse&, CInputStream&);

#endif // _SLDREMPULSE
