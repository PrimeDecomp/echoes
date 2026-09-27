#include "MetroidPrime/CMapWorldInfo.hpp"

#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CWorldSaveGameInfo.hpp"

#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"
#include "rstl/algorithm.hpp"

CMapWorldInfo::CMapWorldInfo() : mMapStationUsed(false) {}

CMapWorldInfo::CMapWorldInfo(CBitStreamReader& in, const CWorldSaveGameInfo& saveInfo,
                             CAssetId worldId)
: mMapStationUsed(false) {
  const CSaveWorldMemory& worldMemory = gpMemoryCard->GetSaveWorldMemory(worldId);
  int areaCount = worldMemory.GetAreaCount();
  mVisitedAreas.reserve(areaCount);
  for (int i = 0; i < areaCount; ++i) {
    mVisitedAreas.push_back(in.ReadPackedBool());
  }
  mMappedAreas.reserve(areaCount);
  for (int i = 0; i < areaCount; ++i) {
    mMappedAreas.push_back(in.ReadPackedBool());
  }
  for (int i = 0; i < saveInfo.GetDoors().size(); ++i) {
    if (in.ReadBits(1)) {
      SetDoorVisited(TEditorId(saveInfo.GetDoors()[i]), true);
    }
  }
  for (int i = 0; i < saveInfo.GetUnmappableObjects().size(); ++i) {
    if (in.ReadBits(1)) {
      SetObjectUnmapped(TEditorId(saveInfo.GetUnmappableObjects()[i]), true);
    }
  }
  mMapStationUsed = in.ReadPackedBool();
  rstl::sort_by_key(mVisitedDoors);
  rstl::sort_by_key(mUnmappedObjects);
}

void CMapWorldInfo::PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveInfo,
                          CAssetId worldId) const {
  int areaCount = gpMemoryCard->GetSaveWorldMemory(worldId).GetAreaCount();
  for (int i = 0; i < areaCount; ++i) {
    if (i < mVisitedAreas.size()) {
      out.WriteBits(mVisitedAreas[i], 1);
    } else {
      out.WriteBits(0, 1);
    }
  }
  for (int i = 0; i < areaCount; ++i) {
    if (i < mMappedAreas.size()) {
      out.WriteBits(mMappedAreas[i], 1);
    } else {
      out.WriteBits(0, 1);
    }
  }

  {
    rstl::bit_vector<> doors(saveInfo.GetDoors().size(), false);
    for (int i = 0; i < saveInfo.GetDoors().size(); ++i) {
      if (IsDoorVisited(TEditorId(saveInfo.GetDoors()[i]))) {
        doors[i] = true;
      }
    }
    for (int i = 0; i < doors.size(); ++i) {
      out.WriteBits(doors[i], 1);
    }
  }

  {
    rstl::bit_vector<> objects(saveInfo.GetUnmappableObjects().size(), false);
    for (int i = 0; i < saveInfo.GetUnmappableObjects().size(); ++i) {
      if (IsObjectUnmapped(TEditorId(saveInfo.GetUnmappableObjects()[i]))) {
        objects[i] = true;
      }
    }
    for (int i = 0; i < objects.size(); ++i) {
      out.WriteBits(objects[i], 1);
    }
  }

  out.WriteBits(mMapStationUsed != false, 1);
}

void CMapWorldInfo::SetDoorVisited(TEditorId eid, bool visited) {
  mVisitedDoors.reserve(mVisitedDoors.size() + 1);
  rstl::vector< rstl::pair< TEditorId, bool > >::iterator it = rstl::lower_bound(
      mVisitedDoors.begin(), mVisitedDoors.end(), eid,
      rstl::default_pair_sorter_finder< rstl::vector< rstl::pair< TEditorId, bool > > >());
  if (it == mVisitedDoors.end() || it->first != eid) {
    mVisitedDoors.insert(it, rstl::pair< TEditorId, bool >(eid, visited));
  } else {
    it->second = visited;
  }
}

