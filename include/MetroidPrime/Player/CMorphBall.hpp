#ifndef _CMORPHBALL
#define _CMORPHBALL

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CActorLights;
class CDamageInfo;
class CElementGen;
class CFinalInput;
class CFrustumPlanes;
class CGenDescription;
class CInt32POINode;
class CMaterialFilter;
class CModelData;
class CMorphBallShadow;
class CParticleSwoosh;
class CPlayer;
class CRainSplashGenerator;
class CScriptMsg;
class CSwooshDescription;
class CSkinnedModel;
struct SSkinningWorkspace;
class CWorldShadow;
class CDeferredParticleEffect; // Guessed name: dependency-backed wake effect, target size 0x44.

// G2ME01 structure pass. See Echoes research/CMorphBall-G2ME01.md for evidence and uncertainties.
class CMorphBall {
public:
  enum EBallBoostState { kBBS_BoostAvailable, kBBS_BoostDisabled };
  enum EBombJumpState { kBJS_BombJumpAvailable, kBJS_BombJumpDisabled };
  // Guessed name. Echoes combines Prime's separate spider/boost/projectile states.
  enum EBallState {
    kBS_Normal,
    kBS_Boost,
    kBS_Spider,
    kBS_SpiderBoost,
    kBS_ScrewAttack,
    kBS_ScrewAttackWallJump,
    kBS_ScrewAttackRecovery,
    kBS_Projectile
  };
  // Guessed name. This replaces Prime's isSpiderSurface boolean.
  enum ESpiderSurfaceType { kSST_None, kSST_Waypoint, kSST_ScriptedSurface, kSST_CollisionSurface };

  CMorphBall(CPlayer& player, float radius, bool multiplayer);
  ~CMorphBall();
  EBallBoostState GetBallBoostState() const;
  void SetBallBoostState(EBallBoostState state);
  EBombJumpState GetBombJumpState() const;
  EBallState GetBallState() const { return mBallState; }
  void SetBoostEnabled(bool enabled) { mBoostEnabled = enabled; }
  float GetBoostChargeTimer() const;
  float GetTimeNotInBoost() const; // Guessed name
  bool IsBoosting() const;
  bool IsBoostShieldActive() const;
  bool InScrewAttackMode() const;
  bool IsProjectile() const;
  void SetAsProjectile(bool projectile);
  void SetDamageTimer(float time);
  void SetDisableSpiderBallTime(float time);
  void TakeDamage(float damage);

  const CCollidableSphere& GetCollidableSphere() const { return mCollisionSphere; }
  const CTransform4f& GetSurfaceToWorld() const { return mSurfaceToWorld; }
  int GetLastWallCollisionFrame() const { return mLastWallCollisionFrame; }
  int GetLastFloorCollisionFrame() const { return mLastFloorCollisionFrame; }
  CVector3f GetBallPosition() const; // Guessed name
  CTransform4f GetBallToWorld() const;
  CTransform4f GetSwooshToWorld() const;
  float GetBallRadius() const;
  float GetBallTouchRadius() const;
  CAABox GetRenderBounds(const CStateManager& mgr) const; // Guessed name

