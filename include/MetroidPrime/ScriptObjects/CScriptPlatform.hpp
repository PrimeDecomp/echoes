#ifndef _CSCRIPTPLATFORM
#define _CSCRIPTPLATFORM

#include "Kyoto/Math/CGameSplineDesc.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class COBBTreeGroup;
class CFluidPlane;

class CSpline;
// Guessed name; the owned waypoint helper remains incompletely scaffolded.
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
  CScriptPlatform(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, const CModelData& model, const CActorParameters& params,
                  const CAABox& bounds,
                  const rstl::optional_object< TLockedToken< const COBBTreeGroup > >& dcln,
                  const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                  const CMaterialList& materials, bool renderRainSplashes, uint maxRainSplashes,
                  uint rainGenRate, const CGameSplineDesc& motionSpline, uint motionFlags,
                  const CVector3f& conveyorVelocity, const CMayaSpline& rollSpline,
                  const CMayaSpline& yawSpline, const CMayaSpline& pitchSpline, float initialTime,
                  float randomAnimationOffset);

  // CEntity
  ~CScriptPlatform() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override;
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

  bool IsMotionActive() const { return mMotionActive; } // Reconstructed name.

  void ResetMotion(float time, CStateManager& mgr); // Reconstructed name.
  CQuaternion CalculateRotationDelta();             // Guessed name
  void SetTransformExplicitly(const CTransform4f& xf);
  void SetTransformIfNoPositionSpline(const CTransform4f& xf); // Reconstructed name.
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
  void SetControlledAnimation(bool controlled) { mControlledAnimation = controlled; }
  void SetActorRotateId(TUniqueId id) { mActorRotateId = id; } // Guessed name.
  float GetMotionDuration() const { return mMotionDuration; }  // Guessed name

  typedef rstl::reserved_vector< ushort, 1024 > TMovedList;
  typedef rstl::reserved_vector< TUniqueId, 1024 > TNearList;
  static bool IsInMovedList(TUniqueId id, const TMovedList& moved);
  void DragSlaves(CStateManager& mgr, TMovedList& moved);
  void DragSlave(CStateManager& mgr, TMovedList& moved, const SRiders& slave);
  void MoveRiders(CStateManager& mgr, float dt, bool active, rstl::vector< SRiders >& riders,
                  rstl::vector< SRiders >& collidedRiders, const TNearList& nearList,
                  const CTransform4f& oldXf, const CTransform4f& newXf, const CVector3f& dragDelta,
                  CQuaternion rotDelta);
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
  rstl::optional_object< TLockedToken< const COBBTreeGroup > > mTreeGroupContainer;
  rstl::single_ptr< CCollisionPrimitive > mTreeGroup;
  rstl::vector< SRiders > mRiders;
  rstl::vector< SRiders > mStaticSlaves;
  rstl::vector< SRiders > mDynamicSlaves;
  uint mMaxRainSplashes;
  uint mRainGenRate;
  TUniqueId mBoundsTrigger;
  rstl::single_ptr< CGameSplineDesc > mMotionSpline;
  rstl::single_ptr< CSpline > mSplineController;
  float mMotionTime;
  uint mMotionFlags;
  float mInitialTime;
  float mMotionDuration;
  rstl::single_ptr< CPlatformWaypointTracker > mWaypointTracker;
  rstl::single_ptr< CMayaSpline > mRollSpline;
  rstl::single_ptr< CMayaSpline > mYawSpline;
  rstl::single_ptr< CMayaSpline > mPitchSpline;
  TUniqueId mActorRotateId;
  TUniqueId x452_;
  TUniqueId mLookAtTarget;
  float mRandomAnimationOffset;
  CTransform4f mInitialTransform;
  bool mDead : 1;
  bool mControlledAnimation : 1;
  bool mRenderRainSplashes : 1;
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
