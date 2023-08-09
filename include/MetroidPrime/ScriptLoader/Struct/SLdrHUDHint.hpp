#ifndef _SLDRHUDHINT
#define _SLDRHUDHINT

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrHUDHint {
    SLdrHUDHint();
    ~SLdrHUDHint();

    SLdrEditorProperties editor_properties;
    CAssetId hud_texture;
    float unknown_0x6078a651;
    float unknown_0xf00bb6bb;
    float icon_scale;
    float animation_time;
    int animation_frames;
    int unknown_0xd993f97b;
};

void LoadTypedefSLdrHUDHint(SLdrHUDHint&, CInputStream&);

#endif // _SLDRHUDHINT
