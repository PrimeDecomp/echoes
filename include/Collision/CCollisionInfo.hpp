#ifndef _CCOLLISIONINFO
#define _CCOLLISIONINFO

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CAABox;

class CCollisionInfo {
public:
  CCollisionInfo();
  CCollisionInfo(const CVector3f& point, const CMaterialList& rightMaterial,
                 const CMaterialList& leftMaterial, const CVector3f& normal, ushort value);
  CCollisionInfo(const CVector3f& point, const CMaterialList& rightMaterial,
                 const CMaterialList& leftMaterial, const CVector3f& leftNormal,
                 const CVector3f& rightNormal, ushort value);
  CCollisionInfo(const CAABox& box, const CMaterialList& rightMaterial,
                 const CMaterialList& leftMaterial, const CVector3f& leftNormal,
                 const CVector3f& rightNormal, ushort value);

  bool IsValid() const { return mValid; }
  bool HasExtents() const { return mHasExtents; }
  const CVector3f& GetPoint() const { return mPoint; }
  CVector3f GetExtreme() const;
  const CMaterialList& GetMaterialLeft() const { return mMaterialLeft; }
  const CMaterialList& GetMaterialRight() const { return mMaterialRight; }
  const CVector3f& GetNormalLeft() const { return mNormalLeft; }
  const CVector3f& GetNormalRight() const { return mNormalRight; }
  TUniqueId GetObjectId() const { return mObjectId; }
  void Swap();

private:
  CVector3f mPoint;
  CVector3f mExtentX;
  CVector3f mExtentY;
  CVector3f mExtentZ;
  CMaterialList mMaterialLeft;
  CMaterialList mMaterialRight;
  CVector3f mNormalLeft;
  CVector3f mNormalRight;
  TUniqueId mObjectId;
  bool mValid : 1;
  bool mHasExtents : 1;
};
CHECK_SIZEOF(CCollisionInfo, 0x60)

#endif // _CCOLLISIONINFO
