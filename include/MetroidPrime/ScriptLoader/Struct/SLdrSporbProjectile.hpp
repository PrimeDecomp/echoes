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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    CAssetId ball_spit_particle_effect;
    CAssetId ball_escape_particle_effect;
};

void LoadTypedefSLdrSporbProjectile(SLdrSporbProjectile&, CInputStream&);

#endif // _SLDRSPORBPROJECTILE
