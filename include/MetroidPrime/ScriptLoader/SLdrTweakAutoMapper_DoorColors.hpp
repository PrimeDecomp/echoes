#ifndef _SLDRTWEAKAUTOMAPPER_DOORCOLORS
#define _SLDRTWEAKAUTOMAPPER_DOORCOLORS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakAutoMapper_DoorColors {
    SLdrTweakAutoMapper_DoorColors();
    ~SLdrTweakAutoMapper_DoorColors();

    int numDoorColors;
    CColor blueDoorColor;
    CColor missileDoorColor;
    CColor darkBeamDoorColor;
    CColor unknown;
    CColor annihilatorBeamDoorColor;
    CColor lightBeamDoorColor;
    CColor superMissileDoorColor;
    CColor seekerDoorColor;
    CColor powerBombDoorColor;
    CColor greyDoorColor;
    CColor whiteDoorColor;
};

void LoadTypedefSLdrTweakAutoMapper_DoorColors(SLdrTweakAutoMapper_DoorColors&, CInputStream&);

#endif // _SLDRTWEAKAUTOMAPPER_DOORCOLORS
