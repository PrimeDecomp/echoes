#ifndef _CREZBIT
#define _CREZBIT

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRezbit.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CGenDescription;
class CSkinnedModel;

// Guessed class: a flying virus-infected enemy that can de-rez, raise shields and fire lasers.
class CRezbit : public CPatterned {
public:
  CRezbit(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
          const CModelData& modelData, const CActorParameters& actorParams,
          const CPatternedInfo& patternedInfo, const SLdrRezbitData& data);

  // CEntity
  ~CRezbit() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CProjectileInfo* ProjectileInfo() override;
  void SetupStateMachine(CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;
  bool IsListening() const override { return true; }

  // CRezbit
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void Idle(CStateManager& mgr, EStateMsg msg, float dt);
  void Alert(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowAttackPath(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void Strafe(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeRezzed(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeDeRezzed(CStateManager& mgr, EStateMsg msg, float dt);
  void CuttingLaserAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void EnergyBoltAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void VirusAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void SelectTarget(CStateManager& mgr, float dt);
  void SelectStrafeDirection(CStateManager& mgr, float dt);
  void SetTargetDest(CStateManager& mgr, float dt);
  void ResetAttackTimes(CStateManager& mgr, float dt);
  void RaiseShields(CStateManager& mgr, float dt);
  void SetNormalRezState(CStateManager& mgr, float dt);
  void SetDeRezzedState(CStateManager& mgr, float dt);
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool IsDeRezzed(CStateManager& mgr, const CTriggerData& data) const;
  bool IsAlert(CStateManager& mgr, const CTriggerData& data) const;
  bool HasAttackPath(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackPathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBecomeRezzed(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBecomeDeRezzed(CStateManager& mgr, const CTriggerData& data) const;
  bool HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldStrafe(CStateManager& mgr, const CTriggerData& data) const;
  bool FoundStrafeDirection(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldLaserAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldEnergyBoltAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldVirusAttack(CStateManager& mgr, const CTriggerData& data) const;

private:
  void QuitTeam(CStateManager& mgr); // Guessed name
  void JoinTeam(CStateManager& mgr); // Guessed name
  void CheckVerticalClearance(CStateManager& mgr,
                              rstl::reserved_vector< bool, 4 >& available); // Guessed name
  void CheckNeighbors(CStateManager& mgr,
                      rstl::reserved_vector< bool, 4 >& available);               // Guessed name
  int FindStrafeDirection(CStateManager& mgr);                                    // Guessed name
  bool CanStrafe(CStateManager& mgr, float distance, const CVector3f& direction); // Guessed name
  void UpdateSeparation(CStateManager& mgr);                                      // Guessed name
  void UpdateDeflectSfx(float dt);                                                // Guessed name
  void DeflectMissiles(CStateManager& mgr);                                       // Guessed name
  void StartVirusAttack(CStateManager& mgr, const rstl::string& locator);         // Guessed name
  void FireEnergyBolt(CStateManager& mgr, float dt);                              // Guessed name
  void DeleteLaser(CStateManager& mgr);                                           // Guessed name
  void UpdateLaser(CStateManager& mgr, float dt);                                 // Guessed name
  void SpawnLaser(CStateManager& mgr, float dt);                                  // Guessed name
  bool SetupLaserTargets(CStateManager& mgr);                                     // Guessed name
  void UpdatePatrolMovement();                                                    // Guessed name
  void BreakShield(CStateManager& mgr);                                           // Guessed name
  void LowerShields(CStateManager& mgr);                                          // Guessed name
  void UpdateWobble(CStateManager& mgr);                                          // Guessed name
  void UpdateShield(CStateManager& mgr, float dt);                                // Guessed name
  void UpdateTimers(float dt);                                                    // Guessed name
  void CheckShieldBroken(CStateManager& mgr, TUniqueId id);                       // Guessed name
  void HandleShieldHit(CStateManager& mgr, const TUniqueId& id);                  // Guessed name
  void CreateShieldCollisionActor(CStateManager& mgr);                            // Guessed name
  void BuildDerezModel(CAssetId model, CAssetId skinRules);                       // Guessed name

  SLdrRezbitData mData;                                                          // Guessed name
  CPathFindSearch mPathFindSearch;                                               // Guessed name
  int mRezState;                                                                 // Guessed name
  CLineOfSightTracker mLineOfSight;                                              // Guessed name
  TUniqueId mShieldActorId;                                                      // Guessed name
  CSegId mShieldBottomLeftSeg;                                                   // Guessed name
  CSegId mShieldTopRightSeg;                                                     // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mShieldExplodeEffect; // Guessed name
  CSfxHandle mShieldSfx;                                                         // Guessed name
  rstl::reserved_vector< TUniqueId, 5 > mDeflectedMissiles;                      // Guessed name
  CSfxHandle mDeflectSfx;                                                        // Guessed name
  float mDeflectSfxTime;                                                         // Guessed name
  rstl::optional_object< TLockedToken< CSkinnedModel > > mDerezModel;            // Guessed name
  float mDerezTimer;                                                             // Guessed name
  float mShieldTimer;                                                            // Guessed name
  float mDerezHealth;                                                            // Guessed name
  float mAlertDelay;                                                             // Guessed name
  int x0c40_;
  CVector3f mStrafeDirections[4];    // Guessed name
  float mStrafeTimer;                // Guessed name
  float mStrafeDistance;             // Guessed name
  int mStrafeIndex;                  // Guessed name
  TUniqueId mTeamAiMgrId;            // Guessed name
  TUniqueId mTargetId;               // Guessed name
  int mAttackKind;                   // Guessed name
  CTransform4f mWobbleTransform;     // Guessed name
  float mAttackTimer;                // Guessed name
  int mAttackSelection;              // Guessed name
  CProjectileInfo mCuttingLaserInfo; // Guessed name
  TUniqueId mLaserId;                // Guessed name
  CSfxHandle mLaserSfx;              // Guessed name
  CVector3f mLaserStart;             // Guessed name
  CVector3f mLaserEnd;               // Guessed name
  float mLaserDuration;              // Guessed name
  float mLaserElapsed;               // Guessed name
  CProjectileInfo mEnergyBoltInfo;   // Guessed name
  float mBoltDuration;               // Guessed name
  float mBoltTimer;                  // Guessed name
  int mBoltCount;                    // Guessed name
  CVector3f mLastBoltDirection;      // Guessed name
  float mVirusTimer;                 // Guessed name
  CDamageInfo mVirusDamage;          // Guessed name
  bool mAlert : 1;                   // Guessed name
  bool mHasPathDestination : 1;      // Guessed name
  bool mShieldBroken : 1;            // Guessed name
};
CHECK_SIZEOF(CRezbit, 0xd78)

#endif // _CREZBIT
