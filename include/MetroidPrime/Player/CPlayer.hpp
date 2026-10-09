#ifndef _CPLAYER
#define _CPLAYER

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameHint.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGunDrawBlockSet.hpp"
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
class CPlayerRagDoll;

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
  bool IsInSafeZone() const { return mInSafeZone; }
  enum ESurfaceRestraints {
    kSR_Normal,
    kSR_Air,
    kSR_Ice,
    kSR_Organic,
    kSR_Water,
    kSR_Phazon,
    kSR_Lava,
    kSR_Shrubbery,
  };
  enum EPlayerCameraState {
    kCS_FirstPerson,
    kCS_Ball,
    kCS_MorphBall,
    kCS_Transitioning,
    kCS_MorphBallTransition,
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
    kOR_Respawn, // Target-derived: teleport/respawn resets this player and other players orbit.
    kOR_EnterMorphBall = 2, // Target-derived: interrupts orbit on entering Morph Ball.
    kOR_Default = 3,
    kOR_InvalidateTarget = 6,
    kOR_BadVerticalAngle = 7,
    kOR_ActivateOrbitSource = 8, // Guessed name, correlated with Prime's orbit-break request.
    kOR_ProjectileCollide = 9,   // Guessed Prime name; projectile visor impact interrupts orbit.
    kOR_Freeze = 10,             // Target-derived: interrupts orbit when the player freezes.
    kOR_KnockBack = 11,          // Guessed name; knockback-driven orbit interruption.
    kOR_LostGrappleLineOfSight = 12,
    kOR_BoostBall = 13,   // Guessed name; requested when a boost charge releases.
    kOR_EnterTurret = 13, // Target-derived alias; interrupts other players' orbit on turret entry.
    kOR_TargetingThroughDoor = 14,
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
    SFrozenResources()
    : mSteamTexture(0x6fc03d46)
    , mIceTexture(0x2b757945)
    , mSinglePlayerFreezeSfx(0x1aeb)
    , mMultiplayerFreezeSfx(0x281b) {}

    CAssetId mSteamTexture;
    CAssetId mIceTexture;
    ushort mSinglePlayerFreezeSfx;
    ushort mMultiplayerFreezeSfx;
  };
  enum ETurretState { kTS_None, kTS_Entering, kTS_Exiting, kTS_Active, kTS_Four, kTS_Ejected };

  enum EBreakFrozenState { kBFS_Break, kBFS_BreakWithEffects, kBFS_Two };
  // Guessed state names; numeric values established by the Rezbit state handlers.
  enum ERezbitState { kRS_None, kRS_Infected, kRS_Recovering, kRS_Recovered };
  // Original Wii enum name; channel meanings remain unresolved on GameCube.
  enum EFootstepSfx { kFS_None, kFS_Left, kFS_Right };
  enum EMultiPlayerSoundPan { kMSP_0, kMSP_1, kMSP_2, kMSP_3, kMSP_4, kMSP_Player = 4 };

  CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
          CAssetId stateMachine, float mass, float stepUp, float stepDown, float ballRadius,
          const CMaterialList& ml, CPlayerState*, CCameraManager*, bool, int playerIndex, int,
          int charIdx);

  // CEntity
  ~CPlayer() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void ClearFluidList(CStateManager& mgr) override;
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
  const CVector2i& GetScreenPosition() const { return mScreenPosition; } // Guessed name
  const CSegId& GetGunParticleLocator() const { return mGunParticleLocator; }
  float GetFlatMoveSpeed() const { return mFlatMoveSpeed; }
  float GetMoveSpeed() const { return mMoveSpeed; }
  bool DampsBoostEntryVelocity() const { return mDampBoostEntryVelocity; }
  const CVector3f& GetLastSpaceJumpPosition() const { return mLastSpaceJumpPosition; }
  void SetLastSpaceJumpPosition(const CVector3f& pos) { mLastSpaceJumpPosition = pos; }
  const CVector3f& GetLookDir() const { return mLookDir; }
  const CVector3f& GetControlDirFlat() const { return mControlDirFlat; }
  bool GetSpiderBallControlXY() const { return mSpiderBallControlXY; }
  const CVector3f& GetMovementDirection() const { return mMoveDir; }
  const CVector3f& GetLeaveMorphDirection() const { return mLeaveMorphDir; }
  NPlayer::EPlayerMovementState GetPlayerMovementState() const { return mMovementState; }
  EGrappleState GetGrappleState() const { return mGrappleState; }
  EPlayerOrbitState GetOrbitState() const { return mOrbitState; }
  const CVector3f& GetOrbitPoint() const { return mOrbitPoint; }
  TUniqueId GetOrbitTargetId() const { return mOrbitTargetId; }
  EPlayerOrbitRequest GetOrbitRequest() const {
    return static_cast< EPlayerOrbitRequest >(mOrbitRequest);
  }
  TUniqueId GetScanningObject() const { return mScanningObject; }
  bool IsNewScanScanning() const { return mNewScanScanning; }
  TUniqueId GetOrbitNextTargetId() const { return mOrbitNextTargetId; }
  void SetOrbitNextTargetId(TUniqueId id) { mOrbitNextTargetId = id; } // Guessed name
  TUniqueId GetAimTarget() const { return mAimTarget; }
  CMorphBall* GetMorphBall() { return mMorphBall.get(); }
  const CMorphBall* GetMorphBall() const { return mMorphBall.get(); }
  CPlayerState* GetPlayerState() { return mPlayerState; }
  const CPlayerState* GetPlayerState() const { return mPlayerState; }
  float GetViewportScaleX() const { return mViewportScaleX; }
  float GetViewportScaleY() const { return mViewportScaleY; }
  CPlayerKnockBackMgr& GetKnockBackManager() { return mKnockBackManager; }
  const CPlayerTargeting* GetTargeting() const { return mTargeting.get(); }

  CPlayerBodyController* BodyController() { return mBodyController.get(); }

  const CPlayerRagDoll* GetPlayerRagDoll() const { return mRagDoll.get(); }

  rstl::single_ptr< CPlayerRagDoll >& PlayerRagDoll() { return mRagDoll; }

  EPlayerMorphBallState GetMorphballTransitionState() const { return mMorphBallState; }
  EPlayerMorphBallState GetSpawnedMorphballState() const { return mSpawnedMorphBallState; }
  int Get_x12f8() const { return mTurretState; }

  CSfxHandle PlaySfxForPlayer(uint sfxId, short param_4, TAreaId nextAreaId, bool, int);

  float fn_8000BE98() const { return GetDeathAlpha(); }
  void EmitMultiplayerBeamParticles(CStateManager& mgr);
  void ResetPlayerState(CStateManager&, int);
  void fn_8000d3ac(const CVector3f&, CStateManager&);
  bool fn_8000d40c(const CVector3f&, CStateManager&);
  CTweakPlayer* GetTweakPlayer() const;

  void Teleport(const CTransform4f& xf, CStateManager& mgr, bool resetBallCam);
  void SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  const CCameraManager* GetCameraManager() const { return mCameraManager; }
  CCameraManager* CameraManager() { return mCameraManager; }
  bool IsOutOfBallLookAtHintActor() const { return mOutOfBallLookAtHintActor; }
  bool IsOverrideRadarRadius() const { return mOverrideRadarRadius; }
  float GetRadarXYRadiusOverride() const { return mRadarXYRadiusOverride; }
  float GetRadarZRadiusOverride() const { return mRadarZRadiusOverride; }
  float GetEchoPulsePhase() const { return mEchoPulsePhase; }    // Guessed name
  uint GetEchoPulseCounter() const { return mEchoPulseCounter; } // Guessed name
  EPlayerCameraState GetCameraState() const { return mCameraState; }
  bool GetDoneSidewaysDashing() const { return mDoneSidewaysDashing; }
  bool GetSidewaysDashing() const { return mSidewaysDashing; } // Guessed name

  void Update(float dt, CStateManager& mgr);
  void PostUpdate(float dt, CStateManager& mgr);
  void DoThink(float dt, CStateManager& mgr);
  void DoPreThink(float dt, CStateManager& mgr);
  void ProcessInput(const CFinalInput& input, CStateManager& mgr);
  void UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateFreeLook(float dt);
  void ComputeFreeLook(const CFinalInput& input, CStateManager& mgr);
  void UpdateMorphBallState(const CFinalInput& input, float dt, CStateManager& mgr);
  void SetMorphBallState(EPlayerMorphBallState state, EPlayerMorphBallState spawnedState);
  void SetCameraState(EPlayerCameraState state, CStateManager& mgr);
  bool IsMorphBallTransitioning() const;
  float GetMorphBallTransitionFactor() const {
    return mMorphDuration == 0.f ? 0.f : CMath::Clamp(0.f, mMorphTime / mMorphDuration, 1.f);
  }
  bool CanEnterMorphBallState(CStateManager& mgr, float dt) const;
  bool CanLeaveMorphBallState(CStateManager& mgr, CVector3f& position) const;
  bool AttachActorToPlayer(TUniqueId actor, bool disableGun);
  void EnableLeaveMorphBall(bool enabled) { mCanStartUnmorphTransition = enabled; } // Prime name.
  void DetachActorFromPlayer();
  void UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt);
  bool ValidateScanning(const CFinalInput& input, CStateManager& mgr) const;
  void SetScanningState(EPlayerScanState state, CStateManager& mgr);
  EPlayerScanState GetPlayerScanState() const { return mScanState; }
  float GetScanningTime() const { return mScanningTime; }
  bool ObjectInScanningRange(TUniqueId id, const CStateManager& mgr);
  void FinishNewScan(CStateManager& mgr);
  void UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateVisorTransition(float dt, CStateManager& mgr);
  void UpdateCrosshairsState(const CFinalInput& input);
  bool GetDrawCrosshairs() const { return mDrawCrosshairs; }
  void UpdateFrozenState(const CFinalInput& input, CStateManager& mgr);
  bool GetFrozenState() const;
  void Freeze(float timeout, CStateManager& mgr, CAssetId steamTexture, uint sfx,
              CAssetId iceTexture);
  void BreakFrozenState(CStateManager& mgr, EBreakFrozenState state, bool recordEscape);
  void SetVisorSteam(float targetAlpha, float alphaInDuration, float alphaOutDuration,
                     CAssetId texture);
  const CVisorSteam& GetVisorSteam() const { return mVisorSteam; }
  float GetVisorSteamAlpha() const { return mVisorSteam.GetAlpha(); }
  float GetVisorStaticAlpha() const { return mVisorStaticAlpha; }
  static const float skDefaultHudFadeOutSpeed;
  // Guessed name. Hard landing sounds by material, indexed by multiplayer.
  static const ushort skPlayerLandSfxHard[2][26];
  static int SfxIdFromMaterial(const CMaterialList& mat, const ushort* idList, int tableLen,
                               ushort defId);
  static const float skDefaultHudFadeInSpeed;
  // Guessed names. Morph-transition scan-line filter timing, defined beside the HUD fade speeds.
  static const float skTransitionFilterStartTime;
  static const float skTransitionFilterFadeInTime;
  static const float skTransitionFilterFadeOutTime;
  static const float skTransitionFilterHoldTime;
  static const float skTransitionFilterEndTime;
  static const float skTransitionFilterMaxAlpha;
  void SetHudDisable(float staticTimer, float fadeOutSpeed = skDefaultHudFadeOutSpeed,
                     float fadeInSpeed = skDefaultHudFadeInSpeed);
  float GetStaticTimer() const { return mStaticTimer; }
  void SetNoDamageLoopSfx(bool noSfx) { mNoDamageLoopSfx = noSfx; }
  bool WasDamaged() const;
  float GetDamageAmount() const;
  float GetPrevDamageAmount() const;
  CVector3f GetDamageLocationWR() const;
  float GetDeathAlpha() const;
  float GetDeathTime() const { return mDeathTime; }

  // Guessed names; native knockback and death-effect consumers.
  void SetDeathFadeEnabled(bool enabled) { mDeathFadeEnabled = enabled; }

  void SetDeathFadeDuration(float duration) { mDeathFadeDuration = duration; }

  void SetDeathFadeDelay(float delay) { mDeathFadeDelay = delay; }

  void SetDeathEffectId(TUniqueId id) { mDeathEffectId = id; }

  void SetDeathRenderingSuppressed(bool suppressed) { mDeathRenderingSuppressed = suppressed; }

  bool IsEnergyLow() const;
  void PushSustainedDamage();
  void PopSustainedDamage();
  void SetControlDirectionInterpolation(float duration);
  void ResetControlDirectionInterpolation();
  void SetPlayerHitWallDuringMove();
  void SetPlayerIsSlidingOnWall(bool sliding) { mSlidingOnWall = sliding; }
  void SetAimTarget(TUniqueId target);
  void UpdateAssistedAiming(const CTransform4f& transform, CStateManager& mgr);
  void UpdateGunTransform(const CVector3f& position, CStateManager& mgr);
  void UpdateArmAndGunTransforms(float dt, CStateManager& mgr);
  void ForceGunOrientation(const CTransform4f& transform, CStateManager& mgr);
  void UpdateGunAlpha(const CStateManager& mgr);
  void UpdateDebugCamera(CStateManager& mgr);
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
  CControlMapper& GetControlMapper() { return mControlMapper; }
  const CControlMapper& GetControlMapper() const { return mControlMapper; }
  CPlayerGun* GetPlayerGun();
  CModelData* BallTransitionBeamModel() { return mBallTransitionBeamModel.get(); } // Guessed name
  const CPlayerGun* GetPlayerGun() const;
  // Guessed name: inline gun access used by REL code (the out-of-line accessors above are
  // DOL-only).
  const CPlayerGun* GetGun() const { return mGun.get(); }
  ETurretState GetTurretState() const { return mTurretState; }
  float GetTurretTimer() const { return mTurretTimer; }
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
  void RenderMultiplayerBeamParticles(const CStateManager& mgr) const; // Guessed name.
  void SetMultiplayerBeamAuxParticlesEnabled(CStateManager& mgr, bool createNew);
  float
  GetDarkWorldDamageExposureFraction() const; // Guessed name; normalized grace-period exposure.
  float GetDarkAetherDamage() const;
  CElementGen* GetUnderwaterParticles() const { return mUnderwaterParticles.get(); }
  CElementGen* GetDarkAetherParticles() const { return mDarkAetherParticles.get(); }
  CColor GetDarkAetherDamageColor(const CStateManager& mgr, int view) const;
  void UpdateUnderwaterParticles(float dt, CStateManager& mgr);
  void UpdateEchoVisorEffects(float dt, CStateManager& mgr);
  short GetSoundPan(EMultiPlayerSoundPan channel) const;
  int CalculateSoundPan(const CStateManager& mgr, int channel) const; // Guessed name.
  CTransform4f GetTurretTransform(CStateManager& mgr) const;
  void ProcessTurretActions(const CFinalInput& input, CStateManager& mgr); // Guessed name.
  void ProcessTurretInput(const CFinalInput& input, CStateManager& mgr);   // Guessed name.
  void ExitTurret(CStateManager& mgr);
  float GetAttachedActorStruggle() const;

  TUniqueId GetAttachedActorId() const { return mAttachedActor; }
  TUniqueId GetRidingPlatform() const { return mRidingPlatform; }
  const CPlayerEnergyDrain& GetEnergyDrain() const { return mEnergyDrain; } // Guessed name
  CPlayerEnergyDrain& GetEnergyDrain() { return mEnergyDrain; }
  const CVector3f& GetLastVelocity() const { return mLastVelocity; } // Guessed name
  bool IsInFreeLook() const { return mInFreeLook; }
  bool IsLookButtonHeld() const { return mLookButtonHeld; }
  bool GetFreeLookStickState() const { return mLookAnalogHeld; }
  bool IsLandingStrikePending() const { return mLandingStrikePending; }
  void SetLandingStrikePending(bool pending) { mLandingStrikePending = pending; }
  float GetFreeLookAngleX() const { return mFreeLookPitchAngle; }
  float GetFreeLookAngleZ() const { return mFreeLookYawAngle; }
  float GetJumpCameraTimer() const { return mJumpCameraTimer; }
  float GetTimeSinceJump() const {
    return mTimeSinceJump;
  } // Guessed from Prime and native contact handling.
  void SetTimeSinceJump(float time) { mTimeSinceJump = time; } // Guessed name.
  float GetFallCameraTimer() const { return mFallCameraTimer; }
  bool GetOrbitLockAcquired() const { return mOrbitLockEstablished; }
  CPlayerCameraBob* CameraBobObject() { return mCameraBob.get(); }
  const CPlayerCameraBob* CameraBobObject() const { return mCameraBob.get(); }
  bool GetSelectFluidBallSound() const { return mSelectFluidBallSound; }
  void SetSelectFluidBallSound(bool select) { mSelectFluidBallSound = select; }

  bool StartSamusVoiceSfx(ushort sfx, short volume, int priority);
  void ApplySubmergedPitchBend(CSfxHandle handle);
  void UpdateDamageTimers(float dt);
  bool IsPlayerDeadEnough(const CStateManager& mgr) const;
  void CollectBallTransitionAnimationTokens();
  uint GetDamageWeaponType() const; // Reconstructed name; retained damage-event weapon type.
  const CColor& GetScreenFilterColor() const { return mScreenFilterColor; }      // Guessed name.
  void SetScreenFilterColor(const CColor& color) { mScreenFilterColor = color; } // Guessed name.
  void SetHoldScreenFilterAlpha(bool hold) { mHoldScreenFilterAlpha = hold; }    // Guessed name.
  TUniqueId GetEnemyLockOnActorId() const { return mEnemyLockOnActorId; }        // Guessed name.
  char GetEnemyLockOnCount() const { return mEnemyLockOnCount; }                 // Guessed name.
  void TakeDamage(bool significant, const CVector3f& location, float damage, TUniqueId source,
                  TUniqueId owner, const CDamageInfo& damageInfo, CStateManager& mgr);
  bool GetExplorationMode() const;
  bool GetCombatMode() const;
  void UpdateTransitionAlpha(CStateManager& mgr); // Guessed name; morph-transition opacity.
  void RenderGun(const CStateManager& mgr, const CVector3f& position) const;
  void RenderReflectedPlayer(CStateManager& mgr);
  void UpdateModelScale(CStateManager& mgr); // Guessed name; adjusts the viewed player's scale.
  float GetMaximumPlayerPositiveVerticalVelocity(const CStateManager& mgr) const;
  void ResolveUnmorphCollision(CStateManager& mgr); // Guessed name.
  void UpdateCameraTimers(float dt, const CFinalInput& input);
  void UpdateCameraState(CStateManager& mgr);
  void UpdateCinematicState(CStateManager& mgr);
  void UpdateFootstepSounds(float dt, const CFinalInput& input, CStateManager& mgr);
  ushort GetMaterialSoundUnderPlayer(CStateManager& mgr, const ushort* table, int length,
                                     ushort defaultId);
  void UpdatePlayerSounds(float dt);
  void UpdatePlayerDrawFlags(CStateManager& mgr);
  bool fn_80019e20(const CStateManager& mgr) const;

  void EndGravityBoost(CStateManager& mgr);
  void ApplyGravityBoost(float dt, CStateManager& mgr);
  void StartGravityBoost(CStateManager& mgr);
  bool IsGravityBoostActive() const;
  void UpdateMorphBallTransition(float dt, CStateManager& mgr);
  void UpdateTransitionFilter(float dt, CStateManager& mgr);
  void ActivateMorphBallCamera(CStateManager& mgr);
  void RequestScrewAttackTransition(EPlayerMorphBallState state);
  void BeginUnmorphTransition(float dt, CStateManager& mgr, EPlayerMorphBallState state);
  bool PrepareToLeaveMorphBallState(float dt, CStateManager& mgr, EPlayerMorphBallState state);
  void BeginMorphTransition(float dt, CStateManager& mgr, EPlayerMorphBallState state);
  void PrepareToEnterMorphBallState(float dt, CStateManager& mgr);
  void SetOutOfBallReadyAnimation(float dt, CStateManager& mgr);
  void UpdatePlayerBodyController(float dt, CStateManager& mgr);
  const bool UpdatePlayerRagDoll(float dt, CStateManager& mgr);
  void SetIntoBallReadyAnimation(float dt, EPlayerMorphBallState state);
  float UpdateCameraBob(float dt, CStateManager& mgr);
  void SetEyeZBias(float bias);
  float GetUnbiasedEyeHeight() const;
  void UpdateSubmerged(const CStateManager& mgr);
  void BombJump(const CVector3f& position, CStateManager& mgr);
  int GetBombJumpCounter() const { return mBombJumpCount; }
  CTransform4f CreateTransformFromMovementDirection() const;
  const CCollidableSphere* GetCollidableSphere() const;
  float GetActualBallMaxVelocity(float dt) const;
  float GetActualFirstPersonMaxVelocity(float dt) const;
  float GetBallMaxVelocity() const;
  void CalculateLeaveMorphBallDirection(const CFinalInput& input);
  void CalculatePlayerMovementDirection(float dt, const CVector3f& displacement);
  void SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr);
  float JumpInput(const CFinalInput& input, CStateManager& mgr);
  float TurnInput(const CFinalInput& input, CStateManager& mgr) const;
  float StrafeInput(const CFinalInput& input) const;
  float ForwardInput(const CFinalInput& input, float turnInput) const;
  void ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr);
  CVector3f CalculateLeftStickEdgePosition(float strafeInput, float forwardInput) const;
  void BeginSidewaysDash(float strafeInput, CStateManager& mgr);
  void FinishSidewaysDash(CStateManager& mgr);
  bool SidewaysDashAllowed(float strafeInput, float forwardInput, const CFinalInput& input,
                           CStateManager& mgr) const;
  void UpdateStepCameraZBias(float dt, CStateManager& mgr);
  void UpdateBombJumpStuff();
  float GetGravity() const;
  float GetAcceleration() const;
  float GetAverageSpeed() const;
  CVector3f GetDampedClampedVelocityWR() const;
  void UpdateScreenSpaceMotion();
  void UpdateGrappleArmTransform(const CVector3f& offset, CStateManager& mgr, float dt);
  void ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt);
  bool ValidateFPPosition(CVector3f position, CStateManager& mgr);
  void UpdateGrappleState(const CFinalInput& input, CStateManager& mgr);
  void ApplyGrappleJump(CStateManager& mgr);
  void BeginGrapple(CVector3f& direction, CStateManager& mgr);
  void BreakGrapple(EPlayerOrbitRequest request, CStateManager& mgr);
  void SetOrbitRequest(EPlayerOrbitRequest request, CStateManager& mgr);
  void SetOrbitRequestForTarget(TUniqueId target, EPlayerOrbitRequest request, CStateManager& mgr);
  void SetOrbitRequestForOtherPlayers(EPlayerOrbitRequest request, CStateManager& mgr);
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
  bool ValidateAimTargetId(TUniqueId target, CStateManager& mgr);
  bool ValidateObjectForMode(TUniqueId target, CStateManager& mgr) const;
  void UpdateAimCandidates(CStateManager& mgr);
  TUniqueId FindAimTargetId(CStateManager& mgr);
  TUniqueId CheckEnemyAgainstOrbitZone(TUniqueId target, EPlayerZoneInfo zone, EPlayerZoneType type,
                                       CStateManager& mgr);
  TUniqueId FindOrbitTargetId(CStateManager& mgr);
  TUniqueId FindScanTargetId(const CStateManager& mgr) const; // Guessed name
  void UpdateOrbitableObjects(CStateManager& mgr);
  TUniqueId FindBestOrbitableObject(const rstl::reserved_vector< TUniqueId, 64 >& objects,
                                    EPlayerZoneInfo zone, CStateManager& mgr);
  void FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& candidates,
                            rstl::reserved_vector< TUniqueId, 64 >& objects, EPlayerZoneInfo zone,
                            EPlayerZoneType type, CStateManager& mgr, bool onScreenTest);
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
  float GetOrbitMaxLockDistance(CStateManager& mgr) const;
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
  void ResetRezbitRecoveryInput();
  const bool BoostHeld(const CFinalInput& input) const; // Guessed name.
  const bool ChargeBeamHeld(const CFinalInput& input) const;
  uchar JumpPressed(const CFinalInput& input) const;
  const bool JumpHeld(const CFinalInput& input) const;
  const bool AutoFireHeld(const CFinalInput& input) const;
  const bool FireBeamPressed(const CFinalInput& input) const;
  const bool FireBeamHeld(const CFinalInput& input) const;
  bool IsAligningGrappleSwingTurn() const { return mAligningGrappleSwingTurn; }
  void SetAligningGrappleSwingTurn(bool aligning) { mAligningGrappleSwingTurn = aligning; }
  const bool SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr);
  void ResetPlayerHintState(CStateManager& mgr);
  void CalculatePlayerControlDirection(CStateManager& mgr);
  void UpdatePlayerControlDirection(float dt, CStateManager& mgr);

  void LeaveMorphBallState(CStateManager& mgr);
  void EnterMorphBallState(CStateManager& mgr, EPlayerMorphBallState state);

  void UpdateWaterInhabitants(float dt, CStateManager& mgr);
  void GetDamageSfx(float damage, TUniqueId source, TUniqueId owner, EWeaponType weaponType,
                    const CStateManager& mgr, ushort& impactSfx, ushort& loopSfx, ushort& voiceSfx);
  void SetMinimalAccelerationTimer(float duration);
  void RenderThirdPersonGrappleBeam(const CStateManager& mgr) const; // Guessed name.
  rstl::pair< bool, CColor > GetHackedEffectColor() const;
  void RenderIceModel(const CModelFlags& flags) const;
  void SetViewportScaleY(float value); // Target-derived names; GUI scale consumers.
  void SetViewportScaleX(float value);
  const CTransform4f& GetFirstPersonCameraTransform() const;
  void UpdateAimPrediction(const CTransform4f& transform, CStateManager& mgr);
  void* GetDepthLowTextureData() const;
  void* GetDepthHighTextureData() const;
  void* GetScanTargetIdTextureData() const;
  CVector3f GetCameraForwardPoint() const;
  int ValidateCurrentOrbitTargetId(CStateManager& mgr);
  bool ValidateOrbitTargetIdAndPointer(TUniqueId target, const CStateManager& mgr) const;
  TUniqueId DisableControls(CStateManager& mgr, uint controls, TUniqueId source, float duration,
                            CGameHint::EBreakHintType breakType);

