#ifndef _CSPLITTERMAINCHASSIS
#define _CSPLITTERMAINCHASSIS

#include "REL/REL_Setup.h"
#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplitterMainChassis.hpp"

#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CScriptTeamAiMgr;
class CSplitterCommandModule;

// Guessed struct: the tuned values of the Splitter main chassis, assembled by the script loader.
struct SSplitterMainChassisData {
  explicit SSplitterMainChassisData(const SLdrSplitterMainChassisData& data);

  CDamageInfo legStabDamage;                    // Guessed name
  CDamageInfo spinAttackDamage;                 // Guessed name
  CDamageVulnerability spinAttackVulnerability; // Guessed name
  SLdrSplitterMainChassisData data;             // Guessed name
};
CHECK_SIZEOF(SSplitterMainChassisData, 0x3bc)

// Guessed class: the walker body of the Splitter, which carries a detachable command module.
class REL_EXPORT CSplitterMainChassis : public CPatterned {
public:
  CSplitterMainChassis(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& modelData,
                       const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                       const SSplitterMainChassisData& data);

  // CEntity
  ~CSplitterMainChassis() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
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

  // CPatterned
  CProjectileInfo* ProjectileInfo() override { return nullptr; }
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  void SetupStateMachine(CStateManager& mgr) override;
  bool CanBeIngPossessed(CStateManager& mgr) const override;
  void SetIngPossessed(bool possessed, CStateManager& mgr) override;
  void SetIngPossessed(bool possessed, float duration, CStateManager& mgr) override;
  void SetAttackTarget(CStateManager& mgr, TUniqueId target) override;

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool IsIntact(CStateManager& mgr, const CTriggerData& data) const;
  bool IsDisabled(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldWaitForSnatch(CStateManager& mgr, const CTriggerData& data) const;
  bool ShotAt(CStateManager& mgr, const CTriggerData& data) const;
  bool LostHead(CStateManager& mgr, const CTriggerData& data) const;
  bool HasHead(CStateManager& mgr, const CTriggerData& data) const;
  bool HasRetreatPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool HasTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool CanReachTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PatrolPathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldLegStab(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMorphballStab(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpinAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldLaserSweep(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;
  bool IsSpinAttackOver(CStateManager& mgr, const CTriggerData& data) const;
  bool SpunIntoPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool SpunIntoGeometry(CStateManager& mgr, const CTriggerData& data) const;
  bool BreakOutOfSpin(CStateManager& mgr, const CTriggerData& data) const;
  bool TooManyBounces(CStateManager& mgr, const CTriggerData& data) const;
  bool DeployFromHoldingTube(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDeploy(CStateManager& mgr, const CTriggerData& data) const;
  bool DeployPathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PrepareForDocking(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void Idle(CStateManager& mgr, EStateMsg msg, float dt);
  void Inactive(CStateManager& mgr, EStateMsg msg, float dt);
  void Activate(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Scanning(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void LegStabAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void SpinAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt);
  void LaserSweepAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void SpinCollisionReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void SpinBounce(CStateManager& mgr, EStateMsg msg, float dt);
  void SpinTelegraph(CStateManager& mgr, EStateMsg msg, float dt);
  void SpinToIdle(CStateManager& mgr, EStateMsg msg, float dt);
  void SpawnDrop(CStateManager& mgr, EStateMsg msg, float dt);
  void HoldingTube(CStateManager& mgr, EStateMsg msg, float dt);
  void WaitForDocking(CStateManager& mgr, EStateMsg msg, float dt);
  void WaitForSnatch(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowDeployPath(CStateManager& mgr, EStateMsg msg, float dt);
  void DeploymentLanding(CStateManager& mgr, EStateMsg msg, float dt);
  void HeadExplosion(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void SelectTarget(CStateManager& mgr, float dt);
  void SetTargetDest(CStateManager& mgr, float dt);
  void SetRetreatDest(CStateManager& mgr, float dt);
  void SetupSpinAttack(CStateManager& mgr, float dt);
  void EndSpinAttack(CStateManager& mgr, float dt);
  void SetupLaserSweep(CStateManager& mgr, float dt);
  void SetMorphballStabLeg(CStateManager& mgr, float dt);
  void FindBestStabLeg(CStateManager& mgr, float dt);
  void FindBestDodgeDirection(CStateManager& mgr, float dt);
  void BounceOffPlayer(CStateManager& mgr, float dt);
  void BounceOffGeometry(CStateManager& mgr, float dt);

  // Called by the command module
  void AutoDestruct(float time);
  CVector3f GetAttachPosition() const;
  bool DockCommandModule(CStateManager& mgr, TUniqueId moduleId);
  void CancelDocking(TUniqueId moduleId);
  bool RequestDocking(TUniqueId moduleId);
  TUniqueId GetCommandModuleId() const { return mCommandModuleId; }
  TUniqueId GetTargetId() const { return mTargetId; }
  float GetScanTimer() const { return mScanTimer; }

private:
  void QuitTeam(CStateManager& mgr);
  void JoinTeam(CStateManager& mgr);
  void ResetAttackTimers();
  CVector3f CalculateSeparation(CStateManager& mgr);
  bool CanStepInDirection(CStateManager& mgr, const CVector3f& direction, float distance) const;
  pas::EStepDirection FindBestDodgeStep(CStateManager& mgr, const CVector3f& threatDirection);
  void UpdateAdditive(float dt);
  void MoveSpinning(float dt, const CVector3f& direction);
  void UpdateLocomotion(CStateManager& mgr);
  void SetCollisionVulnerabilities(CStateManager& mgr,
                                   const CDamageVulnerability& bodyVulnerability,
                                   const CDamageVulnerability& otherVulnerability);
  void HandleHitObject(CStateManager& mgr, const TUniqueId& collisionActorId);
  void HandleResistedDamage(CStateManager& mgr, const TUniqueId& collisionActorId);
  void HandleDamage(CStateManager& mgr, const TUniqueId& collisionActorId);
  void SyncCollisionActorHealth(CStateManager& mgr);
  void SetupCollisionActors(CStateManager& mgr);
  void SetLocomotionTypeFromFlags();
  void ReleasePlayerHint(CStateManager& mgr);
  void AcquirePlayerHint(CStateManager& mgr);
  void FindConnectedObjects(CStateManager& mgr);
  void UpdateCommandModule(float dt, CStateManager& mgr);
  void UpdateTimers(float dt, CStateManager& mgr);

  SSplitterMainChassisData mData;                                    // Guessed name
  CPathFindSearch mPathFindSearch;                                   // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  TUniqueId mCommandModuleId;                                        // Guessed name
  TUniqueId mDockingModuleId;                                        // Guessed name
  CLineOfSightTracker mLineOfSight;                                  // Guessed name
  float mTurnRate;                                                   // Guessed name
  int mAdditiveAnimation;                                            // Guessed name
  float mAdditiveWeight;                                             // Guessed name
  TUniqueId mSnatchId;                                               // Guessed name
  TUniqueId mTeamAiMgrId;                                            // Guessed name
  TUniqueId mTargetId;                                               // Guessed name
  TUniqueId mBodyCollisionId;                                        // Guessed name
  TUniqueId mPlayerHintId;                                           // Guessed name
  TUniqueId mWaypointId;                                             // Guessed name
  CSegId mAttachLocator;                                             // Guessed name
  CSegId mTargetLocator;                                             // Guessed name
  CVector3f mLandingPoint;                                           // Guessed name
  CVector3f mSpinVelocity;                                           // Guessed name
  CVector3f mCollisionNormal;                                        // Guessed name
  CVector3f mBounceDirection;                                        // Guessed name
  uint mBounceCount;                                                 // Guessed name
  int mDeployState;                                                  // Guessed name
  pas::EStepDirection mDodgeDirection;                               // Guessed name
  int mStabLeg;                                                      // Guessed name
  float mLocomotionRatio;                                            // Guessed name
  float mLegStabTimer;                                               // Guessed name
  float mSpinAttackTimer;                                            // Guessed name
  float mLaserSweepTimer;                                            // Guessed name
  float mDodgeTimer;                                                 // Guessed name
  float mAutoDestructTimer;                                          // Guessed name
  float mBounceTimer;                                                // Guessed name
  float mScanTimer;                                                  // Guessed name
  float mSpinTime;                                                   // Guessed name
  float mSpinElapsed;                                                // Guessed name
  float mTimeSinceShot;                                              // Guessed name
  float mStepDistance;                                               // Guessed name
  float mPathBlockedTime;                                            // Guessed name
  bool mSpinAttackAllowed : 1;                                       // Guessed name
  bool mDisabled : 1;                                                // Guessed name
  bool mHasDestination : 1;                                          // Guessed name
  bool mNearDestination : 1;                                         // Guessed name
  bool mSpunIntoPlayer : 1;                                          // Guessed name
  bool mSpunIntoGeometry : 1;                                        // Guessed name
  bool mBreakOutOfSpin : 1;                                          // Guessed name
  bool mLegStabActive : 1;                                           // Guessed name
  bool mSpinAttackActive : 1;                                        // Guessed name
  bool mLaserSweepActive : 1;                                        // Guessed name
  bool mTargetFar : 1;                                               // Guessed name
  bool mDodging : 1;                                                 // Guessed name
  bool mActivating : 1;                                              // Guessed name
  bool mShouldDeploy : 1;                                            // Guessed name
  bool mLostHead : 1;                                                // Guessed name
  bool mAutoDestructRequested : 1;                                   // Guessed name
};
CHECK_SIZEOF(CSplitterMainChassis, 0xd48)

#endif // _CSPLITTERMAINCHASSIS
