#ifndef _SLDRCONNECTION
#define _SLDRCONNECTION

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/vector.hpp"

struct SLdrConnection {
    SLdrConnection();
    ~SLdrConnection();

    short connection_index;
    rstl::vector< float > activation_times;
    bool unknown;
};

void LoadTypedefSLdrConnection(SLdrConnection&, CInputStream&);

#endif // _SLDRCONNECTION
