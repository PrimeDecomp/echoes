#ifndef _CPILLBUG
#define _CPILLBUG

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CWallCrawler.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

// Guessed class: a wall-crawling bug that patrols a waypoint path, rolls up when hurt and falls
// when it loses its footing.
class CPillBug : public CWallCrawler {
public:
  enum EMode { kM_Crawl, kM_Wander, kM_Injured }; // Guessed names

  CPillBug(TUniqueId uid, const rstl::string& name, EFlavorType flavor, CEntityInfo& info,
           const CTransform4f& xf, const CModelData& modelData, const CPatternedInfo& patternedInfo,
           const CActorParameters& actorParams, int planarConstraint,
           const CDamageVulnerability& damageVulnerability,
           const CDamageVulnerability& wanderVulnerability, float crawlRadius, float rollRadius,
           float floorTurnSpeed, float stickRadius, float waypointApproachDistance,
           float visibleDistance, float unknown_0x519c7197, float collisionLookAheadTime,
           float forwardPriority, float unknown_0x558c0692, float unknown_0x0f991bf1,
           float unknown_0x385a1bed, float unknown_0xcf4ea141);

  // CEntity
  ~CPillBug() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  bool IsOnStaticGround() const override;

  // CAi
  const CDamageVulnerability* GetDamageVulnerability() const override;

  // CPatterned
  void ThinkAboutMove(float dt) override;
  CDamageInfo GetContactDamage() const override;
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsOnGround() const override;

  // CWallCrawler
  TUniqueId GetNextWaypoint(CStateManager& mgr, const CScriptWaypoint* waypoint,
                            bool reverse) override;

  // CPillBug
  void Pause(CStateManager& mgr, EStateMsg msg, float dt);
  void Wander(CStateManager& mgr, EStateMsg msg, float dt);
  void Turn(CStateManager& mgr, EStateMsg msg, float dt);
  void Injured(CStateManager& mgr, EStateMsg msg, float dt);
  void Fall(CStateManager& mgr, EStateMsg msg, float dt);
  void ReturnToPath(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void FastPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool Bombed(CStateManager& mgr, const CTriggerData& data) const;
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool CloseToPath(CStateManager& mgr, const CTriggerData& data) const;
  bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  bool HitPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool HasReturnPath(CStateManager& mgr, const CTriggerData& data) const;

private:
  void PatrolImpl(CStateManager& mgr, EStateMsg msg, float dt, bool fast); // Guessed name
  int GetClosestPathIndex(CStateManager& mgr) const;                       // Guessed name
  TUniqueId FindReturnWaypoint(CStateManager& mgr) const;                  // Guessed name
  bool HasInnerPathLoop(CStateManager& mgr) const;                         // Guessed name
  void BuildPath(CStateManager& mgr);                                      // Guessed name
  int GetCurrentConstraint() const;                                        // Guessed name

  TUniqueId mCurrentWaypointId;                            // Guessed name
  float mCrawlRadius;                                      // Guessed name
  float mRollRadius;                                       // Guessed name
  float mCollisionLookAheadTime;                           // Guessed name
  float mForwardPriority;                                  // Guessed name
  int mPlanarConstraint;                                   // Guessed name
  CDamageVulnerability mDamageVulnerability;               // Guessed name
  CDamageVulnerability mWanderVulnerability;               // Guessed name
  rstl::vector< TUniqueId > mPath;                         // Guessed name
  int mAnimSubState;                                       // Guessed name
  EMode mMode;                                             // Guessed name
  CVector3f x8f8_;                                         //
  CVector3f mTurnFaceDirection;                            // Guessed name
  rstl::reserved_vector< CVector3f, 24 > mPositionHistory; // Guessed name
  bool mAttacked : 1;                                      // Guessed name
  bool mBombed : 1;                                        // Guessed name
  bool mPathOver : 1;                                      // Guessed name
  bool mPathForward : 1;                                   // Guessed name
  bool mPathLoops : 1;                                     // Guessed name
  bool mLanded : 1;                                        // Guessed name
  bool mTouchingStaticGround : 1;                          // Guessed name
  bool mHitPlayer : 1;                                     // Guessed name
  bool mReturningToPath : 1;                               // Guessed name
};
CHECK_SIZEOF(CPillBug, 0xa38)

#endif // _CPILLBUG
