#ifndef _CELITEPIRATE
#define _CELITEPIRATE

#include "types.h"

#include "Collision/CCollidableAABox.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/SPositionHistory.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActor;
class CElitePirateGrenadeLauncher;
class CGenericFSM2;
class CPFArea;
class CSkinnedModel;

// Guessed class: the tuned values of an Elite Pirate, assembled by the script loader.
class CElitePirateData {
public:
  CElitePirateData(
      CAssetId stateMachine, int initialAnimation, const CDamageInfo& meleeDamage,
      float maxMeleeRange, float minShockwaveRange, float maxShockwaveRange, float minRocketRange,
      float maxRocketRange, float lightShieldLimit, float darkShieldLimit, float tauntInterval,
      CAssetId darkShield, ushort darkShieldSound, CAssetId darkShieldPop, CAssetId lightShield,
      ushort lightShieldSound, CAssetId lightShieldPop, float tauntVariance, float shockwaveWeight0,
      float shockwaveWeight1, float shockwaveWeight2, float shockwaveWeight5,
      float repeatedAttackChance, float energyAttractionForce, CAssetId energyAbsorbEffect,
      ushort energyAbsorbSound, const CActorParameters& launcherActorParams,
      const CAnimationParameters& launcherAnimation,
      const CAnimationParameters& ingLauncherAnimation, CAssetId rocket,
      const CDamageInfo& rocketDamage, int minRockets, int maxRockets, CAssetId visorElectricEffect,
      ushort sound_VisorElectric, const SLdrShockWaveInfo& singleShockWave,
      const SLdrShockWaveInfo& doubleShockWave, CAssetId shieldedModel, CAssetId shieldedSkinRules)
  : mStateMachine(stateMachine)
  , mInitialAnimation(initialAnimation)
  , mMeleeDamage(meleeDamage)
  , mTauntInterval(tauntInterval)
  , mTauntVariance(tauntVariance)
  , mRepeatedAttackChance(repeatedAttackChance)
  , mEnergyAttractionForce(energyAttractionForce)
  , mEnergyAbsorbEffect(energyAbsorbEffect)
  , mEnergyAbsorbSound(energyAbsorbSound)
  , mLauncherActorParams(launcherActorParams)
  , mLauncherAnimation(launcherAnimation)
  , mIngLauncherAnimation(ingLauncherAnimation)
  , mMinRockets(minRockets)
  , mMaxRockets(maxRockets)
  , mVisorElectricEffect(visorElectricEffect)
  , mSound_VisorElectric(sound_VisorElectric)
  , mLightShieldLimit(lightShieldLimit)
  , mDarkShieldLimit(darkShieldLimit)
  , mDarkShield(darkShield)
  , mDarkShieldSound(darkShieldSound)
  , mDarkShieldPop(darkShieldPop)
  , mLightShield(lightShield)
  , mLightShieldSound(lightShieldSound)
  , mLightShieldPop(lightShieldPop)
  , mSingleShockWave(singleShockWave)
  , mDoubleShockWave(doubleShockWave)
  , mShockwaveWeight0(shockwaveWeight0)
  , mShockwaveWeight1(shockwaveWeight1)
  , mShockwaveWeight2(shockwaveWeight2)
  , mShockwaveWeight5(shockwaveWeight5)
  , mMaxMeleeRange(maxMeleeRange)
  , mMinShockwaveRange(minShockwaveRange)
  , mMaxShockwaveRange(maxShockwaveRange)
  , mMinRocketRange(minRocketRange)
  , mMaxRocketRange(maxRocketRange)
  , mRocket(rocket)
  , mRocketDamage(rocketDamage)
  , mShieldedModel(shieldedModel)
  , mShieldedSkinRules(shieldedSkinRules) {}

