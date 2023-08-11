#ifndef _SLDRPLASMABEAMINFO
#define _SLDRPLASMABEAMINFO

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrPlasmaBeamInfo {
    SLdrPlasmaBeamInfo();
    ~SLdrPlasmaBeamInfo();

    int unknown;
    CAssetId weaponSystem;
    CAssetId contactEffect;
    CAssetId pulseEffect;
    CAssetId beamTexture;
    CAssetId glowTexture;
    float length;
    float radius;
    float expansionSpeed;
    float lifeTime;
    float pulseSpeed;
    float shutdownTime;
    float contactEffectScale;
    float pulseEffectScale;
    float travelSpeed;
    CColor innerColor;
    CColor outerColor;
    CAssetId beamStreaks;
};

void LoadTypedefSLdrPlasmaBeamInfo(SLdrPlasmaBeamInfo&, CInputStream&);

#endif // _SLDRPLASMABEAMINFO
