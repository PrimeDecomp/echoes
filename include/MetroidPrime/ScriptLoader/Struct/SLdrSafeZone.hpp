#ifndef _SLDRSAFEZONE
#define _SLDRSAFEZONE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSafeZoneStructA.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSafeZoneStructB.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTriggerInfo.hpp"

struct SLdrSafeZone {
    SLdrSafeZone();
    ~SLdrSafeZone();

    SLdrEditorProperties editorProperties;
    SLdrTriggerInfo trigger;
    bool deactivateOnEnter;
    bool deactivateOnExit;
    float activationTime;
    float deactivationTime;
    float lifetime;
    float randomLifetimeOffset;
    CAssetId impactEffect;
    bool filterSoundEffects;
    int unknown_0x414379ea;
    bool ignoreCinematicCamera;
    SLdrSafeZoneStructB normalSafeZoneStruct;
    SLdrSafeZoneStructB energizedSafeZoneStruct;
    SLdrSafeZoneStructB superchargedSafeZoneStruct;
    SLdrDamageInfo normalDamage;
    SLdrDamageInfo damageInfo;
    float insideFadeStart;
    float insideFadeTime;
    float unknown_0x6c14904c;
    float flashTime;
    float flashBrightness;
    int flashSound;
    int safezoneShape;
    bool mobile;
    bool generateMobileLight;
    CVector3f mobileLightOffset;
    CColor unknown_0xe71b43e1;
    float unknown_0x9f638987;
    SLdrSafeZoneStructA safeZoneStructA_0x8a09f99a;
    SLdrSafeZoneStructA safeZoneStructA_0xafb855b8;
    SLdrEchoParameters echoParameters;
};

void LoadTypedefSLdrSafeZone(SLdrSafeZone&, CInputStream&);

#endif // _SLDRSAFEZONE
