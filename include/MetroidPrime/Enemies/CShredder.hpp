#ifndef _CSHREDDER
#define _CSHREDDER

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

struct SLdrShredderData;

class CShredder : public CPatterned {
public:
  CShredder(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& modelData,
            const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
            const SLdrShredderData& data);

  // CEntity
  ~CShredder() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& other, CStateManager& mgr) override;
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  bool IsListening() const override;
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CShredder
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  void Generate(CStateManager& mgr, EStateMsg msg, float dt);
  void AttackPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  bool ShouldWakeUp(CStateManager& mgr, const CTriggerData& data) const;

private:
  void AddReactionAnimation();                       // Guessed name
  void Explode(CStateManager& mgr);                  // Guessed name
  bool IsUnderWater(const CStateManager& mgr) const; // Guessed name
  void ApplySeparation(CStateManager& mgr);          // Guessed name

  int mStartState;
  CDamageInfo mExplosionDamage;
  float mMinHeight;
  float mMaxHeight;
  float mMinDownHeight;
  float mMaxDownHeight;
  float mSeparationDistance;
  float mMinLifeTime;
  float mMaxLifeTime;
  float mNormalKnockback;
  float mHeavyKnockback;
  float mKnockbackDecline;
  bool mIsDarkShredder : 1;
  bool mAttacking : 1;
  bool mWokenUp : 1;
  bool mLurking : 1;
  bool mExploded : 1;
  float mDamageTaken;
  float mFuseProgress;
  float mFuseRate;
  float mGenerateDuration;
  float mGenerateSpeed;
  float mDamageRate;
  float mKnockbackSpeed;
  float mDesiredDistance;
  int mReactionAnim;
};
CHECK_SIZEOF(CShredder, 0x830)

#endif // _CSHREDDER
