#ifndef _SLDRFLAREDEF
#define _SLDRFLAREDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrFlareDef {
    SLdrFlareDef();
    ~SLdrFlareDef();

    CAssetId texture;
    float position;
    float scale;
    CColor color;
};

void LoadTypedefSLdrFlareDef(SLdrFlareDef&, CInputStream&);

#endif // _SLDRFLAREDEF
