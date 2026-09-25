#ifndef _CCUBESURFACE
#define _CCUBESURFACE

#include "Kyoto/Math/CAABox.hpp"

class CCubeModel;
class CCubeSurface {
  struct SSurfaceData {
    CVector3f mCenter;
    uint mMaterialIndex;
    uint mDisplayListSizeAndNormalHint;
    CCubeModel* mParent;
    CCubeSurface* mNextSurface;
    uint mExtraSize;
    CVector3f mNormal;
    uint x2c_;
    CAABox mBounds;
  };

  static const CVector3f skDefaultNormal;
  const SSurfaceData* mData;

public:
  CAABox GetBounds() const;
  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CVector3f& GetNormalHint() const { return mData->mNormal; }
};
#endif // _CCUBESURFACE
