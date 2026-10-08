#ifndef _CINGPUDDLE
#define _CINGPUDDLE

#include "types.h"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CGenericFSM2State.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPuddle.hpp"
#include "rstl/optional_object.hpp"

class CGenDescription;

// Guessed class: an Ing puddle that crawls along a waypoint path hugging the surfaces it meets,
// leaving a blob effect behind. It is driven by an FSM2 state machine.
class CIngPuddle : public CPhysicsActor {
public:
  CIngPuddle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CVector3f& scale, const CActorParameters& actorParams,
             const SLdrIngPuddleData& data);

  // CEntity
  ~CIngPuddle() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  CHealthInfo* HealthInfo() override { return &mHealthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CIngPuddle
  void Start(CStateManager& mgr, int msg, float dt);        // Guessed name
  void Patrol(CStateManager& mgr, int msg, float dt);       // Guessed name
  void Idle(CStateManager& mgr, int msg, float dt);         // Guessed name
  void Dead(CStateManager& mgr, int msg, float dt);         // Guessed name
  bool StateOver(CStateManager& mgr, const float& arg);     // Guessed name
  bool IsDead(CStateManager& mgr, const float& arg);        // Guessed name
  bool HasPatrolPath(CStateManager& mgr, const float& arg); // Guessed name

private:
  enum EPatrolState { kPS_Invalid = -1, kPS_Patrolling = 1, kPS_Finished = 2 }; // Guessed names

  void HandleDamage(CStateManager& mgr, TUniqueId weaponId);                       // Guessed name
  void UpdateTouchBounds();                                                        // Guessed name
  void MoveAlongSurface(const CVector3f& direction, float speed, float dt);        // Guessed name
  void UpdateBlobEffect(CStateManager& mgr, float dt);                             // Guessed name
  void SpawnBlobEffect(CStateManager& mgr,
                       const TLockedToken< CGenDescription >& desc); // Guessed name
  void InitializeStateMachine(CStateManager& mgr);                   // Guessed name
  const CGenericFSM2* GetStateMachine() const;                       // Guessed name

  SLdrIngPuddleData mProperties;                                             // Guessed name
  CHealthInfo mHealthInfo;                                                   // Guessed name
  CDamageVulnerability mDamageVulnerability;                                 // Guessed name
  rstl::optional_object< CAABox > mTouchBounds;                              // Guessed name
  CVector3f mScale;                                                          // Guessed name
  rstl::optional_object< CToken > mStateMachineToken;                        // Guessed name
  CGenericFSM2State< CPatterned > mStateMachine;                             // Guessed name
  EPatrolState mPatrolState;                                                 // Guessed name
  CSurfaceAlignmentHelper mSurfaceAlignment;                                 // Guessed name
  TUniqueId mBlobEffectId;                                                   // Guessed name
  CVector3f mMoveDirection;                                                  // Guessed name
  TUniqueId mCurrentWaypointId;                                              // Guessed name
  TUniqueId mNextWaypointId;                                                 // Guessed name
  CVector3f mLastWaypointPosition;                                           // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mDeathEffect;     // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mNormalHitEffect; // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mHeavyHitEffect;  // Guessed name
  CSfxHandle mSfxHandle;                                                     // Guessed name
  bool mIdling : 1;                                                          // Guessed name
};
CHECK_SIZEOF(CIngPuddle, 0x5f0)

#endif // _CINGPUDDLE
