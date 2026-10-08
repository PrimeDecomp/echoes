#ifndef _CSPANKWEED
#define _CSPANKWEED

#include "types.h"

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "rstl/single_ptr.hpp"

class CCollisionActorManager;

// Guessed class: a stationary tentacle plant that hides until the player comes near.
// This is the Prime enemy; its state functions survive here as plain virtuals that nothing
// registers with the state machine.
class CSpankWeed : public CPatterned {
  friend class CSpankWeedCollisionActor;

public:
  CSpankWeed(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CModelData& modelData,
             const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
             float maxDetectionRange, float maxHearingRange, float maxSightRange, float hideTime);

  // CEntity
  ~CSpankWeed() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CSpankWeed
  virtual bool InDetectionRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HearPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Delay(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void FadeIn(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void FadeOut(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Flinch(CStateManager& mgr, EStateMsg msg, float dt);

private:
  bool IsPlayerNear(CStateManager& mgr, float range) const; // Guessed name

  float mMaxDetectionRange;
  float mHeightRange; // Guessed name; copy of the patterned detection height range.
  float mMaxHearingRange;
  float mMaxSightRange;
  float mHideTime;
  bool mCanKnockBack;
  float x7d8_;
  CVector3f mRetreatOrigin;
  TUniqueId mCollisionActorId; // Guessed name; never assigned.
  rstl::single_ptr< CCollisionActorManager > mCollisionMgr;
  bool mIsHiding;
  CVector3f mLockonOffset;
  CVector3f mLockonTarget;
  int mState;
  int mPreviousState;
  int mAnimPhase;
};
CHECK_SIZEOF(CSpankWeed, 0x818)

#endif // _CSPANKWEED
