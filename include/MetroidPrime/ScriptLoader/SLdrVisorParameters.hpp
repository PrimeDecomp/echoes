#ifndef _SLDRVISORPARAMETERS
#define _SLDRVISORPARAMETERS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrVisorParameters {
    SLdrVisorParameters();
    ~SLdrVisorParameters();

    bool scan_through;
    int visor_flags;
};

void LoadTypedefSLdrVisorParameters(SLdrVisorParameters&, CInputStream&);

#endif // _SLDRVISORPARAMETERS
