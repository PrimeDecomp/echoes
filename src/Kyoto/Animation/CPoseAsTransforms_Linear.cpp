#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

static rstl::vector< CSegId >::const_iterator ConnectedPartsEnd(const CCharLayoutInfo& layout,
                                                                const CSegId& seg);

CPoseAsTransforms_Linear::CPoseAsTransforms_Linear(int count, int withScale, int withOffsets)
: mElements(count, CElementType(CMatrix3f::sIdentity, CVector3f::sZeroVector,
                                CVector3f::sZeroVector))
, mScales(withScale == 1 ? count : 0, CVector3f::sOneVector)
, mUnscaledRotations(withScale == 1 ? count : 0, CMatrix3f::sIdentity)
, x30_(withOffsets == 1 ? count : 0, CVector3f::sZeroVector)
, x40_24_(false)
, x41_(0) {}

const CMatrix3f& CPoseAsTransforms_Linear::GetTransformMinusOffset(const CSegId& seg) const {
  return mElements[seg.val()].mRotation;
}

CMatrix3f CPoseAsTransforms_Linear::GetRotation(const CSegId& seg) const {
  if (mScales.size() != 0) {
    return mUnscaledRotations[seg.val()];
  }
  return GetTransformMinusOffset(seg);
}

const CVector3f& CPoseAsTransforms_Linear::GetOffset(const CSegId& seg) const {
  return mElements[seg.val()].mOffset;
}

CTransform4f CPoseAsTransforms_Linear::GetTransform(const CSegId& seg) const {
  const CElementType& elem = mElements[seg.val()];
  if (mScales.size() != 0) {
    return CTransform4f(mUnscaledRotations[seg.val()], elem.mOffset);
  }
  return CTransform4f(elem.mRotation, elem.mOffset);
}

void CPoseAsTransforms_Linear::BuildPose(const CCharLayoutInfo& layout,
                                         const CJointData_LinearStorage& data) {
  x40_24_ = false;
  const uchar* rotations = data.GetRotations();
  const uchar* translations = data.GetTranslations();
  const uchar* scales = data.GetScales();
  int stride = data.GetStride();
  if (mScales.size() != 0) {
    CElementType* elem = mElements.data();
    const CSegId* parent = layout.GetLinearParents().data();
    CMatrix3f* unscaled = mUnscaledRotations.data();
    CVector3f* scale = mScales.data();
    elem->mRotation = CMatrix3f::sIdentity;
    elem->mOffset = CVector3f::sZeroVector;
    *unscaled = CMatrix3f::sIdentity;
    *scale = CVector3f::sOneVector;
    const uchar* scaleBase = scales;
    rotations += stride;
    translations += stride;
    scales += stride;
    ++elem;
    ++unscaled;
    ++parent;
    ++scale;
    int count = mElements.size();
    for (int i = 1; i < count; ++i) {
      uchar parentId = parent->val();
      const CElementType& parentElem = mElements[parentId];
      const CMatrix3f& parentRotation = mUnscaledRotations[parentId];
      *scale = *reinterpret_cast< const CVector3f* >(scales);
      *unscaled =
          parentRotation * reinterpret_cast< const CQuaternion* >(rotations)->BuildTransform();
      elem->mRotation = *unscaled * CMatrix3f::Scale(scale->GetX(), scale->GetY(), scale->GetZ());
      elem->mLocalOffset = *reinterpret_cast< const CVector3f* >(translations);
      elem->mOffset =
          parentElem.mOffset +
          parentRotation *
              (elem->mLocalOffset *
               *reinterpret_cast< const CVector3f* >(scaleBase + stride * parentId));
      rotations += stride;
      translations += stride;
      scales += stride;
      ++parent;
      ++unscaled;
      ++scale;
      ++elem;
    }
  } else {
    CElementType* elem = mElements.data();
    const CSegId* parent = layout.GetLinearParents().data();
    elem->mRotation = CMatrix3f::sIdentity;
    elem->mOffset = CVector3f::sZeroVector;
    rotations += stride;
    translations += stride;
    ++parent;
    ++elem;
    int count = mElements.size();
    for (int i = 1; i < count; ++i) {
      const CElementType& parentElem = mElements[parent->val()];
      elem->mRotation =
          parentElem.mRotation * reinterpret_cast< const CQuaternion* >(rotations)->BuildTransform();
      elem->mLocalOffset = *reinterpret_cast< const CVector3f* >(translations);
      elem->mOffset = parentElem.mOffset + parentElem.mRotation * elem->mLocalOffset;
      rotations += stride;
      translations += stride;
      ++parent;
      ++elem;
    }
  }
}

void CPoseAsTransforms_Linear::SetRotation(const CCharLayoutInfo& layout, const CSegId& seg,
                                           const CMatrix3f& rotation) {
  x40_24_ = false;
  CMatrix3f delta = rotation * GetRotation(seg).GetTranspose();
  RotateHierarchy(layout, seg, delta, 1);
}

void CPoseAsTransforms_Linear::RotateHierarchy(const CCharLayoutInfo& layout, const CSegId& seg,
                                               const CMatrix3f& rotation, int order) {
  int id = seg.val();
  x40_24_ = false;
  CElementType& elem = mElements[id];
  if (mScales.size() == 0) {
    if (order == 1) {
      elem.mRotation = rotation * elem.mRotation;
    } else {
      elem.mRotation = elem.mRotation * rotation;
    }
    elem.mRotation = elem.mRotation.Orthonormalized();
  } else {
    CMatrix3f unscaled(CMatrix3f::sIdentity);
    if (order == 1) {
      unscaled = rotation * elem.mRotation;
    } else {
      unscaled = elem.mRotation * rotation;
    }
    const CVector3f& scale = mScales[id];
    elem.mRotation = unscaled * CMatrix3f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    mUnscaledRotations[id] = unscaled;
  }

  if (layout.GetSegmentData(seg).GetNumConnectedParts() >= 2) {
    rstl::vector< CSegId >::const_iterator it = layout.GetSegmentData(seg).GetConnectedParts().begin();
    rstl::vector< CSegId >::const_iterator end = ConnectedPartsEnd(layout, seg);
    ++it;
    bool hasScale = mScales.size() != 0;
    const CVector3f& scale = hasScale ? mScales[id] : CVector3f::sOneVector;
    const CMatrix3f& parentRotation = hasScale ? mUnscaledRotations[id] : elem.mRotation;
    for (; it != end; ++it) {
      CElementType& child = mElements[it->val()];
      child.mOffset = elem.mOffset + parentRotation * (child.mLocalOffset * scale);
      RotateHierarchy(layout, *it, rotation, order);
    }
  }
}

static rstl::vector< CSegId >::const_iterator ConnectedPartsEnd(const CCharLayoutInfo& layout,
                                                                const CSegId& seg) {
  return layout.GetSegmentData(seg).GetConnectedParts().end();
}

void CPoseAsTransforms_Linear::AllocateScale() {
  mScales.resize(mElements.size(), CVector3f::sOneVector);
  mUnscaledRotations.resize(mElements.size(), CMatrix3f::sIdentity);
}
