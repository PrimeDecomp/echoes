#ifndef _CSTATICGEOMETRYMAP
#define _CSTATICGEOMETRYMAP

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

// Guessed names; the EGMC stream contains model-index/editor-ID pairs.
class CStaticGeometryMapData {
public:
  typedef rstl::pair< int, TEditorId > TMapping;
  const rstl::vector< TMapping >& GetMappings() const { return mMappings; }

private:
  rstl::vector< TMapping > mMappings;
};
CHECK_SIZEOF(CStaticGeometryMapData, 0x10)

// Guessed names for the EGMC resource and its runtime token owner.
class CStaticGeometryMap {
public:
  explicit CStaticGeometryMap(const TLockedToken< CStaticGeometryMapData >& data);
  const CStaticGeometryMapData& GetData() const;

private:
  TLockedToken< CStaticGeometryMapData > mData;
};
CHECK_SIZEOF(CStaticGeometryMap, 0xc)

#endif // _CSTATICGEOMETRYMAP
