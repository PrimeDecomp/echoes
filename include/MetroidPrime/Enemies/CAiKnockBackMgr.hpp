#ifndef _CAIKNOCKBACKMGR
#define _CAIKNOCKBACKMGR

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

class CAiKnockBackMgr : public CKnockBackMgr {
public:
  explicit CAiKnockBackMgr(CAssetId rules);
  ~CAiKnockBackMgr();
  void Update(float dt, CStateManager& mgr, CActor& actor) override;
  void KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info) override;
  void SetPhysicsKnockBackType(EPhysicsKnockBackType type);
  void EnableKnockBackPhysics(bool enabled);
  void SetAdditiveFlinchWeight(float weight);

private:
  EPhysicsKnockBackType mPhysicsKnockBackType;
  int mImpulseDurationIndex;
  float x6c_;
  int x70_;
  CVector3f mImpulseDirection;
  float mImpulseRemainingTime;
  float x84_;
  float mAdditiveFlinchWeight;
  float x8c_;
  bool mKnockBackPhysicsEnabled : 1;
  bool mAutoResetImpulse : 1;
  bool x90_26_ : 1;
  bool x90_27_ : 1;
};
CHECK_SIZEOF(CAiKnockBackMgr, 0x94)

#endif
