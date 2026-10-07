#include "MetroidPrime/CAABoxFilter.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CollisionUtil.hpp"

void CAABoxFilter::FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out) {
  float minZ = 10000.f;
  for (const CCollisionInfo* info = in.Begin(); info != in.End(); ++info) {
    if (info->GetMaterialLeft().HasMaterial(kMT_Wall) && info->GetPoint().GetZ() < minZ) {
      minZ = info->GetPoint().GetZ();
    }
  }

  CCollisionInfoList temp;
  for (const CCollisionInfo* info = in.Begin(); info != in.End(); ++info) {
    if (info->GetMaterialLeft().HasMaterial(kMT_Floor)) {
      if (info->GetPoint().GetZ() < minZ) {
        temp.Add(*info);
      }
    } else {
      temp.Add(*info);
    }
  }
  CollisionUtil::AddAverageToFront(temp, out);
}

void CAABoxFilter::Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const {
  FilterBoxFloorCollisions(in, out);
}
