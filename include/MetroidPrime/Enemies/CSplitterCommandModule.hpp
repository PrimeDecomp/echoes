#ifndef _CSPLITTERCOMMANDMODULE
#define _CSPLITTERCOMMANDMODULE

#include "types.h"

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplitterCommandModule.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CParticleGenInfo;

// Guessed struct: the script loader's command module properties plus the converted damage values.
struct SSplitterCommandModuleData : public SLdrSplitterCommandModuleData {
  SSplitterCommandModuleData(const SLdrSplitterCommandModuleData& data);

  CDamageInfo laserPulseDamageInfo;                  // Guessed name
  CDamageInfo laserSweepDamageInfo;                  // Guessed name
  CDamageVulnerability lightShieldVulnerabilityData; // Guessed name
  CDamageVulnerability darkShieldVulnerabilityData;  // Guessed name
};
CHECK_SIZEOF(SSplitterCommandModuleData, 0x5ac)

// Guessed class: the detachable command module of the Splitter. It scans for the player, fires
// laser pulses and sweeps, shields itself and docks with the main chassis.
class CSplitterCommandModule : public CPatterned {
public:
  // Guessed names
  enum EVulnerabilityState {
    kVS_Normal,        // Uses the patterned vulnerability.
    kVS_Reflective,    // Reflects everything.
    kVS_ChassisShield, // Reflects everything; the aim point blends towards the main chassis.
    kVS_Invulnerable,  // Reflects everything and is not orbitable in any visor.
  };
  // Guessed names
  enum EShieldType { kST_None, kST_Light, kST_Dark };

