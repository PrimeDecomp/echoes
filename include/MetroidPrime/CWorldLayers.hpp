#ifndef _CWORLDLAYERS
#define _CWORLDLAYERS

#include "types.h"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CWorldLayers {
public:
  class Area {
  public:
    explicit Area(CInputStream& in) : mLayerCount(in.ReadInt32()), mLayerBits(in.ReadInt64()) {}

    int mLayerCount;
    u64 mLayerBits;
  };

  static void ReadWorldLayers(CInputStream& in, int version, CAssetId mlvlId);
};
NESTED_CHECK_SIZEOF(CWorldLayers, Area, 0x10)

#endif // _CWORLDLAYERS
