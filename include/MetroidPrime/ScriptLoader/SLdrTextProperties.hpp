#ifndef _SLDRTEXTPROPERTIES
#define _SLDRTEXTPROPERTIES

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTextProperties {
    SLdrTextProperties();
    ~SLdrTextProperties();

    int textBoundingWidth;
    int textBoundingHeight;
    float lineSpacing;
    int lineExtraSpace;
    int characterExtraSpace;
    CColor foregroundColor;
    CColor outlineColor;
    CColor geometryColor;
    CAssetId defaultFont;
    int unknown_0x18dd95cd;
    int unknown_0x42091548;
    bool wrapText;
};

void LoadTypedefSLdrTextProperties(SLdrTextProperties&, CInputStream&);

#endif // _SLDRTEXTPROPERTIES
