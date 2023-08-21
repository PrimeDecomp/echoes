#ifndef _SLDRSEQUENCETIMER
#define _SLDRSEQUENCETIMER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrConnection.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/vector.hpp"

struct SLdrSequenceConnections {
    SLdrSequenceConnections();
    explicit SLdrSequenceConnections(CInputStream&);

    rstl::vector< SLdrConnection > array;

    SLdrSequenceConnections& operator=(const SLdrSequenceConnections&);
};

SLdrSequenceConnections& SLdrSequenceConnections::operator=(const SLdrSequenceConnections& other) {
    array = other.array;
    return *this;
}

struct SLdrSequenceTimer {
    SLdrEditorProperties editorProperties;
    SLdrSequenceConnections sequenceConnections;
    float startTime;
    float maxTime;
    float loopStartTime;
    bool isAutostart;
    bool isLoop;
    bool takeExternalTime;
};

void LoadTypedefSLdrSequenceTimer(SLdrSequenceTimer&, CInputStream&);

#endif // _SLDRSEQUENCETIMER
