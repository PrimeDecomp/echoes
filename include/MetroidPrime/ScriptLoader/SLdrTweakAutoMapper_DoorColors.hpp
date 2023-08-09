#ifndef _SLDRTWEAKAUTOMAPPER_DOORCOLORS
#define _SLDRTWEAKAUTOMAPPER_DOORCOLORS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakAutoMapper_DoorColors {
    SLdrTweakAutoMapper_DoorColors();
    ~SLdrTweakAutoMapper_DoorColors();

    int num_door_colors;
    CColor blue_door_color;
    CColor missile_door_color;
    CColor dark_beam_door_color;
    CColor unknown;
    CColor annihilator_beam_door_color;
    CColor light_beam_door_color;
    CColor super_missile_door_color;
    CColor seeker_door_color;
    CColor power_bomb_door_color;
    CColor grey_door_color;
    CColor white_door_color;
};

void LoadTypedefSLdrTweakAutoMapper_DoorColors(SLdrTweakAutoMapper_DoorColors&, CInputStream&);

#endif // _SLDRTWEAKAUTOMAPPER_DOORCOLORS
