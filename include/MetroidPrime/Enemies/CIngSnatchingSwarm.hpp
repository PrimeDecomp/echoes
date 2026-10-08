#ifndef _CINGSNATCHINGSWARM
#define _CINGSNATCHINGSWARM

#include "types.h"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGenericFSM2State.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CGenDescription;

// Guessed name: the swarm tuning values the loader hands to the constructor.
struct SIngSnatchingSwarmData {
  SIngSnatchingSwarmData(CAssetId stateMachine, CAssetId swarmParticle, CAssetId secondaryParticle,
                         float loiterGeneratorRate, float trailDelayScale, float lifetime,
                         float maxLinearSpeed, float maxLinearAcceleration, float maxTurnSpeed,
                         bool useSteering, bool ignorePlayer, float unknown0xe6b57a25,
                         float exitPortalDistance, float loiterTime, float loiterTimeVariation,
                         float arcJitter, float beginSnatchingRange, CAssetId explosionEffect,
                         const CDamageInfo& impactDamage, ushort impactSound, ushort idleSound,
                         ushort moveSound, float health, const CDamageVulnerability& vulnerability);

  CAssetId mStateMachine;
  CAssetId mSwarmParticle;
  CAssetId mSecondaryParticle;
  float mTrailDelayScale;
  float mLoiterGeneratorRate;
  float mLifetime;
  float mMaxLinearSpeed;
  float mMaxLinearAcceleration;
  float mMaxTurnSpeed;
  float mUnknown0xe6b57a25;
  float mExitPortalDistance;
  float mLoiterTime;
  float mLoiterTimeVariation;
  float mArcJitter;
  float mBeginSnatchingRangeSq;
  CAssetId mExplosionEffect;
  CDamageInfo mImpactDamage;
  ushort mImpactSound;
  ushort mIdleSound;
  ushort mMoveSound;
  float mHealth;
  CDamageVulnerability mVulnerability;
  bool mUseSteeringForMovement : 1;
  bool mIgnorePlayer : 1;
};
CHECK_SIZEOF(SIngSnatchingSwarmData, 0x9c)

