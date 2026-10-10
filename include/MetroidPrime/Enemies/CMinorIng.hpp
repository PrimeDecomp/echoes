#ifndef _CMINORING
#define _CMINORING

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CIngSpotData.hpp"
#include "MetroidPrime/Enemies/CIngSpotPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CGenDescription;
class CScriptAIHint;
class CCollisionActor;
class CWeapon;

// Guessed struct: the tuned values of a Minor Ing, assembled by the script loader.
struct SMinorIngData {
  SMinorIngData(CAssetId projectile, const CDamageInfo& projectileDamage,
                const CDamageInfo& ingSpotDamage, const CIngSpotData& ingSpot,
                const CHealthInfo& health, const CBouncyGrenadeData& grenade, int minGeneration,
                int maxGeneration, float unknown_0xa03e450c, float speedModifier,
                float morphballPursuitDistance, float bombStunDuration, float unknown_0x7569fdba,
                float sfxFallOff, float minLaunchSpeed, float maxLaunchSpeed, float maxTurnAngle,
                float unknown_0xfbf8ea0a, float unknown_0x47f99fbc, float attackAngleLimit,
                float lineOfSightHeightOffset, float unknown_0xd6c8eac2, float unknown_0x2a5449ba,
                float hearingRadius, bool allowLockOn, bool allowPuddleLockOn,
                bool unknown_0x09207f51, bool allowProjectileDuringAttackPattern,
                bool unknown_0xbce16644, bool unknown_0x142433d3, bool stayOnPointPathFinding,
                bool allowProjectileDuringStayOnPointPathFinding)
  : projectile(projectile)
  , projectileDamage(projectileDamage)
  , unknown_0xa03e450c(unknown_0xa03e450c)
  , morphballPursuitDistance(morphballPursuitDistance)
  , ingSpotDamage(ingSpotDamage)
  , bombStunDuration(bombStunDuration)
  , unknown_0x7569fdba(unknown_0x7569fdba)
  , sfxFallOff(sfxFallOff)
  , ingSpot(ingSpot)
  , health(health)
  , grenade(grenade)
  , minLaunchSpeed(minLaunchSpeed)
  , maxLaunchSpeed(maxLaunchSpeed)
  , minGeneration(minGeneration)
  , maxGeneration(maxGeneration)
  , maxTurnAngle(maxTurnAngle)
  , unknown_0xfbf8ea0a(unknown_0xfbf8ea0a)
  , unknown_0x47f99fbc(unknown_0x47f99fbc)
  , attackAngleLimit(attackAngleLimit)
  , lineOfSightHeightOffset(lineOfSightHeightOffset)
  , unknown_0xd6c8eac2(unknown_0xd6c8eac2)
  , unknown_0x2a5449ba(unknown_0x2a5449ba)
  , hearingRadius(hearingRadius)
  , speedModifier(speedModifier)
  , allowLockOn(allowLockOn)
  , unknown_0x09207f51(unknown_0x09207f51)
  , allowProjectileDuringAttackPattern(allowProjectileDuringAttackPattern)
  , unknown_0xbce16644(unknown_0xbce16644)
  , unknown_0x142433d3(unknown_0x142433d3)
  , stayOnPointPathFinding(stayOnPointPathFinding)
  , allowProjectileDuringStayOnPointPathFinding(allowProjectileDuringStayOnPointPathFinding)
  , allowPuddleLockOn(allowPuddleLockOn) {}

  CAssetId projectile;
  CDamageInfo projectileDamage;
  float unknown_0xa03e450c;
  float morphballPursuitDistance;
  CDamageInfo ingSpotDamage;
  float bombStunDuration;
  float unknown_0x7569fdba;
  float sfxFallOff;
  CIngSpotData ingSpot;
  CHealthInfo health;
  CBouncyGrenadeData grenade;
  float minLaunchSpeed;
  float maxLaunchSpeed;
  int minGeneration;
  int maxGeneration;
  float maxTurnAngle;
  float unknown_0xfbf8ea0a;
  float unknown_0x47f99fbc;
  float attackAngleLimit;
  float lineOfSightHeightOffset;
  float unknown_0xd6c8eac2;
  float unknown_0x2a5449ba;
  float hearingRadius;
  float speedModifier;
  bool allowLockOn : 1;
  bool unknown_0x09207f51;
  bool allowProjectileDuringAttackPattern : 1;
  bool unknown_0xbce16644 : 1;
  bool unknown_0x142433d3 : 1;
  bool stayOnPointPathFinding : 1;
  bool allowProjectileDuringStayOnPointPathFinding : 1;
  bool allowPuddleLockOn : 1;
};
CHECK_SIZEOF(SMinorIngData, 0x158)

// Guessed class: the Minor Ing, an Ing that slides across the ground as a puddle, rises into a
// corporeal form to attack and hides in safe zones.
class CMinorIng : public CPatterned {
public:
  // Guessed names
  enum EForm {
    kF_Invalid = -1,
    kF_Corporeal,
    kF_Puddle,
    kF_IntoCorporeal,
    kF_IntoPuddle,
  };

  CMinorIng(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& modelData,
            const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
            const SMinorIngData& data);

  // CEntity
  ~CMinorIng() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  CTransform4f GetPrimitiveTransform() const override;

