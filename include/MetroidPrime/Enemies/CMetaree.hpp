#ifndef _CMETAREE
#define _CMETAREE

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"

// Prime 1 has the same class; the Echoes version drops from its perch with a launch impulse
// and plays a configurable attack sound.
class CMetaree : public CPatterned {
public:
  CMetaree(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& modelData, const CActorParameters& actorParams,
           const CPatternedInfo& patternedInfo, const CDamageInfo& damageInfo, float dropHeight,
           const CVector3f& offset, float attackSpeed, float delay, float haltDelay,
           float launchSpeed, const SLdrAudioPlaybackParms& attackSound);

  // CEntity
  void Think(float dt, CStateManager& mgr) override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CPatterned
  void ThinkAboutMove(float dt) override;
  void SetupStateMachine(CStateManager& mgr) override;

  // CMetaree
  virtual bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DropDelay(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AttackDelay(CStateManager& mgr, const CTriggerData& data) const;
  virtual void InActive(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void InActiveReady(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Active(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Flee(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Halt(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Explode(CStateManager& mgr, EStateMsg msg, float dt);

private:
  float mDelay;
  float mHaltDelay;
  float mDropHeight;
  float mLaunchSpeed;
  CVector3f mOffset;
  float mAttackSpeed;
  CVector3f mLookPos;
  CVector3f mProjectileDelta;
  CVector3f mVelocity;
  int mFleeState;
  CDamageInfo mDamageInfo;
  SLdrAudioPlaybackParms mAttackSound;
  bool x83c_24_ : 1;
  bool mStarted : 1;
  bool mDropped : 1;
};
CHECK_SIZEOF(CMetaree, 0x840)

#endif // _CMETAREE
