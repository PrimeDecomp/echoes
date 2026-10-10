#ifndef _CSPACEPIRATE
#define _CSPACEPIRATE

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CIkChain.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "REL/REL_Setup.h"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class REL_EXPORT CSpacePirate : public CPatterned {
public:
  // Guessed struct: the grenade launcher setup assembled by the script loader.
  enum EEquippedWeapon {
    kEW_None,
    kEW_GrenadeLauncher, // Guessed name
  };

  class CSpacePirateWeaponData {
  public:
    CSpacePirateWeaponData(int equippedWeapon, CAssetId grenadeLauncher,
                           const CBouncyGrenadeData& grenade, int grenadeCount,
                           float grenadeMinLaunchSpeed, float grenadeMaxLaunchSpeed,
                           float grenadeMinAttackDist, float grenadeMaxAttackDist)
    : mEquippedWeapon(equippedWeapon)
    , mGrenadeLauncher(grenadeLauncher)
    , mGrenade(grenade)
    , mGrenadeCount(grenadeCount)
    , mGrenadeMinLaunchSpeed(grenadeMinLaunchSpeed)
    , mGrenadeMaxLaunchSpeed(grenadeMaxLaunchSpeed)
    , mGrenadeMinAttackDist(grenadeMinAttackDist)
    , mGrenadeMaxAttackDist(grenadeMaxAttackDist) {}

    int mEquippedWeapon;
    CAssetId mGrenadeLauncher;
    CBouncyGrenadeData mGrenade;
    int mGrenadeCount; // Guessed name; the maximum number of grenades launched per attack.
    float mGrenadeMinLaunchSpeed;
    float mGrenadeMaxLaunchSpeed;
    float mGrenadeMinAttackDist;
    float mGrenadeMaxAttackDist;
  };

  // Guessed struct: the tuned values of a Space Pirate, assembled by the script loader.
  class CSpacePirateData {
  public:
    CSpacePirateData(float aggressionCheck, float coverCheck, float searchRadius,
                     float fallBackCheck, float fallBackRadius, float hearingRadius, uint flags,
                     bool x1c, CAssetId projectile, CDamageInfo projectileDamage,
                     ushort sound_Projectile, CDamageInfo bladeDamage, float kneelAttackChance,
                     CAssetId kneelAttackShot, CDamageInfo kneelAttackDamage, float dodgeCheck,
                     ushort sound_Impact, float intraBurstShotTime, float intraBurstShotVariation,
                     float ingAverageNextShotTime, float ingNextShotTimeVariation,
                     ushort sound_Alert, float gunTrackDelay, int firstBurstCount,
                     float cloakOpacity, float maxCloakOpacity, float breakDodgeMinTime,
                     float breakDodgeMaxTime, ushort sound_Hurled, ushort sound_Death, float xbc,
                     float avoidDistance, float minLosClearTime,
                     const CSpacePirateWeaponData& weaponData)
    : mAggressionCheck(aggressionCheck)
    , mCoverCheck(coverCheck)
    , mSearchRadius(searchRadius)
    , mFallBackCheck(fallBackCheck)
    , mFallBackRadius(fallBackRadius)
    , mHearingRadius(hearingRadius)
    , mFlags(flags)
    , x1c_(x1c)
    , mProjectile(projectile)
    , mProjectileDamage(projectileDamage)
    , mSound_Projectile(sound_Projectile)
    , mBladeDamage(bladeDamage)
    , mKneelAttackChance(kneelAttackChance)
    , mKneelAttackShot(kneelAttackShot)
    , mKneelAttackDamage(kneelAttackDamage)
    , mDodgeCheck(dodgeCheck)
    , mSound_Impact(sound_Impact)
    , mAverageNextShotTime(intraBurstShotTime)
    , mNextShotTimeVariation(intraBurstShotVariation)
    , mIngAverageNextShotTime(ingAverageNextShotTime)
    , mIngNextShotTimeVariation(ingNextShotTimeVariation)
    , mSound_Alert(sound_Alert)
    , mGunTrackDelay(gunTrackDelay)
    , mFirstBurstCount(firstBurstCount)
    , mCloakOpacity(cloakOpacity)
    , mMaxCloakOpacity(maxCloakOpacity)
    , mDodgeDelayTimeMin(breakDodgeMinTime)
    , mDodgeDelayTimeMax(breakDodgeMaxTime)
    , mSound_Hurled(sound_Hurled)
    , mSound_Death(sound_Death)
    , xbc_(xbc)
    , mAvoidDistance(avoidDistance)
    , mMinLosClearTime(minLosClearTime)
    , mWeaponData(weaponData) {}

    /*
     * 0x1: pendingAmbush
     * 0x2: ceilingAmbush
     * 0x4: nonAggressive
     * 0x8: melee
     * 0x10: noShuffleCloseCheck
     * 0x20: onlyAttackInRange
     * 0x40: unknown
     * 0x80: noKnockbackImpulseReset
     * 0x200: noMeleeAttack
     * 0x400: breakAttack
     * 0x1000: seated
     * 0x2000: shadowPirate
     * 0x4000: alertBeforeCloak
     * 0x8000: noBreakDodge
     * 0x10000: floatingCorpse
     * 0x20000: ragdollNoAiCollision
     * 0x40000: trooper
     */
    float mAggressionCheck;
    float mCoverCheck;
    float mSearchRadius;
    float mFallBackCheck;
    float mFallBackRadius;
    float mHearingRadius;
    uint mFlags;
    bool x1c_; // Unknown; read by no code in the REL (the Prime equivalent is also unused).
    CAssetId mProjectile;
    CDamageInfo mProjectileDamage;
    ushort mSound_Projectile;
    CDamageInfo mBladeDamage;
    float mKneelAttackChance;
    CAssetId mKneelAttackShot;
    CDamageInfo mKneelAttackDamage;
    float mDodgeCheck;
    ushort mSound_Impact;
    float mAverageNextShotTime;
    float mNextShotTimeVariation;
    float mIngAverageNextShotTime;   // Guessed name; shot time used while ing-possessed.
    float mIngNextShotTimeVariation; // Guessed name; variation used while ing-possessed.
    ushort mSound_Alert;
    float mGunTrackDelay;
    int mFirstBurstCount;
    float mCloakOpacity;
    float mMaxCloakOpacity;
    float mDodgeDelayTimeMin;
    float mDodgeDelayTimeMax;
    ushort mSound_Hurled;
    ushort mSound_Death;
    float xbc_; // Unknown; read by no code in the REL (loader default 0.2).
    float mAvoidDistance;
    float mMinLosClearTime; // Guessed name; the loader always passes 0.5.
    CSpacePirateWeaponData mWeaponData;
  };

  CSpacePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& modelData,
               const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
               const CSpacePirateData& data);

  // CEntity
  ~CSpacePirate() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool IsListening() const override { return true; }
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;

  // CPatterned
  bool IsScanVisorSelfRender() const override { return true; }
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  TUniqueId GetAttackTarget() const override { return mTargetId; }
  float GetGravityConstant() const override { return skGravityConstant; }
  uchar GetModelAlphau8(const CStateManager& mgr) const override;
  CProjectileInfo* ProjectileInfo() override;
  void SetupStateMachine(CStateManager& mgr) override;
  CRagDoll* GetRagDoll() const override;
  void SetAttackTarget(CStateManager& mgr, TUniqueId target) override;
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;
  bool TryToBeCaptured(CStateManager& mgr) override;

  bool AttachActorToPirate(TUniqueId id);
  void DetachActorFromPirate();

  TUniqueId GetAttachedActor() const { return mAttachedActor; }
  bool GetEnableAim() const { return mEnableAim; } // Guessed Prime name.
  bool AllEnergyDrained() const { return mAllEnergyDrained; }
  void SetPortalPlane(const CPlane& plane) { mPortalPlane = plane; } // Guessed name.

  // Triggers
  bool Stuck(CStateManager& mgr, const CTriggerData& data) const;
  bool PatternShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool HearShot(CStateManager& mgr, const CTriggerData& data) const;
  bool HearPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool AggressionCheck(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverCheck(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverFind(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverBlown(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverNearlyBlown(CStateManager& mgr, const CTriggerData& data) const;
  bool CoveringFire(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool LineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool PatternOver(CStateManager& mgr, const CTriggerData& data) const;
  bool SpotPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldRetreat(CStateManager& mgr, const CTriggerData& data) const;
  bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldCrouch(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMove(CStateManager& mgr, const CTriggerData& data) const;
  bool ShotAt(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool HasTargetingPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldWallHang(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldStrafe(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpecialAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool StartAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool BreakAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool LostInterest(CStateManager& mgr, const CTriggerData& data) const;
  bool BounceFind(CStateManager& mgr, const CTriggerData& data) const;
  bool OffLine(CStateManager& mgr, const CTriggerData& data) const;
  bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldJumpBack(CStateManager& mgr, const CTriggerData& data) const;
  bool Leash(CStateManager& mgr, const CTriggerData& data) const;
  bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;
  bool IsAmbushing(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldWarpIn(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldLaunchGrenade(CStateManager& mgr, const CTriggerData& data) const;
  bool InProjectileRange(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Ambushing(CStateManager& mgr, EStateMsg msg, float dt);
  void WarpIn(CStateManager& mgr, EStateMsg msg, float dt);
  void WarpOut(CStateManager& mgr, EStateMsg msg, float dt);
  void PostWarpOut(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt);
  void CoverAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Halt(CStateManager& mgr, EStateMsg msg, float dt);
  void Run(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Shuffle(CStateManager& mgr, EStateMsg msg, float dt);
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt);
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt);
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void Cover(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetCover(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  void Approach(CStateManager& mgr, EStateMsg msg, float dt);
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt);
  void WallDetach(CStateManager& mgr, EStateMsg msg, float dt);
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt);
  void Generate(CStateManager& mgr, EStateMsg msg, float dt);
  void Skid(CStateManager& mgr, EStateMsg msg, float dt);
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt);
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt);
  void Bounce(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float dt);
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt);
  void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt);
  void LaunchGrenade(CStateManager& mgr, EStateMsg msg, float dt);
  void Captured(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void RemoveFromWorld(CStateManager& mgr, EStateMsg msg, float dt);

private:
  void SetupWeaponModel(const CSpacePirateWeaponData& weaponData); // Guessed name.
  void UpdateCantSeePlayer(CStateManager& mgr, float dt);          // Guessed name.
  void UpdateHeldPosition(CStateManager& mgr, float dt);
  void CheckBlade(CStateManager& mgr);
  void CheckForProjectiles(CStateManager& mgr);
  bool IsPathClear(CStateManager& mgr, const CVector3f& dir, float dist); // Guessed name.
  void AvoidActors(CStateManager& mgr);
  pas::EStepDirection GetStrafeDir(CStateManager& mgr, float dist);
  bool LineOfSightTest(CStateManager& mgr, const CVector3f& eyePos, const CVector3f& targetPos,
                       const CMaterialList& excludeList);     // Guessed name.
  TUniqueId ChooseTarget(CStateManager& mgr) const;           // Guessed name.
  TUniqueId ChooseTargetPlayer(CStateManager& mgr) const;     // Guessed name.
  void SetTeamMemberTarget(CStateManager& mgr);               // Guessed name.
  void JoinTeam(CStateManager& mgr);                          // Guessed name.
  void SetEyeParticleActive(CStateManager& mgr, bool active); // Guessed Prime name.
  void SquadReset(CStateManager& mgr);                        // Guessed Prime name.
  void SquadRemove(CStateManager& mgr);                       // Guessed Prime name.
  void SetCinematicCollision(CStateManager& mgr);             // Guessed Prime name.
  void SetNonCinematicCollision(CStateManager& mgr);          // Guessed Prime name.
  void UpdateLeashTimer(float dt);                            // Guessed Prime name.
  void SetVelocityForJump();                                  // Guessed Prime name.
  void RequestWarpOut(CStateManager& mgr, bool deleteAfter);  // Guessed name.
  void RenderGrenadeLauncher(const CStateManager& mgr, const CTransform4f& xf,
                             const CModelFlags& flags) const; // Guessed name.
  void LaunchBouncyGrenade(CStateManager& mgr);               // Guessed name.
  CVector3f GetGrenadeTargetPosition(const CStateManager& mgr,
                                     const CActor* target) const; // Guessed name.
  void SolveGrenadeLaunch(const CVector3f& target, const CVector3f& origin, float& angle,
                          float& speed) const;           // Guessed name.
  bool FireProjectile(float dt, CStateManager& mgr);     // Guessed Prime name.
  void UpdateCloak(float dt, CStateManager& mgr);        // Guessed Prime name.
  void UpdateKnockBackSfx();                             // Guessed name.
  bool ShouldFrenzy(CStateManager& mgr);                 // Guessed Prime name.
  void UpdateAimBodyState(float dt, CStateManager& mgr); // Guessed Prime name.
  void UpdateAttacks(float dt, CStateManager& mgr);      // Guessed Prime name.
  bool CheckTargetable(CStateManager& mgr);              // Guessed Prime name.
  CVector3f GetTargetPos(CStateManager& mgr);            // Guessed Prime name.

  static const float skGravityConstant;
  static const SBurst skBurstsQuick[];
  static const SBurst skBurstsStandard[];
  static const SBurst skBurstsFrenzied[];
  static const SBurst skBurstsJumping[];
  static const SBurst skBurstsInjured[];
  static const SBurst skBurstsSeated[];
  static const SBurst skBurstsQuickOOV[];
  static const SBurst skBurstsStandardOOV[];
  static const SBurst skBurstsFrenziedOOV[];
  static const SBurst skBurstsJumpingOOV[];
  static const SBurst skBurstsInjuredOOV[];
  static const SBurst skBurstsSeatedOOV[];
  static const SBurst* skBursts[];
  static rstl::list< TUniqueId > mChargePlayerList;

  CSpacePirateData mPirateData;

  bool mPendingAmbush : 1;
  bool mCeilingAmbush : 1;
  bool mNonAggressive : 1;
  bool mMelee : 1;
  bool mNoShuffleCloseCheck : 1;
  bool mOnlyAttackInRange : 1;
  bool x8f4_30_ : 1; // Mirrors flag 0x40; read by no code.
  bool mNoKnockbackImpulseReset : 1;
  bool mNoMeleeAttack : 1;
  bool mBreakAttack : 1;
  bool mSeated : 1;
  bool mShadowPirate : 1;
  bool mAlertBeforeCloak : 1;
  bool mNoBreakDodge : 1;
  bool mFloatingCorpse : 1;
  bool mRagdollNoAiCollision : 1;
  bool mTrooper : 1;
  mutable bool mHearNoise : 1;
  bool mEnableMeleeAttack : 1;
  bool x8f6_27_ : 1; // Always false and read by no code.
  bool x8f6_28_ : 1; // Always false and read by no code.
  bool mEnableRetreat : 1;
  bool mShuffleClose : 1;
  bool mInAttackState : 1;
  bool mEnablePatrol : 1;
  bool mEnableAim : 1; // Guessed Prime name; read by flying pirate taunts.
  bool mHearPlayerFire : 1;
  bool mInProjectilePath : 1;
  bool mNoPlayerLos : 1;
  bool mInWallHang : 1;
  bool mJumpVelSet : 1;
  bool mPrevInCineCam : 1;
  bool mPendingFrenzyChance : 1;
  bool mAppliedBladeDamage : 1;
  bool mAlwaysAggressive : 1;
  bool mCoverCheck : 1;
  bool mEnableDodge : 1;
  bool mNoPlayerDodge : 1;
  bool mAllEnergyDrained : 1; // Guessed Prime name; read by the Metroid energy drain.
  mutable bool mMayStartAttack : 1;
  bool x8f9_24_ : 1; // Always false and read by no code.
  bool mUseJumpBackJump : 1;
  bool mStarted : 1;
  bool mInRange : 1;
  bool mSatUp : 1;
  bool mEnableBreakDodge : 1;
  bool mCloseMelee : 1;
  bool mSentAttackMsg : 1;
  bool mNormalDodge : 1;
  bool mGettingUp : 1;          // Guessed name; set during the GetUp state.
  bool mWarpInRequested : 1;    // Guessed name; ShouldWarpIn waits for it.
  bool mDeleteAfterWarpOut : 1; // Guessed name; PostWarpOut removes the pirate from the world.
  bool mWarpTimeCaptured : 1;   // Guessed name; mWarpTime was taken from the generate animation.
  bool mInJump : 1;        // Guessed name; set when a jump starts, cleared on landing; never read.
  bool mCannotShoot : 1;   // Guessed name; blocks the additive aim (wall hang cannot shoot).
  bool mWallDetaching : 1; // Guessed name; set while in WallDetach, blocks velocity for jump.

  int mFrenzyFrames;
  TUniqueId mCoverPoint;
  TUniqueId mPreviousCoverPoint;
  float mSteeringSpeed;
  CVector3f mTargetDelta;
  CVector3f mCoverPointRearDir;
  CPathFindSearch mPathFindSearch;
  float mUnkTimer; // Guessed Prime name; the constructor leaves it uninitialized.
  float mSteeringDelayTimer;
  uint xa14_; // Unknown; only zero-initialized (x74c_ in Prime).
  float mInitialHP;
  float mCoverRange;
  CSegId mHeadSeg;
  uint xa24_; // Unknown; set to Random % 6 on create and never read (x75c_ in Prime).
  pas::ETauntType mTaunt;
  CBoneTracking mBoneTracking;
  pas::ECoverDirection mCoverDir;
  uchar xa6c_[4]; // Unknown; uninitialized and never accessed (padding before mIntoJumpDist).
  float mIntoJumpDist;
  float mEyeHeight;
  float mTimeLosClear; // Guessed name; time since the last blocked line-of-sight check.
  float mTimeNoPlayerLos;
  float mLosCheckTimer;     // Guessed name; 0.1s interval of the line-of-sight check.
  TUniqueId mAttachedActor; // Guessed member name.
  CSegId mGunSeg;
  CSegId mElbowSeg;
  CSegId mWristSeg;
  CSegId mSwooshSeg;
  CSegId mLeftHipSeg;  // Guessed name.
  CSegId mRightHipSeg; // Guessed name.
  CSegId mCollarSeg;   // Guessed name.
  float mAttackRemTime;
  TUniqueId mTargetId;
  CBurstFire mBurstFire;
  float mJumpHeight;
  CVector3f mPatrolDestPos;
  mutable pas::EStepDirection mSkidDir;
  float mStrafeDelayTimer;
  pas::ESeverity mMeleeSeverity;
  TUniqueId mJumpPoint;
  pas::EStepDirection mDodgeDir;
  float mDodgeDist;
  float mBreakDodgeDist;
  float mTimeSinceHitByPlayer;
  float mLowHealthFrenzyTimer;
  float mRagdollDelayTimer;
  rstl::single_ptr< CPirateRagDoll > mRagDoll;
  CIkChain mIkChain;
  float mCloakDelayTimer;
  float mElectricParticleTimer;
  float mCloakStepTime;
  float mShadowPirateAlpha;
  float mMinCloakAlpha;
  float mMaxCloakAlpha;
  float mDodgeDelayTimer;
  float mAimDelayTimer;
  float mAimReleaseTimer; // Guessed name; 0.5s of holding the additive aim forward.
  TUniqueId mTeamAiMgrId;
  CVector2f mHeldPosition;
  float mHoldPositionTime;
  float mLeashTimer;
  CVector3f mPlayerFirePos;      // Guessed name; where the player last fired within earshot.
  mutable uint mHearPlayerIndex; // Guessed name; round-robin index of the player heard.
  CVector3f mAttackTargetPos;    // Guessed name.
  float mWarpTime;               // Guessed name; duration of the generate animation when warping.
  rstl::optional_object< CProjectileInfo > mProjectileInfo;
  CSfxHandle mKnockBackSfx; // Guessed name; the last hurled or death sound emitted.
  int mWarpPhase;           // Guessed name; -1 when idle, 0 and 1 while warping.
  rstl::optional_object< CModelData > mGrenadeLauncherModel; // Guessed name.
  int mGrenadesToLaunch;                                     // Guessed name.
  CPlane mPortalPlane; // Guessed name; written by the WispTentacle while it drags the pirate.
};
CHECK_SIZEOF(CSpacePirate, 0xc70)

#endif
