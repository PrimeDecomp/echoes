#ifndef _CSHRIEKER
#define _CSHRIEKER

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CElementGen;
class CGenDescription;

// Guessed class: the tunable properties of a Shrieker.
class CShriekerData {
public:
  CShriekerData(float detectionHeight, float rustleDetectionRadius, float popDetectionRadius,
                float morphballDetectionRadius, CAssetId shriekEffect,
                const CDamageInfo& shriekDamage, CAssetId projectile,
                const CDamageInfo& projectileDamage, uchar combatVisorMaxVolume,
                uchar echoVisorMaxVolume, CAssetId meleeEffect, const CDamageInfo& meleeDamage,
                float meleeRange, float meleeAverageAttackTime, float meleeAttackTimeVariation,
                const CDamageVulnerability& buriedVulnerability, float hostileAccumulatePriority,
                float hoverHeight, const CVector3f& missileDeflectionOffset,
                float missileDeflectionRadius, float missileDeflectRate,
                ushort missileDeflectionSound, float dodgeTime, float dodgePercentage,
                float visibilityChangeTime);

  float mDetectionHeight;          // Guessed name
  float mRustleDetectionRadius;    // Guessed name
  float mPopDetectionRadius;       // Guessed name
  float mMorphballDetectionRadius; // Guessed name
  CAssetId mShriekEffect;          // Guessed name
  CDamageInfo mShriekDamage;       // Guessed name
  CAssetId mProjectile;            // Guessed name
  CDamageInfo mProjectileDamage;   // Guessed name
  ushort x50_;
  uchar mCombatVisorMaxVolume;               // Guessed name
  uchar mEchoVisorMaxVolume;                 // Guessed name
  CAssetId mMeleeEffect;                     // Guessed name
  CDamageInfo mMeleeDamage;                  // Guessed name
  float mMeleeRangeSquared;                  // Guessed name
  float mMeleeAverageAttackTime;             // Guessed name
  float mMeleeAttackTimeVariation;           // Guessed name
  CDamageVulnerability mBuriedVulnerability; // Guessed name
  float mHostileAccumulatePriority;          // Guessed name
  float mHoverHeight;                        // Guessed name
  CVector3f mMissileDeflectionOffset;        // Guessed name
  float mMissileDeflectionRadius;            // Guessed name
  float mMissileDeflectRate;                 // Guessed name
  ushort mMissileDeflectionSound;            // Guessed name
  float mDodgeTime;                          // Guessed name
  float mDodgePercentage;                    // Guessed name
  float mVisibilityChangeTime;               // Guessed name
};
CHECK_SIZEOF(CShriekerData, 0xdc)

// Guessed class: a burrowed enemy that rises from the ground to shriek, fire and bite.
class CShrieker : public CPatterned {
public:
  CShrieker(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& modelData,
            const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
            const CShriekerData& data);

  // CEntity
  ~CShrieker() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool IsListening() const override { return true; }

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  void SetupStateMachine(CStateManager& mgr) override;

