#ifndef _CPROJECTILEWEAPONDATAFACTORY
#define _CPROJECTILEWEAPONDATAFACTORY

#include "Kyoto/CFactoryMgr.hpp"

class CWeaponDescription;
class CSimplePool;

// Class and method spellings are reconstructed from Prime and native WPSM behavior.
class CProjectileWeaponDataFactory {
public:
  static CWeaponDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);
  static CWeaponDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* pool);
  static bool CreateWPSM(CWeaponDescription* desc, CInputStream& in, CSimplePool* pool);
};

CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in,
                                              const CVParamTransfer& transfer);

#endif // _CPROJECTILEWEAPONDATAFACTORY
