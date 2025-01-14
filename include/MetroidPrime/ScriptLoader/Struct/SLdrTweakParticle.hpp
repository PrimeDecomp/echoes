#ifndef _SLDRTWEAKPARTICLE
#define _SLDRTWEAKPARTICLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakParticle {
    SLdrTweakParticle();
    ~SLdrTweakParticle();

    rstl::string instanceName;
    rstl::string pakFile;
    rstl::string primaryWeapon;
    rstl::string secondaryWeapon;
};

void LoadTypedefSLdrTweakParticle(SLdrTweakParticle&, CInputStream&);

#endif // _SLDRTWEAKPARTICLE
