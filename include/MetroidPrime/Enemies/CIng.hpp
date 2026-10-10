#ifndef _CING
#define _CING

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Enemies/CIngSpotData.hpp"
#include "MetroidPrime/Enemies/CIngSpotPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CGenDescription;
class CScriptAIHint;
class CWeapon;

// Guessed struct: how the Ing hurls its body at the player.
struct SIngBodyProjectileData {
  SIngBodyProjectileData(const CDamageInfo& contactDamage, float suckDamagePerSecond,
                         float suckTime, CAssetId splatEffect, ushort sound, ushort splatWallSound,
                         float speed, float dropTime, float minAttackDistance,
                         float maxAttackDistance, float odds)
  : contactDamage(contactDamage)
  , suckDamagePerSecond(suckDamagePerSecond)
  , splatEffect(splatEffect)
  , sound(sound)
  , splatWallSound(splatWallSound)
  , speed(speed)
  , dropTime(dropTime)
  , minAttackDistance(minAttackDistance)
  , maxAttackDistance(maxAttackDistance)
  , suckTime(suckTime)
  , odds(odds) {}

  CDamageInfo contactDamage;
  float suckDamagePerSecond;
  CAssetId splatEffect;
  ushort sound;
  ushort splatWallSound;
  float speed;
  float dropTime;
  float minAttackDistance;
  float maxAttackDistance;
  float suckTime;
  float odds;
};
CHECK_SIZEOF(SIngBodyProjectileData, 0x40)

// Guessed struct: the Ing's grapple of the morph ball.
struct SIngGrappleData {
  SIngGrappleData(float holdDamagePerSecond, const CDamageInfo& exitDamage, float spitForce,
                  ushort exitSound, ushort grappleSound, float maxHoldTime, float postWaitTime,
                  float pursuitRange, const CDamageVulnerability& vulnerability)
  : holdDamagePerSecond(holdDamagePerSecond)
  , exitDamage(exitDamage)
  , spitForce(spitForce)
  , exitSound(exitSound)
  , grappleSound(grappleSound)
  , maxHoldTime(maxHoldTime)
  , postWaitTime(postWaitTime)
  , pursuitRange(pursuitRange)
  , vulnerability(vulnerability) {}

  float holdDamagePerSecond;
  CDamageInfo exitDamage;
  float spitForce;
  ushort exitSound;
  ushort grappleSound;
  float maxHoldTime;
  float postWaitTime;
  float pursuitRange;
  CDamageVulnerability vulnerability;
};
CHECK_SIZEOF(SIngGrappleData, 0x64)

// Guessed struct: the mini portals the Ing opens to fire from.
struct SIngMiniPortalData {
  SIngMiniPortalData(float minAttackDistance, float maxAttackDistance, CAssetId effect,
                     ushort sound, const CDamageInfo& damage, const SLdrPlasmaBeamInfo& beamInfo)
  : minAttackDistance(minAttackDistance)
  , maxAttackDistance(maxAttackDistance)
  , effect(effect)
  , sound(sound)
  , damage(damage)
  , beamInfo(beamInfo) {}

  float minAttackDistance;
  float maxAttackDistance;
  CAssetId effect;
  ushort sound;
  CDamageInfo damage;
  SLdrPlasmaBeamInfo beamInfo;
};
CHECK_SIZEOF(SIngMiniPortalData, 0x74)

// Guessed struct: the effects and sounds of the Ing swarm that moves between hosts.
struct SIngSwarmData {
  SIngSwarmData(CAssetId possessionHudEffect, CAssetId exitHostSwarmEffect,
                CAssetId exitHostTrailEffect, float exitHostTrailLength,
                CAssetId exitHostSmokeEffect, float exitHostSpeed, float exitHostHomingTime,
                float exitHostHomingStrength, ushort swarmMoveSound, ushort exitHostSound,
                ushort exitHostSafeZoneSound, ushort insideHostSound, ushort exitHostSmokeSound)
  : possessionHudEffect(possessionHudEffect)
  , exitHostSwarmEffect(exitHostSwarmEffect)
  , exitHostTrailEffect(exitHostTrailEffect)
  , exitHostTrailLength(exitHostTrailLength)
  , exitHostSmokeEffect(exitHostSmokeEffect)
  , exitHostSpeed(exitHostSpeed)
  , exitHostHomingTime(exitHostHomingTime)
  , exitHostHomingStrength(exitHostHomingStrength)
  , swarmMoveSound(swarmMoveSound)
  , exitHostSound(exitHostSound)
  , exitHostSafeZoneSound(exitHostSafeZoneSound)
  , insideHostSound(insideHostSound)
  , exitHostSmokeSound(exitHostSmokeSound) {}

