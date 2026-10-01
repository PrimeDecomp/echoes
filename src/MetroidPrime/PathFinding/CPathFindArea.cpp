#include "MetroidPrime/PathFinding/CPathFindArea.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/algorithm.hpp"

#include <dolphin/os.h>
#include <float.h>

class CVParamTransfer;

class CPFMemoryStream {
public:
  CPFMemoryStream(uchar* data, int size) : mData(data), mSize(size), mCurrent(data) {}
  int ReadInt32() {
    int value = *reinterpret_cast< int* >(mCurrent);
    mCurrent += sizeof(int);
    return value;
  }
  void* GetBlock(int count, int size) {
    void* block = mCurrent;
    mCurrent += count * size;
    return block;
  }

private:
  uchar* mData;
  int mSize;
  uchar* mCurrent;
};

inline uint CPFAreaOctree::GetChildIndex(const CVector3f& point) const {
  uint index = 0;
  if (point[kDX] > mCenter[kDX]) {
    index = 1;
  }
  if (point[kDY] > mCenter[kDY]) {
    index |= 2;
  }
  if (point[kDZ] > mCenter[kDZ]) {
    index |= 4;
  }
  return index;
}

inline rstl::prereserved_vector< CPFRegion* >*
CPFAreaOctree::GetRegionList(const CVector3f& point) {
  if (mIsLeaf) {
    return &mRegions;
  }
  return mChildren[GetChildIndex(point)]->GetRegionList(point);
}

inline void CPFAreaOctree::GetRegionListList(
    rstl::reserved_vector< rstl::prereserved_vector< CPFRegion* >*, 32 >& lists,
    const CVector3f& point, float padding) {
  if (lists.size() >= lists.capacity()) {
    return;
  }
  if (mIsLeaf) {
    lists.push_back(&mRegions);
  } else {
    for (int i = 0; i < 8; ++i) {
      if (mChildren[i]->IsPointInsidePaddedAABox(point, padding)) {
        mChildren[i]->GetRegionListList(lists, point, padding);
      }
    }
  }
}

