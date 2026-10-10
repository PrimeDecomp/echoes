#ifndef _CCOMMANDOPIRATE
#define _CCOMMANDOPIRATE

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

// Guessed struct: the E-grenade attack tuning assembled by the script loader.
class CCommandoGrenadeData : public CBouncyGrenadeData {
public:
  CCommandoGrenadeData(const CBouncyGrenadeData& grenade, float eMPDuration,
                       float minAttackInterval, float postAttackPause, float attackChance,
                       float minAttackDist, float maxAttackDist, float minLaunchSpeed,
                       float maxLaunchSpeed)
  : CBouncyGrenadeData(grenade)
  , mEMPDuration(eMPDuration)
  , mAttackChance(attackChance)
  , mMinAttackInterval(minAttackInterval)
  , mPostAttackPause(postAttackPause)
  , mMinAttackDist(minAttackDist)
  , mMaxAttackDist(maxAttackDist)
  , mMinLaunchSpeed(minLaunchSpeed)
  , mMaxLaunchSpeed(maxLaunchSpeed) {}

  float mEMPDuration;
  float mAttackChance;
  float mMinAttackInterval;
  float mPostAttackPause;
  float mMinAttackDist;
  float mMaxAttackDist;
  float mMinLaunchSpeed;
  float mMaxLaunchSpeed;
};
CHECK_SIZEOF(CCommandoGrenadeData, 0x70)

// Guessed struct: the shield charge and arm shield tuning assembled by the script loader.
class CCommandoShieldData {
public:
  CCommandoShieldData(const CDamageInfo& chargeDamage, const CDamageVulnerability& vulnerability,
                      float chargeMinAttackDist, float chargeMaxAttackDist, float chargeSpeed,
                      CAssetId explodeEffect, ushort sound_Explode, float armDamageThreshold,
                      float armDamageDecayTime, float armChance, float armTime,
                      float armTimeVariation, CAssetId armExplodeEffect, CAssetId chargeEffect,
                      CAssetId armEffect, ushort sound_TurnOn, ushort sound_TurnOff)
  : mChargeDamage(chargeDamage)
  , mVulnerability(vulnerability)
  , mChargeMinAttackDist(chargeMinAttackDist)
  , mChargeMaxAttackDist(chargeMaxAttackDist)
  , mChargeSpeed(chargeSpeed)
  , mExplodeEffect(explodeEffect)
  , mSound_Explode(sound_Explode)
  , mArmDamageThreshold(armDamageThreshold)
  , mArmDamageDecayTime(armDamageDecayTime)
  , mArmChance(armChance)
  , mArmTime(armTime)
  , mArmTimeVariation(armTimeVariation)
  , mArmExplodeEffect(armExplodeEffect)
  , mChargeEffect(chargeEffect)
  , mArmEffect(armEffect)
  , mSound_TurnOn(sound_TurnOn)
  , mSound_TurnOff(sound_TurnOff) {}

  CDamageInfo mChargeDamage;
  CDamageVulnerability mVulnerability;
  float mChargeMinAttackDist;
  float mChargeMaxAttackDist;
  float mChargeSpeed;
  CAssetId mExplodeEffect;
  ushort mSound_Explode;
  float mArmDamageThreshold; // Guessed name; recent damage that makes the pirate arm its shield
  float mArmDamageDecayTime; // Guessed name; seconds for the recent damage to decay from the
                             // threshold
  float mArmChance;
  float mArmTime;
  float mArmTimeVariation;
  CAssetId mArmExplodeEffect;
  CAssetId mChargeEffect;
  CAssetId mArmEffect;
  ushort mSound_TurnOn;
  ushort mSound_TurnOff;
};
CHECK_SIZEOF(CCommandoShieldData, 0x84)