  CAssetId possessionHudEffect;
  CAssetId exitHostSwarmEffect;
  CAssetId exitHostTrailEffect;
  float exitHostTrailLength;
  CAssetId exitHostSmokeEffect;
  float exitHostSpeed;
  float exitHostHomingTime;
  float exitHostHomingStrength;
  ushort swarmMoveSound;
  ushort exitHostSound;
  ushort exitHostSafeZoneSound;
  ushort insideHostSound;
  ushort exitHostSmokeSound;
};
CHECK_SIZEOF(SIngSwarmData, 0x2c)

// Guessed struct: the tuned values of an Ing, assembled by the script loader.
struct SIngData {
  SIngData(uint flags, float hearingRadius, float coverLeashDistance, float formChangeInterval,
           float frustrationTime, float tauntChance, float aggressiveness, const CColor& lightColor,
           float lightAttenuation, const CDamageInfo& armSwipeDamage,
           const CDamageVulnerability& triggerVulnerability,
           const SIngBodyProjectileData& bodyProjectile, const CIngSpotData& ingSpot,
           const SIngGrappleData& grapple, const SIngMiniPortalData& miniPortal,
           const SIngSwarmData& swarm)
  : hearingRadius(hearingRadius)
  , coverLeashDistance(coverLeashDistance)
  , formChangeInterval(formChangeInterval)
  , frustrationTime(frustrationTime)
  , tauntChance(tauntChance)
  , aggressiveness(aggressiveness)
  , lightColor(lightColor)
  , lightAttenuation(lightAttenuation)
  , armSwipeDamage(armSwipeDamage)
  , triggerVulnerability(triggerVulnerability)
  , bodyProjectile(bodyProjectile)
  , ingSpot(ingSpot)
  , grapple(grapple)
  , miniPortal(miniPortal)
  , swarm(swarm)
  , startsAsIngSpot((flags & 0x01) != 0)
  , disableArmSwipe((flags & 0x02) != 0)
  , disableBodyProjectile((flags & 0x04) != 0)
  , disableMiniPortal((flags & 0x08) != 0)
  , disableGrapple((flags & 0x10) != 0)
  , disableFormChange((flags & 0x20) != 0)
  , disablePathMovement((flags & 0x40) != 0)
  , alwaysTargetable((flags & 0x80) != 0) {}

  float hearingRadius;
  float coverLeashDistance;
  float formChangeInterval;
  float frustrationTime;
  float tauntChance;
  float aggressiveness;
  CColor lightColor;
  float lightAttenuation;
  CDamageInfo armSwipeDamage;
  CDamageVulnerability triggerVulnerability;
  SIngBodyProjectileData bodyProjectile;
  CIngSpotData ingSpot;
  SIngGrappleData grapple;
  SIngMiniPortalData miniPortal;
  SIngSwarmData swarm;
  bool startsAsIngSpot : 1; // Guessed names
  bool disableArmSwipe : 1;
  bool disableBodyProjectile : 1;
  bool disableMiniPortal : 1;
  bool disableGrapple : 1;
  bool disableFormChange : 1;
  bool disablePathMovement : 1;
  bool alwaysTargetable : 1;
};
CHECK_SIZEOF(SIngData, 0x214)

// Guessed class: the Ing, the possessing creature that slips between hosts, safe zones and
// the corporeal, spot and projectile forms.
class CIng : public CPatterned {
public:
  enum EForm {
    kF_PossessingHost = 0,
    kF_ExitingHost = 1,
    kF_Corporeal = 2,      // Guessed name
    kF_IngSpot = 3,        // Guessed name
    kF_BodyProjectile = 4, // Guessed name
    kF_Evaporating = 5,
    kF_BecomingCorporeal = 6,
    kF_BecomingIngSpot = 7,
    kF_BecomingBodyProjectile = 8,
    kF_BecomingWallProjectile = 9,
    kF_Invalid = -1, // Guessed name
  };

