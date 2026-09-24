#ifndef _CCOLLISIONINFO
#define _CCOLLISIONINFO

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CVector3f.hpp"

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

  bool IsValid() const { return x5a_24_valid; }
  bool HasExtents() const { return x5a_25_hasExtents; }
  const CVector3f& GetPoint() const { return x0_point; }
  const CMaterialList& GetMaterialLeft() const { return x30_materialLeft; }
  const CMaterialList& GetMaterialRight() const { return x38_materialRight; }
  const CVector3f& GetNormalLeft() const { return x40_normalLeft; }
  const CVector3f& GetNormalRight() const { return x4c_normalRight; }
  void Swap();

private:
  CVector3f x0_point;
  CVector3f xc_extentX;
  CVector3f x18_extentY;
  CVector3f x24_extentZ;
  CMaterialList x30_materialLeft;
  CMaterialList x38_materialRight;
  CVector3f x40_normalLeft;
  CVector3f x4c_normalRight;
  ushort x58_; // Echoes addition; initialized to 0xffff. Meaning not established.
  bool x5a_24_valid : 1;
  bool x5a_25_hasExtents : 1;
};
CHECK_SIZEOF(CCollisionInfo, 0x60)

#endif // _CCOLLISIONINFO