// Guessed struct: the tuned values of a Commando Pirate, assembled by the script loader.
class CCommandoPirateData {
public:
  CCommandoPirateData(uint flags, float aggressiveness, float coverCheck, float searchRadius,
                      float dodgeCheck, ushort sound_Impact, ushort sound_Hurled,
                      ushort sound_Death, ushort sound_Drowning, int drowningEffect,
                      const CDamageInfo& bladeDamage, CAssetId projectile,
                      const CDamageInfo& projectileDamage, ushort sound_Projectile,
                      float hearingRadius, float intraBurstShotTime, float intraBurstShotVariation,
                      const CCommandoGrenadeData& grenade, const CCommandoShieldData& shield)
  : mSound_Impact(sound_Impact)
  , mSound_Hurled(sound_Hurled)
  , mSound_Death(sound_Death)
  , mSound_Drowning(sound_Drowning)
  , mDrowningEffect(drowningEffect)
  , mBladeDamage(bladeDamage)
  , mProjectile(projectile)
  , mProjectileDamage(projectileDamage)
  , mSound_Projectile(sound_Projectile)
  , mAggressiveness(aggressiveness)
  , mCoverCheck(coverCheck)
  , mSearchRadius(searchRadius)
  , mDodgeCheck(dodgeCheck)
  , mHearingRadius(hearingRadius)
  , mIntraBurstShotTime(intraBurstShotTime)
  , mIntraBurstShotVariation(intraBurstShotVariation)
  , mGrenade(grenade)
  , mShield(shield)
  , mPendingAmbush(flags & 0x1)
  , mBreakAmbush(flags & 0x2)
  , mNoDodge(flags & 0x4)
  , mMelee(flags & 0x8)
  , mStationary(flags & 0x10)
  , mNoKnockbackImpulseReset(flags & 0x20)
  , mNoMeleeAttack(flags & 0x40)
  , mFloatingCorpse(flags & 0x80)
  , mTrooper(flags & 0x100)
  , mNoShieldCharge(flags & 0x200)
  , mNoArmShield(flags & 0x400)
  , mNoGrenadeAttack(flags & 0x800) {}

