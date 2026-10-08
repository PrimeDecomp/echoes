#ifndef _CWISPTENTACLE
#define _CWISPTENTACLE

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/single_ptr.hpp"

class CCollisionActorManager;

// Guessed class: a tentacle that rises out of a portal, hurts the player on contact and drags a
// space pirate into the portal.
class CWispTentacle : public CPatterned {
public:
  CWispTentacle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& modelData,
                const CPatternedInfo& patternedInfo, const CDamageInfo& attackDamage,
                const CActorParameters& actorParams, bool spawnFromPortal, float wakeUpDistance,
                float searchDistance, float attackDistance, float detectionHeight,
                float hurtSleepDelay, float grabBlendTime);

  // CEntity
  ~CWispTentacle() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CWispTentacle
  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Spawn(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Search(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void Withdraw(CStateManager& mgr, EStateMsg msg, float dt);
  void Flinch(CStateManager& mgr, EStateMsg msg, float dt);
  void Grab(CStateManager& mgr, EStateMsg msg, float dt);
  void Pull(CStateManager& mgr, EStateMsg msg, float dt);
  bool ShouldSleep(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSearch(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldGrab(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool Delay(CStateManager& mgr, const CTriggerData& data) const;

private:
  enum EState {
    kS_Invalid = -1,
    kS_Sleep = 0,
    kS_Patrol = 1,
    kS_Search = 2,
    kS_Attack = 3,
    kS_Flinch = 4,
    kS_Spawn = 5,
    kS_Withdraw = 6,
    kS_Grab = 7,
  };

  bool IsPlayerNearby(float distance, CStateManager& mgr) const; // Guessed name
  void SnapWaypointToFloor(CStateManager& mgr);                  // Guessed name
  void UpdateGrabbedPirate(float dt, CStateManager& mgr);        // Guessed name

  bool mSpawnFromPortal;                                        // Guessed name
  float mWakeUpDistance;                                        // Guessed name
  float mSearchDistance;                                        // Guessed name
  float mAttackDistance;                                        // Guessed name
  CDamageInfo mAttackDamage;                                    // Guessed name
  CVector3f mArmEndPosition;                                    // Guessed name
  float mDetectionHeight;                                       // Guessed name
  bool mLocatorsCached;                                         // Guessed name
  EState mState;                                                // Guessed name
  EState mPreviousState;                                        // Guessed name
  float mHurtSleepDelay;                                        // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionManager; // Guessed name
  CVector3f mLockOnOffset;                                      // Guessed name
  CTransform4f mInitialTransform;                               // Guessed name
  CVector3f x84c_;
  CPlane mPortalPlane;              // Guessed name
  TUniqueId mPirateId;              // Guessed name
  bool mShouldGrab;                 // Guessed name
  bool mPirateCaptured;             // Guessed name
  CVector3f mGrabOffset;            // Guessed name
  TUniqueId mWaypointId;            // Guessed name
  float mGrabTimer;                 // Guessed name
  CQuaternion mPirateStartRotation; // Guessed name
  CVector3f mPirateStartPosition;   // Guessed name
  float mGrabBlendTime;             // Guessed name
  mutable bool mAttacked : 1;       // Guessed name
};
CHECK_SIZEOF(CWispTentacle, 0x8a8)

#endif // _CWISPTENTACLE
