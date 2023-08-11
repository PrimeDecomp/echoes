#ifndef _SLDRACTOR
#define _SLDRACTOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrActor {
    SLdrActor();
    ~SLdrActor();

    SLdrEditorProperties editorProperties;
    CVector3f collisionBox;
    CVector3f collisionOffset;
    float mass;
    float gravity;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    CAssetId model;
    CAssetId collisionModel;
    SLdrAnimationParameters animationInformation;
    SLdrActorParameters actorInformation;
    SLdrEchoParameters echoInformation;
    bool isLoop;
    bool immovable;
    bool isSolid;
    bool isCameraThrough;
    bool isScanThrough;
    int renderTextureSet;
    bool drawsShadow;
    bool scaleAnimation;
    bool aiShootThrough;
    float randomAnimationOffset;
    CAssetId projectile;
    SLdrDamageInfo projectileDamage;
};

void LoadTypedefSLdrActor(SLdrActor&, CInputStream&);

#endif // _SLDRACTOR
