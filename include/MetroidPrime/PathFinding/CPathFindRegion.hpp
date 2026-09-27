#ifndef _CPATHFINDREGION
#define _CPATHFINDREGION

#include <Kyoto/Math/CAABox.hpp>
#include <Kyoto/Math/CVector3f.hpp>
#include <rstl/vector.hpp>

class CPFArea;

// The obstruction categories are not yet identified.
enum EPathFindObstructions {
  kPFO_Unknown0,
  kPFO_Unknown1,
  kPFO_Unknown2,
};

class CPFPoint {
public:
  void Fixup(CPFArea& area);
  const CVector3f& GetPosition() const { return mPosition; }
  void SetPosition(const CVector3f& position) { mPosition = position; }

private:
  CVector3f mPosition;
  uint xc_;
  int mNumLinks;
  int* mLinks;
  uint* mLinkData; // Guessed name; per-link data encoding remains unresolved.
};
CHECK_SIZEOF(CPFPoint, 0x1c)

class CPFNode {
public:
  const CVector3f& GetPos() const { return mPosition; }
  const CVector3f& GetNormal() const { return mNormal; }

private:
  CVector3f mPosition;
  CVector3f mNormal;
};
CHECK_SIZEOF(CPFNode, 0x18)

class CPFLink {
public:
  int GetNode() const { return mNode; }
  int GetRegion() const { return mRegion; }
  float Get2dWidth() const { return mWidth2d; }
  float GetOO2dWidth() const { return mOo2dWidth; }

private:
  int mNode;
  int mRegion;
  float mWidth2d;
  float mOo2dWidth;
};
CHECK_SIZEOF(CPFLink, 0x10)

class CPFRegion;
class CPFRegionData {
public:
  CPFRegionData();

  void SetOpenLess(CPFRegion* region) { mOpenLess = region; }
  CPFRegion* GetOpenLess() { return mOpenLess; }

  void SetOpenMore(CPFRegion* region) { mOpenMore = region; }
  CPFRegion* GetOpenMore() { return mOpenMore; }

  float GetCost() { return mCost; }

  CPFRegion* GetParent() { return mParent; }
  void Setup(CPFRegion* region, float g) {
    mParent = region;
    mG = g;
    mCost = mG + mH;
  }
  void Setup(CPFRegion* region, float g, float h) {
    mParent = region;
    mG = g;
    mH = h;
    mCost = mG + mH;
  }
  float GetG() { return mG; }
  int GetPathLink() const { return mParentLink; }
  void SetPathLink(int link) { mParentLink = link; }

  void SetBestPoint(const CVector3f& point) { mBestPoint = point; }
  const CVector3f& GetBestPoint() const { return mBestPoint; }
  void SetBestDistanceSquared(float distance) { mBestPointDistSq = distance; }
  float GetBestDistanceSquared() const { return mBestPointDistSq; }

  void SetCookie(int cookie) { mCookie = cookie; }
  int GetCookie() const { return mCookie; }
  uint GetAvoidanceFlags() const { return mAvoidanceFlags; }

private:
  float mBestPointDistSq;
  CVector3f mBestPoint;
  int mCookie;
  float mCost;
  float mG;
  float mH;
  CPFRegion* mParent;
  CPFRegion* mOpenLess;
  CPFRegion* mOpenMore;
  int mParentLink;
  uint mAvoidanceFlags; // Guessed name
};
CHECK_SIZEOF(CPFRegionData, 0x34)

class CPFRegion {
public:
  // Guessed enumerator names; the enum type is exported by Echoes Wii.
  enum ERootPosition { kRP_Center, kRP_Bottom };
  CPFRegion();
  void Fixup(CPFArea& area, int& numNodes);
  void SetData(CPFRegionData* data) { mData = data; }
  CPFRegionData* Data() const { return mData; }
  int GetIndex() const { return mRegionIdx; }
  uint GetFlags() const { return mFlags; }
  int GetNumLinks() const { return mNumLinks; }
  const CPFLink* GetLink(int index) const { return &mStartLink[index]; }
  const CPFLink* GetPathLink() const { return &mStartLink[mData->GetPathLink()]; }
  const CVector3f& GetCentroid() const { return mCentroid; }
  void SetCentroid(const CVector3f& point) { mCentroid = point; }
  float GetHeight() const { return mHeight; }
  int GetNumNodes() const { return mNumNodes; }
  const CPFNode& GetNode(int index) const { return mStartNode[index]; }
  const CVector3f& GetNormal() const { return mNormal; }
  bool IsPointInside(const CVector3f& point) const;
  bool Intersects(const CAABox& box) const; // Guessed name
  void ModifyObstructionCount(EPathFindObstructions obstruction, int delta);
  int GetObstructionCount(EPathFindObstructions obstruction) const;
  bool IsObstructed(uint flags) const {
    return mObstructionCounts[2] > 0 || ((flags & 0x100) != 0 && mObstructionCounts[0] > 0) ||
           ((flags & 0x200) != 0 && mObstructionCounts[1] > 0);
  }
  bool IsPointInsidePaddedAABox(const CVector3f& point, float padding) const {
    return point[kDX] >= mBounds.GetMinPoint()[kDX] - padding &&
           point[kDX] <= mBounds.GetMaxPoint()[kDX] + padding &&
           point[kDY] >= mBounds.GetMinPoint()[kDY] - padding &&
           point[kDY] <= mBounds.GetMaxPoint()[kDY] + padding &&
           point[kDZ] >= mBounds.GetMinPoint()[kDZ] - padding &&
           point[kDZ] <= mBounds.GetMaxPoint()[kDZ] + padding;
  }
  float PointHeight(const CVector3f& point) const;
  bool FindClosestPointOnPolygon(const rstl::vector< CVector3f >& polyPoints,
                                 const CVector3f& normal, const CVector3f& point,
                                 bool excludePolyPoints);
  bool FindBestPoint(rstl::vector< CVector3f >& polyPoints, const CVector3f& point, uint flags,
                     float paddingSq);
  void SetLinkTo(int index);
  void DropToGround(CVector3f& point) const;
  CVector3f GetLinkMidPoint(const CPFLink& link) const;
  CVector3f FitThroughLink2d(const CVector3f& source, const CPFLink& link,
                             const CVector3f& destination, float radius) const;
  CVector3f FitThroughLink3d(const CVector3f& source, const CPFLink& link, float height,
                             const CVector3f& destination, const float radius, float halfHeight,
                             ERootPosition rootPosition) const;

private:
  int mNumNodes;
  const CPFNode* mStartNode;
  int mNumLinks;
  CPFLink* mStartLink;
  uint mFlags;
  float mHeight;
  CVector3f mNormal;
  int mRegionIdx;
  CVector3f mCentroid;
  CAABox mBounds;
  int mObstructionCounts[3];
  CPFRegionData* mData;
};
CHECK_SIZEOF(CPFRegion, 0x5c)

#endif // _CPATHFINDREGION
