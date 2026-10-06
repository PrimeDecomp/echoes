#include "MetroidPrime/CStaticGeometryMap.hpp"

#include "Kyoto/CFactoryMgr.hpp"

CStaticGeometryMapData::CStaticGeometryMapData(CInputStream& in) {}

CStaticGeometryMapData::~CStaticGeometryMapData() {}

CStaticGeometryMap::CStaticGeometryMap(const TLockedToken< CStaticGeometryMapData >& data)
: mData(data) {}

const CStaticGeometryMapData& CStaticGeometryMap::GetData() const {
  CStaticGeometryMapData* data = *mData;
  return *data;
}

CFactoryFnReturn FEditorGeometryToStaticGeometryFactory(const SObjectTag& tag, CInputStream& in,
                                                        const CVParamTransfer& xfer) {
  return rs_new CStaticGeometryMapData(in);
}
