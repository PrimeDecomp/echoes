#ifndef _CCAMERACOLLIDERGROUP
#define _CCAMERACOLLIDERGROUP

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/Cameras/CCameraCollider.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CStateManager;

// Guessed name: shared collider-ring state extracted from Prime's CBallCamera.
class CCameraColliderGroup {
public:
  CCameraColliderGroup();
  virtual ~CCameraColliderGroup() {}

  void SetupColliders(float xMag, float zMag, float radius, int count, float startAngle);
  void TeleportColliders(CVector3f position);
  void UpdateCollidersDistances(float xMag, float zMag, float angleOffset);
  void UpdateColliders(const CTransform4f& xf, const CVector3f& lookPosition, int count,
                       float tolerance, const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                       CStateManager& mgr);
  CVector3f CalculateCollidersCentroid() const;
  CAABox CalculateCollidersBoundingBox() const;
  int CountObscuredColliders() const;

  const CVector3f& GetCentroid() const { return mCentroid; }

private:
  rstl::vector< CCameraCollider > mColliders;
  CVector3f mCentroid;
  CVector3f mLookPosition;
  float mClearColliderThreshold;
  int x30_;
  int x34_;
  int mColliderIterator;
  bool x3c_24_ : 1;
};
CHECK_SIZEOF(CCameraColliderGroup, 0x40)

#endif // _CCAMERACOLLIDERGROUP
