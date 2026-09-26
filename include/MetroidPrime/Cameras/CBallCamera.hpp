#ifndef _CBALLCAMERA
#define _CBALLCAMERA

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "MetroidPrime/Cameras/CCameraColliderGroup.hpp"
#include "MetroidPrime/Cameras/CCameraSpring.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "rstl/single_ptr.hpp"

class CBallCamera : public CGameCamera {
public:
  enum EBallCameraState {
    kBCS_Default,
    kBCS_FreeLook,
    kBCS_Chase,
    kBCS_Boost,
    kBCS_ToBall,
    kBCS_FromBall,
  };
  enum EBallCameraBehaviour {
    kBCB_Default,
    kBCB_FreezeLookPosition,
    kBCB_HintBallToCam,
    kBCB_HintInitializePosition,
    kBCB_Unknown4,
    kBCB_Unknown5,
    kBCB_Unknown6,
    kBCB_Unknown7,
    kBCB_Unknown8,
    kBCB_Unknown9,
    kBCB_HintLocalOffset,
    kBCB_FixedTransform,
  };
  enum ESplineState {
    kBSS_Invalid,
    kBSS_One,
    kBSS_Two,
  };

  CBallCamera(TUniqueId uid, TUniqueId watchedId, const CTransform4f& xf, float fovY, float nearZ,
              float farZ, float aspect, int index, int controllerIdx);

  // CEntity
  ~CBallCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  void TeleportCamera(const CVector3f& position, CStateManager& mgr);
  void TeleportCamera(const CTransform4f& xf, CStateManager& mgr);
  void TeleportLookAtStuff(CStateManager& mgr);
  void ResetToTweaks(CStateManager& mgr);
  void SetState(EBallCameraState state, CStateManager& mgr);
  void OverrideCameraInfo(CStateManager& mgr);
  void DoorClosed(TUniqueId uid);
  void DoorClosing(TUniqueId uid);
  bool CheckDoorProximity(const CVector3f& position, const CStateManager& mgr) const;

  const CVector3f& GetFixedLookPos() const { return mFixedLookPos; }
  const CVector3f& GetLookAtPosition() const { return mLookPos; }
  const CVector3f& GetLookPosAhead() const { return mLookPosAhead; }
  float GetDistance() const { return mCurMinDistance; }
  float GetElevation() const { return mElevation; }
  EBallCameraState GetState() const { return mState; }
  EBallCameraBehaviour GetBehaviourType() const { return mBehaviour; }
  TUniqueId GetTooCloseActorId() const { return mTooCloseActorId; }
  float GetTooCloseActorDistance() const { return mTooCloseActorDist; }
  void InvalidateSpline();

private:
  // Guessed name
  struct SFromBallTransition {
    SFromBallTransition();
    CTransform4f mPlayerXf;
    CVector3f mLookPos;
    CMotionSpline mSpline;
  };
  // Guessed name
  struct SToBallTransition {
    SToBallTransition();
    CTransform4f mPlayerXf;
    CVector3f mLookPos;
    CMotionSpline mSpline;
  };

  CVector3f ApplyColliders();
  CVector3f AvoidGeometry(const CTransform4f& xf,
                          const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                          CStateManager& mgr);
  CVector3f AvoidGeometryFull(const CTransform4f& xf,
                              const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                              CStateManager& mgr);
  static bool DetectCollision(const CVector3f& from, const CVector3f& to, float radius,
                              float& distance, const CStateManager& mgr, int controllerIdx);
  CVector3f FindDesiredPosition(float distance, float elevation, CVector3f direction,
                                CStateManager& mgr, bool fullTest);
  CTransform4f FindDesiredTransform(CVector3f direction, CStateManager& mgr);
  bool ConstrainElevationAndDistance(float& elevation, float& distance, float dt,
                                     CStateManager& mgr);
  CVector3f ConstrainYawAngle(const CPlayer& player, float yawSpeed, float dampenAngle, float dt,
                              CStateManager& mgr);
  void UpdateTransform(const CVector3f& lookDirection, const CVector3f& position, float dt,
                       CStateManager& mgr);
  void UpdatePlayerMovement(float dt, CStateManager& mgr);
  CVector3f InterpolateCameraElevation(CVector3f position, float dt);
  bool ShouldResetSpline(CStateManager& mgr) const;
  void BuildSpline(CStateManager& mgr);
  void UpdateUsingSpline(float dt, CStateManager& mgr);
  // Collision-search helpers whose original names remain unresolved.
  bool fn_801a67a4(float radius, const CVector3f& from, const CVector3f& direction,
                   const rstl::reserved_vector< TUniqueId, 1024 >& nearList, CVector3f& result,
                   CStateManager& mgr);
  bool fn_801a6b20(const CVector3f& from, const CVector3f& direction, CVector3f& result,
                   CStateManager& mgr);
  bool fn_801a36f0(float distance, float dt, CVector3f& position, CStateManager& mgr);
  bool fn_801a39d0(float distance, float dt, CVector3f& position, CStateManager& mgr);
  void UpdateUsingColliders(float dt, CStateManager& mgr);
  void UpdateUsingFreeLook(float dt, CStateManager& mgr);
  void UpdateUsingTransitions(float dt, CStateManager& mgr);
  void UpdateLookAtPosition(float dt, CStateManager& mgr, bool teleport);
  void UpdateObjectTooCloseId(CStateManager& mgr);
  void UpdateAnglePerSecond(float dt);
  CVector3f ComputeVelocity(CVector3f currentVelocity, CVector3f positionDelta, float dt);
  CVector3f TweenVelocity(const CVector3f& currentVelocity, const CVector3f& newVelocity,
                          float rate, float dt);
  CVector3f MoveCollisionActor(const CVector3f& position, float dt, CStateManager& mgr);
  CVector3f ClampElevationToWater(CVector3f position, CStateManager& mgr) const;
  void CheckFailSafe(float dt, CStateManager& mgr);
  void ActivateFailSafe(float dt, CStateManager& mgr);
  bool SplineIntersectTest(CMaterialList& intersectMaterial, CStateManager& mgr) const;

