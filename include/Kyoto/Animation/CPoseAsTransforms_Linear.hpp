#ifndef _CPOSEASTRANSFORMS_LINEAR
#define _CPOSEASTRANSFORMS_LINEAR

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"

class CCharLayoutInfo;
class CJointData_LinearStorage;

class CPoseAsTransforms_Linear {
public:
  class CElementType {
  public:
    CElementType(const CMatrix3f& rotation, const CVector3f& offset, const CVector3f& localOffset)
    : mRotation(rotation), mOffset(offset), mLocalOffset(localOffset) {}

    CMatrix3f mRotation;
    CVector3f mOffset;
    CVector3f mLocalOffset;
  };

  CPoseAsTransforms_Linear(int count, int withScale, int withOffsets);

  void AllocateScale();
  void RotateHierarchy(const CCharLayoutInfo& layout, const CSegId& seg, const CMatrix3f& rotation,
                       int order);
  void SetRotation(const CCharLayoutInfo& layout, const CSegId& seg, const CMatrix3f& rotation);
  void BuildPose(const CCharLayoutInfo& layout, const CJointData_LinearStorage& data);
  CTransform4f GetTransform(const CSegId& seg) const;
  const CVector3f& GetOffset(const CSegId& seg) const;
  CMatrix3f GetRotation(const CSegId& seg) const;
  const CMatrix3f& GetTransformMinusOffset(const CSegId& seg) const;

private:
  rstl::vector< CElementType > mElements;
  rstl::vector< CVector3f > mScales;
  rstl::vector< CMatrix3f > mUnscaledRotations;
  rstl::vector< CVector3f > x30_;
  bool x40_24_ : 1;
  uchar x41_;
};
CHECK_SIZEOF(CPoseAsTransforms_Linear, 0x44)

#endif // _CPOSEASTRANSFORMS_LINEAR
