#ifndef _SLDRTWEAKGUICOLORS
#define _SLDRTWEAKGUICOLORS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_HUDColorsTypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_Multiplayer.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_TurretHudTypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_HudColorTypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_VisorColorSchemeTypedef.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGuiColors {
    SLdrTweakGuiColors();
    ~SLdrTweakGuiColors();

    rstl::string instanceName;
    SLdrTweakGuiColors_HUDColorsTypedef hudColors;
    SLdrTweakGuiColors_Misc misc;
    SLdrTweakGuiColors_Multiplayer multiplayer;
    SLdrTweakGui_VisorColorSchemeTypedef combatHudColorScheme;
    SLdrTweakGui_VisorColorSchemeTypedef echoHudColorScheme;
    SLdrTweakGui_VisorColorSchemeTypedef scanHudColorScheme;
    SLdrTweakGui_VisorColorSchemeTypedef darkHudColorScheme;
    SLdrTweakGui_VisorColorSchemeTypedef ballHudColorScheme;
    SLdrTweakGui_HudColorTypedef combatHud;
    SLdrTweakGui_HudColorTypedef scanHud;
    SLdrTweakGui_HudColorTypedef xRayHud;
    SLdrTweakGui_HudColorTypedef thermalHud;
    SLdrTweakGui_HudColorTypedef ballHud;
    SLdrTweakGuiColors_TurretHudTypedef turretHud;
};

void LoadTypedefSLdrTweakGuiColors(SLdrTweakGuiColors&, CInputStream&);

#endif // _SLDRTWEAKGUICOLORS
