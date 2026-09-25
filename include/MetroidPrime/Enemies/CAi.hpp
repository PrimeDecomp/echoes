#ifndef _CAI
#define _CAI

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"

#include "Kyoto/CToken.hpp"
#include "rstl/optional_object.hpp"

class CKnockBackInfo;
class CStateMachine;
class CStateMachine2;
class CTeamAiRole;

class CAi : public CPhysicsActor {
public:
  CAi(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint castFlags,
      const CTransform4f& xf, const CModelData& modelData, const CAABox& bounds, float mass,
      const CHealthInfo& health, const CDamageVulnerability& vulnerability,
      const CMaterialList& materials, CAssetId stateMachine, CAssetId stateMachine2,
      const CActorParameters& params, float stepUp, float stepDown);
  // CEntity
  ~CAi() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;

  // CAi
  virtual void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) = 0;
  virtual void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) = 0;
  virtual CDamageVulnerability* DamageVulnerability();
  virtual void TakeDamage(const CVector3f& direction, float magnitude);
  // Names and unused parameters of these four slots are Prime-based hypotheses.
  virtual bool CanBeShot(const CStateManager&, int) { return true; }
  virtual bool IsListening() const { return false; }
  virtual bool Listen(const CVector3f&, EListenNoiseType) { return false; }
  virtual CVector3f GetOrigin(const CStateManager&, const CTeamAiRole&, const CVector3f&) const {
    return GetTranslation();
  }

  CStateMachine* GetStateMachine();
  CStateMachine2* GetStateMachine2();

private:
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  rstl::optional_object< CToken > mStateMachine;
};
CHECK_SIZEOF(CAi, 0x330)

#endif