private:
  friend class CSamusHud;

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
  float mAirborneTimer;                                          // 0x318
  bool mCancelCameraPitch;                                       // 0x31c
  float mTimeSinceJump;                                          // 0x320
  float mTimeSinceDoubleJump;                                    // 0x324
  float mTimeSinceScrewAttackRequest;                            // 0x328
  CVector3f mLastJumpPosition;                                   // 0x32c
  CVector3f mLastSpaceJumpPosition;                              // 0x338
  ESurfaceRestraints mSurfaceRestraint;                          // 0x344
  rstl::reserved_vector< float, 6 > mAccelerationTable;          // 0x348
  int mCurAcceleration;                                          // 0x364
  float mAccelerationChangeTimer;                                // 0x368
  CAABox mFpBounds;                                              // 0x36c
  float mBallTransHeight;                                        // 0x384
  EPlayerCameraState mCameraState;                               // 0x388
  EPlayerMorphBallState mMorphBallState;                         // 0x38c
  EPlayerMorphBallState mSpawnedMorphBallState;                  // 0x390
  bool mScrewAttackTransitionPending;                            // 0x394
  EPlayerMorphBallState mScrewAttackTransitionState;             // 0x398
  EPlayerMorphBallState mCinematicMorphBallState;                // 0x39c
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
  bool mOrbitTargetLineOfSightClear;                             // 0x591
  float mOrbitOcclusionTimer;                                    // 0x594
  int mOrbitCandidateIndex;                                      // 0x598
  int mOrbitCandidateRefreshFrames;                              // 0x59c
  float mOrbitTargetDistance;                                    // 0x5a0
  float mOrbitTargetScreenDistance;                              // 0x5a4
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
  bool mFreeLookAnglesHeld;                                      // 0x5f4
  bool mFreeLookInputLatched;                                    // 0x5f5
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
  int mTransitionBeamShader;
  rstl::single_ptr< CPlayerTargeting > mTargeting;
  rstl::single_ptr< CPlayerBodyController > mBodyController;
  CPlayerKnockBackMgr mKnockBackManager;
  rstl::single_ptr< CPlayerRagDoll > mRagDoll;
  rstl::single_ptr< CPlayerStuckTracker > mPlayerStuckTracker;
  TReservedAverage< float, 20 > mMoveSpeedAvg; // 0xf74
  float mMoveSpeed;                            // 0xfc8
  float mFlatMoveSpeed;                        // 0xfcc
  CVector3f mLookDir;                          // 0xfd0
  CVector3f mMoveDir;
  CVector3f mLeaveMorphDir;
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
  uint mDamageWeaponType;
  float mImmuneTimer;
  float mMorphTime;
  float mMorphDuration;
  float mAlpha;
  bool mCanStartUnmorphTransition : 1; // Guessed name.
  bool mCanStartMorphTransition : 1;   // Guessed name.
  float mStaticTimer;
  float mStaticOutSpeed;
  float mStaticInSpeed;
  float mVisorStaticAlpha;
  float mFrozenTimeout;
  int mIceBreakJumps;
  float mFrozenDamage;
  ERezbitState mRezbitState;
  CGunDrawBlockSet mRezbitGunDrawBlocks;
  TUniqueId mRezbitControlHintId;
  float mRezbitVirusMemoTimer;
  rstl::single_ptr< CMorphBall > mMorphBall; // 0x1174
  rstl::single_ptr< CPlayerCameraBob > mCameraBob;
  CSfxHandle mDamageLoopSfx;
  float mSamusVoiceTimeout;
  CSfxHandle mDashSfx;
  CSfxHandle mSamusVoiceSfx;
  int mSamusVoicePriority;
  float mDamageSfxTimer;
  float mTimeSinceDamageImpactSfx;
  ushort mDamageLoopSfxId;
  CSfxHandle mDarkAetherDamageLoopSfx;
  float mDarkAetherDamageLoopSfxTimer;
  ushort mDarkAetherDamageLoopSfxId;
  float mFootstepSfxTimer;
  int mFootstepSfxSel;
  CVector3f mLastVelocity;
  CVisorSteam mVisorSteam;
  float mViewportScaleX; // Target-derived: combined with camera-filter width scales.
  float mViewportScaleY; // Target-derived: combined with camera-filter height scales.
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
  CSfxHandle mGravityBoostEndSfx;
  bool mGravityBoostUsed;
  CColor mScreenFilterColor;
  rstl::single_ptr< CHintManager > mPlayerHintManager;
  bool mVisorChangeRequested : 1;
  bool mDrawCrosshairs : 1;
  bool x1268_26_ : 1;
  bool mCanEnterMorphBall : 1;
  bool mCanLeaveMorphBall : 1;
  bool mSpiderBallControlXY : 1; // Guessed name (Prime)
  bool mControlDirectionOverridden : 1;
  bool mInSafeZone : 1;
  bool mSlidingOnWall : 1;
  bool mHitWallDuringMove : 1;
  bool mSelectFluidBallSound : 1;
  bool mStepCameraZBiasDirty : 1;
  bool mExtendTargetDistance : 1;
  bool mInterpolatingControlDir : 1;
  bool mOutOfBallLookAtHint : 1;
  bool mIgnoreDarkWorldDamage : 1;
  bool mNoSafeZoneHealing : 1;
  bool mNoMorphBallDamageTimer : 1;
  bool mAimingAtProjectile : 1;
  bool mAligningGrappleSwingTurn : 1;
  bool mNewScanScanning : 1;
  bool mOverrideRadarRadius : 1;
  bool mNoDamageLoopSfx : 1;
  bool mOutOfBallLookAtHintActor : 1;
  bool mModelDepthUpdateEnabled : 1; // Guessed name; controls model depth writes in PreRender.
  bool mHoldScreenFilterAlpha : 1;
  bool x126b_26_ : 1; // Fluid type 2; semantic name unresolved (see Dynamics research).
  bool mBeamParticleDescriptionsInitialized : 1;
  bool mDeathRenderingSuppressed : 1; // Guessed name; suppresses gun and actor rendering.
  bool mDeathFadeEnabled : 1;
  bool mUseAlternateBeam : 1;
  bool mLandingStrikePending : 1;   // Guessed name; hard-landing gun reaction pending.
  bool mDampBoostEntryVelocity : 1; // Guessed name: player hint flag 0x800000.
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
  TUniqueId mDeathEffectId; // Guessed name; death particle actor, including the Morph Ball gib.
  float mPreThinkDt;
  CAssetId mSteamTextureId;
  CAssetId mIceTextureId;
  CSfxHandle mFreezeSfx;
  int mSustainedDamageCount;
  float mSustainedDamageTime;
  float x12cc_;
  float mRadarXYRadiusOverride;
  float mRadarZRadiusOverride;
  float mAttachedActorStruggle;
  int mFramesSinceDamageSfx;
  float mSamusExhaustedVoiceTimer;
  float mDamageColorTimer;
  uint x12e8_;
  uint x12ec_;
  float x12f0_;
  float mInvulnerabilityTimer;
  ETurretState mTurretState;
  TUniqueId mTurretId;
  CGunDrawBlockSet mTurretGunDrawBlocks;
  float mTurretTimer;
  short mPlayerSoundPan[5];
  CPlayerState* mPlayerState;     // 0x1314
  CCameraManager* mCameraManager; // 0x1318
  rstl::single_ptr< SFrozenResources > mFrozenResources;
  int mControlScheme;
  float mEchoPulsePhase;  // Guessed name: normalized repeating Echo Visor pulse phase
  uint mEchoPulseCounter; // Guessed name: incremented whenever the echo pulse phase wraps.
  int mEchoVisorAuxEffectId;
  CSfxHandle mEchoPulseLeftSfx;
  CSfxHandle mEchoPulseRightSfx;
  int mCharacterIndex;
  CVector3f mPreviousCameraForwardPoint;
  CVector3f mPreviousEyePosition;
  CVector2i mScreenPosition;
  float mDarkWorldDamageExposureTime;
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
  // EFB copies read by FindScanTargetId (see CStateManager::CapturePlayerTextures): an 8-bit
  // alpha target id, then the 24-bit depth as a Z16 high word and a Z8L low byte. The retail
  // object emits single_ptr<void> destructors for these, so they stay untyped.
  rstl::single_ptr< void > mScanTargetIdTextureData;
  rstl::single_ptr< void > mDepthHighTextureData;
  rstl::single_ptr< void > mDepthLowTextureData;
  uint mRezbitRecoveryDirection;
  uint mRezbitRecoveryInputCount;
  CControlMapper mControlMapper;
  rstl::single_ptr< CHintManager > mControlHintManager;
  TUniqueId mPlayerHintControlHintId;
  float mBackwardInput; // Guessed name: cached unfiltered backward command.
  TUniqueId mEnemyLockOnActorId;
  char mEnemyLockOnCount;
};
CHECK_SIZEOF(CPlayer, 0x14c8)
NESTED_CHECK_SIZEOF(CPlayer, CVisorSteam, 0x28)

extern const bool gkAutoAim;
extern const bool gkAutoAimAtOrbitedObject;
extern const bool gkFreeLookPreventsOrbitMovement;
extern const bool gkWorldOnlyReflection;
extern const bool gkDisablePlayerTargeting; // Guessed name
extern const bool kBoostBallBreaksOrbit;    // Guessed name
extern const bool kDoubleJumpBreaksOrbit;
extern const bool kDashDoubleJumpBreaksOrbit;
extern const int gkMorphBallOrbitMode;

// Guessed names; Samus model locators shared by the player and its transition copies.
extern const char* const kGunLocator;
extern const char* const kGrappleLocator;

// Guessed names; size of the per-player scan target ID capture (8-bit IDs, 16-bit high depth,
// 8-bit low depth) copied from the viewport centre each frame.
extern const uint kScanTargetTextureWidth;
extern const uint kScanTargetTextureHeight;
extern const uint kScanTargetTextureSize;

#endif // _CPLAYER
