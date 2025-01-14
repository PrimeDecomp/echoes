#ifndef _SLDRTWEAKPLAYER
#define _SLDRTWEAKPLAYER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_AimStuff.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Collision.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_DarkWorld.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_FirstPersonCamera.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Frozen.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Grapple.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_GrappleBeam.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Motion.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Orbit.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_ScanVisor.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Shield.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_SuitDamageReduction.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayer {
    SLdrTweakPlayer();
    ~SLdrTweakPlayer();

    rstl::string instanceName;
    SLdrTweakPlayer_DarkWorld darkWorld;
    SLdrTweakPlayer_GrappleBeam grappleBeam;
    SLdrTweakPlayer_Motion motion;
    SLdrTweakPlayer_Misc misc;
    SLdrTweakPlayer_AimStuff aimStuff;
    SLdrTweakPlayer_Orbit orbit;
    SLdrTweakPlayer_ScanVisor scanVisor;
    SLdrTweakPlayer_Grapple grapple;
    SLdrTweakPlayer_Collision collision;
    SLdrTweakPlayer_FirstPersonCamera firstPersonCamera;
    SLdrTweakPlayer_Shield shield;
    SLdrTweakPlayer_Frozen frozen;
    SLdrTweakPlayer_SuitDamageReduction suitDamageReduction;
};

void LoadTypedefSLdrTweakPlayer(SLdrTweakPlayer&, CInputStream&);

#endif // _SLDRTWEAKPLAYER