// Guessed class: a swarm of Ing that loiters near its portal, then flies along a curved path
// to a target and snatches it. It is driven by an FSM2 state machine.
class CIngSnatchingSwarm : public CActor {
public:
  CIngSnatchingSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CVector3f& scale,
                     const SIngSnatchingSwarmData& data);

  // CEntity
  ~CIngSnatchingSwarm() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override { return &mHealthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CIngSnatchingSwarm
  void Start(CStateManager& mgr, int msg, float dt);           // Guessed name
  void Dead(CStateManager& mgr, int msg, float dt);            // Guessed name
  void ExitPortal(CStateManager& mgr, int msg, float dt);      // Guessed name
  void FollowArcPath(CStateManager& mgr, int msg, float dt);   // Guessed name
  void DiveToTarget(CStateManager& mgr, int msg, float dt);    // Guessed name
  void SnatchTarget(CStateManager& mgr, int msg, float dt);    // Guessed name
  bool StateOver(CStateManager& mgr, const float& arg);        // Guessed name
  bool IsDead(CStateManager& mgr, const float& arg);           // Guessed name
  bool LifetimeOver(CStateManager& mgr, const float& arg);     // Guessed name
  bool ShouldLoiter(CStateManager& mgr, const float& arg);     // Guessed name
  bool HasTarget(CStateManager& mgr, const float& arg);        // Guessed name
  bool InSnatchingRange(CStateManager& mgr, const float& arg); // Guessed name
  bool HitPlayer(CStateManager& mgr, const float& arg);        // Guessed name
  bool HitTarget(CStateManager& mgr, const float& arg);        // Guessed name
  bool HitWorld(CStateManager& mgr, const float& arg);         // Guessed name
  void SetPortalSwarmPath(CStateManager& mgr, float dt);       // Guessed name
  void SelectTarget(CStateManager& mgr, float dt);             // Guessed name
  void SetTargetDest(CStateManager& mgr, float dt);            // Guessed name
  void SetPlayerDest(CStateManager& mgr, float dt);            // Guessed name
  void DamagePlayer(CStateManager& mgr, float dt);             // Guessed name
  void Explode(CStateManager& mgr, float dt);                  // Guessed name

private:
  enum EPathState { kPS_Idle, kPS_Following, kPS_Finished }; // Guessed names

  void InitializeStateMachine(CStateManager& mgr);     // Guessed name
  const CGenericFSM2* GetStateMachine() const;         // Guessed name
  void RemoveSfxEmitter();                             // Guessed name
  void UpdateSfxEmitter();                             // Guessed name
  void CheckWorldCollision(CStateManager& mgr);        // Guessed name
  void UpdateTouchBounds();                            // Guessed name
  void UpdateMovement(float dt);                       // Guessed name
  void UpdateParticles(CStateManager& mgr, float dt);  // Guessed name
  CVector3f RandomArcOffset(CStateManager& mgr) const; // Guessed name
  static CParticleGen* CreateParticle(const CVector3f& scale, const CVector3f& translation,
                                      const CToken& token);         // Guessed name
  void MoveTowards(const CVector3f& target, float speed, float dt); // Guessed name
  bool FollowArc(float time);                                       // Guessed name
  float CalculateArcLength() const;                                 // Guessed name

  SIngSnatchingSwarmData mData;                                              // Guessed name
  CHealthInfo mHealthInfo;                                                   // Guessed name
  CPathFindSearch mPathFindSearch;                                           // Guessed name
  float mDeltaTime;                                                          // Guessed name
  float mAge;                                                                // Guessed name
  rstl::optional_object< CAABox > mTouchBounds;                              // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mExplosionEffect; // Guessed name
  CVector3f mHitNormal;                                                      // Guessed name
  CVector3f mHitPoint;                                                       // Guessed name
  CVector3f mScale;                                                          // Guessed name
  float mSpeed;                                                              // Guessed name
  CVector3f mVelocity;                                                       // Guessed name
  CVector3f mPreviousPosition;                                               // Guessed name
  rstl::optional_object< CToken > mStateMachineToken;                        // Guessed name
  CGenericFSM2State< CPatterned > mStateMachine;                             // Guessed name
  EPathState mPathState;                                                     // Guessed name
  CToken mSwarmToken;                                                        // Guessed name
  rstl::auto_ptr< CParticleGen > mSwarmParticle;                             // Guessed name
  CToken mSecondaryToken;                                                    // Guessed name
  rstl::auto_ptr< CParticleGen > mSecondaryParticle;                         // Guessed name
  CVector3f mStartPosition;                                                  // Guessed name
  CVector3f mStartForward;                                                   // Guessed name
  TUniqueId mHitId;                                                          // Guessed name
  TUniqueId mTargetId;                                                       // Guessed name
  CVector3f mUnknownVector;                                                  // Guessed name
  float mLoiterEndTime;                                                      // Guessed name
  rstl::reserved_vector< CVector3f, 4 > mArcPoints;                          // Guessed name
  float mArcLength;                                                          // Guessed name
  float mArcSpeedScale;                                                      // Guessed name
  rstl::reserved_vector< CVector3f, 60 > mPositionHistory;                   // Guessed name
  int mHistoryIndex;                                                         // Guessed name
  CSfxHandle mSfxHandle;                                                     // Guessed name
  bool mAlive : 1;                                                           // Guessed name
  bool mStarted : 1;                                                         // Guessed name
  bool mCheckCollision : 1;                                                  // Guessed name
  bool mHitWorld : 1;                                                        // Guessed name
  bool mLoiterEnded : 1;                                                     // Guessed name
  bool mVisible : 1;                                                         // Guessed name
};
CHECK_SIZEOF(CIngSnatchingSwarm, 0x740)

#endif // _CINGSNATCHINGSWARM
