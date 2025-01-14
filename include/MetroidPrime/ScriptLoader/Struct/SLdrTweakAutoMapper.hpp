#ifndef _SLDRTWEAKAUTOMAPPER
#define _SLDRTWEAKAUTOMAPPER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_Base.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_DoorColors.hpp"
#include "rstl/string.hpp"

struct SLdrTweakAutoMapper {
    SLdrTweakAutoMapper();
    ~SLdrTweakAutoMapper();

    rstl::string instanceName;
    SLdrTweakAutoMapper_Base base;
    SLdrTweakAutoMapper_DoorColors doorColors;
};

void LoadTypedefSLdrTweakAutoMapper(SLdrTweakAutoMapper&, CInputStream&);

#endif // _SLDRTWEAKAUTOMAPPER