  // CAi
  bool IsListening() const override { return true; }
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CPathFindPointSearch* GetPointSearchPath() override { return &mPointSearch; }
  CProjectileInfo* ProjectileInfo() override { return &mProjectileInfo; }
  void MassiveDeath(CStateManager& mgr) override;
  void SetupStateMachine(CStateManager& mgr) override;

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPathFind(CStateManager& mgr, const CTriggerData& data) const;
  bool CancelAnim(CStateManager& mgr, const CTriggerData& data) const;
  bool StunOver(CStateManager& mgr, const CTriggerData& data) const;
  bool FoundMovementPos(CStateManager& mgr, const CTriggerData& data) const;
  bool FoundPointPath(CStateManager& mgr, const CTriggerData& data) const;
  bool IsPointPathFinding(CStateManager& mgr, const CTriggerData& data) const;
  bool PointPathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool HasCoverPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool TargetInSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool IsLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool AllowProjectileDuringStayOnPointPathFinding(CStateManager& mgr,
                                                   const CTriggerData& data) const;
  bool StayOnPointPathFinding(CStateManager& mgr, const CTriggerData& data) const;
  bool IsInCorporealForm(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldEvaporate(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMoveUnderBall(CStateManager& mgr, const CTriggerData& data) const;
  bool IsBeingKnockedback(CStateManager& mgr, const CTriggerData& data) const;
  bool CancelStun(CStateManager& mgr, const CTriggerData& data) const;
  bool PuddleHitByWeapon(CStateManager& mgr, const CTriggerData& data) const;
  bool InMaxRange(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  bool LineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool LeavePatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool PlayerLeashReached(CStateManager& mgr, const CTriggerData& data) const;
  bool HasPatrolPath(CStateManager& mgr, const CTriggerData& data) const;
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFollowingAttackPath(CStateManager& mgr, const CTriggerData& data) const;
  bool InsideSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool RecalculatePath(CStateManager& mgr, const CTriggerData& data) const;
  bool AllowAttackPatternStun(CStateManager& mgr, const CTriggerData& data) const;
  bool AllowAttackPatternProjectile(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const;
  bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;
  bool IsMoveAwayFromSafeZoneOver(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void PointPathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void Stunned(CStateManager& mgr, EStateMsg msg, float dt);
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void SetWallPointCoverDest(CStateManager& mgr, EStateMsg msg, float dt);
  void SetCoverDest(CStateManager& mgr, EStateMsg msg, float dt);
  void IntoPuddleForm(CStateManager& mgr, EStateMsg msg, float dt);
  void IntoCorporealForm(CStateManager& mgr, EStateMsg msg, float dt);
  void Idle(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt);
  void MoveAwayFromSafeZone(CStateManager& mgr, EStateMsg msg, float dt);
  void AlwaysPointPathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void WaitForFSMTransition(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void SelectTarget(CStateManager& mgr, float dt);
  void FindCoverPoint(CStateManager& mgr, float dt);
  void SetPointCoverDest(CStateManager& mgr, float dt);
  void SetLuredDest(CStateManager& mgr, float dt);
  void SetLuredPointDest(CStateManager& mgr, float dt);

private:
  SMinorIngData mData;                                                       // Guessed name
  CProjectileInfo mProjectileInfo;                                           // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mNormalHitEffect; // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mHeavyHitEffect;  // Guessed name
  CPathFindSearch mPathFindSearch;                                           // Guessed name
  CPathFindPointSearch mPointSearch;                                         // Guessed name
  CSurfaceAlignmentHelper mSurfaceAlignment;                                 // Guessed name
  CIngSpotPathFindNavigation mPointNavigation;                               // Guessed name
  CVector3f mUnknown0xc40;                                                   // Guessed name
  rstl::optional_object< CAABox > mPuddleBounds;                             // Guessed name
  rstl::optional_object< CAABox > mBodyBounds;                               // Guessed name
  CLineOfSightTracker mLineOfSight;                                          // Guessed name
  int mForm;                                                                 // Guessed name
  TUniqueId mTeamManagerId;                                                  // Guessed name
  TUniqueId mTargetId;                                                       // Guessed name
  TUniqueId mCoverPointId;                                                   // Guessed name
  TUniqueId mPreviousCoverPointId;                                           // Guessed name
  TUniqueId mUniqueIdCd4;                                                    // Guessed name
  TUniqueId mUniqueIdCd6;                                                    // Guessed name
  TUniqueId mUniqueIdCd8;                                                    // Guessed name
  TUniqueId mSafeZoneId;                                                     // Guessed name
  TUniqueId mBlobEffectId;                                                   // Guessed name
  TUniqueId mCollisionActorId;                                               // Guessed name
  rstl::reserved_vector< CSfxHandle, 2 > mSfxHandles;                        // Guessed name
  CVector3f mVectorCec;                                                      // Guessed name
  CVector3f mScale;                                                          // Guessed name
  float mFloatD04;
  float mFloatD08;
  float mFloatD0c;
  float mFloatD10;
  float mFloatD14;
  float mFloatD18;
  float mFloatD1c;
  float mFloatD20;
  float mFloatD24;
  int mIntD28;
  int mCorporealFormAnim;
  int mStunnedAnim;
  float mFloatD34;
  float mFloatD38;
  CSegId mLockOnSegment;
  bool mFlagD3d0 : 1;
  bool mFlagD3d1 : 1;
  bool mFlagD3d2 : 1;
  bool mFlagD3d3 : 1;
  bool mFlagD3d4 : 1;
  bool mFlagD3d5 : 1;
  bool mFlagD3d6 : 1;
  bool mFlagD3d7 : 1;
  bool mFlagD3e0 : 1;
  bool mFlagD3e1 : 1;
  bool mFlagD3e2 : 1;
  bool mFlagD3e3 : 1;
  bool mFlagD3e4 : 1;
  bool mFlagD3e5 : 1;
  bool mFlagD3e6 : 1;
};
CHECK_SIZEOF(CMinorIng, 0xd40)

#endif // _CMINORING
