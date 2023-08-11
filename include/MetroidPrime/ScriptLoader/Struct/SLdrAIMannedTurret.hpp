#ifndef _SLDRAIMANNEDTURRET
#define _SLDRAIMANNEDTURRET

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct3.hpp"

struct SLdrAIMannedTurret {
    SLdrAIMannedTurret();
    ~SLdrAIMannedTurret();

    SLdrEditorProperties editorProperties;
    SLdrUnknownStruct3 unknownStruct3;
    spline patrolHorizSpline;
    spline patrolVerticalSpline;
};

void LoadTypedefSLdrAIMannedTurret(SLdrAIMannedTurret&, CInputStream&);

#endif // _SLDRAIMANNEDTURRET
