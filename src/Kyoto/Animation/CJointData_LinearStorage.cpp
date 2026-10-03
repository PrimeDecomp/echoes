#include "Kyoto/Animation/CJointData_LinearStorage.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/LockedCache.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"

void* CJointData_LinearStorage::Allocate(int count, EAllocateFrom requested,
                                         EAllocateFrom& selected) {
  void* storage = nullptr;
  selected = requested;
  if (selected == kAF_Pool) {
    storage = AllocateLockedCache4(count * (sizeof(CQuaternion) + 2 * sizeof(CVector3f)));
    if (storage == nullptr) {
      selected = kAF_Heap;
    }
  }

  if (selected != kAF_Pool) {
    storage = CMemory::Alloc(count * (sizeof(CQuaternion) + 2 * sizeof(CVector3f)));
  }
  return storage;
}

CJointData_LinearStorage::CJointData_LinearStorage(int count, EAllocateFrom allocateFrom)
: mStorage(Allocate(count, allocateFrom, mAllocationType))
, mCount(count)
, mHasScales(false)
, mHasOffsets(false)
, mUseZeroOffsets(false)
, mRotations(static_cast< uchar* >(mStorage))
, mTranslations(mRotations + sizeof(CQuaternion))
, mScales(mTranslations + sizeof(CVector3f))
, mStride(sizeof(CQuaternion) + 2 * sizeof(CVector3f)) {}

CJointData_LinearStorage::~CJointData_LinearStorage() {
  if (mAllocationType == kAF_Pool) {
    FreeLockedCache(mStorage);
  } else if (mAllocationType == kAF_Heap) {
    CMemory::Free(mStorage);
  }
}

void CJointData_LinearStorage::SetZeroRotation() {
  for (int i = 0; i < mCount; ++i) {
    Rotation(i) = CQuaternion::NoRotation();
  }
}

void CJointData_LinearStorage::ResetScales() {
  for (int i = 0; i < mCount; ++i) {
    Scale(i) = CVector3f::One();
  }
}

void CJointData_LinearStorage::SetReferenceOffsets(const CCharLayoutInfo& layout) {
  for (int i = 0; i < mCount; ++i) {
    Translation(i) = layout.GetLinearParentOffsets()[i];
  }
}

void CJointData_LinearStorage::ResetFlags() {
  mHasOffsets = false;
  mHasScales = false;
  mUseZeroOffsets = false;
}

void CJointData_LinearStorage::Blend(const CJointData_LinearStorage& other, float weight) {
  if (mHasScales) {
    for (int i = 0; i < mCount; ++i) {
      Rotation(i) = CAnimMathUtils::SlerpLocal(Rotation(i), other.Rotation(i), weight);
      Translation(i) = CVector3f::Lerp(Translation(i), other.Translation(i), weight);
      Scale(i) = CVector3f::Lerp(Scale(i), other.Scale(i), weight);
    }
  } else {
    for (int i = 0; i < mCount; ++i) {
      Rotation(i) = CAnimMathUtils::SlerpLocal(Rotation(i), other.Rotation(i), weight);
      Translation(i) = CVector3f::Lerp(Translation(i), other.Translation(i), weight);
    }
  }
}

void CJointData_LinearStorage::Add(const CJointData_LinearStorage& other, float weight) {
  if (mHasScales) {
    for (int i = 0; i < mCount; ++i) {
      Rotation(i) *= CAnimMathUtils::Slerp(CQuaternion::NoRotation(), other.Rotation(i), weight);
      Translation(i) += weight * other.Translation(i);
      Scale(i) = CVector3f::ByElementMultiply(
          Scale(i), CVector3f::Lerp(CVector3f::One(), other.Scale(i), weight));
    }
  } else {
    for (int i = 0; i < mCount; ++i) {
      Rotation(i) *= CAnimMathUtils::Slerp(CQuaternion::NoRotation(), other.Rotation(i), weight);
      Translation(i) += weight * other.Translation(i);
    }
  }
}
