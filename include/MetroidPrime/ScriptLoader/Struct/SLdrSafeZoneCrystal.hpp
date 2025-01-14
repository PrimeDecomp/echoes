#ifndef _SLDRSAFEZONECRYSTAL
#define _SLDRSAFEZONECRYSTAL

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSafeZoneCrystal {
    SLdrSafeZoneCrystal();
    ~SLdrSafeZoneCrystal();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorParameters;
    CAssetId scannableInfoCollapsed;
    CAssetId scannableInfoEntangled;
    CAssetId scannableInfoLight;
    CAssetId scannableInfoAnnihilator;
    int safezoneType;
    bool initiallyEntangled;
    CAssetId collapsedEffect;
    CAssetId expandedEffect;
    CAssetId entangledEffect;
    CAssetId part;
    CAssetId echoEffect;
    CAssetId normalCrystal;
    CAssetId entangledCrystal;
    CAssetId energizedModel;
    CAssetId echoCrystal;
    float maxTimeExpanded;
    float maxTimeEntangled;
    float unknown_0xf0a45c32;
    float unknown_0xd8116003;
    float unknown_0x415046ed;
    float unknown_0xec9c01b2;
    float unknown_0x545540e5;
    CAssetId powerBeamRefreshEffect;
    CVector3f hitRadius;
    CVector3f hitOffset;
    CVector3f effectOffset;
    SLdrSpline unknown_0xbbbee60b;
};

void LoadTypedefSLdrSafeZoneCrystal(SLdrSafeZoneCrystal&, CInputStream&);

#endif // _SLDRSAFEZONECRYSTAL
