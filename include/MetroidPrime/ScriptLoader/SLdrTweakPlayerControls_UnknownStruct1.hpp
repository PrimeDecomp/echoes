#ifndef _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT1
#define _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT1

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerControls_UnknownStruct1 {
    SLdrTweakPlayerControls_UnknownStruct1();
    ~SLdrTweakPlayerControls_UnknownStruct1();

    int forward;
    int backward;
    int turn_left;
    int turn_right;
    int strafe_left;
    int strafe_right;
    int look_left;
    int look_right;
    int look_up;
    int look_down;
    int jump;
    int jump2;
    int fire_beam;
    int fire_beam2;
    int auto_fire_beam;
    int charge_beam;
    int charge_beam2;
    int use_item;
    int aim_up;
    int aim_down;
    int cycle_beam_up;
    int cycle_beam_down;
    int cycle_item;
    int select_power_beam;
    int select_ice_beam;
    int select_wave_beam;
    int select_plasma_beam;
    int gun_toggle_holster;
    int orbit_close;
    int orbit_far;
    int orbit_object;
    int orbit_select;
    int orbit_confirm;
    int orbit_left;
    int orbit_right;
    int orbit_up;
    int orbit_down;
    int hold_look1;
    int hold_look2;
    int look_zoom_in;
    int look_zoom_out;
    int hold_aim;
    int map_circle_up;
    int map_circle_down;
    int map_circle_left;
    int map_circle_right;
    int map_move_forward;
    int map_move_back;
    int map_move_left;
    int map_move_right;
    int map_zoom_in;
    int map_zoom_out;
    int spider_ball;
    int chase_camera;
    int x_ray_visor;
    int thermo_visor;
    int enviro_visor;
    int no_visor;
    int visor_menu;
    int cycle_visor_up;
    int cycle_visor_down;
    int dark_visor_toggle;
    int crosshairs;
    int unknown_0x29293fb1;
    int use_shield;
    int scan_item;
    int inventory_screen;
    int map_screen;
    int options_screen;
    int log_screen;
    int unknown_0xbf218f4f;
    int unknown_0x05ef2422;
    int boost_ball;
    int morph_into_ball;
    int morph_from_ball;
};

void LoadTypedefSLdrTweakPlayerControls_UnknownStruct1(SLdrTweakPlayerControls_UnknownStruct1&, CInputStream&);

#endif // _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT1