  CSplitterCommandModule(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& modelData,
                         const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                         const SSplitterCommandModuleData& data);

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CProjectileInfo* ProjectileInfo() override { return &mLaserPulseProjectileInfo; }
  void SetupStateMachine(CStateManager& mgr) override;
  bool CanBeIngPossessed(CStateManager& mgr) const override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool IsListening() const override { return true; }
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CEntity
  ~CSplitterCommandModule() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CSplitterCommandModule
  void AutoDestruct(float time);
  CVector3f GetBeamPosition() const;                                    // Guessed name
  void RequestLaserSweep(const CVector3f& start, const CVector3f& end); // Guessed name
  void SetNormalState(CStateManager& mgr);                              // Guessed name
  void SetReflectiveState(CStateManager& mgr);                          // Guessed name
  void SetChassisShieldState(CStateManager& mgr);                       // Guessed name
  void SetInvulnerableState(CStateManager& mgr);                        // Guessed name
  TUniqueId GetMainChassisId() const { return mMainChassisId; }
  TUniqueId GetDockingTargetId() const { return mDockingTargetId; }
  const CVector3f& GetFacingDirection() const { return mFacingDirection; }
  bool IsLaserSweepActive() const { return mLaserSweepStep != -1; }

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool InDetectionRange(CStateManager& mgr, const CTriggerData& data) const;
  bool IsScanning(CStateManager& mgr, const CTriggerData& data) const;
  bool IsInitiallyDocked(CStateManager& mgr, const CTriggerData& data) const;
  bool IsDocked(CStateManager& mgr, const CTriggerData& data) const;
  bool HasDockingPath(CStateManager& mgr, const CTriggerData& data) const;
  bool HasDockingTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool DockingPathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool HasTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool InHoverRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InLaserPulseRange(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFireAgain(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldLaserSweep(CStateManager& mgr, const CTriggerData& data) const;
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void Idle(CStateManager& mgr, EStateMsg msg, float dt);
  void SpawnIdle(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void Scanning(CStateManager& mgr, EStateMsg msg, float dt);
  void FaceTarget(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void Hover(CStateManager& mgr, EStateMsg msg, float dt);
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt);
  void LaserPulse(CStateManager& mgr, EStateMsg msg, float dt);
  void LaserSweep(CStateManager& mgr, EStateMsg msg, float dt);
  void LostChassisReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void SeekMainChassis(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowDockingPath(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void NotifyDocking(CStateManager& mgr, float dt);
  void SelectTarget(CStateManager& mgr, float dt);
  void SetTargetDest(CStateManager& mgr, float dt);
  void SetDockingDest(CStateManager& mgr, float dt);
  void FindBestDodgeDirection(CStateManager& mgr, float dt);
  void RaiseShields(CStateManager& mgr, float dt);
  void ResetAttackTimes(CStateManager& mgr, float dt);

private:
  CTransform4f GetScanBeamTransform() const;                                        // Guessed name
  void UpdateScanBeam(CStateManager& mgr, float dt);                                // Guessed name
  void UpdateEffects(CStateManager& mgr);                                           // Guessed name
  void StopLaserSweep(CStateManager& mgr);                                          // Guessed name
  void UpdateLaserSweep(CStateManager& mgr, float dt);                              // Guessed name
  void StartLaserSweep(CStateManager& mgr, const CVector3f& target);                // Guessed name
  void FireLaserPulse(CStateManager& mgr, const rstl::string& locatorName);         // Guessed name
  void MoveTowards(const CVector3f& position, float dt);                            // Guessed name
  void UpdateCollisionTimer(float dt, CStateManager& mgr);                          // Guessed name
  void FindDockingTarget(CStateManager& mgr);                                       // Guessed name
  CVector3f GetSeparationVector(CStateManager& mgr);                                // Guessed name
  bool IsPathClear(CStateManager& mgr, const CVector3f& direction, float distance); // Guessed name
  pas::EStepDirection ChooseDodgeDirection(CStateManager& mgr);                     // Guessed name
  void LevelOut(float dt);                                                          // Guessed name
  CParticleGenInfo* GetShieldEffect();                                              // Guessed name
  void OnShieldHit(TUniqueId id);                                                   // Guessed name
  void UpdateShieldCollision(CStateManager& mgr, bool playSound);                   // Guessed name
  void UpdateShield(CStateManager& mgr);                                            // Guessed name
  void UpdateChassisLink(CStateManager& mgr);                                       // Guessed name
  void UpdateTimers(float dt, CStateManager& mgr);                                  // Guessed name
  void CreateCollisionActors(CStateManager& mgr);                                   // Guessed name
  void FindDockingTargetFromConnection(CStateManager& mgr);                         // Guessed name

  SSplitterCommandModuleData mData;                                  // Guessed name
  CPathFindSearch mPathFindSearch;                                   // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  CProjectileInfo mLaserPulseProjectileInfo;                         // Guessed name
  CProjectileInfo mLaserSweepProjectileInfo;                         // Guessed name
  CDamageVulnerability mDamageVulnerability;                         // Guessed name
  EVulnerabilityState mVulnerabilityState;                           // Guessed name
  EShieldType mShieldType;                                           // Guessed name
  EShieldType mLastShieldType;                                       // Guessed name
  float mStepDistance;                                               // Guessed name
  int mDodgeCount;                                                   // Guessed name
  float mSelfDestructTimer;                                          // Guessed name
  float mCollisionTime;                                              // Guessed name
  CSegId mBeamLocator;                                               // Guessed name
  TUniqueId mMainChassisId;                                          // Guessed name
  TUniqueId mDockingTargetId;                                        // Guessed name
  TUniqueId mTargetId;                                               // Guessed name
  TUniqueId mShieldCollisionActorId;                                 // Guessed name
  TUniqueId mLaserSweepProjectileId;                                 // Guessed name
  CColor mFlashColor;                                                // Guessed name
  float mFlashTimer;                                                 // Guessed name
  float mAttackTimer;                                                // Guessed name
  float mShieldTimer;                                                // Guessed name
  float mDockSearchTimer;                                            // Guessed name
  float mTimeInState;                                                // Guessed name
  float mTimeSinceChassisShield;                                     // Guessed name
  int mShotsRemaining;                                               // Guessed name
  CVector3f mFacingDirection;                                        // Guessed name
  pas::EStepDirection mDodgeDirection;                               // Guessed name
  int mLaserSweepStep;                                               // Guessed name
  CVector3f mLaserSweepStart;                                        // Guessed name
  CVector3f mLaserSweepEnd;                                          // Guessed name
  CVector3f mLaserSweepDirection;                                    // Guessed name
  CSfxHandle mLaserSweepSfx;                                         // Guessed name
  CSfxHandle mShieldSfx;                                             // Guessed name
  CSfxHandle mScanBeamSfx;                                           // Guessed name
  TUniqueId mScanBeamEffectId;                                       // Guessed name
  bool mHasDestination : 1;                                          // Guessed name
  bool mPathObstructed : 1;                                          // Guessed name
  bool mLevelingOut : 1;                                             // Guessed name
  bool mCanBreakLockOn : 1;                                          // Guessed name
  bool mShieldsRaised : 1;                                           // Guessed name
  bool mAutoDestructing : 1;                                         // Guessed name
  bool mFirstDocking : 1;                                            // Guessed name
  bool mCollided : 1;                                                // Guessed name
};
CHECK_SIZEOF(CSplitterCommandModule, 0xf70)

CEntity* REL_LoadSplitterCommandModule(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _CSPLITTERCOMMANDMODULE
