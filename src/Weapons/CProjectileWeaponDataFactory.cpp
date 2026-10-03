#include "Weapons/CProjectileWeaponDataFactory.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Weapons/CWeaponDescription.hpp"

CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in,
                                              const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  return CProjectileWeaponDataFactory::GetGeneratorDesc(in, pool);
}

CWeaponDescription* CProjectileWeaponDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                   CSimplePool* pool) {
  rstl::vector< CAssetId > assets;
  return CreateGeneratorDescription(in, pool);
}

CWeaponDescription* CProjectileWeaponDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                             CSimplePool* pool) {
  if (CParticleDataFactory::GetClassID(in) != 'WPSM') {
    return nullptr;
  }

  CWeaponDescription* desc = rs_new CWeaponDescription();
  CreateWPSM(desc, in, pool);
  return desc;
}

bool CProjectileWeaponDataFactory::CreateWPSM(CWeaponDescription* desc, CInputStream& in,
                                              CSimplePool* pool) {
  // Nonfunctional scaffold: the tagged particle-resource parser remains unimplemented.
  return false;
}