bool CMapWorldInfo::IsDoorVisited(TEditorId eid) const {
  rstl::vector< rstl::pair< TEditorId, bool > >::iterator it = rstl::lower_bound(
      mVisitedDoors.begin(), mVisitedDoors.end(), eid,
      rstl::default_pair_sorter_finder< rstl::vector< rstl::pair< TEditorId, bool > > >());
  if (it == mVisitedDoors.end()) {
    return false;
  }
  if (it->first != eid) {
    return false;
  }
  return it->second;
}

void CMapWorldInfo::SetObjectUnmapped(TEditorId eid, bool unmapped) {
  mUnmappedObjects.reserve(mUnmappedObjects.size() + 1);
  rstl::vector< rstl::pair< TEditorId, bool > >::iterator it = rstl::lower_bound(
      mUnmappedObjects.begin(), mUnmappedObjects.end(), eid,
      rstl::default_pair_sorter_finder< rstl::vector< rstl::pair< TEditorId, bool > > >());
  if (it == mUnmappedObjects.end() || it->first != eid) {
    mUnmappedObjects.insert(it, rstl::pair< TEditorId, bool >(eid, unmapped));
  } else {
    it->second = unmapped;
  }
}

bool CMapWorldInfo::IsObjectUnmapped(TEditorId eid) const {
  rstl::vector< rstl::pair< TEditorId, bool > >::iterator it = rstl::lower_bound(
      mUnmappedObjects.begin(), mUnmappedObjects.end(), eid,
      rstl::default_pair_sorter_finder< rstl::vector< rstl::pair< TEditorId, bool > > >());
  if (it == mUnmappedObjects.end()) {
    return false;
  }
  if (it->first != eid) {
    return false;
  }
  return it->second;
}

void CMapWorldInfo::SetAreaVisited(TAreaId areaId, bool visited) {
  if (areaId.Value() + 1 > mVisitedAreas.size()) {
    mVisitedAreas.reserve(areaId.Value() + 1);
    mVisitedAreas.insert(mVisitedAreas.end(), areaId.Value() - mVisitedAreas.size() + 1, false);
  }
  mVisitedAreas[areaId.Value()] = visited;
}

void CMapWorldInfo::SetIsMapped(TAreaId areaId, bool mapped) {
  if (areaId.Value() + 1 > mMappedAreas.size()) {
    mMappedAreas.reserve(areaId.Value() + 1);
    mMappedAreas.insert(mMappedAreas.end(), areaId.Value() - mMappedAreas.size() + 1, false);
  }
  mMappedAreas[areaId.Value()] = mapped;
}

bool CMapWorldInfo::IsWorldVisible(TAreaId areaId, bool inDarkWorld) const {
  if (IsMapped(areaId)) {
    return true;
  }
  if (mMapStationUsed) {
    return !inDarkWorld;
  }
  return false;
}

bool CMapWorldInfo::IsMapped(TAreaId areaId) const {
  if (areaId.Value() + 1 > mMappedAreas.size()) {
    mMappedAreas.reserve(areaId.Value() + 1);
    mMappedAreas.insert(mMappedAreas.end(), areaId.Value() - mMappedAreas.size() + 1, false);
  }
  return mMappedAreas[areaId.Value()];
}

bool CMapWorldInfo::IsAreaVisited(TAreaId areaId) const {
  if (areaId.Value() + 1 > mVisitedAreas.size()) {
    mVisitedAreas.reserve(areaId.Value() + 1);
    mVisitedAreas.insert(mVisitedAreas.end(), areaId.Value() - mVisitedAreas.size() + 1, false);
  }
  return mVisitedAreas[areaId.Value()];
}

bool CMapWorldInfo::IsAreaVisible(TAreaId areaId) const {
  return IsAreaVisited(areaId) || IsMapped(areaId);
}

bool CMapWorldInfo::IsAnythingSet() {
  for (int i = 0; i < mVisitedAreas.size(); ++i) {
    if (mVisitedAreas[i]) {
      return true;
    }
  }
  for (int i = 0; i < mMappedAreas.size(); ++i) {
    if (mMappedAreas[i]) {
      return true;
    }
  }
  return mMapStationUsed;
}
