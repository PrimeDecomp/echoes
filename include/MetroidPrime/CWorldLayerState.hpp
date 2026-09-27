#ifndef _CWORLDLAYERSTATE
#define _CWORLDLAYERSTATE

#include "types.h"

#include "MetroidPrime/CWorldLayers.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

// Name from CWorldState; interface adapted from Prime's CScriptLayerManager.
class CWorldLayerState {
public:
  void InitializeWorldLayers(const rstl::vector< CWorldLayers::Area >& areas,
                             const rstl::rc_ptr< rstl::vector< rstl::string > >& names,
                             const rstl::rc_ptr< rstl::vector< int > >& indices);
};

#endif // _CWORLDLAYERSTATE
