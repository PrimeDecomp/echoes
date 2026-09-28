#ifndef _COBBOX
#define _COBBOX

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CInputStream;
class CMRay;

class COBBox {
public:
  COBBox(const CTransform4f& xf, const CVector3f& extents);
  explicit COBBox(CInputStream& in);

  CAABox CalculateAABox(const CTransform4f& xf) const;
  static COBBox FromAABox(const CAABox& box, const CTransform4f& xf);
  bool OBBIntersectsBox(const COBBox& other) const;
  bool IntersectsAABox(const CAABox& box) const;
  // Guessed name; uses the same face ordering as CAABox.
  CQuad GetQuad(CAABox::EBoxFaceId face) const;
  // Guessed overload for the query with optional world-space normal output.
  bool LineIntersectsBox(const CMRay& ray, CVector3f& point, float& penetration,
                         CVector3f* normal) const;
  bool LineIntersectsBox(const CMRay& ray, float& penetration) const;
  const CTransform4f& GetTransform() const { return mTransform; }
  const CVector3f& GetSize() const { return mExtents; }

private:
  CTransform4f mTransform;
  CVector3f mExtents;
};
CHECK_SIZEOF(COBBox, 0x3c)

#endif // _COBBOX
