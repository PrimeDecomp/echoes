#include "MetroidPrime/CAABoxFilter.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CollisionUtil.hpp"

void CAABoxFilter::FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out) {
  float minZ = 10000.f;
  const CCollisionInfo* end = in.End();
  for (const CCollisionInfo* it = in.Begin(); it != end; ++it) {
    const CCollisionInfo& info = *it;
    if (info.GetMaterialLeft().HasMaterial(kMT_Wall) && info.GetPoint().GetZ() < minZ) {
      minZ = info.GetPoint().GetZ();
    }
  }

  CCollisionInfoList temp;
  for (const CCollisionInfo* it = in.Begin(); it != in.End(); ++it) {
    const CCollisionInfo& info = *it;
    if (info.GetMaterialLeft().HasMaterial(kMT_Floor)) {
      if (info.GetPoint().GetZ() < minZ) {
        temp.Add(info);
      }
    } else {
      temp.Add(info);
    }
  }
  CollisionUtil::AddAverageToFront(temp, out);
}

void CAABoxFilter::Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const {
  FilterBoxFloorCollisions(in, out);
}
