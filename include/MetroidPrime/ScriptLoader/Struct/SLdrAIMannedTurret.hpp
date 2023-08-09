#ifndef _SLDRAIMANNEDTURRET
#define _SLDRAIMANNEDTURRET

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct3.hpp"

struct SLdrAIMannedTurret {
    SLdrAIMannedTurret();
    ~SLdrAIMannedTurret();

    SLdrEditorProperties editor_properties;
    SLdrUnknownStruct3 unknown_struct3;
    spline patrol_horiz_spline;
    spline patrol_vertical_spline;
};

void LoadTypedefSLdrAIMannedTurret(SLdrAIMannedTurret&, CInputStream&);

#endif // _SLDRAIMANNEDTURRET
