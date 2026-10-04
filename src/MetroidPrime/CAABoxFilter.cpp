#include "MetroidPrime/CAABoxFilter.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CollisionUtil.hpp"

void CAABoxFilter::FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out) {
  float minZ = 10000.f;
  for (int i = 0; i < in.GetCount(); ++i) {
    const CCollisionInfo& info = in[i];
    if (info.GetMaterialLeft().HasMaterial(kMT_Wall) && info.GetPoint().GetZ() < minZ) {
      minZ = info.GetPoint().GetZ();
    }
  }

  CCollisionInfoList temp;
  for (int i = 0; i < in.GetCount(); ++i) {
    const CCollisionInfo& info = in[i];
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
