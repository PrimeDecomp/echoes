#ifndef _COLLISIONUTIL
#define _COLLISIONUTIL

class CCollisionInfoList;
class CVector3f;

namespace CollisionUtil {
void FilterByClosestNormal(const CVector3f& normal, const CCollisionInfoList& in,
                           CCollisionInfoList& out);
}

#endif // _COLLISIONUTIL
