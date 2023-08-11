#ifndef _SLDRBRIZGEE
#define _SLDRBRIZGEE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrBrizgee {
    SLdrBrizgee();
    ~SLdrBrizgee();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float waypointApproachDistance;
    float wallTurnSpeed;
    float floorTurnSpeed;
    float downTurnSpeed;
    float visibleDistance;
    float forwardMovingPriority;
    CAssetId noShellModel;
    CAssetId noShellSkin;
    SLdrDamageVulnerability shellVulnerability;
    float shellHealth;
    SLdrDamageInfo shellContactDamage;
    float unknown;
    SLdrDamageInfo poisonDamage;
    float poisonTime;
    int shellBreakSound;
    int poisonHitSound;
    int playerPoisonSound;
};

void LoadTypedefSLdrBrizgee(SLdrBrizgee&, CInputStream&);

#endif // _SLDRBRIZGEE
