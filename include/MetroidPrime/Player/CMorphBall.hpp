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
  EBallState GetBallState() const { return xc80_ballState; }
  void SetBoostEnabled(bool enabled) { x101c_24_boostEnabled = enabled; }
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

  const CCollidableSphere& GetCollidableSphere() const { return x38_collisionSphere; }
  const CTransform4f& GetSurfaceToWorld() const { return xd28_surfaceToWorld; }
  int GetLastWallCollisionFrame() const { return xc78_lastWallCollisionFrame; }
  int GetLastFloorCollisionFrame() const { return xc7c_lastFloorCollisionFrame; }
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

  CPlayer& x0_player;
  int x4_loadedModelId;
  uint x8_ballGlowColorIdx;
  float xc_radius;
  CVector3f x10_boostControlForce;
  CVector3f x1c_controlForce;
  bool x28_tireMode;
  float x2c_tireLeanAngle;
  float x30_ballTiltAngle;
  CCollidableSphere x38_collisionSphere;
  rstl::single_ptr< CModelData > x58_ballModel;
  uint x5c_ballModelShader;
  rstl::single_ptr< CModelData > x60_spiderBallGlassModel;
  uint x64_spiderBallGlassModelShader;
  rstl::single_ptr< CModelData > x68_lowPolyBallModel;
  uint x6c_lowPolyBallModelShader;
  rstl::single_ptr< CModelData > x70_frozenBallModel;
  CCollisionInfoList x74_collisionInfos;
  int xc78_lastWallCollisionFrame;
  int xc7c_lastFloorCollisionFrame;
  EBallState xc80_ballState;
  CVector3f xc84_playerToSpiderNormal;
  float xc90_spiderPullMovement;
  CVector3f xc94_spiderTrackPoint;
  CVector3f xca0_spiderInterpBetweenPoints;
  CVector3f xcac_spiderBetweenPoints;
  float xcb8_linearVelocityDamping;
  float xcbc_angularVelocityDamping;
  bool xcc0_spiderNearby;
  bool xcc1_touchingSpider;
  bool xcc2_spiderBallSwinging;
  bool xcc3_spiderSwingInAir;
  ESpiderSurfaceType xcc4_spiderSurfaceType;
  CTransform4f xcc8_spiderSurfaceTransform;
  float xcf8_spiderSurfacePivotAngle;
  float xcfc_spiderSurfacePivotTargetAngle;
  float xd00_refPullVelocity;
  float xd04_playerToSpiderTrackDistance;
  float xd08_swingControlDirection;
  float xd0c_swingControlTime;
  CVector2f xd10_normalizedSpiderSurfaceForces;
  float xd18_spiderTrackForceMagnitude;
  float xd1c_spiderViewControlMagnitude;
  float xd20_damageTimer;
  bool xd24_spiderForcesReset;
  CTransform4f xd28_surfaceToWorld;
  TToken< CSwooshDescription > xd58_slowBlueTailSwoosh;
  TToken< CSwooshDescription > xd60_slowBlueTailSwoosh2;
  TToken< CSwooshDescription > xd68_jaggyTrail;
  TToken< CSwooshDescription > xd70_sideSwoosh;
  TToken< CGenDescription > xd78_wallSpark;
  TToken< CGenDescription > xd80_ballInnerGlow;
  TToken< CGenDescription > xd88_spiderBallMagnet;
  TToken< CGenDescription > xd90_boostBallGlow;
  TToken< CGenDescription > xd98_morphBallTransitionFlash;
  TToken< CGenDescription > xda0_morphBallIceBreak;
  TToken< CGenDescription > xda8_boostEffect;
  TToken< CGenDescription > xdb0_deathBallOuterShell;
  TToken< CGenDescription > xdb8_deathBallSpikes;
  TToken< CGenDescription > xdc0_screwAttackJumpFlash;
  rstl::single_ptr< CParticleSwoosh > xdc8_slowBlueTailSwooshGen;
  rstl::single_ptr< CParticleSwoosh > xdcc_slowBlueTailSwooshGen2;
  rstl::single_ptr< CParticleSwoosh > xdd0_slowBlueTailSwoosh2Gen;
  rstl::single_ptr< CParticleSwoosh > xdd4_slowBlueTailSwoosh2Gen2;
  rstl::single_ptr< CParticleSwoosh > xdd8_jaggyTrailGen;
  rstl::single_ptr< CParticleSwoosh > xddc_sideSwooshGen;
  rstl::single_ptr< CParticleSwoosh > xde0_sideSwooshGen2;
  rstl::single_ptr< CElementGen > xde4_wallSparkGen;
  rstl::single_ptr< CElementGen > xde8_ballInnerGlowGen;
  rstl::single_ptr< CElementGen > xdec_spiderBallMagnetGen;
  rstl::single_ptr< CElementGen > xdf0_boostBallGlowGen;
  rstl::single_ptr< CElementGen > xdf4_boostEffectGen;
  rstl::single_ptr< CElementGen > xdf8_morphBallTransitionFlashGen;
  rstl::single_ptr< CElementGen > xdfc_morphBallIceBreakGen;
  rstl::single_ptr< CElementGen > xe00_deathBallOuterShellGen;
  rstl::single_ptr< CElementGen > xe04_deathBallSpikesGen;
  rstl::single_ptr< CElementGen > xe08_screwAttackJumpFlashGen;
  rstl::single_ptr< CElementGen > xe0c_screwAttackWallJumpFlashGen;
  rstl::reserved_vector< rstl::auto_ptr< CDeferredParticleEffect >, 6 > xe10_wakeEffects;
  int xe44_wakeEffectIndex;
  TUniqueId xe48_ballInnerGlowLight;
  bool xe4a_ballLightActive;
  rstl::single_ptr< CWorldShadow > xe4c_worldShadow;
  rstl::single_ptr< CActorLights > xe50_actorLights;
  rstl::single_ptr< CRainSplashGenerator > xe54_rainSplashGen;
  float xe58_tireFactor;
  float xe5c_maxTireFactor;
  float xe60_tireInterpolationSpeed;
  bool xe64_tireInterpolating;
  float xe68_boostOverLightFactor;
  float xe6c_boostLightFactor;
  float xe70_spiderLightFactor;
  TReservedAverage< CQuaternion, 5 > xe74_ballOrientationAverage;
  TReservedAverage< CVector3f, 5 > xec8_ballPositionAverage;
  TReservedAverage< float, 15 > xf08_liftSpeedAverage;
  TReservedAverage< CVector3f, 15 > xf48_liftControlForceAverage;
  uint x1000_failsafeCounter;
  CVector3f x1004_velocityBeforeFailsafe; // Guessed name
  CVector3f x1010_velocityAfterFailsafe;  // Guessed name: adds an upward recovery velocity.
  bool x101c_24_boostEnabled : 1;
  bool x101c_25_touchedFloorDuringBoost
      : 1; // Guessed name; set by IsFloor, cleared on boost/morph.
  float x1020_boostChargeTime;
  float x1024_timeNotInBoost;
  float x1028_; // Zero-initialized; no other access identified in this TU.
  float x102c_boostDrainTime;
  float x1030_boostEffectTime;
  float x1034_boostDamageScale;
  float x1038_disableSpiderBallTime;
  CVector3f x103c_spiderBoostDirection;
  bool x1048_hasSpiderBoostDirection;
  rstl::reserved_vector< TUniqueId, 1024 > x104c_boostDamagedObjects;
  float x1850_boostTrailFadeTimer;
  bool x1854_24_inHalfPipeMode : 1;
  bool x1854_25_inHalfPipeModeInAir : 1;
  bool x1854_26_touchedHalfPipeRecently : 1;
  bool x1854_27_ballCloseToCollision : 1;
  float x1858_closeToCollisionTime;
  float x185c_touchHalfPipeCooldown;
  float x1860_disableControlCooldown;
  float x1864_touchedHalfPipeRecentCooldown;
  CVector3f x1868_prevHalfPipeNormal;
  CVector3f x1874_halfPipeNormal;
  int x1880_ballAnimationIndex;
  CSfxHandle x1884_boostChargeSfx;
  CSfxHandle x1888_boostReleaseSfx;
  CSfxHandle x188c_wallHitSfx;
  CSfxHandle x1890_rollSfx;
  CSfxHandle x1894_spiderSfx;
  CSfxHandle x1898_deathBallSfx;
  CSfxHandle x189c_screwAttackSfx;
  ushort x18a0_rollSfxId;
  ushort x18a2_landSfxId;
  uint x18a4_wallSparkFrameCountdown;
  // Guessed names: recovered from collision, jump-input and recovery transitions.
  bool x18a8_24_endScrewAttackRequested : 1;
  bool x18a8_25_touchingWall : 1;
  bool x18a8_26_pendingRecoil : 1;
  bool x18a8_27_recoiling : 1;
  bool x18a8_28_wallJumpInputPending : 1;
  bool x18a8_29_collidedDuringRecovery : 1;
  bool x18a8_30_ : 1; // Collision flag which halves recovery timer advancement.
  bool x18a8_31_forcedScrewJumpInput : 1;
  int x18ac_screwAttackJumpCount;
  int x18b0_wallJumpCount;
  int x18b4_screwAttackExitAnimationFrames;
  int x18b8_screwAttackGroundedFrames;
  float x18bc_timeSinceScrewAttackJump;
  float x18c0_wallContactTime;
  float x18c4_screwAttackRecoveryCollisionTime;
  CVector3f x18c8_wallNormal;
  CVector3f x18d4_screwAttackDirection;
  rstl::vector< rstl::pair< TUniqueId, float > > x18e0_deathBallDamageCooldowns;
  EBallBoostState x18f0_boostState;
  EBombJumpState x18f4_bombJumpState;
  float x18f8_damageEffect;
  float x18fc_damageEffectDecaySpeed;
  float x1900_damageTime;
  bool x1904_24_multiplayer : 1;
  rstl::single_ptr< CMorphBallShadow > x1908_shadow;
};
CHECK_SIZEOF(CMorphBall, 0x1910)

#endif // _CMORPHBALL
