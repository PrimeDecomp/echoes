#ifndef _SLDRTGUNRESOURCES
#define _SLDRTGUNRESOURCES

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTGunResources {
    SLdrTGunResources();
    ~SLdrTGunResources();

    rstl::string powerBeam;
    rstl::string iceBeam;
    rstl::string waveBeam;
    rstl::string plasmaBeam;
    rstl::string phazonBeam;
};

void LoadTypedefSLdrTGunResources(SLdrTGunResources&, CInputStream&);

#endif // _SLDRTGUNRESOURCES
