#ifndef _SLDRTGUNRESOURCES
#define _SLDRTGUNRESOURCES

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTGunResources {
    SLdrTGunResources();
    ~SLdrTGunResources();

    rstl::string power_beam;
    rstl::string ice_beam;
    rstl::string wave_beam;
    rstl::string plasma_beam;
    rstl::string phazon_beam;
};

void LoadTypedefSLdrTGunResources(SLdrTGunResources&, CInputStream&);

#endif // _SLDRTGUNRESOURCES
