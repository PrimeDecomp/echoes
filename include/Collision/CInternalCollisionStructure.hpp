#ifndef _CINTERNALCOLLISIONSTRUCTURE
#define _CINTERNALCOLLISIONSTRUCTURE

#include "Kyoto/Math/CTransform4f.hpp"

class CCollisionPrimitive;
class CMaterialFilter;

class CInternalCollisionStructure {
public:
  class CPrimDesc {
  public:
    CPrimDesc(const CCollisionPrimitive& primitive, const CMaterialFilter& filter,
              const CTransform4f& transform)
    : mPrimitive(primitive), mFilter(filter), mTransform(transform) {}

    const CCollisionPrimitive& GetPrim() const { return mPrimitive; }
    const CMaterialFilter& GetFilter() const { return mFilter; }
    const CTransform4f& GetTransform() const { return mTransform; }

  private:
    const CCollisionPrimitive& mPrimitive;
    const CMaterialFilter& mFilter;
    CTransform4f mTransform;
  };

  CInternalCollisionStructure(const CPrimDesc& left, const CPrimDesc& right)
  : mLeft(left), mRight(right) {}

  CInternalCollisionStructure GetSwapped() const {
    return CInternalCollisionStructure(mRight, mLeft);
  }

  const CPrimDesc& GetLeft() const { return mLeft; }
  const CPrimDesc& GetRight() const { return mRight; }

private:
  CPrimDesc mLeft;
  CPrimDesc mRight;
};
NESTED_CHECK_SIZEOF(CInternalCollisionStructure, CPrimDesc, 0x38)
CHECK_SIZEOF(CInternalCollisionStructure, 0x70)

#endif // _CINTERNALCOLLISIONSTRUCTURE