  void LoadMorphBallModel();
  static CModelData* GetMorphBallModel(const rstl::string& name, float radius);
  void TouchModel(const CStateManager& mgr) const;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum);
  void Render(const CStateManager& mgr, const CActorLights* lights) const;
  void RenderDamageEffects(const CStateManager& mgr, const CTransform4f& transform) const;
  void RenderScrewAttackJumpEffects() const; // Guessed name
  void RenderIceBreakEffect(const CStateManager& mgr) const;
  void UpdateIceBreakEffect(float dt);
  void ResetMorphBallIceBreak();
  bool IsMorphBallTransitionFlashValid() const;
  void RenderMorphBallTransitionFlash(const CStateManager& mgr) const;
  void UpdateMorphBallTransitionFlash(float dt);
  void ResetMorphBallTransitionFlash();
  void CreateBallShadow();
  void DeleteBallShadow();
  void RenderToShadowTex(CStateManager& mgr);
  void DrawBallShadow(CStateManager& mgr);

  void Update(float dt, CStateManager& mgr);
  void UpdateEffects(float dt, CStateManager& mgr);
  void UpdateBallLight(float dt, CStateManager& mgr); // Guessed name
  void UpdateDeathBall(float dt, CStateManager& mgr); // Guessed name
  void SetBallLightActive(CStateManager& mgr, bool active);
  void DeleteLight(CStateManager& mgr);
  void StopParticleWakes();
  void StopSounds();
  void StartLandingSfx();
  void StartScrewAttackSfx(); // Guessed name
  void EnterMorphBallState(CStateManager& mgr, EBallState state);
  void LeaveMorphBallState(CStateManager& mgr);
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg);
  bool DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                       EUserEventType type); // Guessed name
  void Touch(CActor& actor, CStateManager& mgr);
  void FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr);
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& collisions, CStateManager& mgr);

  void ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void ComputeScrewAttackMovement(const CFinalInput& input, CStateManager& mgr,
                                  float dt);                 // Guessed name
  void UpdateScrewAttackRecovery(float dt);                  // Guessed name
  void ApplyScrewAttackDamage(float dt, CStateManager& mgr); // Guessed name
  void ResetScrewAttackExitAnimationTimer();                 // Guessed name
  int GetScrewAttackGroundedFrames() const;                  // Guessed name
  void SetScrewAttackActive(bool active); // Inherited annotation: sets the forced-input bit.
  bool IsMovementAllowed() const;
  void EnterBoosting(CStateManager& mgr, bool skipImpulse);
  void LeaveBoosting();
  void CancelBoosting();
  void ApplyBoostBallDamage(CStateManager& mgr, TUniqueId id, const CDamageInfo& damage, float dt);
  float ForwardInput(const CFinalInput& input) const;
  float BallTurnInput(const CFinalInput& input) const;
  float ComputeMaxSpeed() const;
  float GetGravityAcceleration() const; // Guessed name
  void ApplyGravity();
  void ApplyFriction(float friction);
  float CalculateSurfaceFriction() const;
  void SpinToSpeed(float speed, const CVector3f& direction, float dt);
  void DampLinearAndAngularVelocities(float linearDamping, float angularDamping, float dt);
  void ComputeLiftForces(const CVector3f& controlForce, const CVector3f& velocity,
                         const CStateManager& mgr);
  void UpdateBallDynamics(CStateManager& mgr, float dt);
  bool UpdateMarbleDynamics(CStateManager& mgr, float dt, const CVector3f& point);
  void SwitchToTire();
  void SwitchToMarble();
  float GetMinimumAlignmentSpeed() const;
  bool IsClimbable(const CCollisionInfo& collision) const;
  bool CalculateBallContactInfo(CVector3f& normal, CVector3f& point) const;
  CTransform4f CalculateSurfaceToWorld(const CVector3f& normal, const CVector3f& point,
                                       const CVector3f& direction) const;
  bool BallCloseToCollision(const CStateManager& mgr, float distance,
                            const CMaterialFilter& filter) const;
  void UpdateHalfPipeStatus(CStateManager& mgr, float dt);
  void DisableHalfPipeStatus();
  void SetIsInHalfPipeMode(bool active);
  void SetIsInHalfPipeModeInAir(bool active);
  void SetTouchedHalfPipeRecently(bool touched);
  bool GetIsInHalfPipeMode() const;
  bool GetIsInHalfPipeModeInAir() const;
  bool GetTouchedHalfPipeRecently() const;

  void UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt);
  void CreateSpiderBallParticles(CStateManager& mgr, const CVector3f& ballPosition,
                                 const CVector3f& trackPoint);
  // Guessed name: moving-platform rotation/translation applied to Spider Ball state.
  void TransformSpiderBallState(const CQuaternion& rotation, const CVector3f& translation);
  bool FindClosestSpiderBallWaypoint(CStateManager& mgr, const CVector3f& center,
                                     CVector3f& trackPoint, CVector3f& interpolatedDirection,
                                     CVector3f& direction, float& distance, CVector3f& normal,
                                     ESpiderSurfaceType& surfaceType, TUniqueId& surfaceId,
                                     CTransform4f& surfaceTransform) const;
  bool CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const;
  void SetSpiderBallSwingingState(bool swinging);
  void ResetSpiderBallSwingControllerMovementTimer();
  void UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt);
  float GetSpiderBallSwingControllerMovementScalar() const;
  float GetSpiderBallControllerMovement(const CFinalInput& input) const;
  void ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt);
  void ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr, float dt);
  void ResetSpiderBallForces();
  CVector2f CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const;
  CVector3f TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr) const;
  CVector3f TransformSpiderBallForcesXY(CVector2f& forces, CStateManager& mgr) const;

