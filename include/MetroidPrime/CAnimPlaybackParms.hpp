#ifndef _CANIMPLAYBACKPARMS
#define _CANIMPLAYBACKPARMS

#include "types.h"

class CQuaternion;
class CTransform4f;
class CVector3f;

class CAnimPlaybackParms {
private:
  int mAnimA;
  int mAnimB;
  float mBlendWeight;
  int xc_;
  const CVector3f* mTargetPos;
  const CQuaternion* mDeltaOrient;
  const CTransform4f* mObjectXf;
  const CVector3f* mObjectScale;
  bool mUseLocator;
  bool mAnimating;

public:
  CAnimPlaybackParms(int animA, int animB, float blendWeight, bool animating)
  : mAnimA(animA)
  , mAnimB(animB)
  , mBlendWeight(blendWeight)
  , xc_(0)
  , mTargetPos(nullptr)
  , mDeltaOrient(nullptr)
  , mObjectXf(nullptr)
  , mObjectScale(nullptr)
  , mUseLocator(false)
  , mAnimating(animating) {}

  CAnimPlaybackParms(int anim, const CQuaternion* deltaOrient, const CVector3f* targetPos,
                     const CTransform4f* xf, const CVector3f* scale, bool useLocator)
  : mAnimA(anim)
  , mAnimB(-1)
  , mBlendWeight(1.f)
  , xc_(0)
  , mTargetPos(targetPos)
  , mDeltaOrient(deltaOrient)
  , mObjectXf(xf)
  , mObjectScale(scale)
  , mUseLocator(useLocator)
  , mAnimating(true) {}

  int GetAnimationId() const { return mAnimA; }
};
CHECK_SIZEOF(CAnimPlaybackParms, 0x24)

#endif // _CANIMPLAYBACKPARMS
