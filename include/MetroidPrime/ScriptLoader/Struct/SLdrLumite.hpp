#ifndef _SLDRLUMITE
#define _SLDRLUMITE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrLumite {
    SLdrLumite();
    ~SLdrLumite();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float unknown_0x2d9ebd7f;
    float unknown_0x6dd1c509;
    CAssetId smallShotProjectile;
    SLdrDamageInfo smallShotDamage;
    float unknown_0x6d5356bb;
    float unknown_0x2d1c2ecd;
    CAssetId bigShotProjectile;
    SLdrDamageInfo bigShotDamage;
    CAssetId trailEffect;
    CAssetId sunlightEnterExitEffect;
    float unknown_0xe05d93ef;
    float unknown_0x47691396;
    int phaseInSound;
    int phaseOutSound;
};

void LoadTypedefSLdrLumite(SLdrLumite&, CInputStream&);

#endif // _SLDRLUMITE
