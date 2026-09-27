#ifndef _CCOLLIDABLEAABOXSPHERE
#define _CCOLLIDABLEAABOXSPHERE

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"

// Guessed name, correlated with Prime's ABSH compound primitive.
class CCollidableAABoxSphere : public CCollisionPrimitive {
public:
  const CCollidableAABox& GetCollidableAABox() const { return mAabb; }
  const CCollidableSphere& GetCollidableSphere() const { return mSphere; }

private:
  CCollidableAABox mAabb;
  CCollidableSphere mSphere;
};
CHECK_SIZEOF(CCollidableAABoxSphere, 0x58)

#endif // _CCOLLIDABLEAABOXSPHERE
