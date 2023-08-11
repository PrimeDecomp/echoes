#ifndef _SLDRATOMICALPHA
#define _SLDRATOMICALPHA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrAtomicAlpha {
    SLdrAtomicAlpha();
    ~SLdrAtomicAlpha();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    CAssetId bombWeapon;
    CAssetId bombModel;
    SLdrDamageInfo bombDamage;
    float bombDropDelay;
    float bombReappearDelay;
    float bombReappearTime;
    bool invisible;
    bool homeWhileCharging;
};

void LoadTypedefSLdrAtomicAlpha(SLdrAtomicAlpha&, CInputStream&);

#endif // _SLDRATOMICALPHA
