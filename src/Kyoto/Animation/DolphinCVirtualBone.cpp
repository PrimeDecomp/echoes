#include "Kyoto/Animation/CVirtualBone.hpp"

#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/MemoryCopy.hpp"

#include "dolphin/mtx.h"

static rstl::reserved_vector< SSkinWeighting, 3 > StreamInSkinWeighting(CInputStream& in) {
  rstl::reserved_vector< SSkinWeighting, 3 > weights;
  const int weightCount = in.Get< int >();

  if (weightCount > weights.capacity()) {
    for (int i = 0; i < weights.capacity(); ++i) {
      weights.push_back(SSkinWeighting(in));
    }

    for (int i = weights.capacity(); i < weightCount; ++i) {
      SSkinWeighting tmp(in);
    }
  } else {
    for (int i = 0; i < weightCount; ++i) {
      weights.push_back(SSkinWeighting(in));
    }
  }

  return weights;
}

CVirtualBone::CVirtualBone(CInputStream& in)
: mWeights(StreamInSkinWeighting(in)), mVertexCount(in.ReadInt32()) {}

#ifdef __MWERKS__
void TransformFromMatrixDelta(register CTransform4f* xf, register const CMatrix3f* rot,
                              register const CVector3f* point) {
  asm volatile {
    psq_l f3, CVector3f.mX(point), 0, 0;
    psq_l f1, CMatrix3f.m02(rot), 1, 0;
    psq_l f0, CMatrix3f.m00(rot), 0, 0;
    ps_merge00 f1, f1, f3;
    psq_l f2, CMatrix3f.m12(rot), 1, 0;
    psq_st f0, CTransform4f.m00(xf), 0, 0;
    psq_l f0, CMatrix3f.m10(rot), 0, 0;
    ps_merge01 f2, f2, f3;
    psq_st f1, CTransform4f.m02(xf), 0, 0;
    psq_l f3, CVector3f.mZ(point), 1, 0;
    psq_st f0, CTransform4f.m10(xf), 0, 0;
    psq_l f1, CMatrix3f.m22(rot), 1, 0;
    psq_st f2, CTransform4f.m12(xf), 0, 0;
    psq_l f0, CMatrix3f.m20(rot), 0, 0;
    ps_merge00 f1, f1, f3;
    psq_st f0, CTransform4f.m20(xf), 0, 0;
    psq_st f1, CTransform4f.m22(xf), 0, 0;
  }
}
#else
void TransformFromMatrixDelta(CTransform4f* xf, const CMatrix3f* rot, const CVector3f* point) {
  *xf = CTransform4f(*rot, *point);
}
#endif

#ifdef __MWERKS__
void Transform2FromMatrixData(register CTransform4f* xf, register const CMatrix3f* rot,
                              register const CVector3f* point, register float weight0,
                              register const CMatrix3f* rotation1, register const CVector3f* point1,
                              register float weight1) {
  asm volatile {
    fmr f4, weight0;
    psq_l f5, CVector3f.mX(point), 0, 0;
    psq_l f1, CMatrix3f.m02(rot), 1, 0;
    psq_l f3, CMatrix3f.m12(rot), 1, 0;
    ps_merge00 f6, f4, f2;
    psq_l f0, CMatrix3f.m00(rot), 0, 0;
    ps_merge00 f1, f1, f5;
    psq_l f2, CMatrix3f.m10(rot), 0, 0;
    ps_merge01 f3, f3, f5;
    psq_l f4, CMatrix3f.m20(rot), 0, 0;
    ps_muls0 f0, f0, f6;
    psq_l f5, CMatrix3f.m00(rotation1), 0, 0;
    psq_l f7, CVector3f.mX(point1), 0, 0;
    ps_muls0 f1, f1, f6;
    psq_l f8, CMatrix3f.m02(rotation1), 1, 0;
    ps_muls0 f2, f2, f6;
    ps_madds1 f0, f5, f6, f0;
    psq_l f10, CMatrix3f.m12(rotation1), 1, 0;
    ps_merge00 f8, f8, f7;
    psq_l f9, CMatrix3f.m10(rotation1), 0, 0;
    psq_l f5, CVector3f.mZ(point), 1, 0;
    ps_merge01 f10, f10, f7;
    ps_madds1 f1, f8, f6, f1;
    psq_st f0, CTransform4f.m00(xf), 0, 0;
    psq_l f0, CMatrix3f.m22(rot), 1, 0;
    ps_muls0 f3, f3, f6;
    ps_madds1 f2, f9, f6, f2;
    psq_l f8, CMatrix3f.m22(rotation1), 1, 0;
    ps_merge00 f0, f0, f5;
    psq_st f1, CTransform4f.m02(xf), 0, 0;
    psq_l f1, CVector3f.mZ(point1), 1, 0;
    ps_muls0 f4, f4, f6;
    psq_l f5, CMatrix3f.m20(rotation1), 0, 0;
    ps_madds1 f3, f10, f6, f3;
    psq_st f2, CTransform4f.m10(xf), 0, 0;
    ps_merge00 f8, f8, f1;
    ps_muls0 f0, f0, f6;
    ps_madds1 f4, f5, f6, f4;
    psq_st f3, CTransform4f.m12(xf), 0, 0;
    ps_madds1 f0, f8, f6, f0;
    psq_st f4, CTransform4f.m20(xf), 0, 0;
    psq_st f0, CTransform4f.m22(xf), 0, 0;
  }
}
#else
void Transform2FromMatrixData(CTransform4f* xf, const CMatrix3f* rot, const CVector3f* point,
                              float weight0, const CMatrix3f* rotation1, const CVector3f* point1,
                              float weight1) {
  const CMatrix3f rotation(*rot, weight0, *rotation1, weight1);
  *xf = CTransform4f(rotation, *point * weight0 + *point1 * weight1);
}
#endif