  /*
   * Script flags (SLdr property 0x7abed4ce); names are inferred from how the REL uses each bit.
   * 0x1: pendingAmbush
   * 0x2: breakAmbush
   * 0x4: noDodge
   * 0x8: melee (never shoots)
   * 0x10: stationary (never retreats, jumps back, takes cover or path finds)
   * 0x20: noKnockbackImpulseReset
   * 0x40: noMeleeAttack
   * 0x80: floatingCorpse
   * 0x100: trooper (warps in and out)
   * 0x200: noShieldCharge
   * 0x400: noArmShield
   * 0x800: noGrenadeAttack
   */
  ushort mSound_Impact;
  ushort mSound_Hurled;
  ushort mSound_Death;
  ushort mSound_Drowning; // Guessed name; played when the pirate drowns
  int mDrowningEffect;    // Guessed name; CAssetId of the explosion spawned when the pirate drowns
  CDamageInfo mBladeDamage;
  CAssetId mProjectile;
  CDamageInfo mProjectileDamage;
  ushort mSound_Projectile;
  float mAggressiveness;
  float mCoverCheck; // Guessed name
  float mSearchRadius;
  float mDodgeCheck;
  float mHearingRadius;
  float mIntraBurstShotTime;
  float mIntraBurstShotVariation;
  CCommandoGrenadeData mGrenade;
  CCommandoShieldData mShield;
  bool mPendingAmbush : 1;           // Guessed name
  bool mBreakAmbush : 1;             // Guessed name
  bool mNoDodge : 1;                 // Guessed name
  bool mMelee : 1;                   // Guessed name
  bool mStationary : 1;              // Guessed name
  bool mNoKnockbackImpulseReset : 1; // Guessed name
  bool mNoMeleeAttack : 1;           // Guessed name
  bool mFloatingCorpse : 1;          // Guessed name
  bool mTrooper : 1;                 // Guessed name
  bool mNoShieldCharge : 1;          // Guessed name
  bool mNoArmShield : 1;             // Guessed name
  bool mNoGrenadeAttack : 1;         // Guessed name
};
CHECK_SIZEOF(CCommandoPirateData, 0x160)
// Guessed class: the Commando Pirate, a shielded pirate that charges, jumps, boosts, fires
// E-grenades and takes cover.
class CCommandoPirate : public CPatterned {
public:
  CCommandoPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, const CModelData& modelData,
                  const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                  const CCommandoPirateData& data);

  // CEntity
  ~CCommandoPirate() override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }

  // CAi
  bool IsListening() const override { return true; }

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  CDamageInfo GetContactDamage() const override;
  float GetGravityConstant() const override { return 50.f; }
  void SetupStateMachine(CStateManager& mgr) override;

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool ShouldWarpIn(CStateManager& mgr, const CTriggerData& data) const;       // Guessed name
  bool ShouldMeleeAttack(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool ShouldFireEGrenade(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  bool ShouldRetreat(CStateManager& mgr, const CTriggerData& data) const;      // Guessed name
  bool ShouldJumpBack(CStateManager& mgr, const CTriggerData& data) const;     // Guessed name
  bool ShouldAmbush(CStateManager& mgr, const CTriggerData& data) const;       // Guessed name
  bool BreakAmbush(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const;     // Guessed name
  bool UnderFire(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool HeardShot(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool HasTarget(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool HasNewTarget(CStateManager& mgr, const CTriggerData& data) const;       // Guessed name
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;           // Guessed name
  bool IsOffPath(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;   // Guessed name
  bool IsFacingTarget(CStateManager& mgr, const CTriggerData& data) const;     // Guessed name
  bool AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool TooClose(CStateManager& mgr, const CTriggerData& data) const;           // Guessed name
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool ShouldArmShield(CStateManager& mgr, const CTriggerData& data) const;    // Guessed name
  bool ShouldShieldCharge(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  bool ShouldBoost(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool AbortShieldCharge(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool IsAggressive(CStateManager& mgr, const CTriggerData& data) const;       // Guessed name
  bool FoundJumpPoint(CStateManager& mgr, const CTriggerData& data) const;     // Guessed name
  bool ShouldCrouch(CStateManager& mgr, const CTriggerData& data) const;       // Guessed name
  bool ShouldWallHang(CStateManager& mgr, const CTriggerData& data) const;     // Guessed name
  bool ShouldCover(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool FoundCover(CStateManager& mgr, const CTriggerData& data) const;         // Guessed name
  bool ShouldCoverAttack(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool CoverBlown(CStateManager& mgr, const CTriggerData& data) const;         // Guessed name
  bool AbortSeekCover(CStateManager& mgr, const CTriggerData& data) const;     // Guessed name

  // States
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);                 // Guessed name
  void Ambush(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void Alert(CStateManager& mgr, EStateMsg msg, float dt);                // Guessed name
  void FaceTarget(CStateManager& mgr, EStateMsg msg, float dt);           // Guessed name
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float dt);         // Guessed name
  void SetTargetDest(CStateManager& mgr, EStateMsg msg, float dt);        // Guessed name
  void SetRetreatDest(CStateManager& mgr, EStateMsg msg, float dt);       // Guessed name
  void SetJumpDest(CStateManager& mgr, EStateMsg msg, float dt);          // Guessed name
  void SetPathMeshDest(CStateManager& mgr, EStateMsg msg, float dt);      // Guessed name
  void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);          // Guessed name
  void EGrenadeAttack(CStateManager& mgr, EStateMsg msg, float dt);       // Guessed name
  void PostEGrenadeAttack(CStateManager& mgr, EStateMsg msg, float dt);   // Guessed name
  void JumpPointFind(CStateManager& mgr, EStateMsg msg, float dt);        // Guessed name
  void JetBoost(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt);  // Guessed name
  void WarpIn(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void WarpOut(CStateManager& mgr, EStateMsg msg, float dt);              // Guessed name
  void PostWarpOut(CStateManager& mgr, EStateMsg msg, float dt);          // Guessed name
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt);                // Guessed name
  void ArmShield(CStateManager& mgr, EStateMsg msg, float dt);            // Guessed name
  void ShieldCharge(CStateManager& mgr, EStateMsg msg, float dt);         // Guessed name
  void ScriptedShieldCharge(CStateManager& mgr, EStateMsg msg, float dt); // Guessed name
  void RestoreOrientation(CStateManager& mgr, EStateMsg msg, float dt);   // Guessed name
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void WallDetach(CStateManager& mgr, EStateMsg msg, float dt);           // Guessed name
  void CoverFind(CStateManager& mgr, EStateMsg msg, float dt);            // Guessed name
  void SetCoverDest(CStateManager& mgr, EStateMsg msg, float dt);         // Guessed name
  void Cover(CStateManager& mgr, EStateMsg msg, float dt);                // Guessed name
  void CoverAttack(CStateManager& mgr, EStateMsg msg, float dt);          // Guessed name
  void BreakCover(CStateManager& mgr, EStateMsg msg, float dt);           // Guessed name
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt);                // Guessed name
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);                 // Guessed name

private:
  void UpdateTimers(float dt, CStateManager& mgr);    // Guessed name
  void UpdateBurstFire(float dt, CStateManager& mgr); // Guessed name
  bool FireProjectile(float dt, CStateManager& mgr);  // Guessed name
  void LaunchGrenade(CStateManager& mgr);             // Guessed name
  CVector3f GetGrenadeTargetPosition(const CStateManager& mgr,
                                     const CActor& target) const; // Guessed name
  void SolveGrenadeLaunch(const CVector3f& target, const CVector3f& origin, float& angle,
                          float& speed) const;                     // Guessed name
  void UpdateAdditiveAim(CStateManager& mgr);                      // Guessed name
  bool CanFireAtTarget() const;                                    // Guessed name
  void UpdateEmitters();                                           // Guessed name
  void SetupCollisionActors(CStateManager& mgr);                   // Guessed name
  void ThinkRagDoll(float dt, CStateManager& mgr, bool noRagDoll); // Guessed name
  void RequestWarpOut(CStateManager& mgr, bool deleteAfter);       // Guessed name
  bool GetTargetAimPosition(CStateManager& mgr, CVector3f& position,
                            float dt) const;                       // Guessed name
  void ApplyChargeDamage(const TUniqueId& id, CStateManager& mgr); // Guessed name
  void SetVelocityForJump();                                       // Guessed name
  pas::EStepDirection ChooseDodgeDirection(CStateManager& mgr);    // Guessed name
  int SelectMeleeVariant(CStateManager& mgr) const;                // Guessed name
  bool IsPathClear(CStateManager& mgr, const CVector3f& direction,
                   float distance) const;                              // Guessed name
  void CheckDrowning(CStateManager& mgr);                              // Guessed name
  void HandleShieldHit(CStateManager& mgr, const TUniqueId& id);       // Guessed name
  bool FindShieldChargeDest(CStateManager& mgr, bool useNextWaypoint); // Guessed name
  void UpdateShieldCharge(float dt);                                   // Guessed name
  void BreakShield(CStateManager& mgr);                                // Guessed name
  void TurnShieldOn();                                                 // Guessed name
  void TurnShieldOff();                                                // Guessed name
  void UpdateShieldEffects(CStateManager& mgr, float dt);              // Guessed name
  void ApplySeparation(CStateManager& mgr);                            // Guessed name
  void JoinTeam(CStateManager& mgr);                                   // Guessed name
  void QuitTeam(CStateManager& mgr);                                   // Guessed name

  static const SBurst skBurstsA[];
  static const SBurst skBurstsB[];
  static const SBurst skBurstsC[];
  static const SBurst skBurstsD[];
  static const SBurst* skBursts[];

  CCommandoPirateData mData;                                      // Guessed name
  int mWarpPhase;                                                 // Guessed name
  int mActionPhase;                                               // Guessed name
  CPathFindSearch mPathFindSearch;                                // Guessed name
  CProjectileInfo mProjectileInfo;                                // Guessed name
  rstl::single_ptr< CCollisionActorManager > mShieldCollisionMgr; // Guessed name
  rstl::single_ptr< CCollisionActorManager > mBladeCollisionMgr;  // Guessed name
  CDamageVulnerability mShieldVulnerability;                      // Guessed name
  CBoneTracking mBoneTracking;                                    // Guessed name
  CLineOfSightTracker mLineOfSightTracker;                        // Guessed name
  CBurstFire mBurstFire;                                          // Guessed name
  uint mBurstCount;                                               // Guessed name
  uint mNextShieldChargeBurst;                                    // Guessed name
  uint mNextArmShieldBurst;                                       // Guessed name
  float mBurstCooldown;                                           // Guessed name
  float mTimeSinceHeardShot;                                      // Guessed name
  float mTimeSinceHit;                                            // Guessed name
  float mDodgeCheckTimer;                                         // Guessed name
  float mCoverCheckTimer;                                         // Guessed name
  float mAggressionCheckTimer;                                    // Guessed name
  float mGrenadeAttackTimer;                                      // Guessed name
  float mJumpPointSearchTimer;                                    // Guessed name
  float mTimeSinceMelee;                                          // Guessed name
  float mArmShieldTimer;                                          // Guessed name
  float mChargeBlockedTime;                                       // Guessed name
  float mChargeDuration;                                          // Guessed name
  float mSeekCoverTime;                                           // Guessed name
  float mSeekCoverTimeLimit;                                      // Guessed name
  CVector3f mTargetPosition;
  CVector3f mFaceDirection;
  CVector3f mChargeUp;
  CVector3f mChargeCollisionNormal;
  TUniqueId mTeamAiMgrId;             // Guessed name
  TUniqueId mShieldCollisionActorId;  // Guessed name
  TUniqueId mBladeCollisionActorId;   // Guessed name
  TUniqueId mTargetId;                // Guessed name
  TUniqueId mAlertedTargetId;         // Guessed name
  TUniqueId mRetreatWaypointId;       // Guessed name
  TUniqueId mJumpPointId;             // Guessed name
  TUniqueId mCoverPointId;            // Guessed name
  TUniqueId mLastCoverPointId;        // Guessed name
  TUniqueId mWallHangWaypointId;      // Guessed name
  TUniqueId mSpawnedObjectId;         // Guessed name
  TUniqueId mAttackPatternWaypointId; // Guessed name
  pas::EStepDirection mDodgeDir;      // Guessed name
  float mJumpBackDistance;            // Guessed name
  float mDodgeDistance;               // Guessed name
  float mJumpDistance;                // Guessed name
  float mJumpApexHeight;              // Guessed name
  CVector3f mJumpDestination;
  rstl::single_ptr< CPirateRagDoll > mRagDoll;                                      // Guessed name
  float mRagDollTimer;                                                              // Guessed name
  CSfxHandle mHurledSfxHandle;                                                      // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mShieldExplodeEffect;    // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mArmShieldExplodeEffect; // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mDrowningEffect;         // Guessed name
  CSfxHandle mSfxHandle;                                                            // Guessed name
  rstl::single_ptr< CElementGen > mArmShieldEffect;                                 // Guessed name
  rstl::single_ptr< CElementGen > mShieldChargeEffect;                              // Guessed name
  float mShieldEffectScale;                                                         // Guessed name
  float mShieldEffectTargetScale;                                                   // Guessed name
  int mShieldEffectType;                                                            // Guessed name
  float mRecentDamage;                                                              // Guessed name
  int mMeleeVariant;                                                                // Guessed name
  float mWarpAnimTime;                                                              // Guessed name
  CSegId mHeadSeg;                                                                  // Guessed name
  CSegId mLaunchSeg;                                                                // Guessed name
  CSegId mGunSeg;                                                                   // Guessed name
  CSegId mGrenadeSeg;                                                               // Guessed name
  CSegId mRightWristSeg;                                                            // Guessed name
  CSegId mRightElbowSeg;                                                            // Guessed name
  CSegId mLeftWristSeg;                                                             // Guessed name
  bool mPathBlocked : 1;                                                            // Guessed name
  bool mEnableRetreat : 1;                                                          // Guessed name
  bool mWarpInRequested : 1;                                                        // Guessed name
  bool mDeleteAfterWarpOut : 1;                                                     // Guessed name
  bool mJumpVelSet : 1;                                                             // Guessed name
  bool mEnableDodge : 1;                                                            // Guessed name
  bool mEnableGrenadeAttack : 1;                                                    // Guessed name
  bool mCoverCheck : 1;                                                             // Guessed name
  bool mArmShieldWanted : 1;                                                        // Guessed name
  bool mAggressive : 1;                                                             // Guessed name
  bool mStopped : 1;                                                                // Guessed name
  bool mAdditiveAimEnabled : 1;                                                     // Guessed name
  bool mChargeStarted : 1;                                                          // Guessed name
  bool mAbortShield : 1;                                                            // Guessed name
  bool mChargeBlocked : 1;                                                          // Guessed name
  bool mShieldBroken : 1;                                                           // Guessed name
  bool mPathDestValid : 1;                                                          // Guessed name
  bool mWarpTimeCaptured : 1;                                                       // Guessed name
  bool mMeleeAttacking : 1;                                                         // Guessed name
  bool mGettingUp : 1;                                                              // Guessed name
  bool mArmingShield : 1;                                                           // Guessed name
  bool mShieldCharging : 1;                                                         // Guessed name
  bool mInWallHang : 1;                                                             // Guessed name
  bool mEGrenadeAttacking : 1;                                                      // Guessed name
  bool mFollowingAttackPattern : 1;                                                 // Guessed name
  bool mSeekingCover : 1;                                                           // Guessed name
  bool mShieldOn : 1;                                                               // Guessed name
};
CHECK_SIZEOF(CCommandoPirate, 0xc68)

// Guessed class: the E-grenade, which jams the player's visor after it explodes.
class CCommandoPirateGrenade : public CBouncyGrenade {
public:
  CCommandoPirateGrenade(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& modelData,
                         const CActorParameters& actorParams, TUniqueId parentId,
                         const CCommandoGrenadeData& data, float velocity);

  // CEntity
  ~CCommandoPirateGrenade() override;
  void Think(float dt, CStateManager& mgr) override;

private:
  CCommandoGrenadeData mData; // Guessed name
  float mEMPTime;             // Guessed name
  CRELFileToken mRelToken;    // Guessed name
};
CHECK_SIZEOF(CCommandoPirateGrenade, 0x410)

#endif // _CCOMMANDOPIRATE
