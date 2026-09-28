#ifndef _CSTATICGEOMETRYMAP
#define _CSTATICGEOMETRYMAP

#include "Kyoto/TToken.hpp"

class CStaticGeometryMapData;

// Guessed names for the EGMC resource and its runtime token owner.
class CStaticGeometryMap {
public:
  explicit CStaticGeometryMap(const TLockedToken< CStaticGeometryMapData >& data);

private:
  TLockedToken< CStaticGeometryMapData > mData;
};
CHECK_SIZEOF(CStaticGeometryMap, 0xc)

#endif // _CSTATICGEOMETRYMAP
