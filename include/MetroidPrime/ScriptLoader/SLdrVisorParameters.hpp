#ifndef _SLDRVISORPARAMETERS
#define _SLDRVISORPARAMETERS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrVisorParameters {
    SLdrVisorParameters();
    ~SLdrVisorParameters();

    bool scanThrough;
    int visorFlags;
};

void LoadTypedefSLdrVisorParameters(SLdrVisorParameters&, CInputStream&);

#endif // _SLDRVISORPARAMETERS