private:
  void InitializeWakeEffects();
  void SelectMorphBallSounds(const CMaterialList& material);
  void UpdateMorphBallSound(float dt, CStateManager& mgr);
  // Guessed name. Echoes callback has model, skinning workspace and context arguments.
  static void PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                             void* context);

  CPlayer& mPlayer;
  int mLoadedModelId;
  uint mBallGlowColorIdx;
  float mRadius;
  CVector3f mBoostControlForce;
  CVector3f mControlForce;
  bool mTireMode;
  float mTireLeanAngle;
  float mBallTiltAngle;
  CCollidableSphere mCollisionSphere;
  rstl::single_ptr< CModelData > mBallModel;
  uint mBallModelShader;
  rstl::single_ptr< CModelData > mSpiderBallGlassModel;
  uint mSpiderBallGlassModelShader;
  rstl::single_ptr< CModelData > mLowPolyBallModel;
  uint mLowPolyBallModelShader;
  rstl::single_ptr< CModelData > mFrozenBallModel;
  CCollisionInfoList mCollisionInfos;
  int mLastWallCollisionFrame;
  int mLastFloorCollisionFrame;
  EBallState mBallState;
  CVector3f mPlayerToSpiderNormal;
  float mSpiderPullMovement;
  CVector3f mSpiderTrackPoint;
  CVector3f mSpiderInterpBetweenPoints;
  CVector3f mSpiderBetweenPoints;
  float mLinearVelocityDamping;
  float mAngularVelocityDamping;
  bool mSpiderNearby;
  bool mTouchingSpider;
  bool mSpiderBallSwinging;
  bool mSpiderSwingInAir;
  ESpiderSurfaceType mSpiderSurfaceType;
  CTransform4f mSpiderSurfaceTransform;
  float mSpiderSurfacePivotAngle;
  float mSpiderSurfacePivotTargetAngle;
  float mRefPullVelocity;
  float mPlayerToSpiderTrackDistance;
  float mSwingControlDirection;
  float mSwingControlTime;
  CVector2f mNormalizedSpiderSurfaceForces;
  float mSpiderTrackForceMagnitude;
  float mSpiderViewControlMagnitude;
  float mDamageTimer;
  bool mSpiderForcesReset;
  CTransform4f mSurfaceToWorld;
  TToken< CSwooshDescription > mSlowBlueTailSwoosh;
  TToken< CSwooshDescription > mSlowBlueTailSwoosh2;
  TToken< CSwooshDescription > mJaggyTrail;
  TToken< CSwooshDescription > mSideSwoosh;
  TToken< CGenDescription > mWallSpark;
  TToken< CGenDescription > mBallInnerGlow;
  TToken< CGenDescription > mSpiderBallMagnet;
  TToken< CGenDescription > mBoostBallGlow;
  TToken< CGenDescription > mMorphBallTransitionFlash;
  TToken< CGenDescription > mMorphBallIceBreak;
  TToken< CGenDescription > mBoostEffect;
  TToken< CGenDescription > mDeathBallOuterShell;
  TToken< CGenDescription > mDeathBallSpikes;
  TToken< CGenDescription > mScrewAttackJumpFlash;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwooshGen;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwooshGen2;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwoosh2Gen;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwoosh2Gen2;
  rstl::single_ptr< CParticleSwoosh > mJaggyTrailGen;
  rstl::single_ptr< CParticleSwoosh > mSideSwooshGen;
  rstl::single_ptr< CParticleSwoosh > mSideSwooshGen2;
  rstl::single_ptr< CElementGen > mWallSparkGen;
  rstl::single_ptr< CElementGen > mBallInnerGlowGen;
  rstl::single_ptr< CElementGen > mSpiderBallMagnetGen;
  rstl::single_ptr< CElementGen > mBoostBallGlowGen;
  rstl::single_ptr< CElementGen > mBoostEffectGen;
  rstl::single_ptr< CElementGen > mMorphBallTransitionFlashGen;
  rstl::single_ptr< CElementGen > mMorphBallIceBreakGen;
  rstl::single_ptr< CElementGen > mDeathBallOuterShellGen;
  rstl::single_ptr< CElementGen > mDeathBallSpikesGen;
  rstl::single_ptr< CElementGen > mScrewAttackJumpFlashGen;
  rstl::single_ptr< CElementGen > mScrewAttackWallJumpFlashGen;
  rstl::reserved_vector< rstl::auto_ptr< CDeferredParticleEffect >, 6 > mWakeEffects;
  int mWakeEffectIndex;
  TUniqueId mBallInnerGlowLight;
  bool mBallLightActive;
  rstl::single_ptr< CWorldShadow > mWorldShadow;
  rstl::single_ptr< CActorLights > mActorLights;
  rstl::single_ptr< CRainSplashGenerator > mRainSplashGen;
  float mTireFactor;
  float mMaxTireFactor;
  float mTireInterpolationSpeed;
  bool mTireInterpolating;
  float mBoostOverLightFactor;
  float mBoostLightFactor;
  float mSpiderLightFactor;
  TReservedAverage< CQuaternion, 5 > mBallOrientationAverage;
  TReservedAverage< CVector3f, 5 > mBallPositionAverage;
  TReservedAverage< float, 15 > mLiftSpeedAverage;
  TReservedAverage< CVector3f, 15 > mLiftControlForceAverage;
  uint mFailsafeCounter;
  CVector3f mVelocityBeforeFailsafe; // Guessed name
  CVector3f mVelocityAfterFailsafe;  // Guessed name: adds an upward recovery velocity.
  bool mBoostEnabled : 1;
  bool mTouchedFloorDuringBoost
      : 1; // Guessed name; set by IsFloor, cleared on boost/morph.
  float mBoostChargeTime;
  float mTimeNotInBoost;
  float x1028_; // Zero-initialized; no other access identified in this TU.
  float mBoostDrainTime;
  float mBoostEffectTime;
  float mBoostDamageScale;
  float mDisableSpiderBallTime;
  CVector3f mSpiderBoostDirection;
  bool mHasSpiderBoostDirection;
  rstl::reserved_vector< TUniqueId, 1024 > mBoostDamagedObjects;
  float mBoostTrailFadeTimer;
  bool mInHalfPipeMode : 1;
  bool mInHalfPipeModeInAir : 1;
  bool mTouchedHalfPipeRecently : 1;
  bool mBallCloseToCollision : 1;
  float mCloseToCollisionTime;
  float mTouchHalfPipeCooldown;
  float mDisableControlCooldown;
  float mTouchedHalfPipeRecentCooldown;
  CVector3f mPrevHalfPipeNormal;
  CVector3f mHalfPipeNormal;
  int mBallAnimationIndex;
  CSfxHandle mBoostChargeSfx;
  CSfxHandle mBoostReleaseSfx;
  CSfxHandle mWallHitSfx;
  CSfxHandle mRollSfx;
  CSfxHandle mSpiderSfx;
  CSfxHandle mDeathBallSfx;
  CSfxHandle mScrewAttackSfx;
  ushort mRollSfxId;
  ushort mLandSfxId;
  uint mWallSparkFrameCountdown;
  // Guessed names: recovered from collision, jump-input and recovery transitions.
  bool mEndScrewAttackRequested : 1;
  bool mTouchingWall : 1;
  bool mPendingRecoil : 1;
  bool mRecoiling : 1;
  bool mWallJumpInputPending : 1;
  bool mCollidedDuringRecovery : 1;
  bool x18a8_30_ : 1; // Collision flag which halves recovery timer advancement.
  bool mForcedScrewJumpInput : 1;
  int mScrewAttackJumpCount;
  int mWallJumpCount;
  int mScrewAttackExitAnimationFrames;
  int mScrewAttackGroundedFrames;
  float mTimeSinceScrewAttackJump;
  float mWallContactTime;
  float mScrewAttackRecoveryCollisionTime;
  CVector3f mWallNormal;
  CVector3f mScrewAttackDirection;
  rstl::vector< rstl::pair< TUniqueId, float > > mDeathBallDamageCooldowns;
  EBallBoostState mBoostState;
  EBombJumpState mBombJumpState;
  float mDamageEffect;
  float mDamageEffectDecaySpeed;
  float mDamageTime;
  bool mMultiplayer : 1;
  rstl::single_ptr< CMorphBallShadow > mShadow;
};
CHECK_SIZEOF(CMorphBall, 0x1910)

#endif // _CMORPHBALL
