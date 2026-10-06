#ifndef _CRIPPER
#define _CRIPPER

#include "types.h"

#include "MetroidPrime/CGrappleParameters.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

// Original class name from the Wii SEL exports (TypesMatch__7CRipperCFi). Prime 1 has the same
// class; the Echoes version has no controlled platform and checks every player's grapple.
class CRipper : public CPatterned {
public:
  CRipper(TUniqueId uid, const rstl::string& name, EFlavorType flavor, const CEntityInfo& info,
          const CTransform4f& xf, const CModelData& modelData, const CPatternedInfo& patternedInfo,
          const CActorParameters& actorParams, const CGrappleParameters& grappleParams);

  // CEntity
  ~CRipper() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CRipper
  virtual bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float dt);

private:
  void ProcessGrapplePoint(CStateManager& mgr);
  void AddGrapplePoint(CStateManager& mgr);
  void RemoveGrapplePoint(CStateManager& mgr);

  CGrappleParameters mGrappleParams;
  TUniqueId mGrapplePoint;
  bool mMuted : 1;
};
CHECK_SIZEOF(CRipper, 0x7F8)

#endif // _CRIPPER
