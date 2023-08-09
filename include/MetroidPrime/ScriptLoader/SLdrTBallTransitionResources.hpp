#ifndef _SLDRTBALLTRANSITIONRESOURCES
#define _SLDRTBALLTRANSITIONRESOURCES

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTGunResources.hpp"
#include "rstl/string.hpp"

struct SLdrTBallTransitionResources {
    SLdrTBallTransitionResources();
    ~SLdrTBallTransitionResources();

    rstl::string unknown_0xd48e4124;
    SLdrTGunResources unknown_0x01e12c84;
    SLdrTGunResources unknown_0xf24b055d;
    spline unknown_0xa342c3a6;
    spline unknown_0x15b6840d;
    spline unknown_0x23fb0e93;
    spline unknown_0x564262f0;
    spline movement_control;
};

void LoadTypedefSLdrTBallTransitionResources(SLdrTBallTransitionResources&, CInputStream&);

#endif // _SLDRTBALLTRANSITIONRESOURCES
