#ifndef _COLLISIONUTIL
#define _COLLISIONUTIL

class CCollisionInfoList;
class CVector3f;
class CMRay;
class CAABox;

namespace CollisionUtil {
int RayAABoxIntersection(const CMRay& ray, const CAABox& box, float& tMin, float& tMax);
void FilterByClosestNormal(const CVector3f& normal, const CCollisionInfoList& in,
                           CCollisionInfoList& out);
}

#endif // _COLLISIONUTIL
