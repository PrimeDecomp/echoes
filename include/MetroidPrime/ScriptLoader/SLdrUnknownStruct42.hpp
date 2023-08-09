#ifndef _SLDRUNKNOWNSTRUCT42
#define _SLDRUNKNOWNSTRUCT42

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct42 {
    SLdrUnknownStruct42();
    ~SLdrUnknownStruct42();

    float angle;
    CColor cloud_color1;
    CColor cloud_color2;
    CColor add_color1;
    CColor add_color2;
    float cloud_scale;
    float fade_off_size;
    float open_speed;
};

void LoadTypedefSLdrUnknownStruct42(SLdrUnknownStruct42&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT42
