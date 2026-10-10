#ifndef _CSWAMPBOSSSTAGE1
#define _CSWAMPBOSSSTAGE1

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage1.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActor;
class CCollisionInfoList;
class CGenericFSM2;
class CInt32POINode;
class CJointCollisionDescription;
class CScannableObjectInfo;
class CShockWaveInfo;

// Guessed class name; the Swamp Boss (Chykka) first stage, fought in the water. The constructor
// allocates 0xDB8 bytes and calls the CPatterned constructor with kPAI_SwampBossStage1.
class CSwampBossStage1 : public CPatterned {
public:
  // Guessed names; the collision actor categories and per-joint flags.
  enum ECollisionJointType {
    kCJT_Normal = 0,
    kCJT_WeakSpot = 1,
    kCJT_Tongue = 2,
    kCJT_Any = 3,
  };

  enum ECollisionJointFlags {
    kCJF_Passthrough = 1,
    kCJF_UnusedMarker = 2, // Guessed name; set on Spine_3 only and never read
    kCJF_NoSortingBounds = 4,
  };

  // Guessed names; the tongue extension states.
  enum ETongueState {
    kTS_Retracted = 0,
    kTS_Extending = 1,
    kTS_Hit = 2,
    kTS_Missed = 3,
  };

  // Guessed names; the values stored in the current state member.
  enum EBossState {
    kBS_None = -1,
    kBS_Beach = 0,
    kBS_Bob = 1,
    kBS_Dive = 2,
    kBS_Exposed = 3,
    kBS_PlatformReady = 4,
    kBS_TonguePull = 5,
    kBS_SlideIntoWater = 6,
    kBS_Splash = 7,
    kBS_SplashTelegraph = 8,
    kBS_TongueLoop = 9,
    kBS_Vomit = 10,
  };

  struct SCollisionJoint {
    const char* name;
    float radius;
    float altRadius;
    int type;
    uchar flags;
  };

  CSwampBossStage1(const TUniqueId& uid, const rstl::string& name, CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                   CAssetId stateMachine2,
                   const SLdrSwampBossStage1Data& swampBossStage1Properties);

  // CEntity
  ~CSwampBossStage1() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void OnScanStateChange(EScanState state, CStateManager& mgr) override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void TakeDamage(const CVector3f& direction, float magnitude) override;
  bool IsListening() const override { return true; }

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;

  // Code functions
  void ShreddersOff(CStateManager& mgr, float dt);      // Guessed name
  void ShreddersOn(CStateManager& mgr, float dt);       // Guessed name
  void IncrementMisses(CStateManager& mgr, float dt);   // Guessed name
  void SetSplashRotation(CStateManager& mgr, float dt); // Guessed name

