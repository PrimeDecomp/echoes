#ifndef _CVIRTUALBONE
#define _CVIRTUALBONE

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/reserved_vector.hpp"

#include "dolphin/mtx.h"

struct SSkinWeighting {
  CSegId mId;
  float mWeight;
  explicit SSkinWeighting(CInputStream& in) : mId(in.Get< int >()), mWeight(in.Get< float >()) {}
};
CHECK_SIZEOF(SSkinWeighting, 0x8)

// Guessed name. GPU matrix records occupy three cache lines.
struct SSkinningMatrices {
  Mtx mPosition;
  float mNormal[3][3];
} ATTRIBUTE_ALIGN(32);
CHECK_SIZEOF(SSkinningMatrices, 0x60)

class CPoseAsTransforms_Linear;
class CVirtualBone {
public:
  explicit CVirtualBone(CInputStream& in);
  const rstl::reserved_vector< SSkinWeighting, 3 >& GetWeights() const { return mWeights; }
  uint GetVertexCount() const { return mVertexCount; }

  void BuildAccumulatedTransform(const CPoseAsTransforms_Linear& pose, const CVector3f* points,
                                 CTransform4f& out) const;
  void BuildFinalPosMatrix(const CPoseAsTransforms_Linear& pose, const CVector3f* points,
                           CTransform4f& out) const;
  // Guessed name.
  void BuildSkinningMatrices(const CTransform4f& xf, SSkinningMatrices& out,
                             bool uniformScale) const;

private:
  rstl::reserved_vector< SSkinWeighting, 3 > mWeights;
  uint mVertexCount;
};
CHECK_SIZEOF(CVirtualBone, 0x20)

#endif // _CVIRTUALBONE
