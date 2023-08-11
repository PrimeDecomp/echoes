#ifndef _SLDRDYNAMICLIGHTPARENT
#define _SLDRDYNAMICLIGHTPARENT

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrDynamicLightParent {
    SLdrDynamicLightParent();
    ~SLdrDynamicLightParent();

    CVector3f unknown_0xddd74295;
    CVector3f unknown_0x88f018b3;
    rstl::string locatorName;
    bool useParentRotation;
};

void LoadTypedefSLdrDynamicLightParent(SLdrDynamicLightParent&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTPARENT
