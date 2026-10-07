#ifndef _CCUBESURFACE
#define _CCUBESURFACE

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

class CCubeModel;
class CCubeSurface {
  friend class CCubeModel;

  struct SSurfaceData {
    CVector3f mCenter;
    uint mMaterialIndex;
    uint mDisplayListSizeAndNormalHint;
    CCubeModel* mParent;
    const void* mNextSurface;
    uint mExtraSize;
    CUnitVector3f mNormal;
    short mMatrixBank;
    short x2e_;
    CAABox mBounds;
  };

  static const CVector3f skDefaultNormal;
  const SSurfaceData* mData;

public:
  explicit CCubeSurface(const void* data) : mData(static_cast< const SSurfaceData* >(data)) {}

  CAABox GetBounds() const;
  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CUnitVector3f& GetNormalHint() const { return mData->mNormal; }
  uint GetMaterialIndex() const { return mData->mMaterialIndex; }
  CCubeModel* GetParent() const { return mData->mParent; }
  short GetMatrixBank() const { return mData->mMatrixBank; } // Guessed name.
  short GetShadowBank() const { return mData->x2e_; }        // Target-derived grouping key.
  uint GetDisplayListSize() const { return mData->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  uint GetSurfaceHeaderSize() const { return (sizeof(SSurfaceData) + 7 + mData->mExtraSize) & ~31; }
  const void* GetDisplayList() const {
    return reinterpret_cast< const uchar* >(mData) + GetSurfaceHeaderSize();
  }
  CCubeSurface GetNextSurface() const { return CCubeSurface(mData->mNextSurface); }
  bool IsValid() const { return mData != nullptr; }
};
CHECK_SIZEOF(CCubeSurface, 4)

#endif // _CCUBESURFACE
