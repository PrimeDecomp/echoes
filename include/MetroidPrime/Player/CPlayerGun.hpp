#ifndef _CPLAYERGUN
#define _CPLAYERGUN

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/Player/CFidget.hpp"
#include "MetroidPrime/Player/CPlayerGunBase.hpp"
#include "MetroidPrime/TStateMachineState.hpp"
#include "MetroidPrime/Weapons/CGunWeapon.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

class CAuxWeapon;
class CGameCamera;
class CGrappleArm;
class CInt32POINode;

// Structure-first recovery; see research/CPlayerGun-G2ME01.md for evidence and open questions.
class CPlayerGun : public CPlayerGunBase {
public:
  enum EChargePhase {
    kCP_NotCharging,
    kCP_ChargeRequested,
    kCP_Charging,
    kCP_ChargeFx,
    kCP_Charged,
    kCP_ComboTransfer,
    kCP_ComboTransferDone,
    kCP_ComboRequested,
    kCP_ComboAnimating,
    kCP_ComboFired,
    kCP_ChargeDone
  };
  enum ESeekerChargeState {
    kSCS_NotCharging,
    kSCS_Requested,
    kSCS_Opening,
    kSCS_Charging,
    kSCS_FullyCharged,
    kSCS_Fire = 9,
    kSCS_Done = 10
  };
  enum EMissileState { kMS_Inactive, kMS_Ready, kMS_Shot, kMS_Reloading };
  enum EBeamChangeState { kBCS_Idle, kBCS_Close, kBCS_Morph, kBCS_Open };
  enum EBWeapon { kBW_Bomb, kBW_PowerBomb };
  enum EStateMessage { kSM_Enter, kSM_Update, kSM_Exit };

  class CGunMorph {
  public:
    enum EMorphDir { kMD_In, kMD_Out, kMD_Done };
    enum EGunState { kGS_InWipeDone, kGS_OutWipeDone, kGS_InWipe, kGS_OutWipe };
    enum EWipeEvent { kWE_None, kWE_OutWipeStarted, kWE_OutWipeFinished };
    CGunMorph(float transformTime, float holdTime);
    void StartWipe(EMorphDir direction);
    EWipeEvent Update(float inY, float outY, float dt, const CPlayer& player);

  private:
    float mYLerp;
    float mGunTransformTime;
    float mRemTime;
    float mSpeed;
    float mHoloHoldTime;
    float mRemHoldTime;
    float mTransitionFactor;
    EMorphDir mMorphDirection;
    EGunState mGunState;
    bool mMorphing : 1;
    bool mWeaponChanged : 1;
  };

  class CMotionState {
  public:
    enum EMotionState { kMS_Zero, kMS_One, kMS_LockOn, kMS_CancelLockOn };
    enum EFireState { kFS_NotFiring, kFS_StartFire, kFS_Firing };
    explicit CMotionState(float extendDistance);
    void Update(bool firing, float dt, CTransform4f& transform, CStateManager& mgr);

  private:
    float mExtendParabolaDelayTimer;
    float mFireTime;
    float mCurrentExtendDistance;
    float mCurrentRotation;
    float mRotationT;
    float mStartRotation;
    float mEndRotation;
    float mExtendDistance;
    EMotionState mMotionState;
    EFireState mFireState;
    bool mExtendParabola : 1;
  };

