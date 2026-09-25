#ifndef _CVIRTUALBONE
#define _CVIRTUALBONE

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

struct SSkinWeighting {
  CSegId mId;
  float mWeight;
  explicit SSkinWeighting(CInputStream& in) : mId(in), mWeight(in.ReadFloat()) {}
};

class CPoseAsTransforms;
class CVirtualBone {
public:
  explicit CVirtualBone(CInputStream& in);
  const rstl::reserved_vector< SSkinWeighting, 3 >& GetWeights() const { return mWeights; }
  uint GetVertexCount() const { return mVertexCount; }

  void BuildPoints(const ushort*, volatile void*, int) const;
  void BuildNormals(const ushort*, volatile void*, int) const;
  void BuildNormals(const CVector3f*, CVector3f*, int) const;
  void BuildAccumulatedTransform(const CPoseAsTransforms& pose, const CVector3f* points) const;
  void BuildFinalPosMatrix(const CPoseAsTransforms& pose, const CVector3f* points) const;

private:
  rstl::reserved_vector< SSkinWeighting, 3 > mWeights;
  uint mVertexCount;
  mutable CTransform4f mXf;
  mutable CMatrix3f mRotation;
};

#endif // _CVIRTUALBONE
