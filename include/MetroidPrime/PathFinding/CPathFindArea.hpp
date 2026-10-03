#ifndef _CPATHFINDAREA
#define _CPATHFINDAREA

#include "MetroidPrime/PathFinding/CPFBitSet.hpp"
#include "MetroidPrime/PathFinding/CPFPointSearchState.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/prereserved_vector.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

#include <stdint.h>

class CPFOpenList {
public:
  CPFOpenList();
  void Clear() {
    mRegion.Data()->SetOpenMore(&mRegion);
    mRegion.Data()->SetOpenLess(&mRegion);
    mBitSet.Clear();
  }
  void Push(CPFRegion* region) {
    mBitSet.Add(region->GetIndex());
    CPFRegion* more = mRegion.Data()->GetOpenMore();
    while (more != &mRegion && region->Data()->GetCost() > more->Data()->GetCost()) {
      more = more->Data()->GetOpenMore();
    }
    more->Data()->GetOpenLess()->Data()->SetOpenMore(region);
    region->Data()->SetOpenLess(more->Data()->GetOpenLess());
    more->Data()->SetOpenLess(region);
    region->Data()->SetOpenMore(more);
  }
  CPFRegion* Pop() {
    CPFRegion* region = mRegion.Data()->GetOpenMore();
    CPFRegion* result = nullptr;
    if (region != &mRegion) {
      result = region;
      Pop(region);
    }
    return result;
  }
  void Pop(CPFRegion* region) {
    mBitSet.Rmv(region->GetIndex());
    region->Data()->GetOpenMore()->Data()->SetOpenLess(region->Data()->GetOpenLess());
    region->Data()->GetOpenLess()->Data()->SetOpenMore(region->Data()->GetOpenMore());
    region->Data()->SetOpenMore(nullptr);
    region->Data()->SetOpenLess(nullptr);
  }
  bool Test(CPFRegion* region) {
    if (mBitSet.Test(region->GetIndex())) {
      return true;
    }
    return false;
  }

private:
  CPFBitSet mBitSet;
  CPFRegion mRegion;
  CPFRegionData mRegionData;
};
CHECK_SIZEOF(CPFOpenList, 0xd0)

class CPFAreaOctree {
public:
  void Fixup(CPFArea& area);
  uint GetChildIndex(const CVector3f& point) const;
  rstl::prereserved_vector< CPFRegion* >* GetRegionList(const CVector3f& point);
  void
  GetRegionListList(rstl::reserved_vector< rstl::prereserved_vector< CPFRegion* >*, 32 >& lists,
                    const CVector3f& point, float padding);
  bool IsPointInsidePaddedAABox(const CVector3f& point, float padding) const {
    return point[kDX] >= mBounds.GetMinPoint()[kDX] - padding &&
           point[kDX] <= mBounds.GetMaxPoint()[kDX] + padding &&
           point[kDY] >= mBounds.GetMinPoint()[kDY] - padding &&
           point[kDY] <= mBounds.GetMaxPoint()[kDY] + padding &&
           point[kDZ] >= mBounds.GetMinPoint()[kDZ] - padding &&
           point[kDZ] <= mBounds.GetMaxPoint()[kDZ] + padding;
  }

private:
  bool mIsLeaf;
  CAABox mBounds;
  CVector3f mCenter;
  CPFAreaOctree* mChildren[8];
  rstl::prereserved_vector< CPFRegion* > mRegions;
};
CHECK_SIZEOF(CPFAreaOctree, 0x50)

class CPFArea {
public:
  CPFArea(const rstl::auto_ptr< uchar >& data, int size);
  ~CPFArea();
  const CTransform4f& GetTransform() const { return mTransform; }
  void SetTransform(const CTransform4f& transform);
  int GetPointIndex(const CPFPoint& point) const; // Guessed name
  int& GetPointLink(int index) { return mPointLinks[index]; }
  uint& GetPointLinkData(int index) { return mPointLinkData[index]; }
  CVector3f GetClosestPoint() const { return mClosestPoint; }
  int GetNumRegions() const { return mRegions.size(); }
  CPFRegion& GetRegion(int index) { return mRegions[index]; }
  const CPFNode& GetNode(int index) const { return mNodes[index]; }
  CPFLink& GetLink(int index) { return mLinks[index]; }
  CPFRegionData& GetRegionData(int index) { return mRegionData[index]; }
  CPFAreaOctree& GetOctree(int index) { return mOctree[index]; }
  CPFRegion*& GetOctreeRegionPtrs(int index) { return mOctreeRegions[index]; }
  rstl::prereserved_vector< CPFRegion* >* GetOctreeRegionList(const CVector3f& point);
  CPFOpenList& OpenList() { return mOpenList; }
  CPFBitSet& ClosedSet() { return mClosedSet; }
  int FindRegions(rstl::reserved_vector< CPFRegion*, 8 >& regions, const CVector3f& point,
                  uint flags, uint indexMask, bool ignoreObstructions);
  int FindRegions(rstl::reserved_vector< CPFRegion*, 8 >& regions, const CAABox& box, uint flags,
                  uint indexMask, bool ignoreObstructions);
  CPFRegion* FindClosestRegion(const CVector3f& point, uint flags, uint indexMask, float padding);
  CVector3f FindClosestReachablePoint(rstl::reserved_vector< CPFRegion*, 8 >& regions,
                                      const CVector3f& point, uint flags, uint indexMask);
  bool PathExists(const CPFRegion* source, const CPFRegion* destination, uint flags) const;

private:
  float mBestPointDistSq;
  CVector3f mClosestPoint;
  rstl::vector< CVector3f > mPolyPoints;
  rstl::prereserved_vector< CPFRegion* >* mCachedRegionList;
  CVector3f mCachedRegionListPoint;
  bool mHasCachedRegionList;
  int mRegionFindCookie;
  CPFBitSet mClosedSet;
  CPFOpenList mOpenList;
  rstl::single_ptr< CPFPointSearchState > mPointSearchState;
  int mVersion;
  rstl::single_ptr< uchar > mData;
  rstl::prereserved_vector< CPFNode > mNodes;
  rstl::prereserved_vector< CPFLink > mLinks;
  rstl::prereserved_vector< CPFRegion > mRegions;
  rstl::prereserved_vector< CPFAreaOctree > mOctree;
  rstl::prereserved_vector< CPFRegion* > mOctreeRegions;
  rstl::prereserved_vector< uint > mConnectionsGround;
  rstl::prereserved_vector< uint > mConnectionsFlyers;
  rstl::prereserved_vector< CPFPoint > mPoints;
  rstl::prereserved_vector< int > mPointLinks;
  rstl::prereserved_vector< uint > mPointConnections;
  rstl::prereserved_vector< uint > mPointLinkData; // Guessed name; encoding unresolved.
  rstl::vector< CPFRegionData > mRegionData;
  CTransform4f mTransform;
};
CHECK_SIZEOF(CPFArea, 0x1ec)

inline void CPFAreaOctree::Fixup(CPFArea& area) {
  mIsLeaf = *reinterpret_cast< const int* >(this) != 0;
  if (mIsLeaf) {
    if (mRegions.size() != 0) {
      mRegions.set_data(&area.GetOctreeRegionPtrs(reinterpret_cast< intptr_t >(&mRegions[0])));
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      intptr_t index = reinterpret_cast< intptr_t >(mChildren[i]);
      mChildren[i] = index >= 0 ? &area.GetOctree(index) : nullptr;
    }
  }
}

#endif // _CPATHFINDAREA