  CIng(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
       const CModelData& modelData, const CActorParameters& actorParams,
       const CPatternedInfo& patternedInfo, const SIngData& data);

  // CEntity
  ~CIng() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  bool IsListening() const override { return true; }
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  void TakeDamage(const CVector3f& direction, float magnitude) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CPathFindPointSearch* GetPointSearchPath() override { return &mPointSearch; }
  float GetDeathTimeScale() const override { return 1.f; }
  void UpdateHitDamageTime(float dt) override;
  void SetupStateMachine(CStateManager& mgr) override;

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool IsAlert(CStateManager& mgr, const CTriggerData& data) const;
  bool IsControllingHost(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFacingTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool HasTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool HasNewTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool UnderFire(CStateManager& mgr, const CTriggerData& data) const;
  bool HeardShot(CStateManager& mgr, const CTriggerData& data) const;
  bool AreaClear(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverLeash(CStateManager& mgr, const CTriggerData& data) const;
  bool HasCoverPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool HasWallCoverPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool HasMiniPortals(CStateManager& mgr, const CTriggerData& data) const;
  bool IsCorporeal(CStateManager& mgr, const CTriggerData& data) const;
  bool IsIngSpot(CStateManager& mgr, const CTriggerData& data) const;
  bool IsBodyProjectile(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFrustrated(CStateManager& mgr, const CTriggerData& data) const;
  bool IsAggressive(CStateManager& mgr, const CTriggerData& data) const;
  bool IsLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool StillLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool UseProjectileFSMEntry(CStateManager& mgr, const CTriggerData& data) const;
  bool TargetInSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool TargetInLightSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool EmergePtInSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool CanChangeForm(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldEvaporate(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFleeSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBecomeCorporeal(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBecomeIngSpot(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldArmSwipe(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBodyProjectile(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldOpenMiniPortal(CStateManager& mgr, const CTriggerData& data) const;
  bool ProjectileSplat(CStateManager& mgr, const CTriggerData& data) const;
  bool ProjectileHitTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool FoundMovementPos(CStateManager& mgr, const CTriggerData& data) const;
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PointPathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool FoundPointPath(CStateManager& mgr, const CTriggerData& data) const;
  bool IsOffPath(CStateManager& mgr, const CTriggerData& data) const;
  bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const;
  bool TargetIsBall(CStateManager& mgr, const CTriggerData& data) const;
  bool InBallPursuitRange(CStateManager& mgr, const CTriggerData& data) const;
  bool HasPathToTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool InGrappleRange(CStateManager& mgr, const CTriggerData& data) const;
  bool CanGrappleTarget(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void WaitForFSMTransition(CStateManager& mgr, EStateMsg msg, float dt);
  void ExitHost(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeCorporeal(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeIngSpot(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeBodyProjectile(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeWallProjectile(CStateManager& mgr, EStateMsg msg, float dt);
  void Alert(CStateManager& mgr, EStateMsg msg, float dt);
  void SafeZoneReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void IngSpotPathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void IngSpotPointPathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt);
  void BodyProjectileFlight(CStateManager& mgr, EStateMsg msg, float dt);
  void SuckEnergy(CStateManager& mgr, EStateMsg msg, float dt);
  void SeekWallPoint(CStateManager& mgr, EStateMsg msg, float dt);
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float dt);
  void FaceTarget(CStateManager& mgr, EStateMsg msg, float dt);
  void FindCoverPoint(CStateManager& mgr, EStateMsg msg, float dt);
  void ArmSwipe(CStateManager& mgr, EStateMsg msg, float dt);
  void FailSafeMode(CStateManager& mgr, EStateMsg msg, float dt);
  void GrappleBall(CStateManager& mgr, EStateMsg msg, float dt);
  void FindMiniPortals(CStateManager& mgr, EStateMsg msg, float dt);
  void MiniPortalAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void Evaporate(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void SetTargetDest(CStateManager& mgr, float dt);
  void SetCoverDest(CStateManager& mgr, float dt);
  void SetRecoverDest(CStateManager& mgr, float dt);
  void SetPointCoverDest(CStateManager& mgr, float dt);
  void SetWallPointCoverDest(CStateManager& mgr, float dt);
  void SetExitHostDest(CStateManager& mgr, float dt);
  void SetLuredDest(CStateManager& mgr, float dt);
  void SetLuredPointDest(CStateManager& mgr, float dt);
  void StartRangedAttack(CStateManager& mgr, float dt);
  void EndRangedAttack(CStateManager& mgr, float dt);
  void SplatOntoMesh(CStateManager& mgr, float dt);
  void ExpelGrappledBall(CStateManager& mgr, float dt);

private:
  bool GetTargetAimPosition(CStateManager& mgr, CVector3f& position, float dt) const;
  CScriptAIHint* GetCoverHint(CStateManager& mgr) const;
  void UpdateLuringSafeZone(CStateManager& mgr, const CVector3f& position);
  void UpdateStateMachine(float dt, CStateManager& mgr); // Guessed name
  void UpdateTimers(float dt, CStateManager& mgr);       // Guessed name
  void UpdateBlobEffect(float dt, CStateManager& mgr);   // Guessed name
  void UpdateTargetable(CStateManager& mgr);             // Guessed name
  void UpdateSounds();                                   // Guessed name
  void UpdateTouchBounds();                              // Guessed name
  void UpdateLight(float dt, CStateManager& mgr);        // Guessed name
  void UpdateEchoSafeZone(CStateManager& mgr);           // Guessed name
  CAABox GetSwipeSegmentBounds(float radius, const CVector3f& a,
                               const CVector3f& b) const; // Guessed name
  void ApplySwipeDamage(CStateManager& mgr, const CSegId& shoulder, const CSegId& elbow,
                        const CSegId& forearm, const CSegId& wrist,
                        const CVector3f& direction);                        // Guessed name
  int RollDoubleSwipe(CStateManager& mgr);                                  // Guessed name
  void ApplySeparation(CStateManager& mgr);                                 // Guessed name
  bool FindSafeZoneFloorPoint(CStateManager& mgr, CVector3f& position);     // Guessed name
  void FaceSafeZoneOrTarget(CStateManager& mgr);                            // Guessed name
  void SpawnExitHostSmoke(CStateManager& mgr);                              // Guessed name
  void MoveAlongSurface(const CVector3f& direction, float speed, float dt); // Guessed name
  void StopSounds();                                                        // Guessed name
  void SpawnDamageEffect(CStateManager& mgr);                               // Guessed name
  void CollisionDamage(CStateManager& mgr, TUniqueId senderId);             // Guessed name
  void IngSpotHit(CStateManager& mgr, TUniqueId senderId);                  // Guessed name
  void HandleDamage(CStateManager& mgr, TUniqueId senderId);                // Guessed name
  void TouchDamage(CStateManager& mgr, TUniqueId senderId);                 // Guessed name
  void UpdateCollisionVulnerabilities(CStateManager& mgr);                  // Guessed name
  void SetupCollision(CStateManager& mgr);                                  // Guessed name
  void SpawnBlobEffect(CStateManager& mgr,
                       const TLockedToken< CGenDescription >& desc); // Guessed name
  void CreateLight(CStateManager& mgr);                              // Guessed name
  void SetInitialForm(CStateManager& mgr);                           // Guessed name
  void JoinTeam(CStateManager& mgr);                                 // Guessed name
  void LeaveTeam(CStateManager& mgr);                                // Guessed name
  void AssignCoverHint(CScriptAIHint& hint);                         // Guessed name
  void ReleaseCoverHint(CStateManager& mgr);                         // Guessed name

  SIngData mData;                                                                   // Guessed name
  EForm mForm;                                                                      // Guessed name
  EForm mNextForm;                                                                  // Guessed name
  CPathFindSearch mPathFindSearch;                                                  // Guessed name
  CPathFindPointSearch mPointSearch;                                                // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager;                // Guessed name
  CSurfaceAlignmentHelper mSurfaceAlignment;                                        // Guessed name
  CIngSpotPathFindNavigation mPointNavigation;                                      // Guessed name
  rstl::optional_object< CAABox > mTouchBounds;                                     // Guessed name
  TUniqueId mHostId;                                                                // Guessed name
  TUniqueId mPossessionEffectId;                                                    // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mPossessionHudEffect;    // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mExitHostSmokeEffect;    // Guessed name
  CSfxHandle mSfxHostInside;                                                        // Guessed name
  CSfxHandle mSfxGrapple;                                                           // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mIngSpotNormalHitEffect; // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mIngSpotHeavyHitEffect;  // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mIngSpotDeathEffect;     // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mMiniPortalEffect;       // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mSplatEffect;            // Guessed name
  CSfxHandle mSfxBodyProjectile;                                                    // Guessed name
  TUniqueId mBlobEffectId;                                                          // Guessed name
  CSfxHandle mSfxIngSpotIdle;                                                       // Guessed name
  CSfxHandle mSfxIngSpotMove;                                                       // Guessed name
  TUniqueId mLightId;                                                               // Guessed name
  TUniqueId mTeamManagerId;                                                         // Guessed name
  TUniqueId mTargetId;                                                              // Guessed name
  TUniqueId mLastTargetId;                                                          // Guessed name
  TUniqueId mPatrolWaypointId;                                                      // Guessed name
  TUniqueId mCoverHintId;                                                           // Guessed name
  TUniqueId mLastCoverHintId;                                                       // Guessed name
  TUniqueId mExitHostEffectId;                                                      // Guessed name
  TUniqueId mSafeZoneId;                                                            // Guessed name
  CSegId mCollarSegment;                                                            // Guessed name
  CSegId mHeadSegment;                                                              // Guessed name
  CSegId mRightShoulderSegment;                                                     // Guessed name
  CSegId mRightElbowSegment;                                                        // Guessed name
  CSegId mRightForearmSegment;                                                      // Guessed name
  CSegId mRightWristSegment;                                                        // Guessed name
  CSegId mLeftShoulderSegment;                                                      // Guessed name
  CSegId mLeftElbowSegment;                                                         // Guessed name
  CSegId mLeftForearmSegment;                                                       // Guessed name
  CSegId mLeftWristSegment;                                                         // Guessed name
  CLineOfSightTracker mLineOfSight;                                                 // Guessed name
  CVector3f mDestination;                                                           // Guessed name
  CVector3f mSplatNormal;                                                           // Guessed name
  CVector3f mMoveHeading;                                                           // Guessed name
  int mSwipeIndex;                                                                  // Guessed name
  CPlane mPortalPlane;                                                              // Guessed name
  int mMiniPortalCount;                                                             // Guessed name
  CVector3f mMiniPortalPositions[3];                                                // Guessed name
  int mMiniPortalIndex;                                                             // Guessed name
  int mSafeZoneCount;                                                               // Guessed name
  float mHeardShotTimer;                                                            // Guessed name
  float mUnderFireTimer;                                                            // Guessed name
  float mTimeSinceArmSwipe;                                                         // Guessed name
  float mFormChangeTimer;                                                           // Guessed name
  float mLocomotionTime;                                                            // Guessed name
  float mFrustrationTimer;                                                          // Guessed name
  float mGrappleCooldown;                                                           // Guessed name
  float mGrappleHold;                                                               // Guessed name
  float mLightIntensity;                                                            // Guessed name
  float mDeathDelayTimer;                                                           // Guessed name
  float mProjectileFlightTime;                                                      // Guessed name
  bool mShouldEvaporate : 1;                                                        // Guessed name
  bool mShouldTaunt : 1;
  bool mCanBodyProjectile : 1;
  bool mAggressive : 1;
  bool mFoundMovementPos : 1;
  bool mPathObstructed : 1;
  bool mSwipeDamagePending : 1;
  bool mTakeOffReceived : 1;
  bool mDrawModel : 1;
  bool mUsePortalPlane : 1;
  bool mBlobEffectActive : 1;
  bool mWallProjectileVisible : 1;
  bool mFollowingWaypoint : 1;
  bool mAlert : 1;
  bool mMovingOnSurface : 1;
  bool mGrappling : 1;
  bool mUseProjectileFSMEntry : 1;
  bool mProjectileSplat : 1;
  bool mIngSpotHurt : 1;
  bool mInHurtfulSafeZone : 1;
};
CHECK_SIZEOF(CIng, 0xe60)

#endif // _CING