  CAssetId mStateMachine;
  int mInitialAnimation;
  CDamageInfo mMeleeDamage;
  float mTauntInterval;
  float mTauntVariance;
  float mRepeatedAttackChance;
  float mEnergyAttractionForce;
  CAssetId mEnergyAbsorbEffect;
  ushort mEnergyAbsorbSound;
  CActorParameters mLauncherActorParams;
  CAnimationParameters mLauncherAnimation;
  CAnimationParameters mIngLauncherAnimation;
  int mMinRockets;
  int mMaxRockets;
  CAssetId mVisorElectricEffect;
  ushort mSound_VisorElectric;
  float mLightShieldLimit; // Guessed name; damage the light shield absorbs before it breaks
  float mDarkShieldLimit;  // Guessed name; damage the dark shield absorbs before it breaks
  CAssetId mDarkShield;
  ushort mDarkShieldSound;
  CAssetId mDarkShieldPop;
  CAssetId mLightShield;
  ushort mLightShieldSound;
  CAssetId mLightShieldPop;
  SLdrShockWaveInfo mSingleShockWave;
  SLdrShockWaveInfo mDoubleShockWave;
  float mShockwaveWeight0; // Guessed name; weights of the shockwave severities the pirate picks
  float mShockwaveWeight1;
  float mShockwaveWeight2;
  float mShockwaveWeight5;
  float mMaxMeleeRange;
  float mMinShockwaveRange;
  float mMaxShockwaveRange;
  float mMinRocketRange;
  float mMaxRocketRange;
  CAssetId mRocket;
  CDamageInfo mRocketDamage;
  CAssetId mShieldedModel;
  CAssetId mShieldedSkinRules;
};
CHECK_SIZEOF(CElitePirateData, 0x190)

// Guessed class: the Elite Pirate, a shielded heavy that stomps shockwaves, fires rockets from a
// shoulder launcher and absorbs energy.
class CElitePirate : public CPatterned {
public:
  // Guessed names; value recorded in the attack history by the attacking states.
  enum EAction {
    kEA_Invalid = -1,
    kEA_None = 0,
    kEA_Alert = 1,
    kEA_Projectile = 2,
    kEA_FollowAttackPattern = 3,
    kEA_Melee = 4,
    kEA_PowerDown = 5,
    kEA_PowerUp = 6,
    kEA_Shockwave = 7,
    kEA_SpreadShot = 8,
  };

  // Guessed name; selects the shield colour and the weapons it absorbs.
  enum EShieldType {
    kST_Dark = 0,
    kST_Light = 1,
    kST_None = 2,
  };

  // Guessed names; how the launcher rockets are aimed.
  enum ERocketMode {
    kRM_Homing = 0,
    kRM_Spread = 1,
  };

  struct SJointInfo {
    const char* mFrom;
    const char* mTo;
    float mRadius;
    float mSeparation;
  };

  struct SSphereJointInfo {
    const char* mName;
    float mRadius;
  };

  // Guessed name; the energy shield worn by the pirate.
  struct SShield {
    SShield()
    : x0_(0)
    , mShieldUpTime(-1000.f)
    , mDamageAbsorbed(0.f)
    , mLastHitTime(-1000.f)
    , mFlash(0.f)
    , mType(kST_None)
    , mLightPopEffect()
    , mDarkPopEffect()
    , mEffect()
    , mSfx()
    , mModel()
    , mPopPending(false) {}
    int x0_;
    float mShieldUpTime;
    float mDamageAbsorbed;
    float mLastHitTime;
    float mFlash;
    int mType;
    rstl::optional_object< TLockedToken< CGenDescription > > mLightPopEffect;
    rstl::optional_object< TLockedToken< CGenDescription > > mDarkPopEffect;
    rstl::auto_ptr< CElementGen > mEffect;
    CSfxHandle mSfx;
    rstl::optional_object< TLockedToken< CSkinnedModel > > mModel;
    bool mActive : 1;
    bool mPopPending : 1;
  };

  CElitePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& modelData,
               const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
               const CElitePirateData& data);

  // CEntity
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mCollisionAabb; }

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  bool CanBeUnPossessed(CStateManager& mgr) const override;
  void RenderIngSnatchingTransition(const CStateManager& mgr) const override;

  virtual void SetupStateMachineHelper(CStateManager& mgr);

  // Triggers
  virtual bool Alerted(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AngryAttackOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BreakProjectileAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanShockwave(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ClearLineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DonePursuing(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DoneTurning(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InDetectionRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InPosition(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool NotReachedTarget(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PickedSpreadShot(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerInNoAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PoweredDown(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PoweredUp(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReturnedToPatrol(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReadyToCharge(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShieldKilled(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShockwaveIsNext(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShotAt(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldAlert(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldTurn(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldMeleeAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldShockwave(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SpotPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool StillAngry(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TargetNotOnMesh(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TargetUnreachable(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TooClose(CStateManager& mgr, const CTriggerData& data) const;

  // States
  virtual void Alert(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void AngryAttackBegin(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void AngryAttackEnd(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void InvulnAlert(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void PowerDown(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void PowerUp(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pursue(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Shielding(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void ShieldUp(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Shockwave(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void SpreadShot(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Stunned(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Turn(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Wait(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  virtual void SelectTarget(CStateManager& mgr, float dt);
  virtual void PickAttackType(CStateManager& mgr, float dt);

  virtual void SetupHealthInfo(CStateManager& mgr, bool shielded);         // Guessed name
  virtual void ActivateGrenadeLauncher(CStateManager& mgr, bool activate); // Guessed name
  virtual CShockWaveInfo GetShockWaveInfo() const;                         // Guessed name

private:
  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd); // Guessed name

  void UnmarkPathRegion(CStateManager& mgr);                             // Guessed name
  void MarkPathRegion(CStateManager& mgr);                               // Guessed name
  CPFArea* GetPathArea(CStateManager& mgr) const;                        // Guessed name
  int FindHintType(CStateManager& mgr) const;                            // Guessed name
  bool IsInStopPursuitHint(CStateManager& mgr) const;                    // Guessed name
  bool IsPlayerOnPath(CStateManager& mgr) const;                         // Guessed name
  void SetShotAt(bool shotAt);                                           // Guessed name
  void PushPlayer(CStateManager& mgr, float scale, float verticalSpeed); // Guessed name
  void ProcessStompGround(CStateManager& mgr);                           // Guessed name
  void UpdateAttackTimeLeft(CStateManager& mgr);                         // Guessed name
  void UpdatePathDestination(CStateManager& mgr, const CVector3f& position,
                             float dt); // Guessed name
  void PursuePosition(CStateManager& mgr, EStateMsg msg, const CVector3f& position,
                      float dt);                            // Guessed name
  void UpdateBreadCrumbTrail();                             // Guessed name
  CVector3f GetGrenadeLaunchPos(const CActor& actor) const; // Guessed name
  void UpdateGrenadeLauncher(CStateManager& mgr, TUniqueId& uid,
                             const rstl::string& locator) const; // Guessed name
  void ActivateGrenadeLauncherById(CStateManager& mgr, bool activate,
                                   TUniqueId uid) const;         // Guessed name
  void DeleteGrenadeLauncher(CStateManager& mgr);                // Guessed name
  void CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid); // Guessed name
  void UpdateAttackTimer(float dt);                              // Guessed name
  bool IsShieldActive() const;                                   // Guessed name
  void SetupPathFindSearch();                                    // Guessed name
  void ExtendTouchBounds(CStateManager& mgr, const rstl::reserved_vector< TUniqueId, 7 >& ids,
                         const CVector3f& bounds) const; // Guessed name
  bool IsArmClawCollider(TUniqueId uid,
                         const rstl::reserved_vector< TUniqueId, 7 >& ids) const; // Guessed name
  bool IsArmClawCollider(const rstl::string& name, const char* locator, const SJointInfo* joints,
                         int count) const; // Guessed name
  void AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& list); // Guessed name
  void AddCollisionList(const SJointInfo* joints, int count,
                        rstl::vector< CJointCollisionDescription >& list); // Guessed name
  void SetupCollisionActorInfo(CStateManager& mgr);                        // Guessed name
  void SetupCollisionManager(CStateManager& mgr);                          // Guessed name
  void PopShield(CStateManager& mgr);                                      // Guessed name
  const char* GetShieldModelName() const;                                  // Guessed name
  void SetAttackState(EAction state, EStateMsg msg);                       // Guessed name
  static void PushAttackHistory(rstl::reserved_vector< EAction, 3 >& history,
                                EAction attack);                       // Guessed name
  bool CanFireRocket(CStateManager& mgr, TUniqueId uid) const;         // Guessed name
  void MeleeDamagePlayer(CStateManager& mgr, TUniqueId uid);           // Guessed name
  void CreateShockWave(CStateManager& mgr, const CInt32POINode& node); // Guessed name
  void FireRocket(CStateManager& mgr);                                 // Guessed name
  void UpdateEnergyAbsorb(CStateManager& mgr, EStateMsg msg);          // Guessed name
  void RenderShieldFlash() const;                                      // Guessed name
  void UpdateAttractorBlend(float dt);                                 // Guessed name
  void UpdateShieldEffect(float dt, CStateManager& mgr);               // Guessed name
  void UpdateFlash(float dt);                                          // Guessed name
  void AvoidWall(float dt, CStateManager& mgr);                        // Guessed name
  CGenericFSM2* GetFsm();                                              // Guessed name
  bool HasLocomotionAnimation(int locomotionType) const;               // Guessed name

  // 0x7c0
  int x7c0_;
  CDamageVulnerability mVulnerability;
  rstl::single_ptr< CCollisionActorManager > mShieldCollisionMgr;
  CVector3f mLeftClawPos;
  CVector3f mRightClawPos;
  CVector3f mPathDestination;
  CVector3f mPowerUpPos;
  CElitePirateData mData;
  rstl::single_ptr< CCollisionActorManager > mBodyCollisionMgr;
  CCollidableAABox mCollisionAabb;
  rstl::optional_object< TLockedToken< CGenDescription > > mEnergyAbsorbDesc;
  TUniqueId mHeadId;
  TUniqueId mLauncherId;
  rstl::reserved_vector< TUniqueId, 7 > mRightClawIds;
  rstl::reserved_vector< TUniqueId, 7 > mLeftClawIds;
  TUniqueId mEnergyAttractorId;
  TUniqueId mTargetId;
  float mInitialSpeed;
  float mSteeringSpeed;
  float mHp;
  float mAttackTimer;
  float mShotAtTimer;
  float mElapsedTime;
  float mUnreachableTime;
  float mPursueStartTime;
  float mWallBackoffTime;
  int mMarkedRegionIndex;
  CPathFindSearch mPathFindSearch;
  CVector3f mTargetDestPos;
  SPositionHistory mPositionHistory;
  bool mDamageOn : 1;
  bool mShotAt : 1;
  bool mAlert : 1;
  bool mUnused4 : 1;
  bool mAlertPending : 1;
  bool mReturnedToPatrol : 1;
  bool mLauncherAlive : 1;
  bool mAlertDone : 1;
  rstl::reserved_vector< EAction, 3 > mAttackHistory; // Guessed name
  int xc24_;
  EAction mLastActionFallback; // Guessed name
  EAction mAttackState;
  rstl::optional_object< CToken > mFsm;
  pas::ETauntType mAlertTauntType;             // Guessed name
  pas::ELocomotionType mPoweredLocomotionType; // Guessed name
  bool mPoweredUp : 1;
  pas::ESeverity mMeleeSeverity;     // Guessed name
  pas::ESeverity mNextMeleeSeverity; // Guessed name
  float mLastMeleeTime;
  bool mAttackingRightClaw : 1;
  bool mAttackingLeftClaw : 1;
  SShield mShield;
  pas::ESeverity mShockwaveSeverity;     // Guessed name
  pas::ESeverity mLastShockwaveSeverity; // Guessed name
  CVector3f mTurnDirection;
  CVector3f mPlayerTargetPos;
  CProjectileInfo mProjectileInfo;
  int mRocketsFired;
  int mRocketCount;
  ERocketMode mAttackType;
  ERocketMode mDefaultAttackType;
  bool mBreakProjectileAttack;
  float mAttractorBlend;
  mutable CVector3f mAttractorPos;
  pas::ETauntType mTauntType; // Guessed name
  int mAngryCount;
  bool mShockwaveIsNext : 1;
  bool mAngryAttackOver : 1;
  bool mPrevShockwave : 1;
  bool mHintHandled : 1;
  float mAbsorbFlash;
  bool mAbsorbingEnergy;
};
CHECK_SIZEOF(CElitePirate, 0xd30)

#endif // _CELITEPIRATE
