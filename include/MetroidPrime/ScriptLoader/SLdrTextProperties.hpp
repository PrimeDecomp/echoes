#ifndef _SLDRTEXTPROPERTIES
#define _SLDRTEXTPROPERTIES

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTextProperties {
    SLdrTextProperties();
    ~SLdrTextProperties();

    int text_bounding_width;
    int text_bounding_height;
    float line_spacing;
    int line_extra_space;
    int character_extra_space;
    CColor foreground_color;
    CColor outline_color;
    CColor geometry_color;
    CAssetId default_font;
    int unknown_0x18dd95cd;
    int unknown_0x42091548;
    bool wrap_text;
};

void LoadTypedefSLdrTextProperties(SLdrTextProperties&, CInputStream&);

#endif // _SLDRTEXTPROPERTIES
