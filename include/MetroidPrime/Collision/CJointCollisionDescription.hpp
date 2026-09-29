#ifndef _CJOINTCOLLISIONDESCRIPTION
#define _CJOINTCOLLISIONDESCRIPTION

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/string.hpp"

class CJointCollisionDescription {
public:
  enum ECollisionType {
    kCT_Sphere,
    kCT_SphereSubdivide,
    kCT_AABox,
    kCT_OBBAutoSize,
    kCT_OBB,
    kCT_OBBFromMayaPlugIn
  };

  // Guessed names for the two target-observed orientation modes.
  enum EOrientationType { kOT_Pivot, kOT_BetweenJoints };

  CJointCollisionDescription(ECollisionType type, CSegId pivotId, CSegId nextId,
                             const CVector3f& bounds, const CMatrix3f& orientation,
                             const CVector3f& pivotPoint, float radius, float maxSeparation,
                             EOrientationType orientationType, const rstl::string& name,
                             float mass);

  void ScaleAllBounds(const CVector3f& scale);

  ECollisionType GetType() const { return mType; }
  EOrientationType GetOrientationType() const { return mOrientationType; }
  CSegId GetPivotId() const { return mPivotId; }
  CSegId GetNextId() const { return mNextId; }
  const CVector3f& GetBounds() const { return mBounds; }
  const CVector3f& GetPivotPoint() const { return mPivotPoint; }
  float GetRadius() const { return mRadius; }
  float GetMaxSeparation() const { return mMaxSeparation; }
  const rstl::string& GetName() const { return mName; }
  TUniqueId GetCollisionActorId() const { return mActorId; }
  float GetMass() const { return mMass; }
  const CMatrix3f& GetOrientation() const { return mOrientation; }
  void SetCollisionActorId(TUniqueId id) { mActorId = id; }

  static CJointCollisionDescription SphereCollision(CSegId pivotId, const CVector3f& pivotPoint,
                                                    float radius, const rstl::string& name,
                                                    float mass);
  static CJointCollisionDescription SphereSubdivideCollision(CSegId pivotId, CSegId nextId,
                                                             float radius, float maxSeparation,
                                                             EOrientationType orientationType,
                                                             const rstl::string& name, float mass);
  static CJointCollisionDescription AABoxCollision(CSegId pivotId, const CVector3f& bounds,
                                                   const rstl::string& name, float mass);
  static CJointCollisionDescription OBBAutoSizeCollision(CSegId pivotId, CSegId nextId,
                                                         const CVector3f& bounds,
                                                         EOrientationType orientationType,
                                                         const rstl::string& name, float mass);
  static CJointCollisionDescription OBBCollision(CSegId pivotId, const CVector3f& bounds,
                                                 const CVector3f& pivotPoint,
                                                 const rstl::string& name, float mass);
  static CJointCollisionDescription
  OBBFromMayaPlugInCollision(CSegId pivotId, const CVector3f& bounds, const CMatrix3f& orientation,
                             const CVector3f& pivotPoint, const rstl::string& name, float mass);

private:
  ECollisionType mType;
  EOrientationType mOrientationType;
  CSegId mPivotId;
  CSegId mNextId;
  CVector3f mBounds;
  CVector3f mPivotPoint;
  float mRadius;
  float mMaxSeparation;
  rstl::string mName;
  TUniqueId mActorId;
  float mMass;
  CMatrix3f mOrientation;
};

CHECK_SIZEOF(CJointCollisionDescription, 0x68)

#endif // _CJOINTCOLLISIONDESCRIPTION
