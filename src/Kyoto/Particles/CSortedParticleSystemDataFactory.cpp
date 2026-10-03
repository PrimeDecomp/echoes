#include "Kyoto/Particles/CSortedParticleSystemDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSortedParticleSystemDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

CFactoryFnReturn FSortedParticleSystemDataFactory(const SObjectTag& tag, CInputStream& in,
                                                 const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CSortedParticleSystemDescription* desc = CSortedParticleSystemDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

CSortedParticleSystemDescription* CSortedParticleSystemDataFactory::GetGeneratorDesc(
    CInputStream& in, CSimplePool* pool) {
  rstl::vector< CAssetId > resources;
  return CreateGeneratorDescription(in, pool);
}

CSortedParticleSystemDescription* CSortedParticleSystemDataFactory::CreateGeneratorDescription(
    CInputStream& in, CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'SRSM') {
    return nullptr;
  }
  CSortedParticleSystemDescription* desc = rs_new CSortedParticleSystemDescription();
  CreateSRSM(desc, in, pool);
  return desc;
}

bool CSortedParticleSystemDataFactory::CreateSRSM(CSortedParticleSystemDescription* desc,
                                                CInputStream& in, CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'SPWN': {
      const FourCC childId = CParticleDataFactory::GetClassID(in);
      if (childId == 'CNST') {
        desc->mSPWN = rs_new CSpawnSystemKeyframeData(in);
        desc->mSPWN->LoadAllSpawnedSystemTokens(pool);
      }
      break;
    }
    case '_END':
      done = true;
      break;
    default:
      return false;
    }
  }
  return true;
}
