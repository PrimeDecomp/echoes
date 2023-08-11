#ifndef _SLDRSAFEZONESTRUCTB
#define _SLDRSAFEZONESTRUCTB

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSafeZoneStructB {
    SLdrSafeZoneStructB();
    ~SLdrSafeZoneStructB();

    int turnOnSound;
    float unknown_0xd4839a3f;
    int activeLoopSound;
    int turnOffSound;
    int playerEnterSound;
    int playerExitSound;
    CAssetId darkVisorSpotTexture;
    float darkVisorSpotMaxSize;
    CAssetId shellEnvironmentMap;
    float shell1AnimatedHorizRate;
    float shell1AnimatedVertRate;
    float shell1ScaleHoriz;
    float shell1ScaleVert;
    CAssetId shell1Texture;
    float shell2AnimatedHorizRate;
    float shell2AnimatedVertRate;
    float shell2ScaleHoriz;
    float shell2ScaleVert;
    CAssetId shell2Texture;
    CColor shellColor;
    CColor unknown_0xe68b1fa8;
};

void LoadTypedefSLdrSafeZoneStructB(SLdrSafeZoneStructB&, CInputStream&);

#endif // _SLDRSAFEZONESTRUCTB
