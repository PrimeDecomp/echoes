#ifndef _SLDRPLAYERACTOR
#define _SLDRPLAYERACTOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrPlayerActor {
    SLdrPlayerActor();
    ~SLdrPlayerActor();

    SLdrEditorProperties editorProperties;
    CVector3f collisionBox;
    CVector3f collisionOffset;
    float mass;
    float gravity;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    CAssetId noModel;
    SLdrAnimationParameters animationInformation;
    SLdrActorParameters actorInformation;
    bool isLoop;
    bool immovable;
    bool isSolid;
    int flagsPlayerActor;
    int renderGunOverride;
};

void LoadTypedefSLdrPlayerActor(SLdrPlayerActor&, CInputStream&);

#endif // _SLDRPLAYERACTOR
