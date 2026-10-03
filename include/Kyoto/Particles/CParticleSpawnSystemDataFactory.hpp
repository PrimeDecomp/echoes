#ifndef _CPARTICLESPAWNSYSTEMDATAFACTORY
#define _CPARTICLESPAWNSYSTEMDATAFACTORY

#include "types.h"
#include "Kyoto/CFactoryMgr.hpp"

class CSpawnSystemDescription;
class CInputStream;
class CSimplePool;
class CVParamTransfer;

// Reconstructed class/method names follow the sibling particle factories; no original export is known.
class CParticleSpawnSystemDataFactory {
public:
  static CSpawnSystemDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);

private:
  static CSpawnSystemDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* pool);
  static bool CreateSPSM(CSpawnSystemDescription* desc, CInputStream& in, CSimplePool* pool);
};

// Guessed free-factory name, supported by the SPSC registration and parsed descriptor type.
CFactoryFnReturn FSpawnParticleSystemDataFactory(const SObjectTag& tag, CInputStream& in,
                                                const CVParamTransfer& transfer);

#endif // _CPARTICLESPAWNSYSTEMDATAFACTORY