  // CShrieker
  void Buried(CStateManager& mgr, EStateMsg msg, float dt);
  void BuriedRumbling(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  void UprootShriek(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void ShriekAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Bury(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void ReturnToStart(CStateManager& mgr, EStateMsg msg, float dt);
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt);
  void Melee(CStateManager& mgr, EStateMsg msg, float dt);
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool PlayerEntersProximity(CStateManager& mgr, const CTriggerData& data) const;
  bool PlayerLeavesProximity(CStateManager& mgr, const CTriggerData& data) const;
  bool EnterUprootShriek(CStateManager& mgr, const CTriggerData& data) const;
  bool HasPatrolPath(CStateManager& mgr, const CTriggerData& data) const;
  bool PlayerLeashReached(CStateManager& mgr, const CTriggerData& data) const;
  bool LeavePatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool CloseToStart(CStateManager& mgr, const CTriggerData& data) const;
  bool LineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMelee(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;
  bool InMaxRange(CStateManager& mgr, const CTriggerData& data) const;

private:
  void ResetAttackTimer(CStateManager& mgr, int type);             // Guessed name
  void ApplySeparation(CStateManager& mgr);                        // Guessed name
  void SetTeamMemberTarget(CStateManager& mgr);                    // Guessed name
  void EndTeamAttack(CStateManager& mgr, bool projectile);         // Guessed name
  void QuitTeam(CStateManager& mgr);                               // Guessed name
  void JoinTeam(CStateManager& mgr);                               // Guessed name
  CVector3f GetTranslationCopy() const;                            // Guessed name
  void SetBuriedCollision(CStateManager& mgr, bool buried);        // Guessed name
  bool GetHeightAboveMesh(float& height) const;                    // Guessed name
  void FaceTarget(CStateManager& mgr, float dt);                   // Guessed name
  void DeflectMissiles(CStateManager& mgr);                        // Guessed name
  void DetectPlayer(CStateManager& mgr);                           // Guessed name
  void ApplyMeleeDamage(CStateManager& mgr);                       // Guessed name
  bool IsFacingTarget(CStateManager& mgr) const;                   // Guessed name
  bool UpdateTargetPosition(CStateManager& mgr);                   // Guessed name
  void FindTarget(CStateManager& mgr);                             // Guessed name
  void UpdateValidTarget(CStateManager& mgr);                      // Guessed name
  void FadeAlpha(CStateManager& mgr, bool fadeIn, bool immediate); // Guessed name
  void ReactToDamage(CStateManager& mgr, TUniqueId senderId);      // Guessed name
  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd); // Guessed name

  CShriekerData mData;                                 // Guessed name
  CProjectileInfo mProjectileInfo;                     // Guessed name
  CPathFindSearch mPathFindSearch;                     // Guessed name
  CVector3f mBuriedPosition;                           // Guessed name
  CVector3f mStartPosition;                            // Guessed name
  CLineOfSightTracker mLineOfSightTracker;             // Guessed name
  bool mScriptedProximity : 1;                         // Guessed name
  bool mScriptedAlert : 1;                             // Guessed name
  bool mHasPatrolPath : 1;                             // Guessed name
  bool mBuried : 1;                                    // Guessed name
  bool mChasing : 1;                                   // Guessed name
  bool mPlayerInProximity : 1;                         // Guessed name
  bool mShriekTriggered : 1;                           // Guessed name
  bool mDodgeRight : 1;                                // Guessed name
  bool mDying : 1;                                     // Guessed name
  bool mLaunched : 1;                                  // Guessed name
  bool mSeparating : 1;                                // Guessed name
  bool mDeathKnockBackStarted : 1;                     // Guessed name
  float mReferenceHealth;                              // Guessed name
  float mPreviousHealth;                               // Guessed name
  float mFireTimer;                                    // Guessed name
  float mMeleeTimer;                                   // Guessed name
  float mDodgeTimer;                                   // Guessed name
  float mChaseTimer;                                   // Guessed name
  float mCheckRadius;                                  // Guessed name
  float mDetectionRadius;                              // Guessed name
  TUniqueId mTargetId;                                 // Guessed name
  CVector3f mTargetPos;                                // Guessed name
  rstl::list< TUniqueId > mDamagedIds;                 // Guessed name
  rstl::reserved_vector< TUniqueId, 6 > mDeflectedIds; // Guessed name
  TUniqueId mTeamAiMgrId;                              // Guessed name
  CSfxHandle mDeflectSfx;                              // Guessed name
  float mDeflectSfxTimer;                              // Guessed name
  CSegId mRootSegId;                                   // Guessed name
  CSegId mHoverSegId;                                  // Guessed name
  rstl::reserved_vector< rstl::optional_object< TLockedToken< CGenDescription > >, 2 >
      mEffectDescs;                                                      // Guessed name
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 > mEffectGens; // Guessed name
};
CHECK_SIZEOF(CShrieker, 0xab0)

#endif // _CSHRIEKER
