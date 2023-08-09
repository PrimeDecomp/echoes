#ifndef _SLDRTRANSFORM
#define _SLDRTRANSFORM

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTransform {
    SLdrTransform();
    ~SLdrTransform();

    CVector3f position;
    CVector3f rotation;
    CVector3f scale;
};

void LoadTypedefSLdrTransform(SLdrTransform&, CInputStream&);

#endif // _SLDRTRANSFORM
