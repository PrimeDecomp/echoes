#ifndef _CDRAWABLE
#define _CDRAWABLE

#include "Kyoto/Math/CAABox.hpp"
#include "types.h"

enum EDrawableType {
  kDT_WorldSurface,
  kDT_Particle,
  kDT_Actor,
  kDT_SimpleShadow,
  kDT_Decal,
};

class CDrawable {
public:
  CDrawable(EDrawableType type, ushort extraSort, float distance, const CAABox& bounds,
            const void* data, bool alpha)
  : mType(type)
  , mAlpha(alpha)
  , mExtraSort(extraSort)
  , mData(data)
  , mBounds(bounds)
  , mViewDistance(distance) {}

  EDrawableType GetType() const { return static_cast< EDrawableType >(mType); }
  bool IsAlpha() const { return mAlpha; }
  ushort GetExtraSort() const { return mExtraSort; }
  const void* GetData() const { return mData; }
  const CAABox& GetBounds() const { return mBounds; }
  float GetDistance() const { return mViewDistance; }

private:
  ushort mType : 8;
  ushort mAlpha : 1;
  ushort mExtraSort;
  const void* mData;
  CAABox mBounds;
  float mViewDistance;
};
CHECK_SIZEOF(CDrawable, 0x24)

#endif // _CDRAWABLE
