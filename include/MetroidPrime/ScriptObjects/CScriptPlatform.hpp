#ifndef _CSCRIPTPLATFORM
#define _CSCRIPTPLATFORM

#include "Kyoto/Math/CMayaSpline.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCollidableOBBTreeGroupContainer;
class CFluidPlane;

// Guessed name: an owned control spline, its timing parameters and mode.
struct SPlatformMotionSpline {
  CMayaSpline mSpline;
  float mInitialTime;
  float mDuration;
  bool x4c_;
};
CHECK_SIZEOF(SPlatformMotionSpline, 0x50)

// Guessed names; the two polymorphic motion helpers remain unscaffolded.
class CPlatformSplineController;
class CPlatformWaypointTracker;

struct SRiders {
  TUniqueId mUid;
  rstl::optional_object< float > mDecayTimer;
  CTransform4f mTransform;

  SRiders(TUniqueId uid, const CTransform4f& xf, const rstl::optional_object< float >& decayTimer);
  bool operator==(const SRiders& other) const { return mUid == other.mUid; }
};
CHECK_SIZEOF(SRiders, 0x3c)

class CScriptPlatform : public CPhysicsActor {
public:
  CScriptPlatform(
      TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
      const CModelData& model, const CActorParameters& params, const CAABox& bounds,
      const rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > >& dcln,
      const CHealthInfo& health, const CDamageVulnerability& vulnerability,
      const CMaterialList& materials, bool detectCollision, uint maxRainSplashes, uint rainGenRate,
      const SPlatformMotionSpline& motionSpline, uint motionFlags,
      const CVector3f& conveyorVelocity, const CMayaSpline& rollSpline,
      const CMayaSpline& yawSpline, const CMayaSpline& pitchSpline, float initialTime,
      float xrayAlpha);

  // CEntity
  ~CScriptPlatform() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;

  // CScriptPlatform
  virtual void SplashThink(const CAABox& bounds, const CFluidPlane& fluid, float dt,
                           CStateManager& mgr) const;
  virtual CQuaternion Move(float dt, CStateManager& mgr);

  void fn_800a0200(float time, CStateManager& mgr);
  CQuaternion CalculateRotationDelta(); // Guessed name
  void SetTransformExplicitly(const CTransform4f& xf);
  void fn_8009f4e4(const CTransform4f& xf) {
    // TODO: leave the transform unchanged when the active spline controller owns it.
    CActor::SetTransformAlt(xf);
    mMotionTransformed = true;
  }
  bool IsSlave(TUniqueId id) const;
  bool RemoveRider(TUniqueId id); // Guessed name
  bool IsRider(TUniqueId id) const;
  void UpdateSlaveTransforms(CStateManager& mgr); // Guessed name
  void AddSlave(TUniqueId id, CStateManager& mgr, const rstl::optional_object< float >& decayTimer);
  void AddRider(TUniqueId id, CStateManager& mgr, const rstl::optional_object< float >& decayTimer);
  void fn_800a1df8();
  void RotateMotion(const CQuaternion& rotation, const CVector3f& pivot); // Guessed name
  void TranslateMotion(const CVector3f& delta);                           // Guessed name
  void TeleportToWaypoint(TUniqueId id, CStateManager& mgr);              // Guessed name
  void SetMotionTime(float time, CStateManager& mgr);                     // Guessed name
  void BuildSlaveList(CStateManager& mgr);
  void AdvanceMotionTime(float dt); // Guessed name
  void fn_800a3d18();
  void StopMotion(); // Guessed name

  typedef rstl::reserved_vector< ushort, 1024 > TMovedList;
  typedef rstl::reserved_vector< TUniqueId, 1024 > TNearList;
  static bool IsInMovedList(TUniqueId id, const TMovedList& moved);
  void DragSlaves(CStateManager& mgr, TMovedList& moved);
  void DragSlave(CStateManager& mgr, TMovedList& moved, const SRiders& slave);
  void MoveRiders(CStateManager& mgr, bool active, rstl::vector< SRiders >& riders,
                  rstl::vector< SRiders >& collidedRiders, const TNearList& nearList,
                  const CTransform4f& oldXf, const CTransform4f& newXf, const CVector3f& dragDelta,
                  const CQuaternion& rotDelta);
  static void DecayRiders(rstl::vector< SRiders >& riders, float dt, CStateManager& mgr);
  static TNearList BuildNearListFromRiders(CStateManager& mgr,
                                           const rstl::vector< SRiders >& riders);
  static void AddRider(rstl::vector< SRiders >& riders, TUniqueId id, const CPhysicsActor* ridee,
                       CStateManager& mgr, const rstl::optional_object< float >& decayTimer);

private:
  float mMoveDelay;
  float mCollisionRecoverDelay;
  float mFadeInTime;
  float mFadeOutTime;
  CVector3f mConveyorVelocity;
  CVector3f mDragDelta;
  CQuaternion mRotationDelta;
  CTransform4f mPreviousRotation;
  CTransform4f mCurrentRotation;
  CHealthInfo mInitialHealth;
  CHealthInfo mHealth;
  CDamageVulnerability mDamageVulnerability;
  rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > > mTreeGroupContainer;
  rstl::single_ptr< CCollisionPrimitive > mTreeGroup;
  rstl::vector< SRiders > mRiders;
  rstl::vector< SRiders > mStaticSlaves;
  rstl::vector< SRiders > mDynamicSlaves;
  uint mMaxRainSplashes;
  uint mRainGenRate;
  TUniqueId mBoundsTrigger;
  rstl::single_ptr< SPlatformMotionSpline > mMotionSpline;
  CPlatformSplineController* mSplineController; // Owned; cleanup awaits the helper's interface.
  float mMotionTime;
  uint mMotionFlags;
  float mInitialTime;
  float mMotionDuration;
  CPlatformWaypointTracker* mWaypointTracker; // Owned; cleanup awaits the helper's interface.
  rstl::single_ptr< CMayaSpline > mRollSpline;
  rstl::single_ptr< CMayaSpline > mYawSpline;
  rstl::single_ptr< CMayaSpline > mPitchSpline;
  TUniqueId x450_;
  TUniqueId x452_;
  TUniqueId mLookAtTarget;
  float mXrayAlpha;
  CTransform4f mInitialTransform;
  bool mDead : 1;
  bool mControlledAnimation : 1;
  bool mDetectCollision : 1;
  bool mSquishedRider : 1;
  bool mMotionActive : 1;
  bool mPassedMotionEnd : 1;
  bool mPassedMotionStart : 1;
  bool mMotionForward : 1;
  bool mPreviousMotionForward : 1;
  bool x48d_25_ : 1;
  bool mMotionTransformed : 1;
};
CHECK_SIZEOF(CScriptPlatform, 0x490)

#endif // _CSCRIPTPLATFORM
