#include "MetroidPrime/CWorldLayerState.hpp"

#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"

static const rstl::string skEmptyString(rstl::string::literal_t(), "");

CWorldLayerState::CWorldLayerState() {}

CWorldLayerState::CWorldLayerState(CBitStreamReader& in) {
  const uint count = in.ReadBits(10);
  mSaveLayers.reserve(count);
  for (uint i = 0; i < count; ++i) {
    mSaveLayers.push_back(in.ReadBits(1) != 0);
  }
}

void CWorldLayerState::PutTo(CBitStreamWriter& out) const {
  uint totalLayerCount = 0;
  const int areaCount = mAreaLayers.size();
  for (int i = 0; i < areaCount; ++i) {
    totalLayerCount += GetAreaLayerCount(TAreaId(i)) - 1;
  }
  out.WriteBits(totalLayerCount, 10);
  for (int i = 0; i < areaCount; ++i) {
    const int layerCount = GetAreaLayerCount(TAreaId(i));
    for (int layer = 1; layer < layerCount; ++layer) {
      out.WriteBits(IsLayerActive(TAreaId(i), TLayerId(layer)) ? 1 : 0, 1);
    }
  }
}

void CWorldLayerState::SetLayerActive(TAreaId areaId, TLayerId layer, bool active) {
  CWorldLayers::Area& area = mAreaLayers[areaId.Value()];
  const int layerId = layer.Value();
  if (active) {
    area.mLayerBits |= 1 << layerId;
  } else {
    area.mLayerBits &= ~(1 << layerId);
  }
}

bool CWorldLayerState::IsLayerActive(TAreaId area, TLayerId layer) const {
  const u64& bits = mAreaLayers[area.Value()].mLayerBits;
  return (bits & (1 << layer.Value())) != 0;
}

const rstl::string& CWorldLayerState::GetLayerName(TAreaId area, TLayerId layer) const {
  const int areaId = area.Value();
  const int layerId = layer.Value();
  if (areaId < 0 || areaId >= mAreaLayers.size()) {
    return skEmptyString;
  }
  const CWorldLayers::Area& areaLayers = mAreaLayers[areaId];
  if (layerId < 0 || layerId >= areaLayers.mLayerCount) {
    return skEmptyString;
  }
  if (mLayerNames.IsNull() || mLayerNameOffsets.IsNull()) {
    return skEmptyString;
  }
  if (areaId > mLayerNameOffsets->size()) {
    return skEmptyString;
  }
  const int index = (*mLayerNameOffsets)[areaId] + layerId;
  if (index > mLayerNames->size()) {
    return skEmptyString;
  }
  return (*mLayerNames)[index];
}

void CWorldLayerState::InitializeWorldLayers(
    const rstl::vector< CWorldLayers::Area >& areas,
    const rstl::rc_ptr< rstl::vector< rstl::string > >& names,
    const rstl::rc_ptr< rstl::vector< int > >& indices) {
  if (!mAreaLayers.empty()) {
    return;
  }

  mAreaLayers = areas;
  mLayerNameOffsets = indices;
  mLayerNames = names;
  if (mSaveLayers.size() == 0) {
    return;
  }

  int bit = 0;
  const int areaCount = mAreaLayers.size();
  for (int i = 0; i < areaCount; ++i) {
    const int layerCount = GetAreaLayerCount(TAreaId(i));
    for (int layer = 1; layer < layerCount; ++layer) {
      SetLayerActive(TAreaId(i), TLayerId(layer), mSaveLayers[bit++]);
    }
  }
  mSaveLayers = rstl::bit_vector< rstl::rmemory_allocator >();
}

int CWorldLayerState::GetAreaLayerCount(TAreaId area) const {
  return mAreaLayers[area.Value()].mLayerCount;
}

const rstl::vector< CWorldLayers::Area >& CWorldLayerState::GetAreaLayers() const {
  return mAreaLayers;
}

const rstl::rc_ptr< rstl::vector< rstl::string > >& CWorldLayerState::GetLayerNames() const {
  return mLayerNames;
}

const rstl::rc_ptr< rstl::vector< int > >& CWorldLayerState::GetLayerNameOffsets() const {
  return mLayerNameOffsets;
}