  bool UpdateTransitionToBallCamera(CStateManager& mgr);
  bool UpdateTransitionToBallCamera(float dt, CStateManager& mgr);
  bool TransitionToMorphBallState(CStateManager& mgr);
  bool CheckFailsafeToMorphBallState(CStateManager& mgr);
  bool UpdateTransitionFromBallCamera(CStateManager& mgr);
  bool TransitionFromMorphBallState(CStateManager& mgr);
  bool CheckFailsafeFromMorphBallState(CStateManager& mgr);

  EBallCameraBehaviour mBehaviour;
  bool x204_24_ : 1;
  bool mChaseAllowed : 1;
  bool mBoostAllowed : 1;
  bool mObscureAvoidance : 1;
  bool mVolumeCollider : 1;
  bool mClampAttitude : 1;
  bool mClampAzimuth : 1;
  bool mClearLOS : 1;
  bool mPrevClearLOS : 1;
  bool mAvoidGeometryFull : 1;
  bool mLookAtBall : 1;
  bool mForceProcessing : 1;
  bool mObtuseDirection : 1;
  bool mNoElevationInterp : 1;
  bool mDirectElevation : 1;
  bool mOverrideLookDir : 1;
  bool mNoElevationVelClamp : 1;
  bool mNoSpline : 1;
  bool x206_26_ : 1;
  bool mNearbyDoorClosed : 1;
  bool mNearbyDoorClosing : 1;
  bool x206_29_ : 1;
  float mCurMinDistance;
  float mTargetMinDistance;
  float mMaxDistance;
  float mBackwardsDistance;
  float mElevation;
  float mCurAnglePerSecond;
  float mTargetAnglePerSecond;
  float mAttitudeRange;
  float mAzimuthRange;
  CVector3f mLookAtOffset;
  CVector3f mLookPosAhead;
  CVector3f mFixedLookPos;
  CVector3f mLookPos;
  CTransform4f mNextLookXf;
  CCameraSpring mBallCameraSpring;
  CCameraSpring mBallCameraCentroidSpring;
  CCameraSpring mBallCameraLookAtSpring;
  CCameraSpring mBallCameraCentroidDistanceSpring;
  CCameraColliderGroup mSmallColliders;
  CCameraColliderGroup mMediumColliders;
  CCameraColliderGroup mLargeColliders;
  int mAvoidGeomCycle;
  float mColliderMag;
  CVector3f mDampedPos;
  int x3b0_;
  int x3b4_;
  int x3b8_;
  CVector3f mPrevBallPos;
  float mBallVelFlat;
  float mMaxBallVel;
  CVector3f mBallDelta;
  CVector3f mBallDeltaFlat;
  float mSpeedFactor;
  float mSpeedingTime;
  CAABox mCollidersAABB;
  float mObscuredTime;
  CMaterialList mObscuringMaterial;
  float mUnobscureMag;
  CVector3f mSplineIntermediatePos;
  TUniqueId mObscuringObjectId;
  ESplineState mSplineState;
  bool mReevalSplineEnd : 1;
  float mSplineCtrl;
  float mSplineCtrlRange;
  CMotionSpline mCamSpline;
  CMaterialList mCollisionExcludeList;
  bool mCamBehindFloorOrWall : 1;
  CVector3f mSplineEndPosition;
  float x498_;
  float mElevInterpTimer;
  float mElevInterpStart;
  TUniqueId mTooCloseActorId;
  float mTooCloseActorDist;
  bool mPendingFailsafe;
  float x4b0_;
  float mFreeLookYawDelta;
  float mFreeLookPitchDelta;
  float mFreeLookDistance;
  float mFreeLookZoomOutInput;
  float mFreeLookZoomInInput;
  EBallCameraState mState;
  float mChaseDistance;
  float mChaseYawSpeed;
  float mChaseAnglePerSecond;
  CCameraSpring mBallCameraChaseSpring;
  float mBoostDistance;
  float mBoostYawSpeed;
  float mBoostAnglePerSecond;
  CVector3f mBoostLookAtOffset;
  CCameraSpring mBallCameraBoostSpring;
  CVector3f mOverrideBallToCam;
  CVector3f mHintLocalOffset;
  float mConservativeDoorCamDistance;
  TUniqueId mCollisionActorId;
  float mClampVelTimer;
  float mClampVelRange;
  int mShortMoveCount;
  rstl::single_ptr< SFromBallTransition > mFromBallTransition;
  rstl::single_ptr< SToBallTransition > mToBallTransition;
  CVector3f mInitialForward;
  CTransform4f mFixedTransform;
  CVector3f mDesiredPosition;
};
CHECK_SIZEOF(CBallCamera, 0x598)

#endif // _CBALLCAMERA
