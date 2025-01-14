#ifndef _SLDRTBALLTRANSITIONRESOURCES
#define _SLDRTBALLTRANSITIONRESOURCES

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTGunResources.hpp"
#include "rstl/string.hpp"

struct SLdrTBallTransitionResources {
    SLdrTBallTransitionResources();
    ~SLdrTBallTransitionResources();

    rstl::string unknown_0xd48e4124;
    SLdrTGunResources unknown_0x01e12c84;
    SLdrTGunResources unknown_0xf24b055d;
    SLdrSpline unknown_0xa342c3a6;
    SLdrSpline unknown_0x15b6840d;
    SLdrSpline unknown_0x23fb0e93;
    SLdrSpline unknown_0x564262f0;
    SLdrSpline movementControl;
};

void LoadTypedefSLdrTBallTransitionResources(SLdrTBallTransitionResources&, CInputStream&);

#endif // _SLDRTBALLTRANSITIONRESOURCES