  CPlayerGun(TUniqueId playerId, int characterIndex);
  ~CPlayerGun() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void TouchModel(const CStateManager& mgr) const override;
  void PreRender(CStateManager& mgr, const CVector3f& cameraPosition) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr, const CVector3f& cameraTranslation,
              const CModelFlags& flags) const override;
  void DamageRumble(const CVector3f& position, float damage) override;
  void Reset(CStateManager& mgr) override;
  void Update(float dt, CStateManager& mgr) override;
  float GetBeamVelocity() const override;
  void SetAuxTargetId(TUniqueId target) override;
  TUniqueId GetAuxTargetId() const override;
  TUniqueId CreatePowerBomb(CStateManager& mgr) override;
  void AsyncLoadSuit(CStateManager& mgr) override;
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void PollStateMachine(CStateManager& mgr) override;
  void InitializeStateMachine(CStateManager& mgr) override;

  CVector3f fn_801c6df8() const; // Beam vector's meaning remains unresolved.
  int GetBombsAvailable(CStateManager& mgr) const;
  TUniqueId DropPowerBomb(CStateManager& mgr) const;
  void DropBomb(EBWeapon type, CStateManager& mgr);
  void FireBombs(CStateManager& mgr);
  void TakeDamage(bool bigStrike, bool strikeGrapple, CStateManager& mgr);
  TUniqueId GetTargetId(CStateManager& mgr);
  void PlayAnim(CStateManager& mgr, int animation, bool loop);
  void PlayBeamFireSfx(CStateManager& mgr, CPlayer& player, bool play); // Guessed name
  bool IsOutOfAmmoToShoot(CStateManager& mgr) const;
  bool GetBeamAmmoTypeAndCosts(bool combo, CStateManager& mgr, CPlayerState::EItemType& ammoA,
                               CPlayerState::EItemType& ammoB, int& cost) const;
  void ResetCharge(CStateManager& mgr, bool playAnimation);
  void StopChargeSound(CStateManager& mgr, bool start);
  void EnableChargeFx(CStateManager& mgr, bool enable);
  void UpdateNormalShotCycle(float dt, CStateManager& mgr);
  void UpdateChargeState(float dt, CStateManager& mgr);
  void ResetSeeker(CStateManager& mgr);
  void UpdateSeeker(float dt, CStateManager& mgr);
  void UpdateSeekerEffects(float dt);
  void EnableSeekerFx(CStateManager& mgr, bool enable);
  void FireSecondary(float dt, CStateManager& mgr, TUniqueId target, uint attributes,
                     const CTransform4f* transform, ushort sound);
  void UpdateAuxWeapons(const CTransform4f& transform, CStateManager& mgr);
  void StopContinuousBeam(CStateManager& mgr, bool deactivate);
  void DoUserAnimEvent(float dt, CStateManager& mgr, const CInt32POINode& node,
                       EUserEventType type);
  void DoUserAnimEvents(float dt, CStateManager& mgr);
  void SetGunLightActive(bool active, CStateManager& mgr);
  void UpdateGunLight(const CTransform4f& transform, CStateManager& mgr);
  void SetBeam(CPlayerState::EBeamId beam, CStateManager& mgr);
  void InitMuzzleData(CStateManager& mgr);
  void InitBombData();
  void InitBeamData();
  void ChangeWeapon(CStateManager& mgr);
  bool ProcessGunMorph(float dt, CStateManager& mgr);
  void UpdateBeamChange(float dt, CStateManager& mgr);
  void HandleBeamChange(const CFinalInput& input, CStateManager& mgr);
  void HandleWeaponChange(const CFinalInput& input, CStateManager& mgr);
  void EnterFreeLook(CStateManager& mgr);
  void ReturnArmAndGunToDefault(CStateManager& mgr, bool force);
  void ReturnToDefault(CStateManager& mgr, bool bigStrikeReset);
  void SetFidgetAnimBits(int animSet, bool holster);
  bool IsFidgetLoaded();
  void UnLoadFidget();
  void AsyncLoadFidget(CStateManager& mgr);
  void EnterFidget(CStateManager& mgr);
  void UpdateGunIdle(float dt, CStateManager& mgr);
  void UpdateGunMotion(float dt, CStateManager& mgr);
  void UpdateTimers(float dt);
  void UpdateFreeLook(float dt, CStateManager& mgr);
  void UpdateLeftArmTransform();
  CTransform4f GetLocatorTransform(const CModelData& model, const rstl::string& name,
                                   bool dynamic) const;
  void DrawArm(const CStateManager& mgr, const CVector3f& cameraTranslation,
               const CModelFlags& flags) const;
  void RenderGunWithHologram(const CStateManager& mgr, const CVector3f& cameraTranslation,
                             bool drawSuitArm, const CTransform4f& elbowTransform,
                             const CTransform4f& gunTransform, const CModelFlags& armFlags,
                             const CModelFlags& gunFlags) const;
  void RenderGun(const CStateManager& mgr, const CVector3f& cameraTranslation, bool drawSuitArm,
                 const CTransform4f& elbowTransform, const CTransform4f& gunTransform,
                 const CModelFlags& armFlags, const CModelFlags& gunFlags) const;
  CVector3f ConvertToScreenSpace(const CVector3f& position, const CGameCamera& camera) const;
  void BeginDarkVisorRender(const CStateManager& mgr) const; // Guessed name
  void EndDarkVisorRender(const CStateManager& mgr) const;   // Guessed name
  static void DrawScreenTex();
  static void CopyScreenTex();
  CStateMachine* GetStateMachine();
  void ResetStateMachine(CStateManager& mgr);

  // These names are present in the original state-machine registration tables.
  bool ShouldHolster(CStateManager& mgr, const float& argument);
  bool IsHolstered(CStateManager& mgr, const float& argument);
  bool IsNotHolstered(CStateManager& mgr, const float& argument);
  bool StartCharge(CStateManager& mgr, const float& argument);
  bool InitiateCombo(CStateManager& mgr, const float& argument);
  bool Discharge(CStateManager& mgr, const float& argument);
  bool TransitionToMorphball(CStateManager& mgr, const float& argument);
  bool TransitionToPlayer(CStateManager& mgr, const float& argument);
  bool AnimOver(CStateManager& mgr, const float& argument);
  bool ActivateMissile(CStateManager& mgr, const float& argument);
  bool CloseMissile(CStateManager& mgr, const float& argument);
  bool ChargeDone(CStateManager& mgr, const float& argument);
  bool ButtonRelease(CStateManager& mgr, const float& argument);
  bool ComboOver(CStateManager& mgr, const float& argument);
  bool InterruptEvent(CStateManager& mgr, const float& argument);
  bool GunLoaded(CStateManager& mgr, const float& argument);
  bool Scanning(CStateManager& mgr, const float& argument);
  bool InCinematic(CStateManager& mgr, const float& argument);
  bool StartFidget(CStateManager& mgr, const float& argument);
  bool FidgetOver(CStateManager& mgr, const float& argument);
  bool Grappling(CStateManager& mgr, const float& argument);
  bool IsAlive(CStateManager& mgr, const float& argument);
  bool InPhazon(CStateManager& mgr, const float& argument);
  void Start(CStateManager& mgr, int message, float dt);
  void Main(CStateManager& mgr, int message, float dt);
  void InMorphball(CStateManager& mgr, int message, float dt);
  void Charging(CStateManager& mgr, int message, float dt);
  void Recoil(CStateManager& mgr, int message, float dt);
  void ComboActive(CStateManager& mgr, int message, float dt);
  void Holstered(CStateManager& mgr, int message, float dt);
  void Fidgeting(CStateManager& mgr, int message, float dt);
  void MissileActive(CStateManager& mgr, int message, float dt);
  void MissileClosing(CStateManager& mgr, int message, float dt);
  void EventHandler(CStateManager& mgr, int message, float dt);

