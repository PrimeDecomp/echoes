#ifndef _SLDRUNKNOWNSTRUCT42
#define _SLDRUNKNOWNSTRUCT42

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct42 {
    SLdrUnknownStruct42();
    ~SLdrUnknownStruct42();

    float angle;
    CColor cloudColor1;
    CColor cloudColor2;
    CColor addColor1;
    CColor addColor2;
    float cloudScale;
    float fadeOffSize;
    float openSpeed;
};

void LoadTypedefSLdrUnknownStruct42(SLdrUnknownStruct42&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT42
