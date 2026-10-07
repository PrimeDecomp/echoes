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
  uchar* destination = mRotations;
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CQuaternion* >(destination) = CQuaternion::NoRotation();
    destination += mStride;
  }
}

void CJointData_LinearStorage::ResetScales() {
  uchar* destination = mScales;
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CVector3f* >(destination) = CVector3f::One();
    destination += mStride;
  }
}

void CJointData_LinearStorage::SetReferenceOffsets(const CCharLayoutInfo& layout) {
  const CVector3f* source = layout.GetLinearParentOffsets().data();
  uchar* destination = mTranslations;
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CVector3f* >(destination) = *source++;
    destination += mStride;
  }
}

void CJointData_LinearStorage::ResetFlags() {
  mHasOffsets = false;
  mHasScales = false;
  mUseZeroOffsets = false;
}

void CJointData_LinearStorage::Blend(const CJointData_LinearStorage& other, float weight) {
  uchar* rotations = mRotations;
  uchar* translations = mTranslations;
  const int stride = mStride;
  const uchar* sourceRotations = other.mRotations;
  const uchar* sourceTranslations = other.mTranslations;
  const int sourceStride = other.mStride;

  if (!mHasScales) {
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation = CAnimMathUtils::SlerpLocal(rotation, sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = CVector3f::Lerp(translation, sourceTranslation, weight);
      translations += stride;
      sourceTranslations += sourceStride;
    }
  } else {
    uchar* scales = mScales;
    const uchar* sourceScales = other.mScales;
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation = CAnimMathUtils::SlerpLocal(rotation, sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = CVector3f::Lerp(translation, sourceTranslation, weight);
      translations += stride;
      sourceTranslations += sourceStride;

      CVector3f& scale = *reinterpret_cast< CVector3f* >(scales);
      const CVector3f& sourceScale = *reinterpret_cast< const CVector3f* >(sourceScales);
      scale = CVector3f::Lerp(scale, sourceScale, weight);
      scales += stride;
      sourceScales += sourceStride;
    }
  }
}

void CJointData_LinearStorage::Add(const CJointData_LinearStorage& other, float weight) {
  uchar* rotations = mRotations;
  uchar* translations = mTranslations;
  const int stride = mStride;
  const uchar* sourceRotations = other.mRotations;
  const uchar* sourceTranslations = other.mTranslations;
  const int sourceStride = other.mStride;

  if (!mHasScales) {
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation =
          rotation * CAnimMathUtils::Slerp(CQuaternion::NoRotation(), sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = translation + weight * sourceTranslation;
      translations += stride;
      sourceTranslations += sourceStride;
    }
  } else {
    uchar* scales = mScales;
    const uchar* sourceScales = other.mScales;
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation =
          rotation * CAnimMathUtils::Slerp(CQuaternion::NoRotation(), sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = translation + weight * sourceTranslation;
      translations += stride;
      sourceTranslations += sourceStride;

      CVector3f& scale = *reinterpret_cast< CVector3f* >(scales);
      const CVector3f& sourceScale = *reinterpret_cast< const CVector3f* >(sourceScales);
      scale = CVector3f::ByElementMultiply(scale,
                                           CVector3f::Lerp(CVector3f::One(), sourceScale, weight));
      scales += stride;
      sourceScales += sourceStride;
    }
  }
}
