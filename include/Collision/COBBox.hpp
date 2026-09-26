#ifndef _COBBOX
#define _COBBOX

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CInputStream;

class COBBox {
public:
  COBBox(const CTransform4f& xf, const CVector3f& extents);
  explicit COBBox(CInputStream& in);

  CAABox CalculateAABox(const CTransform4f& xf) const;
  const CTransform4f& GetTransform() const { return mTransform; }
  const CVector3f& GetSize() const { return mExtents; }

private:
  CTransform4f mTransform;
  CVector3f mExtents;
};
CHECK_SIZEOF(COBBox, 0x3c)

#endif // _COBBOX
