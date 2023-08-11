#ifndef _SLDRSPORBPROJECTILE
#define _SLDRSPORBPROJECTILE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrSporbProjectile {
    SLdrSporbProjectile();
    ~SLdrSporbProjectile();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    CAssetId ballSpitParticleEffect;
    CAssetId ballEscapeParticleEffect;
};

void LoadTypedefSLdrSporbProjectile(SLdrSporbProjectile&, CInputStream&);

#endif // _SLDRSPORBPROJECTILE
