#ifndef _CPLAYER
#define _CPLAYER

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerEnergyDrain.hpp"
#include "MetroidPrime/Player/CPlayerKnockBackMgr.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CMorphBall;
class CPlayerState;
class CCameraManager;
class CPlayerGun;
class CTweakPlayer;
class CTweakPlayerControls;
class CPlayerCameraBob;
class CPlayerStuckTracker;
class CPlayerKnockBackMgr;
class CHintManager;
class CElementGen;
class CGenDescription;
class CFinalInput;
class CCollidableSphere;
class CRezbitEffectOptions;
class CScriptPlayerHint;
class CPlayerTargeting;      // Guessed name; targeting/scan resources, independent TU.
class CPlayerBodyController; // Guessed name; CEntity-derived player animation controller.

namespace NPlayer {
enum EPlayerMovementState {
  kMS_OnGround,
  kMS_Jump,
  kMS_ApplyJump,
  kMS_Falling,
  kMS_FallingMorphed,
};
};

class CPlayer : public CPhysicsActor {
public:
  enum ESurfaceRestraints {
    kSR_Normal,
    kSR_Air,
    kSR_Ice,
    kSR_Organic,
    kSR_Water,
    kSR_Lava,
    kSR_Phazon,
    kSR_Shrubbery,
  };
  enum EPlayerCameraState {
    kCS_FirstPerson,
    kCS_Ball,
    kCS_Two,
    kCS_Transitioning,
    kCS_Spawned,
  };
  enum EPlayerMorphBallState {
    kMS_Unmorphed,
    kMS_Morphed,
    kMS_Morphing,
    kMS_Unmorphing,
  };

  enum EPlayerOrbitState {
    kOS_NoOrbit,
    kOS_OrbitObject,
    kOS_OrbitPoint,
    kOS_OrbitCarcass,
    kOS_ForcedOrbitObject,
    kOS_Grapple,
  };
  enum EPlayerOrbitType {
    kOT_Close,
    kOT_Far,
    kOT_Default,
  };
  enum EPlayerOrbitRequest {
    kOR_StopOrbit,
  };
  enum EPlayerZoneInfo {
    kZI_Targeting,
    kZI_Scan,
  };
  enum EPlayerZoneType {
    kZT_Always = -1,
    kZT_Box,
    kZT_Ellipse,
  };
  enum EPlayerScanState {
    kSS_NotScanning,
    kSS_Scanning,
    kSS_ScanComplete,
  };
  enum EGrappleState {
    kGS_None,
    kGS_Firing,
    kGS_Pull,
    kGS_Swinging,
    kGS_JumpOff,
  };

  class CVisorSteam {
  public:
    CVisorSteam(float targetAlpha, float alphaInDuration, float alphaOutDuration, CAssetId texture);
    void SetSteam(float targetAlpha, float alphaInDuration, float alphaOutDuration,
                  CAssetId texture);
    void Reset();
    void Update(float dt);
    CAssetId GetTextureId() const { return mTexture; }
    float GetAlpha() const { return mAlpha; }

  private:
    float mTargetAlpha;
    float mAlphaInDuration;
    float mAlphaOutDuration;
    CAssetId mTexture;
    float mNextTargetAlpha;
    float mNextAlphaInDuration;
    float mNextAlphaOutDuration;
    CAssetId mNextTexture;
    float mAlpha;
    float mDelayTimer;
  };
  // Guessed name; resources used to display and break the frozen state.
  struct SFrozenResources {
    CAssetId mSteamTexture;
    CAssetId mIceTexture;
    ushort mFreezeSfx;
    ushort mBreakSfx;
  };
  enum ETurretState { kTS_None, kTS_Entering, kTS_Exiting, kTS_Active, kTS_Four, kTS_Ejected };
  enum EBreakFrozenState { kBFS_Break, kBFS_One, kBFS_Two };
  // Guessed state names; numeric values established by the Rezbit state handlers.
  enum ERezbitState { kRS_None, kRS_Infected, kRS_Recovering, kRS_Recovered };
  // Original Wii enum name; channel meanings remain unresolved on GameCube.
  enum EMultiPlayerSoundPan { kMSP_0, kMSP_1, kMSP_2, kMSP_3, kMSP_4 };

  CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
          const CVector3f& playerScale, float mass, float stepUp, float stepDown, float ballRadius,
          const CMaterialList& ml, CPlayerState*, CCameraManager*, bool, int playerIndex, int,
          int charIdx);

  // CEntity
  ~CPlayer() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void UnkVtable20(CStateManager& mgr) override;
  void PreRender(CStateManager& mgr) override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  CVector3f GetHomingPosition(const CStateManager& mgr, float dt) const override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  float GetStepUpHeight() const override;
  float GetStepDownHeight() const override;
  float GetWeight() const override;

  // CPlayer
  virtual bool UnkVtable98() const;

  int GetPlayerIndex() const;
  float GetGunAlpha() const { return mGunAlpha; }
  const CVector3f& GetLookDir() const { return mLookDir; }
  const CVector3f& GetMovementDirection() const { return mMoveDir; }
  NPlayer::EPlayerMovementState GetPlayerMovementState() const { return mMovementState; }
  CMorphBall* GetMorphBall() { return mMorphBall; }
  CPlayerState* GetPlayerState() { return mPlayerState; }

  EPlayerMorphBallState GetMorphballTransitionState() const { return mMorphBallState; }
  EPlayerMorphBallState GetSpawnedMorphballState() const { return mSpawnedMorphBallState; }
  int Get_x12f8() const { return mTurretState; }

  CSfxHandle PlaySfxForPlayer(uint sfxId, short param_4, TAreaId nextAreaId, bool, int);

  float fn_8000BE98() const { return GetDeathAlpha(); }
  void fn_8000BC44(CStateManager& mgr);
  void fn_80019E40(CStateManager&, int);
  void fn_8000d3ac(const CVector3f&, CStateManager&);
  bool fn_8000d40c(const CVector3f&, CStateManager&);
  CTweakPlayer* GetTweakPlayer() const;

  void Teleport(const CTransform4f& xf, CStateManager& mgr, bool resetBallCam);
  void SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  const CCameraManager* GetCameraManager() const { return mCameraManager; }

  void Update(float dt, CStateManager& mgr);
  void PostUpdate(float dt, CStateManager& mgr);
  void DoThink(float dt, CStateManager& mgr);
  void DoPreThink(float dt, CStateManager& mgr);
  void ProcessInput(float dt, const CFinalInput& input, CStateManager& mgr);
  void UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateFreeLook(float dt);
  void ComputeFreeLook(const CFinalInput& input, CStateManager& mgr);
  void UpdateMorphBallState(const CFinalInput& input, float dt, CStateManager& mgr);
  void SetMorphBallState(EPlayerMorphBallState state, EPlayerMorphBallState spawnedState);
  void SetCameraState(EPlayerCameraState state, CStateManager& mgr);
  bool IsMorphBallTransitioning() const;
  bool CanEnterMorphBallState() const;
  bool CanLeaveMorphBallState(CStateManager& mgr, CVector3f& position) const;
  bool AttachActorToPlayer(TUniqueId actor, bool disableGun);
  void DetachActorFromPlayer();
  void UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt);
  bool ValidateScanning(const CFinalInput& input, CStateManager& mgr) const;
  void SetScanningState(EPlayerScanState state, CStateManager& mgr);
  bool ObjectInScanningRange(TUniqueId id, const CStateManager& mgr);
  void FinishNewScan(CStateManager& mgr);
  void UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateVisorTransition(float dt, CStateManager& mgr);
  void UpdateCrosshairsState(const CFinalInput& input);
  void UpdateFrozenState(const CFinalInput& input, CStateManager& mgr);
  bool GetFrozenState() const;
  void Freeze(float timeout, CStateManager& mgr, CAssetId steamTexture, uint sfx,
              CAssetId iceTexture);
  void BreakFrozenState(CStateManager& mgr, EBreakFrozenState state, bool playSound);
  void SetVisorSteam(float targetAlpha, float alphaInDuration, float alphaOutDuration,
                     CAssetId texture);
  void SetHudDisable(float staticTimer, float fadeOutSpeed, float fadeInSpeed);
  bool WasDamaged() const;
  float GetDamageAmount() const;
  float GetPrevDamageAmount() const;
  CVector3f GetDamageLocationWR() const;
  float GetDeathAlpha() const;
  bool IsEnergyLow() const;
  void PushSustainedDamage();
  void PopSustainedDamage();
  void SetControlDirectionInterpolation(float duration);
  void ResetControlDirectionInterpolation();
  void SetPlayerHitWallDuringMove();
  void SetAimTarget(TUniqueId target);
  void UpdateAssistedAiming(const CTransform4f& transform, CStateManager& mgr);
  void UpdateGunTransform(const CVector3f& position, CStateManager& mgr);
  void UpdateArmAndGunTransforms(float dt, CStateManager& mgr);
  void ForceGunOrientation(const CTransform4f& transform, CStateManager& mgr);
  void UpdateGunAlpha(const CStateManager& mgr);
  bool HasTransitionBeamModel() const;
  void AsyncLoadSuit(CStateManager& mgr);
  void UpdateWaterSurfaceCameraBias(CStateManager& mgr);
  bool ShouldSampleFailsafe(const CStateManager& mgr) const;
  void UpdateDarkAetherDamage(float dt, CStateManager& mgr);
  ESurfaceRestraints GetSurfaceRestraint() const;
  void SetSurfaceRestraint(ESurfaceRestraints restraint);
  bool IsOnGround() const;
  CTweakPlayerControls* GetTweakPlayerControls() const;
  CPlayerState::EBeamId GetCurrentBeam() const;
  CHintManager* GetPlayerHintManager();
  const CHintManager* GetPlayerHintManager() const;
  CHintManager* GetControlHintManager();
  const CHintManager* GetControlHintManager() const;
  CPlayerGun* GetPlayerGun();
  const CPlayerGun* GetPlayerGun() const;
  ETurretState GetTurretState() const { return mTurretState; }
  bool IsInTurret() const;
  TUniqueId GetTurretId() const;
  void SetTurretState(ETurretState state, CStateManager& mgr);
  void StartTurret(TUniqueId turret, CStateManager& mgr);
  void EjectFromTurret(TUniqueId turret, CStateManager& mgr);
  CVector3f GetEyePosition() const;
  CVector3f GetBallPosition() const;
  float GetEyeHeight() const;
  bool CheckSubmerged() const;

  void SkipMorphTransition();
  void StopSounds();
  void fn_8000bbb4(CStateManager& mgr);
  void fn_8000bd5c(CStateManager& mgr, bool createNew);
  float fn_8000bf1c() const;
  float GetDarkAetherDamage() const;
  CColor GetDarkAetherDamageColor(const CStateManager& mgr, int view) const;
  void fn_8000c124(float dt, CStateManager& mgr);
  void fn_8000ce94(float dt, CStateManager& mgr);
  short GetSoundPan(EMultiPlayerSoundPan channel) const;
  int fn_8000d0ac(const CStateManager& mgr, int channel) const;
  CTransform4f GetTurretTransform(const CStateManager& mgr) const;
  void fn_8000d540(const CFinalInput& input, CStateManager& mgr);
  void fn_8000d5dc(const CFinalInput& input, CStateManager& mgr);
  void ExitTurret(CStateManager& mgr);
  float GetAttachedActorStruggle() const;
  bool StartSamusVoiceSfx(ushort sfx, short volume, int priority);
  void ApplySubmergedPitchBend(CSfxHandle& handle);
  void fn_8000e85c(float dt);
  bool IsPlayerDeadEnough(const CStateManager& mgr) const;
  void fn_8000eba0();
  uint fn_8000f6e8() const;
  void TakeDamage(bool significant, const CVector3f& location, float damage, TUniqueId source,
                  TUniqueId owner, const CWeaponMode& weapon, CStateManager& mgr);
  bool GetExplorationMode() const;
  bool GetCombatMode() const;
  void fn_80010bf4(CStateManager& mgr);
  void RenderGun(const CStateManager& mgr, const CVector3f& position) const;
  void fn_80012040(CStateManager& mgr);
  void RenderReflectedPlayer(CStateManager& mgr);
  float fn_80012e14() const;
  void fn_80012eb8(CStateManager& mgr);
  void fn_80015d64(float dt, const CFinalInput& input);
  void fn_800165ec(CStateManager& mgr);
  void fn_8001660c(CStateManager& mgr);
  float fn_80016ce4(float dt, const CFinalInput& input, CStateManager& mgr);
  void fn_80017358(float dt);
  void UpdatePlayerDrawFlags(CStateManager& mgr);
  bool fn_80019e20(const CStateManager& mgr) const;

  void EndGravityBoost(CStateManager& mgr);
  void ApplyGravityBoost(float dt, CStateManager& mgr);
  void StartGravityBoost(CStateManager& mgr);
  bool IsGravityBoostActive() const;
  void UpdateMorphBallTransition(float dt, CStateManager& mgr);
  void UpdateTransitionFilter(float dt, CStateManager& mgr);
  void ActivateMorphBallCamera(CStateManager& mgr);
  void fn_80184294(EPlayerMorphBallState state);
  void fn_801842c8(float dt, CStateManager& mgr, EPlayerMorphBallState state);
  bool fn_801843d0(CStateManager& mgr, EPlayerMorphBallState state);
  void fn_80184a60(float dt, CStateManager& mgr, EPlayerMorphBallState state);
  void fn_80184ba4(CStateManager& mgr);
  void TransitionFromMorphBallState(float dt, CStateManager& mgr);
  void TransitionToMorphBallState(float dt, CStateManager& mgr);
  bool fn_801858cc(float dt, CStateManager& mgr);
  void fn_80185a88(float dt, CStateManager& mgr);
  float UpdateCameraBob(float dt, CStateManager& mgr);
  void SetEyeZBias(float bias);
  float GetUnbiasedEyeHeight() const;
  void UpdateSubmerged(const CStateManager& mgr);
  void BombJump(const CVector3f& position, CStateManager& mgr);
  CTransform4f CreateTransformFromMovementDirection() const;
  const CCollidableSphere* GetCollidableSphere() const;
  float GetActualBallMaxVelocity(float dt) const;
  float GetActualFirstPersonMaxVelocity(float dt) const;
  float GetBallMaxVelocity() const;
  void CalculateLeaveMorphBallDirection(const CFinalInput& input);
  void CalculatePlayerMovementDirection(float dt, const CVector3f& displacement);
  void SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr);
  float JumpInput(const CFinalInput& input, CStateManager& mgr);
  float TurnInput(const CFinalInput& input) const;
  float StrafeInput(const CFinalInput& input) const;
  float ForwardInput(const CFinalInput& input, float turnInput) const;
  void ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr);
  void fn_801892a0(float dt, CStateManager& mgr);
  void FinishSidewaysDash();
  bool SidewaysDashAllowed(float strafeInput, float forwardInput, const CFinalInput& input) const;
  void UpdateStepCameraZBias(float dt, CStateManager& mgr);
  void UpdateBombJumpStuff();
  float GetGravity() const;
  float GetAcceleration() const;
  float GetAverageSpeed() const;
  CVector3f GetDampedClampedVelocityWR() const;
  void fn_8011c3c0();
  void UpdateGrappleArmTransform(const CVector3f& offset, CStateManager& mgr, float dt);
  void ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt);
  bool ValidateFPPosition(CVector3f position, CStateManager& mgr);
  void UpdateGrappleState(const CFinalInput& input, CStateManager& mgr);
  void ApplyGrappleJump(CStateManager& mgr);
  void BeginGrapple(CVector3f& direction, CStateManager& mgr);
  void BreakGrapple(EPlayerOrbitRequest request, CStateManager& mgr);
  void SetOrbitRequest(EPlayerOrbitRequest request, CStateManager& mgr);
  void SetOrbitRequestForTarget(TUniqueId target, EPlayerOrbitRequest request, CStateManager& mgr);
  void fn_8011eac4(EPlayerOrbitRequest request, CStateManager& mgr);
  bool InGrappleJumpCooldown() const;
  void PreventFallingCameraPitch();
  void OrbitCarcass(CStateManager& mgr);
  void OrbitPoint(EPlayerOrbitType type, CStateManager& mgr);
  float CalculateOrbitMinDistance(EPlayerOrbitType type) const;
  CVector3f GetHUDOrbitTargetPosition() const;
  void SetOrbitState(EPlayerOrbitState state, const CStateManager& mgr);
  void SetOrbitTargetId(TUniqueId target, const CStateManager& mgr);
  void UpdateOrbitPosition(float distance, const CStateManager& mgr);
  void UpdateOrbitZPosition();
  void UpdateOrbitFixedPosition();
  void SetOrbitPosition(float distance);
  void UpdateAimTarget(CStateManager& mgr);
  void UpdateAimTargetTimer(float dt);
  bool ValidateAimTargetId(TUniqueId target, CStateManager& mgr, float dt);
  bool ValidateObjectForMode(TUniqueId target, CStateManager& mgr) const;
  void UpdateAimCandidates(CStateManager& mgr);
  TUniqueId FindAimTargetId(CStateManager& mgr);
  TUniqueId CheckEnemyAgainstOrbitZone(TUniqueId target, EPlayerZoneInfo zone, EPlayerZoneType type,
                                       CStateManager& mgr);
  TUniqueId FindOrbitTargetId(CStateManager& mgr);
  void UpdateOrbitableObjects(CStateManager& mgr);
  TUniqueId FindBestOrbitableObject(const rstl::reserved_vector< TUniqueId, 64 >& objects,
                                    EPlayerZoneInfo zone, CStateManager& mgr);
  void FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& candidates,
                            rstl::reserved_vector< TUniqueId, 64 >& objects, EPlayerZoneInfo zone,
                            EPlayerZoneType type, CStateManager& mgr, bool offScreen);
  bool WithinOrbitScreenBox(const CVector3f& screenPosition, EPlayerZoneInfo zone,
                            EPlayerZoneType type) const;
  bool WithinOrbitScreenEllipse(const CVector3f& screenPosition, EPlayerZoneInfo zone) const;
  bool CheckOrbitDisableSourceList(const CStateManager& mgr);
  bool CheckOrbitDisableSourceList() const;
  void RemoveOrbitDisableSource(TUniqueId id);
  void AddOrbitDisableSource(CStateManager& mgr, TUniqueId id);
  void UpdateOrbitPreventionTimer(float dt);
  void UpdateOrbitModeTimer(float dt);
  void UpdateOrbitZone();
  void UpdateOrbitInput(const CFinalInput& input, float dt, CStateManager& mgr);
  void ActivateOrbitSource(CStateManager& mgr);
  void UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr);
  void UpdateOrbitOrientation(CStateManager& mgr);
  void UpdateOrbitTarget(CStateManager& mgr);
  float GetOrbitMaxLockDistance() const;
  float GetOrbitMaxTargetDistance() const;
  int ValidateOrbitTargetId(TUniqueId target, CStateManager& mgr) const;
  void StopRezbitState(CStateManager& mgr);
  void ResetRezbitState(CStateManager& mgr);
  void BeginRezbitRecovery();
  void UpdateRezbitState(float dt);
  void StartRezbitState(CStateManager& mgr, const CRezbitEffectOptions& options);
  void SetRezbitState(ERezbitState state);
  ERezbitState GetRezbitState() const;
  void UpdateRezbitRecoveryInput(const CFinalInput& input);
  bool fn_8022b7a8(const CFinalInput& input) const;
  bool fn_8022b7f4(const CFinalInput& input) const;
  bool JumpPressed(const CFinalInput& input) const;
  bool JumpHeld(const CFinalInput& input) const;
  bool fn_8022b974(const CFinalInput& input) const;
  bool FireBeamPressed(const CFinalInput& input) const;
  bool FireBeamHeld(const CFinalInput& input) const;
  bool SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr);
  void ResetPlayerHintState(CStateManager& mgr);
  void UpdatePlayerHints(CStateManager& mgr);
  void fn_8022c338(float dt, CStateManager& mgr);

  void LeaveMorphBallState(CStateManager& mgr);
  void EnterMorphBallState(CStateManager& mgr, EPlayerMorphBallState state);

  void fn_8000ba60(float dt, CStateManager& mgr);
  void GetDamageSfx(float damage, TUniqueId source, TUniqueId owner, EWeaponType weaponType,
                    const CStateManager& mgr, ushort& impactSfx, ushort& loopSfx, ushort& voiceSfx);
  void SetMinimalAccelerationTimer(float duration);
  void fn_80010f4c(const CStateManager& mgr);
  rstl::pair< bool, CColor > GetHackedEffectColor() const;
  void fn_80011fc0() const;
  void fn_80016a6c(float value);
  void fn_80016a74(float value);
  const CTransform4f& fn_80019360() const;
  void UpdateAimPrediction(const CTransform4f& transform, CStateManager& mgr);
  void* GetMaskTextureData() const;
  void* GetIndirectTextureData() const;
  void* GetReflectionTextureData() const;
  CVector3f fn_8011ca08() const;
  int ValidateCurrentOrbitTargetId(CStateManager& mgr);
  bool ValidateOrbitTargetIdAndPointer(TUniqueId target, const CStateManager& mgr) const;
  TUniqueId fn_8022af0c(CStateManager& mgr, uint controls, TUniqueId source, float duration,
                        int breakType);

