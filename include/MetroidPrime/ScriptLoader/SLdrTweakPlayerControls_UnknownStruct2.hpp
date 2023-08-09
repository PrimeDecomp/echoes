#ifndef _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT2
#define _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT2

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerControls_UnknownStruct2 {
    SLdrTweakPlayerControls_UnknownStruct2();
    ~SLdrTweakPlayerControls_UnknownStruct2();

    bool unknown_0xff1b0413;
    bool unknown_0xe0c1d958;
    bool toggle_aim_position;
    bool unknown_0x1f99c6ba;
    bool unknown_0x18eb3ab5;
    bool unknown_0xbdc01c71;
    bool fixed_vertical_aim;
    bool unknown_0xda97bbcd;
    bool orbit_around_enemies;
    bool unknown_0xc224d966;
    bool add_grenade_alert;
    bool unknown_0x3fb16819;
    bool unknown_0x4fcf4b70;
    bool unknown_0x07bb06a6;
    bool unknown_0x04d8d57b;
    bool unknown_0x5282c47e;
    bool falling_double_jump;
    bool impulse_double_jump;
    bool unknown_0xa796a8b9;
    bool unknown_0x7c0599c8;
    bool unknown_0x522ab1ac;
};

void LoadTypedefSLdrTweakPlayerControls_UnknownStruct2(SLdrTweakPlayerControls_UnknownStruct2&, CInputStream&);

#endif // _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT2
