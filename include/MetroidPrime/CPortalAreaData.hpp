#ifndef _CPORTALAREADATA
#define _CPORTALAREADATA

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CFactoryFnReturn;
class CVParamTransfer;
class SObjectTag;

// PTLA resource. Nested type and method names are target-derived reconstructions.
class CPortalAreaData {
public:
  struct SBspNode {
    explicit SBspNode(CInputStream& in);

    CPlane mPlane;
    short mFront;
    short mBack;
  };

  struct SVolume {
    explicit SVolume(CInputStream& in);
    bool Intersects(const CAABox& bounds, int node) const;

    rstl::vector< SBspNode > mNodes;
    ushort mPortalIndexStart;
    CAABox mBounds;
  };

  struct SPortal {
    explicit SPortal(CInputStream& in);
    CVector3f GetCenterPoint() const;
    float DistanceToPoint(const CVector3f& point) const;

    rstl::reserved_vector< CVector3f, 4 > mVertices;
    CPlane mPlane;
    ushort mVolumeIndexStart;
  };

  struct SBoundingTreeNode {
    explicit SBoundingTreeNode(CInputStream& in);

    CAABox mBounds;
    short mLeft;
    short mRight;
    short mVolumeIndex;
  };

  class CBoundingTree {
  public:
    explicit CBoundingTree(CInputStream& in);
    void FindOverlappingVolumes(const CAABox& bounds,
                                rstl::reserved_vector< short, 64 >& volumes) const;

  private:
    void FindOverlappingVolumes(const CAABox& bounds, rstl::reserved_vector< short, 64 >& volumes,
                                short node) const;

    rstl::vector< SBoundingTreeNode > mNodes;
  };

  explicit CPortalAreaData(CInputStream& in);
  ~CPortalAreaData();
  void FindOverlappingVolumes(const CAABox& bounds,
                              rstl::reserved_vector< short, 64 >& volumes) const;

  const rstl::vector< SVolume >& GetVolumes() const { return mVolumes; }
  const rstl::vector< SPortal >& GetPortals() const { return mPortals; }
  const rstl::vector< ushort >& GetPortalIndices() const { return mPortalIndices; }
  const rstl::vector< ushort >& GetVolumeIndices() const { return mVolumeIndices; }

private:
  rstl::vector< SVolume > mVolumes;
  rstl::vector< SPortal > mPortals;
  rstl::vector< ushort > mPortalIndices;
  rstl::vector< ushort > mVolumeIndices;
  CBoundingTree mVolumeTree;
};
CHECK_SIZEOF(CPortalAreaData, 0x50)
NESTED_CHECK_SIZEOF(CPortalAreaData, SBspNode, 0x14)
NESTED_CHECK_SIZEOF(CPortalAreaData, SVolume, 0x2c)
NESTED_CHECK_SIZEOF(CPortalAreaData, SPortal, 0x48)
NESTED_CHECK_SIZEOF(CPortalAreaData, SBoundingTreeNode, 0x20)
NESTED_CHECK_SIZEOF(CPortalAreaData, CBoundingTree, 0x10)

CFactoryFnReturn FPortalAreaDataFactory(const SObjectTag& tag, CInputStream& in,
                                        const CVParamTransfer& xfer);

#endif // _CPORTALAREADATA
