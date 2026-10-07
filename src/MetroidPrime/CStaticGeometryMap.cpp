#include "MetroidPrime/CStaticGeometryMap.hpp"

#include "Kyoto/CFactoryMgr.hpp"

CStaticGeometryMapData::CStaticGeometryMapData(CInputStream& in) {
  const int count = in.ReadInt32();
  mMappings.reserve(count);
  for (int i = 0; i < count; ++i) {
    const uint index = in.ReadInt32();
    const TEditorId editorId(in);
    mMappings.push_back_unsafe(TMapping(index, editorId));
  }
}

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
