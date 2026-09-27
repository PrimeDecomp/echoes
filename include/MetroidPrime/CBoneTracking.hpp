#ifndef _CBONETRACKING
#define _CBONETRACKING

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/string.hpp"

enum EBoneTrackingFlags {
  kBTF_None = 0,
  kBTF_NoParent = 1,
  kBTF_NoParentOrigin = 2,
  kBTF_NoHorizontalAim = 4,
  kBTF_ParentIk = 8,
};

class CAnimData;
class CBodyController;
class CCharLayoutInfo;
class CPoseAsTransforms_Linear;
class CStateManager;
class CTransform4f;

class CBoneTracking {
public:
  CBoneTracking(const CAnimData& animData, const rstl::string& bone, float maxTrackingAngle,
                float angSpeed, uint flags);

  void PreThink(CAnimData& animData);
  void Think(float dt);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                 const CVector3f& scale, const CBodyController& controller);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                 const CVector3f& scale, bool tracking);
  void SetActive(bool active);
  void SetTarget(TUniqueId target);
  void SetDisableTrackingDistance(float distance);
  void SetTargetPosition(const CVector3f& target);
  void SetMaxBoneRotation(float angle);

private:
  // Guessed names.
  void UpdateTracking(const CTransform4f& xf, const CVector3f& scale,
                      const CVector3f& targetPosition, const CCharLayoutInfo& layout,
                      CPoseAsTransforms_Linear& pose);
  void UpdateInactive(const CCharLayoutInfo& layout, CPoseAsTransforms_Linear& pose);

  CQuaternion mRotation;
  float x10_;
  CSegId mSegId;
  float mTime;
  float mMaxTrackingAngle;
  float mAngSpeed;
  float mDisableTrackingDistanceSquared; // Guessed name.
  rstl::optional_object< CVector3f > mTargetPosition;
  TUniqueId mTarget;
  bool mActive : 1;
  bool mHasTrackedRotation : 1;
  bool mPreRendered : 1; // Guessed name.
  bool mNoParent : 1;
  bool mNoParentOrigin : 1;
  bool mNoHorizontalAim : 1;
  bool mParentIk : 1;
};

#endif // _CBONETRACKING
