#ifndef _CAMBIENTAI
#define _CAMBIENTAI

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"

// Guessed name, correlated with Prime's ambient-animation actor.
class CAmbientAI : public CPhysicsActor {
public:
  CAmbientAI(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CModelData& model, const CAABox& bounds,
             const CMaterialList& materials, float mass, const CHealthInfo& health,
             const CDamageVulnerability& vulnerability, const CActorParameters& params,
             float detectRadius, float explodeRadius, int reactAnim, int damagedAnim);

  // CEntity
  ~CAmbientAI() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  void RandomizePlaybackRate(CStateManager& mgr);

private:
  // Guessed names; the states retain Prime's animation-state progression.
  enum EAnimationState { kAS_Ready, kAS_Alert, kAS_Impact };

  CHealthInfo mInitialHealthInfo;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDVuln;
  EAnimationState mAnimState;
  float mDetectRadius;
  float mExplodeRadius;
  int mCurrentAnim;
  int mReactAnim;
  int mDamagedAnim;
  bool mDead : 1;
  bool mAnimating : 1;
};
CHECK_SIZEOF(CAmbientAI, 0x360)

#endif // _CAMBIENTAI
