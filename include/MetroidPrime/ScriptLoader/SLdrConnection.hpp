#ifndef _SLDRCONNECTION
#define _SLDRCONNECTION

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/vector.hpp"

struct SLdrConnection {
    SLdrConnection();
    ~SLdrConnection();

    short connectionIndex;
    rstl::vector< float > activationTimes;
    bool unknown;
};

void LoadTypedefSLdrConnection(SLdrConnection&, CInputStream&);

#endif // _SLDRCONNECTION
