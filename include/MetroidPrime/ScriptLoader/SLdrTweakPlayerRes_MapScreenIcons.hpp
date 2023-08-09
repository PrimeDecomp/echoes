#ifndef _SLDRTWEAKPLAYERRES_MAPSCREENICONS
#define _SLDRTWEAKPLAYERRES_MAPSCREENICONS

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerRes_MapScreenIcons {
    SLdrTweakPlayerRes_MapScreenIcons();
    ~SLdrTweakPlayerRes_MapScreenIcons();

    rstl::string l_stick_n;
    rstl::string l_stick_u;
    rstl::string l_stick_ul;
    rstl::string l_stick_l;
    rstl::string l_stick_dl;
    rstl::string l_stick_d;
    rstl::string l_stick_dr;
    rstl::string l_stick_r;
    rstl::string l_stick_ur;
    rstl::string c_stick_n;
    rstl::string c_stick_u;
    rstl::string c_stick_ul;
    rstl::string c_stick_l;
    rstl::string c_stick_dl;
    rstl::string c_stick_d;
    rstl::string c_stick_dr;
    rstl::string c_stick_r;
    rstl::string c_stick_ur;
    rstl::string l_trigger_out;
    rstl::string l_trigger_in;
    rstl::string r_trigger_out;
    rstl::string r_trigger_in;
    rstl::string start_button_out;
    rstl::string start_button_in;
    rstl::string a_button_out;
    rstl::string a_button_in;
    rstl::string b_button_out;
    rstl::string b_button_in;
    rstl::string x_button_out;
    rstl::string x_button_in;
    rstl::string y_button_out;
    rstl::string y_button_in;
};

void LoadTypedefSLdrTweakPlayerRes_MapScreenIcons(SLdrTweakPlayerRes_MapScreenIcons&, CInputStream&);

#endif // _SLDRTWEAKPLAYERRES_MAPSCREENICONS
