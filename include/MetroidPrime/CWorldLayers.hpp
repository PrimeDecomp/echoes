#ifndef _CWORLDLAYERS
#define _CWORLDLAYERS

#include "types.h"

class CWorldLayers {
public:
  class Area {
  public:
    int mLayerCount;
    u64 mLayerBits;
  };
};
NESTED_CHECK_SIZEOF(CWorldLayers, Area, 0x10)

#endif // _CWORLDLAYERS
