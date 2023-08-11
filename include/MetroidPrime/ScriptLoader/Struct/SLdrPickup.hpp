#ifndef _SLDRPICKUP
#define _SLDRPICKUP

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerItem.hpp"

struct SLdrPickup {
    SLdrPickup();

    SLdrEditorProperties editorProperties;
    CVector3f collisionSize;
    CVector3f collisionOffset;
    SLdrPlayerItem itemToGive;
    int capacityIncrease;
    int itemPercentageIncrease;
    int amount;
    float respawnTime;
    float pickupEffectLifetime;
    float lifetime;
    float fadetime;
    CAssetId model;
    SLdrAnimationParameters animationInformation;
    SLdrActorParameters actorInformation;
    SLdrEchoParameters echoInformation;
    float activationDelay;
    CAssetId pickupEffect;
    bool absoluteValue;
    bool calculateVisibility;
    bool unknown;
    float autoHomeRange;
    float delayUntilHome;
    float homingSpeed;
    bool autoSpin;
    bool blinkOut;
    CVector3f orbitOffset;
};

void LoadTypedefSLdrPickup(SLdrPickup&, CInputStream&);

#endif // _SLDRPICKUP