CPFArea::CPFArea(const rstl::auto_ptr< uchar >& data, int size)
: mBestPointDistSq(FLT_MAX)
, mClosestPoint(CVector3f::Zero())
, mCachedRegionList(nullptr)
, mCachedRegionListPoint(CVector3f::Zero())
, mHasCachedRegionList(false)
, mRegionFindCookie(0)
, mVersion(-1)
, mData(data.release())
, mTransform(CTransform4f::Identity()) {
  CPFMemoryStream stream(mData.get(), size);
  mVersion = stream.ReadInt32();

  int numNodes = stream.ReadInt32();
  mNodes.set_size(numNodes);
  mNodes.set_data(static_cast< CPFNode* >(stream.GetBlock(numNodes, sizeof(CPFNode))));
  int numLinks = stream.ReadInt32();
  mLinks.set_size(numLinks);
  mLinks.set_data(static_cast< CPFLink* >(stream.GetBlock(numLinks, sizeof(CPFLink))));
  const int numRegions = stream.ReadInt32();
  mRegions.set_size(numRegions);
  mRegions.set_data(static_cast< CPFRegion* >(stream.GetBlock(numRegions, sizeof(CPFRegion))));
  mRegionData.reserve(numRegions);
  CPFRegionData dataValue = CPFRegionData();
  mRegionData.resize(numRegions, dataValue);
  int maxRegionNodes = 0;
  int i;
  for (i = 0; i < numRegions; ++i) {
    mRegions[i].Fixup(*this, maxRegionNodes);
  }
  maxRegionNodes = maxRegionNodes > 4 ? maxRegionNodes : 4;
  mPolyPoints.reserve(maxRegionNodes);

  int numWords = (numRegions * (numRegions - 1) / 2 + 31) / 32;
  mConnectionsGround.set_size(numWords);
  mConnectionsGround.set_data(static_cast< uint* >(stream.GetBlock(numWords, sizeof(uint))));
  mConnectionsFlyers.set_size(numWords);
  mConnectionsFlyers.set_data(static_cast< uint* >(stream.GetBlock(numWords, sizeof(uint))));
  stream.GetBlock(((numRegions * numRegions + 31) / 32 - numWords) * 2, sizeof(uint));

  int numRegionPtrs = stream.ReadInt32();
  mOctreeRegions.set_size(numRegionPtrs);
  mOctreeRegions.set_data(
      static_cast< CPFRegion** >(stream.GetBlock(numRegionPtrs, sizeof(CPFRegion*))));
  for (i = 0; i < numRegionPtrs; ++i) {
    CPFRegion* const& region = mOctreeRegions[i];
    mOctreeRegions[i] = &mRegions[reinterpret_cast< intptr_t >(region)];
  }
  int numOctreeNodes = stream.ReadInt32();
  mOctree.set_size(numOctreeNodes);
  mOctree.set_data(
      static_cast< CPFAreaOctree* >(stream.GetBlock(numOctreeNodes, sizeof(CPFAreaOctree))));
  for (i = 0; i < numOctreeNodes; ++i) {
    mOctree[i].Fixup(*this);
  }
  if (mVersion > 4) {
    const int numPoints = stream.ReadInt32();
    if (numPoints > 0) {
      mPoints.set_size(numPoints);
      mPoints.set_data(static_cast< CPFPoint* >(stream.GetBlock(numPoints, sizeof(CPFPoint))));
      const int numPointLinks = stream.ReadInt32();
      mPointLinks.set_size(numPointLinks);
      mPointLinks.set_data(static_cast< int* >(stream.GetBlock(numPointLinks, sizeof(int))));
      const int numLinkData = stream.ReadInt32();
      mPointLinkData.set_size(numLinkData);
      mPointLinkData.set_data(static_cast< uint* >(stream.GetBlock(numLinkData, sizeof(uint))));
      const int numPointWords = (numPoints * (numPoints - 1) / 2 + 31) / 32;
      mPointConnections.set_size(numPointWords);
      mPointConnections.set_data(
          static_cast< uint* >(stream.GetBlock(numPointWords, sizeof(uint))));
      for (i = 0; i < numPoints; ++i) {
        mPoints[i].Fixup(*this);
      }
    }
    // TODO: identify the point-search workspace's native class before binding its constructor.
  }
}

rstl::prereserved_vector< CPFRegion* >* CPFArea::GetOctreeRegionList(const CVector3f& point) {
  if (mHasCachedRegionList && close_enough(point, mCachedRegionListPoint)) {
    return mCachedRegionList;
  }
  return mOctree.back().GetRegionList(point);
}

int CPFArea::FindRegions(rstl::reserved_vector< CPFRegion*, 8 >& regions, const CVector3f& point,
                         uint flags, uint indexMask, bool ignoreObstructions) {
  rstl::prereserved_vector< CPFRegion* >* list = GetOctreeRegionList(point);
  for (int i = 0; i < list->size(); ++i) {
    CPFRegion* region = (*list)[i];
    if ((region->GetFlags() & 0xff & flags) && ((region->GetFlags() >> 16) & 0xff & indexMask) &&
        region->IsPointInside(point) && (ignoreObstructions || !region->IsObstructed(flags)) &&
        ((flags & 2) || (flags & 4) || region->PointHeight(point) < 3.f)) {
      regions.push_back(region);
      if (regions.size() == regions.capacity()) {
        break;
      }
    }
  }
  return regions.size();
}

int CPFArea::FindRegions(rstl::reserved_vector< CPFRegion*, 8 >& regions, const CAABox& box,
                         uint flags, uint indexMask, bool ignoreObstructions) {
  for (int i = 0; i < mOctreeRegions.size(); ++i) {
    CPFRegion* region = mOctreeRegions[i];
    if ((region->GetFlags() & 0xff & flags) && ((region->GetFlags() >> 16) & 0xff & indexMask) &&
        region->Intersects(box) && (ignoreObstructions || !region->IsObstructed(flags)) &&
        ((flags & 6) ||
         region->PointHeight(box.ClosestPointAlongVector(region->GetNormal())) < 3.f)) {
      regions.push_back(region);
      if (regions.size() == regions.capacity()) {
        break;
      }
    }
  }
  return regions.size();
}

