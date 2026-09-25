#ifndef _CPARTICLESWOOSHDATAFACTORY
#define _CPARTICLESWOOSHDATAFACTORY

#include "Kyoto/CFactoryMgr.hpp"

class CInputStream;
class CSimplePool;
class CSwooshDescription;

class CParticleSwooshDataFactory {
public:
  static CSwooshDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);

private:
  static CSwooshDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* pool);
  static bool CreateWPSM(CSwooshDescription* desc, CInputStream& in, CSimplePool* pool);
};

const CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,
                                            const CVParamTransfer& transfer);

#endif // _CPARTICLESWOOSHDATAFACTORY