  // Triggers
  bool TooMuchPulling(CStateManager& mgr, const CTriggerData& data) const;
  bool Wait(CStateManager& mgr, const CTriggerData& data) const;
  bool SplashAttackOver(CStateManager& mgr, const CTriggerData& data) const;
  bool EnoughSplashing(CStateManager& mgr, const CTriggerData& data) const;
  bool InVomitRange(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBeach(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSplash(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldExposeBelly(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBob(CStateManager& mgr, const CTriggerData& data) const;
  bool BeachAttackOver(CStateManager& mgr, const CTriggerData& data) const;
  bool MinHealth(CStateManager& mgr, const CTriggerData& data) const;
  bool LightHit(CStateManager& mgr, const CTriggerData& data) const;
  bool HeavyHit(CStateManager& mgr, const CTriggerData& data) const;
  bool GivingUp(CStateManager& mgr, const CTriggerData& data) const;
  bool TongueMissed(CStateManager& mgr, const CTriggerData& data) const;
  bool TongueHit(CStateManager& mgr, const CTriggerData& data) const;
  bool ReadyToTongue(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void TongueLoop(CStateManager& mgr, EStateMsg msg, float dt);
  void LightHitReact(CStateManager& mgr, EStateMsg msg, float dt);
  void HeavyHitReact(CStateManager& mgr, EStateMsg msg, float dt);
  void ExposeBelly(CStateManager& mgr, EStateMsg msg, float dt);
  void ResetPlatform(CStateManager& mgr, EStateMsg msg, float dt);
  void Vomit(CStateManager& mgr, EStateMsg msg, float dt);
  void TonguePull(CStateManager& mgr, EStateMsg msg, float dt);
  void PlatformReady(CStateManager& mgr, EStateMsg msg, float dt);
  void Beach(CStateManager& mgr, EStateMsg msg, float dt);
  void StopBeaching(CStateManager& mgr, EStateMsg msg, float dt);
  void SplashWait(CStateManager& mgr, EStateMsg msg, float dt);
  void SplashTelegraph(CStateManager& mgr, EStateMsg msg, float dt);
  void BeachBubbles(CStateManager& mgr, EStateMsg msg, float dt);
  void BeachWait(CStateManager& mgr, EStateMsg msg, float dt);
  void Null(CStateManager& mgr, EStateMsg msg, float dt);
  void Dive(CStateManager& mgr, EStateMsg msg, float dt);
  void SlideIntoWater(CStateManager& mgr, EStateMsg msg, float dt);
  void BeachRise(CStateManager& mgr, EStateMsg msg, float dt);
  void SplashRise(CStateManager& mgr, EStateMsg msg, float dt);
  void SplashJump(CStateManager& mgr, EStateMsg msg, float dt);
  void SetSplashOver(CStateManager& mgr, EStateMsg msg, float dt);
  void SplashDescend(CStateManager& mgr, EStateMsg msg, float dt);
  void Bob(CStateManager& mgr, EStateMsg msg, float dt);
  void Swim(CStateManager& mgr, EStateMsg msg, float dt);

private:
  // Guessed names; the attack kinds selected from the phase tables.
  enum EAttack {
    kAttack_Splash = 0,
    kAttack_Beach = 1,
    kAttack_Restart = 2,
  };

  // Guessed names; the collision actor update flags used by SetCollisionActorState.
  enum ECollisionFlags {
    kCF_AddTargetMaterials = 1,
    kCF_RemoveTargetMaterials = 2,
    kCF_SetVulnerability = 4,
    kCF_SetReflect = 8,
    kCF_AddRadarObject = 16,
    kCF_RemoveRadarObject = 32,
  };

  // Guessed name; the splash attack (the boss leaps out of the water and lands with a shockwave).
  struct SSplashAttack {
    SSplashAttack(const SLdrShockWaveInfo& shockWave, CAssetId telegraphEffect);
    ~SSplashAttack();

    SLdrShockWaveInfo mShockWave;
    rstl::auto_ptr< CParticleGen > mTelegraphEffect;
    int mSplashCount;
    int mTargetSplashCount;
    float mRotation;
    CAssetId mTelegraphEffectId;
    bool mSplashOver;
    TUniqueId mShockWaveId;
    CSfxHandle mShockWaveSfx;
    CVector3f mShockWavePosition;
    int mStartPhase;
  };

  // Guessed name; the tongue and its two models.
  struct STongue {
    STongue(CAssetId segmentModel, CAssetId tipModel);
    ~STongue();
    void Reset();

    CVector3f mRootPosition;
    CVector3f mTargetPosition;
    CVector3f mTipPosition;
    float mExtension;
    int mState;
    float mPullTime;
    float mAmplitudeScale;
    float mWaveAmplitude;
    rstl::optional_object< CModelData > mSegmentModel;
    rstl::optional_object< CModelData > mTipModel;
    bool mOrbitHidden : 1;
    bool mCollided : 1;
    float mRetractDelay;
    CSfxHandle mRetractSfx;
    CSfxHandle mPullSfx;
  };

  // Guessed name; the blend of the additive aim animation that tracks the player.
  struct SAdditive {
    SAdditive() : mBlend(0.f), mStart(CVector3f::Zero()), mStartPending(true), mTimer(100.f) {}

    float mBlend;
    CVector3f mStart;
    bool mStartPending;
    float mTimer;
  };

  // Guessed name; the belly weak spot exposed after a splash/beach attack.
  struct SWeakSpot {
    explicit SWeakSpot(const CDamageVulnerability& vulnerability);

    float mStartHealth;
    float mDamageTaken;
    bool mHeavyHit;
    CDamageVulnerability mVulnerability;
  };

  // Guessed name; the dark water ring effect that follows the boss.
  struct SWaterRing {
    explicit SWaterRing(CAssetId effect);
    ~SWaterRing();

    rstl::auto_ptr< CElementGen > mEffect;
    TUniqueId mWaterId;
    bool mEnabled : 1;
    CVector3f mDirection;
    float mSpeed;
  };

  // Guessed name; the loaded state machine asset.
  struct SFsm2 {
    explicit SFsm2(CAssetId stateMachine);

    rstl::optional_object< CToken > mToken;
  };

  // Guessed name; the particle effect played on the visor when the player is hit by the barf.
  struct SBarf {
    explicit SBarf(CAssetId effect);
    ~SBarf();

    rstl::optional_object< TToken< CGenDescription > > mEffect;
    int mCount;
  };

  // Guessed name; the spit projectile and its visor effect.
  struct SSpit {
    SSpit(CAssetId projectile, const CDamageInfo& damage, CAssetId visorEffect);

    CProjectileInfo mProjectile;
    rstl::optional_object< TLockedToken< CGenDescription > > mVisorEffect;
  };

  const CGenericFSM2* GetStateMachine2() const;                                    // Guessed name
  void SetupStateMachineHelper(CStateManager& mgr);                                // Guessed name
  bool IsPlayerInsideTongueHint(CStateManager& mgr) const;                         // Guessed name
  void ReleasePlayer(CStateManager& mgr);                                          // Guessed name
  void GrabPlayer(CStateManager& mgr);                                             // Guessed name
  void LaunchPlayer(CStateManager& mgr, const CVector3f& direction);               // Guessed name
  EScriptObjectState GetReleaseState(int index) const;                             // Guessed name
  EScriptObjectState GetGrabState(int index) const;                                // Guessed name
  void RotateToIndex();                                                            // Guessed name
  void ChooseTargetIndex(CStateManager& mgr);                                      // Guessed name
  void AddAdditiveAnimation(CStateManager& mgr, int index, float weight);          // Guessed name
  void UpdateTongueAdditive(CStateManager& mgr, float dt);                         // Guessed name
  CVector3f GetAdditiveAimPoint(CStateManager& mgr, float dt);                     // Guessed name
  void UpdateHitReaction(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void HideOrbit(CStateManager& mgr);                                              // Guessed name
  void ShowOrbit(CStateManager& mgr);                                              // Guessed name
  void PullPlayer(CStateManager& mgr, const CVector3f& position, float dt);        // Guessed name
  void PullPlayerMorphed(CStateManager& mgr, const CVector3f& position, float dt); // Guessed name
  void SendMessageToAttachedActors(CStateManager& mgr, EScriptObjectMessage msg);  // Guessed name
  void BeginTonguePull(CStateManager& mgr);                                        // Guessed name
  void DecrementAttachedActors(CStateManager& mgr);                                // Guessed name
  void BarfVisorGoo(CStateManager& mgr);                                           // Guessed name
  void ClearPlayerOrbit(CStateManager& mgr);                                       // Guessed name
  void SelectOrbitTarget(CStateManager& mgr);                                      // Guessed name
  void StartTelegraph(CStateManager& mgr, const CVector3f& position);              // Guessed name
  void RumblePlayer(CStateManager& mgr);                                           // Guessed name
  int GetCurrentAttack() const;                                                    // Guessed name
  const SLdrSwampBossStage1Struct* GetCurrentPhase() const;                        // Guessed name
  void UpdateState(EBossState state, EStateMsg msg);                               // Guessed name
  void ChooseNextAttack(CStateManager& mgr);                                       // Guessed name
  void RandomizeSwimVariant(CStateManager& mgr);                                   // Guessed name
  void StopTongueSounds();                                                         // Guessed name
  void RetractTongue();                                                            // Guessed name
  void ExtendTongue(CStateManager& mgr);                                           // Guessed name
  void RenderTongue(const CStateManager& mgr) const;                               // Guessed name
  void UpdateTongueRetract(CStateManager& mgr, float dt);                          // Guessed name
  void UpdateTongueExtend(CStateManager& mgr, float dt);                           // Guessed name
  void FadeOutAdditive(float dt);                                                  // Guessed name
  void FadeInAdditive(float dt);                                                   // Guessed name
  void UpdateTongueTip(CStateManager& mgr, bool checkHit, float amplitude);        // Guessed name
  void CheckTongueHit(CStateManager& mgr, const CVector3f& tip);                   // Guessed name
  CVector3f GetPlayerAimPoint(const CStateManager& mgr) const;                     // Guessed name
  float GetPlayerAngle(const CStateManager& mgr) const;                            // Guessed name
  void SpawnSplashShockWave(CStateManager& mgr);                                   // Guessed name
  CShockWaveInfo BuildShockWaveInfo() const;                                       // Guessed name
  void LaunchSpit(CStateManager& mgr);                                             // Guessed name
  void SetCollisionRadii(CStateManager& mgr, int radiusSet);                       // Guessed name
  void SetupCollisionManager(CStateManager& mgr);                                  // Guessed name
  void BuildCollisionDescriptions(const SCollisionJoint* joints, int count,
                                  rstl::vector< CJointCollisionDescription >& out); // Guessed name
  float GetWaterSurfaceHeight(const CStateManager& mgr) const;                      // Guessed name
  void UpdateEffects(CStateManager& mgr, float dt);                                 // Guessed name
  void UpdateShockWaveSfx(CStateManager& mgr);                                      // Guessed name
  void AlertNearbyActors(CStateManager& mgr);                                       // Guessed name
  void SetCollisionActorState(CStateManager& mgr, int type, int flags);             // Guessed name
  bool HasRadarCollisionActor(CStateManager& mgr) const;                            // Guessed name
  bool DoesTypeMatch(int jointType, int type) const;                                // Guessed name
  bool IsCollisionActorOfType(const CStateManager& mgr, const CCollisionActor* actor,
                              int type) const;       // Guessed name
  bool IsDamageableState() const;                    // Guessed name
  pas::ELocomotionType GetSwimLocomotion() const;    // Guessed name
  pas::ELocomotionType GetSurfaceLocomotion() const; // Guessed name

  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd);

  // Guessed member names; recovered from the constructor stores and their consumers.
  SLdrSwampBossStage1Data mProperties;
  rstl::reserved_vector< EBossState, 4 > mPreviousStates;
  int mUnusedState; // Guessed name; only written (-1) by the constructor, never read
  EBossState mState;
  float mElapsedTime;
  float mWaterTime;
  CVector3f mRootPosition;
  CVector3f mHeadPosition;
  CSfxHandle mTelegraphSfx;
  float mPainDamage;
  CAABox mSortingBounds;
  bool mScanned : 1;
  bool mTargetable : 1;
  TUniqueId mLastTouchedProjectile;
  int mShredderState;
  CVector3f mAlertPoint;
  int mAnimationVariant;
  SFsm2 mStateMachine2;
  float mNextBobTime;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  int mPhase;
  int mAttackIndex;
  float mNextAttackTime;
  SSplashAttack mSplashAttack;
  int mConnectedIndex;
  int mTargetIndex;
  float mPlatformReadyTime;
  int mMissCount;
  int mTongueAttempts;
  bool mBeachAttackOver : 1;
  int mBeachStartPhase;
  STongue mTongue;
  SAdditive mAdditive;
  SWeakSpot mWeakSpot;
  SBarf mBarf;
  float mSavedStepUpHeight;
  SWaterRing mWaterRing;
  float mWaitStartTime;
  SSpit mSpit;
};
CHECK_SIZEOF(CSwampBossStage1, 0xdb8)

#endif // _CSWAMPBOSSSTAGE1