CPFRegion* CPFArea::FindClosestRegion(const CVector3f& point, uint flags, uint indexMask,
                                      float padding) {
  rstl::reserved_vector< rstl::prereserved_vector< CPFRegion* >*, 32 > lists;
  CPFRegion* result = nullptr;
  OSGetTick();
  int i, j;
  uint searchTicks = 0;
  mOctree.back().GetRegionListList(lists, point, padding);
  OSGetTick();
  for (i = 0; i < lists.size(); ++i) {
    rstl::prereserved_vector< CPFRegion* >* list = lists[i];
    for (j = 0; j < list->size(); ++j) {
      CPFRegion* region = (*list)[j];
      if (region->Data()->GetCookie() != mRegionFindCookie) {
        region->Data()->SetCookie(mRegionFindCookie);
        if ((region->GetFlags() & 0xff & flags) &&
            ((region->GetFlags() >> 16) & 0xff & indexMask) &&
            region->IsPointInsidePaddedAABox(point, padding) && !region->IsObstructed(flags)) {
          uint startTick = OSGetTick();
          if ((flags & 6) || region->PointHeight(point) < 3.f) {
            if (region->FindBestPoint(mPolyPoints, point, flags, padding * padding)) {
              padding = CMath::FastSqrtF(region->Data()->GetBestDistanceSquared());
              result = region;
              mClosestPoint = region->Data()->GetBestPoint();
            }
            searchTicks += OSGetTick() - startTick;
          }
        }
      }
    }
  }
  OSGetTick();
  ++mRegionFindCookie;
  return result;
}

CVector3f CPFArea::FindClosestReachablePoint(rstl::reserved_vector< CPFRegion*, 8 >& regions,
                                             const CVector3f& point, uint flags, uint indexMask) {
  CVector3f result = CVector3f::Zero();
  float closestDistanceSq = FLT_MAX;
  for (int i = 0; i < GetNumRegions(); ++i) {
    CPFRegion& region = GetRegion(i);
    if ((region.GetFlags() & 0xff & flags) && ((region.GetFlags() >> 16) & 0xff & indexMask) &&
        !region.IsObstructed(flags)) {
      for (int j = 0; j < regions.size(); ++j) {
        CPFRegion* source = regions[j];
        if (PathExists(source, &region, flags)) {
          const CVector3f& delta = region.GetCentroid() - point;
          float distanceSq = delta.MagSquared();
          if (distanceSq < closestDistanceSq) {
            closestDistanceSq = distanceSq;
            result = region.GetCentroid();
            break;
          }
        }
      }
    }
  }
  return result;
}

bool CPFArea::PathExists(const CPFRegion* source, const CPFRegion* destination, uint flags) const {
  if (source == destination || (flags & 0x14)) {
    return true;
  }
  int numRegions = GetNumRegions();
  int sourceIndex = source->GetIndex();
  int destinationIndex = destination->GetIndex();
  if (sourceIndex > destinationIndex) {
    rstl::swap(sourceIndex, destinationIndex);
  }
  int totalConnections = numRegions * (numRegions - 1) / 2;
  int remainingConnections = (numRegions - sourceIndex - 1) * (numRegions - sourceIndex) / 2;
  uint bit = totalConnections - remainingConnections + destinationIndex - (sourceIndex + 1);
  if (flags & 2) {
    return (mConnectionsFlyers[bit / 32] >> (bit % 32)) & 1;
  }
  return (mConnectionsGround[bit / 32] >> (bit % 32)) & 1;
}

void CPFArea::SetTransform(const CTransform4f& transform) {
  const CTransform4f delta = mTransform.GetInverse() * transform;
  for (int i = 0; i < mPoints.size(); ++i) {
    mPoints[i].SetPosition(transform.GetTranslation() + delta.Rotate(mPoints[i].GetPosition()));
  }
  mTransform = transform;
}

int CPFArea::GetPointIndex(const CPFPoint& point) const { return &point - &mPoints[0]; }

CPFArea::~CPFArea() {}

CFactoryFnReturn FPathFindAreaFactory(const SObjectTag& tag,
                                            const rstl::auto_ptr< uchar >& data, int size,
                                            const CVParamTransfer& xfer) {
  return rs_new CPFArea(data, size);
}