private:
  NPlayer::EPlayerMovementState mMovementState;                  // 0x2d0
  rstl::vector< CToken > mBallTransitionsRes;                    // 0x2d4
  TUniqueId mAttachedActor;                                      // 0x2e4
  float mAttachedActorTime;                                      // 0x2e8
  CPlayerEnergyDrain mEnergyDrain;                               // 0x2ec
  float mStartingJumpTimeout;                                    // 0x300
  float mSjTimer;                                                // 0x304
  float mMinJumpTimeout;                                         // 0x308
  float mJumpCameraTimer;                                        // 0x30c
  int mJumpPresses;                                              // 0x310
  float mFallCameraTimer;                                        // 0x314
  float x318_;                                                   // 0x318
  bool mCancelCameraPitch;                                       // 0x31c
  float mTimeSinceJump;                                          // 0x320
  float x324_;                                                   // 0x324
  float x328_;                                                   // 0x328
  CVector3f x32c_;                                               // 0x32c
  CVector3f mLastSpaceJumpPosition;                              // 0x338
  ESurfaceRestraints mSurfaceRestraint;                          // 0x344
  rstl::reserved_vector< float, 6 > mAccelerationTable;          // 0x348
  uint mCurAcceleration;                                         // 0x364
  float mAccelerationChangeTimer;                                // 0x368
  CAABox mFpBounds;                                              // 0x36c
  float mBallTransHeight;                                        // 0x384
  EPlayerCameraState mCameraState;                               // 0x388
  EPlayerMorphBallState mMorphBallState;                         // 0x38c
  EPlayerMorphBallState mSpawnedMorphBallState;                  // 0x390
  bool mScrewAttackTransitionPending;                            // 0x394
  EPlayerMorphBallState mScrewAttackTransitionState;             // 0x398
  EPlayerMorphBallState x39c_;                                   // 0x39c
  float mFallingTime;                                            // 0x3a0
  EPlayerOrbitState mOrbitState;                                 // 0x3a4
  EPlayerOrbitType mOrbitType;                                   // 0x3a8
  int mOrbitRequest;                                             // 0x3ac
  TUniqueId mOrbitTargetId;                                      // 0x3b0
  CVector3f mOrbitPoint;                                         // 0x3b4
  CVector3f mOrbitVector;                                        // 0x3c0
  float mOrbitModeTimer;                                         // 0x3cc
  EPlayerZoneInfo mOrbitZoneMode;                                // 0x3d0
  EPlayerZoneType mOrbitZoneType;                                // 0x3d4
  int mOrbitScreenBoxType;                                       // 0x3d8
  TUniqueId mOrbitNextTargetId;                                  // 0x3dc
  float mOrbitPointDistance;                                     // 0x3e0
  rstl::reserved_vector< TUniqueId, 64 > mNearbyOrbitObjects;    // 0x3e4
  rstl::reserved_vector< TUniqueId, 64 > mOnScreenOrbitObjects;  // 0x468
  rstl::reserved_vector< TUniqueId, 64 > mOffScreenOrbitObjects; // 0x4ec
  bool mOrbitLockEstablished;                                    // 0x570
  float mOrbitPreventionTimer;                                   // 0x574
  bool mSidewaysDashing;                                         // 0x578
  float mStrafeInputAtDash;                                      // 0x57c
  float mDashTimer;                                              // 0x580
  float mDashButtonHoldTime;                                     // 0x584
  bool mDoneSidewaysDashing;                                     // 0x588
  uint mOrbitSource;                                             // 0x58c
  bool mOrbitingEnemy;                                           // 0x590
  bool x591_;                                                    // 0x591
  float x594_;                                                   // 0x594
  int mOrbitCandidateIndex;                                      // 0x598
  int mOrbitCandidateRefreshFrames;                              // 0x59c
  float x5a0_;                                                   // 0x5a0
  float x5a4_;                                                   // 0x5a4
  float mDashSpeedMultiplier;                                    // 0x5a8
  bool mNoStrafeDashBlend;                                       // 0x5ac
  float mDashDuration;                                           // 0x5b0
  float mStrafeDashBlendDuration;                                // 0x5b4
  EPlayerScanState mScanState;                                   // 0x5b8
  float mScanningTime;                                           // 0x5bc
  float mCurScanTime;                                            // 0x5c0
  TUniqueId mScanningObject;                                     // 0x5c4
  uint mScanningObjectId;                                        // 0x5c8
  EGrappleState mGrappleState;                                   // 0x5cc
  float mGrappleSwingTimer;                                      // 0x5d0
  CVector3f mGrappleSwingAxis;                                   // 0x5d4
  float x5e0_;                                                   // 0x5e0
  float x5e4_;                                                   // 0x5e4
  float x5e8_;                                                   // 0x5e8
  float mGrappleJumpTimeout;                                     // 0x5ec
  CSegId mGrappleLocator;                                        // 0x5f0
  bool mInFreeLook;                                              // 0x5f1
  bool mLookButtonHeld;                                          // 0x5f2
  bool mLookAnalogHeld;                                          // 0x5f3
  bool x5f4_;                                                    // 0x5f4
  bool x5f5_;                                                    // 0x5f5
  float mCurFreeLookCenteredTime;                                // 0x5f8
  float mFreeLookYawAngle;                                       // 0x5fc
  float mHorizFreeLookAngleVel;                                  // 0x600
  float mFreeLookPitchAngle;                                     // 0x604
  float mVertFreeLookAngleVel;                                   // 0x608
  TUniqueId mAimTarget;                                          // 0x60c
  CVector3f mTargetAimPosition;                                  // 0x610
  TReservedAverage< CVector3f, 10 > mAimTargetAverage;           // 0x61c
  CVector3f mAssistedTargetAim;                                  // 0x698
  float mAimTargetTimer;                                         // 0x6a4
  rstl::reserved_vector< TUniqueId, 1024 > mAimCandidates;
  int mAimCandidateIndex;
  int mAimCandidateRefreshFrames;
  float mAimTargetDistance;
  float mAimTargetScreenDistance;
  rstl::single_ptr< CPlayerGun > mGun; // 0xebc
  float mGunAlpha;                     // 0xec0
  CModelFlags mPlayerDrawFlags;
  int xed0_; // 0xed0
  CPlayerTargeting* mTargeting;
  CPlayerBodyController* mBodyController;
  CPlayerKnockBackMgr mKnockBackManager;
  int xf6c_; // 0xf6c
  CPlayerStuckTracker* mPlayerStuckTracker;
  TReservedAverage< float, 20 > mMoveSpeedAvg; // 0xf74
  float mMoveSpeed;                            // 0xfc8
  float mFlatMoveSpeed;                        // 0xfcc
  CVector3f mLookDir;                          // 0xfd0
  CVector3f mMoveDir;
  CVector3f xfe8_;
  CVector3f mLastPosForDirCalc;
  CVector3f mGunDir;
  float mTimeMoving;
  CVector3f mControlDir;
  CVector3f mControlDirFlat;
  CDamageVulnerability mVariaSuitVulnerability;
  CDamageVulnerability mDarkSuitVulnerability;
  CDamageVulnerability mLightSuitVulnerability;
  CDamageVulnerability mImmuneVulnerability;
  CDamageVulnerability mScrewAttackVulnerability;
  bool mWasDamaged : 1;
  bool mWasDamagedPrev : 1;
  float mDamageAmount;
  float mPrevDamageAmount;
  CVector3f mDamageLocation;
  uint x1130_;
  float mImmuneTimer;
  float mMorphTime;
  float mMorphDuration;
  float mAlpha;
  bool x1144_24_ : 1;
  bool x1144_25_ : 1;
  float mStaticTimer;
  float mStaticOutSpeed;
  float mStaticInSpeed;
  float mVisorStaticAlpha;
  float mFrozenTimeout;
  int mIceBreakJumps;
  float mIceBreakJumpTimeout;
  ERezbitState mRezbitState;
  uint mRezbitEffectToken;
  TUniqueId mRezbitEffectId;
  float mRezbitRecoveryTimer;
  CMorphBall* mMorphBall; // 0x1174
  CPlayerCameraBob* mCameraBob;
  CSfxHandle mLandingSfx;
  float mLandingSfxTimer;
  CSfxHandle x1184_;
  CSfxHandle x1188_;
  int x118c_;
  float x1190_;
  float x1194_;
  ushort mSamusVoicePriority;
  CSfxHandle mSamusVoiceSfx;
  float mDamageSfxTimer;
  ushort mDamageLoopSfxId;
  float mFootstepSfxTimer;
  int mFootstepSfx;
  CVector3f mLastVelocity;
  CVisorSteam mVisorSteam;
  float x11e4_;
  float x11e8_;
  CPlayerState::EPlayerSuit mTransitionSuit;
  CAnimRes mAnimRes;
  CPlayerState::EBeamId mTransitionBeam;
  rstl::single_ptr< CModelData > mBallTransitionBeamModel;
  CTransform4f mGunWorldXf;
  float mTransitionFilterTimer;
  float mDistanceUnderWater;
  TUniqueId mRidingPlatform;
  float mGravityBoostDuration;
  CSfxHandle mGravityBoostSfx;
  CSfxHandle x1258_;
  bool mGravityBoostActive;
  CColor mGravityBoostColor;
  CHintManager* mPlayerHintManager;
  bool x1268_24_ : 1;
  bool mDrawCrosshairs : 1;
  bool x1268_26_ : 1;
  bool x1268_27_ : 1;
  bool x1268_28_ : 1;
  bool x1268_29_ : 1;
  bool x1268_30_ : 1;
  bool x1268_31_ : 1;
  bool x1269_24_ : 1;
  bool mHitWallDuringMove : 1;
  bool x1269_26_ : 1;
  bool x1269_27_ : 1;
  bool x1269_28_ : 1;
  bool mInterpolatingControlDir : 1;
  bool x1269_30_ : 1;
  bool x1269_31_ : 1;
  uchar x126a_;
  bool x126b_24_ : 1;
  bool x126b_25_ : 1;
  bool x126b_26_ : 1;
  bool x126b_27_ : 1;
  bool x126b_28_ : 1;
  bool mDeathFadeEnabled : 1;
  bool mUseAlternateBeam : 1;
  bool x126b_31_ : 1;
  bool x126c_24_ : 1;
  float mDeathFadeDuration;
  float mDeathFadeDelay;
  float mEyeZBias;
  float mStepCameraZBias;
  int mBombJumpCount;
  int mBombJumpCheckDelayFrames;
  CVector3f mControlDirOverride;
  rstl::reserved_vector< TUniqueId, 5 > mOrbitDisableSources;
  float mDeathTime;
  float mControlDirInterpTime;
  float mControlDirInterpDuration;
  TUniqueId mDeathPowerBomb;
  float mPreThinkDt;
  CAssetId mSteamTextureId;
  CAssetId mIceTextureId;
  CSfxHandle mFreezeSfx;
  int mSustainedDamageCount;
  float mSustainedDamageTime;
  float x12cc_;
  float x12d0_;
  float x12d4_;
  float mAttachedActorStruggle;
  int x12dc_;
  float x12e0_;
  float mDamageColorTimer;
  uint x12e8_;
  uint x12ec_;
  float x12f0_;
  float mInvulnerabilityTimer;
  ETurretState mTurretState;
  TUniqueId mTurretId;
  uint x1300_;
  float mTurretTimer;
  short mPlayerSoundPan[5];
  CPlayerState* mPlayerState;     // 0x1314
  CCameraManager* mCameraManager; // 0x1318
  SFrozenResources* mFrozenResources;
  int mControlScheme;
  float x1324_;
  int x1328_;
  int x132c_;
  CSfxHandle mDarkAetherDamageSfx;
  CSfxHandle mSafeZoneHealSfx;
  int mCharacterIndex;
  CVector3f x133c_;
  CVector3f x1348_;
  CVector2i mScreenPosition;
  float mSafeZoneHealSfxTimer;
  float mDarkAetherDamage;
  float mDarkAetherDamageFlashTime;
  rstl::auto_ptr< rstl::pair< TToken< CGenDescription >, TToken< CGenDescription > > >
      mDarkAetherParticleDescriptions;
  rstl::single_ptr< CElementGen > mDarkAetherParticles;
  rstl::single_ptr< CElementGen > mDarkAetherThirdPersonParticles;
  rstl::auto_ptr< rstl::pair< TToken< CGenDescription >, TToken< CGenDescription > > >
      mUnderwaterParticleDescriptions;
  rstl::single_ptr< CElementGen > mUnderwaterParticles;
  rstl::single_ptr< CElementGen > mUnderwaterThirdPersonParticles;
  CSegId mGunParticleLocator;
  uchar x1389_[3];
  rstl::vector< CToken > mBeamEffectTokens;
  rstl::vector< rstl::pair< CToken, CToken > > mBeamParticleDescriptions;
  rstl::single_ptr< CElementGen > mBeamParticles;
  CPlayerState::EBeamId mParticleBeam;
  rstl::single_ptr< CElementGen > mBeamAuxParticles;
  int mPlayerIndex;
  void* mReflectionTextureData;
  void* mIndirectTextureData;
  void* mMaskTextureData;
  uint mRezbitRecoveryDirection;
  uint mRezbitRecoveryInputCount;
  CControlMapper mControlMapper;
  CHintManager* mControlHintManager;
  TUniqueId x14bc_;
  float x14c0_;
  TUniqueId x14c4_;
  bool x14c6_;
};
CHECK_SIZEOF(CPlayer, 0x14c8)
typedef char CPlayerVisorSteamSizeCheck[check_sizeof< CPlayer::CVisorSteam, 0x28 >::value];

#endif // _CPLAYER