private:
  typedef void (CPlayerGun::*RenderFunc)(const CStateManager&, const CVector3f&, bool,
                                         const CTransform4f&, const CTransform4f&,
                                         const CModelFlags&, const CModelFlags&) const;
  CTransform4f mGunWorldXf;
  CTransform4f mBeamLocalXf;
  CTransform4f mElbowLocalXf;
  CTransform4f mElbowWorldXf;
  CVector3f mDamageLocation;
  TCachedToken< CStateMachine > mStateMachineToken;
  TStateMachineState< CPlayerGun > mStateMachine;
  CGunMorph mGunMorph;
  CMotionState mMotionState;
  CAABox mHologramClipCube;
  RenderFunc mRender;
  CFidget mFidget;
  rstl::single_ptr< CGunMotion > mGunMotion;
  rstl::single_ptr< CGrappleArm > mGrappleArm;
  rstl::single_ptr< CAuxWeapon > mAuxWeapon;
  rstl::single_ptr< CGunWeapon > mPowerBeam;
  rstl::single_ptr< CGunWeapon > mDarkBeam;
  rstl::single_ptr< CGunWeapon > mLightBeam;
  rstl::single_ptr< CGunWeapon > mAnnihilatorBeam;
  rstl::reserved_vector< CGunWeapon*, 4 > mSelectableBeams;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 4 > mAuxMuzzleEffects;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > mAuxMuzzleGenerators;
  rstl::reserved_vector< rstl::reserved_vector< TToken< CGenDescription >, 2 >, 2 > mBombEffects;
  CDependencyGroupToken mBombDependencies;
  CGunWeapon* mCurrentBeam;
  CGunWeapon* mOutgoingBeam;
  CGunWeapon* mLoadingBeam;
  rstl::auto_ptr< CElementGen > mComboTransferGenerator;
  rstl::auto_ptr< CElementGen > mHoloTransitionGenerator;
  float mMissileExitTimer;
  float mComboTransferFactor;
  float mBombReloadTimer;
  float mTimeSinceFire;
  float mRapidFireDecayTimer;
  float mShotSmokeTimer;
  float mMuzzleEffectVisTimer;
  float mEnterFreeLookDelayTimer;
  float mGunStrikeCooldownTimer;
  float mIdleWanderDelayTimer;
  float mDamageAmount;
  float mBigStrikeTimer;
  float mGunStrikeDelayTimer;
  EChargePhase mChargePhase;
  ESeekerChargeState mSeekerChargeState;
  float mSeekerChargeFactor;
  EMissileState mMissileState;
  rstl::vector< CToken > mCommonDependencies;
  CGunWeapon::ESecondaryFxType mSeekerSecondaryFx;
  rstl::reserved_vector< TCachedToken< CGenDescription >, 5 > mSeekerMuzzleEffects;
  rstl::optional_object< TCachedToken< CGenDescription > > mMissileAuxMuzzleEffect;
  rstl::optional_object< TCachedToken< CGenDescription > > mMissileSecondaryEffect;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 5 > mSeekerMuzzleGenerators;
  rstl::reserved_vector< float, 5 > mSeekerFadeRates;
  rstl::auto_ptr< CElementGen > mMissileAuxMuzzleGenerator;
  rstl::auto_ptr< CElementGen > mMissileSecondaryGenerator;
  float mMissileShotInterval;
  EBeamChangeState mBeamChangeState;
  CPlayerState::EBeamId mCurrentBeamId;
  CPlayerState::EBeamId mNextBeamId;
  int mSoundSetIndex;
  uint mFidgetAnimBits;
  int mAnimSfxPitch;
  int mBombCount;
  int mRapidFireShots;
  SamusGun::EAnimationState mGunMotionState;
  int mBeamLoadDelayFrames;
  rstl::pair< ushort, CSfxHandle > mAnimSfx;
  CSfxHandle mChargeSfx;
  CSfxHandle mInvalidSfx;
  short mChargeRumbleHandle;
  float mChargeRumbleTimer;
  TUniqueId mPowerBombId;
  uint x7b4_; // Initialized to zero; no semantic use established.
  int mMaxSeekerTargets;
  CPlayerState::EPlayerVisor mSeekerVisor;
  rstl::reserved_vector< rstl::pair< TUniqueId, float >, 5 > mSeekerTargets;
  TUniqueId mCurrentSeekerTarget;
  float mSeekerLockTimer;
  float mAllSeekersLockedTime;
  CColor mAmbientColor;
  int mAbsorbedPhazonShots;
  rstl::auto_ptr< CElementGen > mPhazonAbsorbFlashGenerator;
  rstl::auto_ptr< CElementGen > mPhazonChargeGenerator;
  bool mStateMachineInitialized : 1;
  bool mComboFiring : 1;
  bool mRequestReturnToDefault : 1;
  bool mInterruptEvent : 1;
  bool mFrozen : 1;
  bool mChargeEffectVisible : 1;
  bool mInFreeLook : 1;
  bool mGunMotionFidgeting : 1;
  bool mAnimPlaying : 1;
  bool mFiring : 1;
  bool mPointBlankWorldSurface : 1;
  bool mGunMotionReturningFromStrike : 1;
  bool mMissileAnimActive : 1;
  bool mMissileCloseAnimDone : 1;
  bool mCommonDependenciesLoaded : 1;
  bool mBeamLoadRequested : 1;
};
NESTED_CHECK_SIZEOF(CPlayerGun, CGunMorph, 0x28)
NESTED_CHECK_SIZEOF(CPlayerGun, CMotionState, 0x2c)
CHECK_SIZEOF(CPlayerGun, 0x814)

#endif // _CPLAYERGUN