void CVirtualBone::BuildFinalPosMatrix(const CPoseAsTransforms_Linear& pose,
                                       const CVector3f* points, CTransform4f& out) const {
  switch (mWeights.size()) {
  case 1: {
    const CSegId id = mWeights[0].mId;
    const CMatrix3f& rotation = pose.GetTransformMinusOffset(id);
    TransformFromMatrixDelta(&out, &rotation, &points[id.val()]);
    break;
  }
  case 2: {
    const CSegId& id0 = mWeights[0].mId;
    const float weight0 = mWeights[0].mWeight;
    const CSegId& id1 = mWeights[1].mId;
    const float weight1 = mWeights[1].mWeight;
    const CMatrix3f& rotation0 = pose.GetTransformMinusOffset(id0);
    const CMatrix3f& rotation1 = pose.GetTransformMinusOffset(id1);
    Transform2FromMatrixData(&out, &rotation0, &points[id0.val()], weight0, &rotation1,
                             &points[id1.val()], weight1);
    break;
  }
  case 3: {
    const CSegId& id0 = mWeights[0].mId;
    const float weight0 = mWeights[0].mWeight;
    const CSegId& id1 = mWeights[1].mId;
    const float weight1 = mWeights[1].mWeight;
    const CSegId& id2 = mWeights[2].mId;
    const float weight2 = mWeights[2].mWeight;
    const CMatrix3f& rotation0 = pose.GetTransformMinusOffset(id0);
    const CMatrix3f& rotation1 = pose.GetTransformMinusOffset(id1);
    CMatrix3f rotation(rotation0, weight0, rotation1, weight1);
    CVector3f offset = weight0 * points[id0.val()] + weight1 * points[id1.val()];
    rotation.AddScaledMatrix(pose.GetTransformMinusOffset(id2), weight2);
    offset += weight2 * points[id2.val()];
    out = CTransform4f(rotation, offset);
    break;
  }
  default:
    out = CTransform4f::Identity();
    break;
  }
}

void CVirtualBone::BuildAccumulatedTransform(const CPoseAsTransforms_Linear& pose,
                                             const CVector3f* points, CTransform4f& out) const {
  BuildFinalPosMatrix(pose, points, out);
}

void CVirtualBone::BuildSkinningMatrices(const CTransform4f& xf, SSkinningMatrices& out,
                                         bool uniformScale) const {
  PSMTXConcat(CGraphics::GetGXModelView().GetCStyleMatrix(), xf.GetCStyleMatrix(), out.mPosition);
  if (uniformScale) {
    memcpy(out.mNormal[0], out.mPosition[0], sizeof(out.mNormal[0]));
    memcpy(out.mNormal[1], out.mPosition[1], sizeof(out.mNormal[1]));
    memcpy(out.mNormal[2], out.mPosition[2], sizeof(out.mNormal[2]));
  } else {
    Mtx inverseTranspose;
    PSMTXInvXpose(out.mPosition, inverseTranspose);
    memcpy(out.mNormal[0], inverseTranspose[0], sizeof(out.mNormal[0]));
    memcpy(out.mNormal[1], inverseTranspose[1], sizeof(out.mNormal[1]));
    memcpy(out.mNormal[2], inverseTranspose[2], sizeof(out.mNormal[2]));
  }
}
