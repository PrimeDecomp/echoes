#ifndef _CCOLLISIONINFO
#define _CCOLLISIONINFO

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CAABox;

class CCollisionInfo {
public:
  // Original Wii export; the native invalid constructor ignores the tag.
  enum EInvalid { kI_Invalid, kI_Valid };

  CCollisionInfo(EInvalid = kI_Invalid);
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
  void SetObjectId(const TUniqueId& id) { mObjectId = id; }
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

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CCollisionInfo)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CCollisionInfo)
}

#endif // _CCOLLISIONINFO
