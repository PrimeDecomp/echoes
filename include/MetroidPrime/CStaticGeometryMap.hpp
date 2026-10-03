#ifndef _CSTATICGEOMETRYMAP
#define _CSTATICGEOMETRYMAP

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

// Guessed EGMC class and member names
class CStaticGeometryMapData {
public:
  typedef rstl::pair< uint, TEditorId > TMapping;
  explicit CStaticGeometryMapData(CInputStream& in);
  ~CStaticGeometryMapData();
  const rstl::vector< TMapping >& GetMappings() const { return mMappings; }

private:
  rstl::vector< TMapping > mMappings;
};
CHECK_SIZEOF(CStaticGeometryMapData, 0x10)

// Guessed runtime-owner class and member names
class CStaticGeometryMap {
public:
  explicit CStaticGeometryMap(const TLockedToken< CStaticGeometryMapData >& data);
  const CStaticGeometryMapData& GetData() const;

private:
  TLockedToken< CStaticGeometryMapData > mData;
};
CHECK_SIZEOF(CStaticGeometryMap, 0xc)

class CFactoryFnReturn;
class CVParamTransfer;
// Guessed name
CFactoryFnReturn FEditorGeometryToStaticGeometryFactory(const SObjectTag& tag, CInputStream& in,
                                                        const CVParamTransfer& xfer);

#endif // _CSTATICGEOMETRYMAP
