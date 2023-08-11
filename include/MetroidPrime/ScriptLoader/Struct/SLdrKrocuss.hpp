#ifndef _SLDRKROCUSS
#define _SLDRKROCUSS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrKrocuss {
    SLdrKrocuss();
    ~SLdrKrocuss();

    SLdrEditorProperties editorProperties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float waypointApproachDistance;
    float visibleDistance;
    float wallTurnSpeed;
    float floorTurnSpeed;
    float downTurnSpeed;
    float unknown_0xd5c25506;
    float projectileBoundsMultiplier;
    float collisionLookAhead;
    float animSpeedScalar;
    bool initiallyPaused;
    float unknown_0xf04cadca;
    float unknown_0x497d54e8;
    float unknown_0x3371c963;
    float unknown_0x22d37771;
    float unknown_0xbbebed9e;
    SLdrDamageVulnerability shellClosedVulnerability;
    CColor wingLightColor;
    CAssetId dpsc;
    int shellOpenSound;
    int shellCloseSound;
    float maxAudibleDistance;
};

void LoadTypedefSLdrKrocuss(SLdrKrocuss&, CInputStream&);

#endif // _SLDRKROCUSS
