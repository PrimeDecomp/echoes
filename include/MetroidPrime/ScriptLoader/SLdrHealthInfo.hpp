#ifndef _SLDRHEALTHINFO
#define _SLDRHEALTHINFO

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrHealthInfo {
    SLdrHealthInfo();
    ~SLdrHealthInfo();

    float health;
    float hi_knock_back_resistance;
};

void LoadTypedefSLdrHealthInfo(SLdrHealthInfo&, CInputStream&);

#endif // _SLDRHEALTHINFO
