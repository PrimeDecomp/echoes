#ifndef _CSPORBPROJECTILE
#define _CSPORBPROJECTILE

#include "types.h"

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

// Guessed class: the glob a Sporb spits out, which flies on its own and lets the morph ball escape.
class CSporbProjectile : public CPatterned {
public:
  // Guessed names; values the owning Sporb stores to select the glob's next behavior.
  enum EState {
    kS_Invalid = -1,
    kS_Launch = 0,
    kS_Close = 1,
    kS_Open = 2,
    kS_Spit = 4,
    kS_Sleeping = 5,
    kS_Patrolling = 6,
    kS_Attacking = 7,
    kS_Fire = 8,
    kS_Reload = 9,
  };

  // Guessed names; whether the glob is solid for the morph ball.
  enum ESolidPhase {
    kSP_Solid,
    kSP_Landed,
    kSP_BallInside,
  };

  CSporbProjectile(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                   CAssetId ballSpitEffect, CAssetId ballEscapeEffect);

  // CEntity
  ~CSporbProjectile() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mSphere; }
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsScanVisorSelfRender() const override { return true; }
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;

  // CSporbProjectile
  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void WakeUp(CStateManager& mgr, EStateMsg msg, float dt);
  void GoToSleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Fire(CStateManager& mgr, EStateMsg msg, float dt);
  void Reload(CStateManager& mgr, EStateMsg msg, float dt);
  void Launch(CStateManager& mgr, EStateMsg msg, float dt);
  void Close(CStateManager& mgr, EStateMsg msg, float dt);
  void Open(CStateManager& mgr, EStateMsg msg, float dt);
  void Spit(CStateManager& mgr, EStateMsg msg, float dt);

  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldReload(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldLaunch(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldClose(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldOpen(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpit(CStateManager& mgr, const CTriggerData& data) const;

  void PlayBallEscapeEffect(CStateManager& mgr);
  void PlayBallSpitEffect(CStateManager& mgr);
  void SetState(EState state) { mState = state; }
  void SetTopId(TUniqueId id) { mTopId = id; }
  int GetSolidPhase() const { return mSolidPhase; }
  void SetSolidPhase(ESolidPhase phase) { mSolidPhase = phase; }
  bool IsPassable() const { return mPassable; }
  void SetPassable(bool passable) { mPassable = passable; }
  bool IsCarryingBall() const { return mCarryingBall; }
  void SetCarryingBall(bool carrying) { mCarryingBall = carrying; }

private:
  int mState;                 // Guessed name
  int mSolidPhase;            // Guessed name
  CCollidableSphere mSphere;  // Guessed name
  float mBallEscapeRadius;    // Guessed name
  bool mPassable;             // Guessed name
  bool mCarryingBall;         // Guessed name; set once the glob retracts with the morph ball
  CAssetId mBallSpitEffect;   // Guessed name
  CAssetId mBallEscapeEffect; // Guessed name
  uchar mEffectIndex;         // Guessed name
  TUniqueId mTopId;           // Guessed name
};
CHECK_SIZEOF(CSporbProjectile, 0x800)

CEntity* LoadSporbProjectile(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _CSPORBPROJECTILE
