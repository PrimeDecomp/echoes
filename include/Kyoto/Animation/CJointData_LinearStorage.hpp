#ifndef _CJOINTDATA_LINEARSTORAGE
#define _CJOINTDATA_LINEARSTORAGE

#include "types.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CCharLayoutInfo;

class CJointData_LinearStorage {
public:
  enum EAllocateFrom {
    kAF_Pool, // Guessed name: falls back to heap allocation when the pool is exhausted.
    kAF_Heap, // Guessed name.
  };

  CJointData_LinearStorage(int count, EAllocateFrom allocateFrom);
  ~CJointData_LinearStorage();
  void SetZeroRotation();
  void SetReferenceOffsets(const CCharLayoutInfo& layout);
  // Guessed names; the native operations preserve the existing flags and use this count.
  void Add(const CJointData_LinearStorage& other, float weight);
  void Blend(const CJointData_LinearStorage& other, float weight);
  // Guessed name; clears flags without modifying the stored pose.
  void ResetFlags();

  const uchar* GetRotations() const { return mRotations; }

  const uchar* GetTranslations() const { return mTranslations; }

  const uchar* GetScales() const { return mScales; }

  int GetStride() const { return mStride; }

  CQuaternion& Rotation(int index) {
    return *reinterpret_cast< CQuaternion* >(mRotations + index * mStride);
  }

  const CQuaternion& Rotation(int index) const {
    return *reinterpret_cast< const CQuaternion* >(mRotations + index * mStride);
  }

  CVector3f& Translation(int index) {
    return *reinterpret_cast< CVector3f* >(mTranslations + index * mStride);
  }

  const CVector3f& Translation(int index) const {
    return *reinterpret_cast< const CVector3f* >(mTranslations + index * mStride);
  }

  CVector3f& Scale(int index) { return *reinterpret_cast< CVector3f* >(mScales + index * mStride); }

  const CVector3f& Scale(int index) const {
    return *reinterpret_cast< const CVector3f* >(mScales + index * mStride);
  }

  bool HasScales() const { return mHasScales; }

  bool UsesZeroOffsets() const { return mUseZeroOffsets; }

  void SetHasOffsets(bool value) { mHasOffsets = value; }

  void SetHasScales(bool value) { mHasScales = value; }

  void ResetScales(); // Guessed name.

private:
  // Guessed name; may change the selected allocation policy on pool exhaustion.
  static void* Allocate(int count, EAllocateFrom requested, EAllocateFrom& selected);

  EAllocateFrom mAllocationType;
  void* mStorage;
  int mCount;
  bool mHasScales : 1;
  bool mHasOffsets : 1;
  bool mUseZeroOffsets : 1;
  uchar* mRotations;
  uchar* mTranslations;
  uchar* mScales;
  int mStride;
};
CHECK_SIZEOF(CJointData_LinearStorage, 0x20)

#endif // _CJOINTDATA_LINEARSTORAGE
