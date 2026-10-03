#include "Kyoto/Particles/CParticleSpawnSystemDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSpawnSystemDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

CFactoryFnReturn FSpawnParticleSystemDataFactory(const SObjectTag& tag, CInputStream& in,
                                                const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CSpawnSystemDescription* desc = CParticleSpawnSystemDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

CSpawnSystemDescription* CParticleSpawnSystemDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                         CSimplePool* pool) {
  rstl::vector< CAssetId > resources;
  return CreateGeneratorDescription(in, pool);
}

CSpawnSystemDescription* CParticleSpawnSystemDataFactory::CreateGeneratorDescription(
    CInputStream& in, CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'SPSM') {
    return nullptr;
  }
  CSpawnSystemDescription* desc = rs_new CSpawnSystemDescription();
  CreateSPSM(desc, in, pool);
  return desc;
}

bool CParticleSpawnSystemDataFactory::CreateSPSM(CSpawnSystemDescription* desc, CInputStream& in,
                                               CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'IVEC':
      desc->mIVEC = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'VBLN':
      desc->mVBLN = CParticleDataFactory::GetRealElement(in);
      break;
    case 'PSLT':
      desc->mPSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'DEOL':
      desc->mDEOL = CParticleDataFactory::GetBool(in);
      break;
    case 'FRCO':
      desc->mFRCO = CParticleDataFactory::GetBool(in);
      break;
    case 'FROV':
      desc->mFROV = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'GIVL':
      desc->mGIVL = CParticleDataFactory::GetIntElement(in);
      break;
    case 'IGGT':
      desc->mIGGT = CParticleDataFactory::GetBool(in);
      break;
    case 'IGLT':
      desc->mIGLT = CParticleDataFactory::GetBool(in);
      break;
    case 'VMD1':
      desc->mVMD1 = CParticleDataFactory::GetBool(in);
      break;
    case 'VMD2':
      desc->mVMD2 = CParticleDataFactory::GetBool(in);
      break;
    case 'VLM1':
      desc->mVLM1 = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLM2':
      desc->mVLM2 = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'PCOL':
      desc->mPCOL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'SCLE':
      desc->mSCLE = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'LSCL':
      desc->mLSCL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'TRNL':
      desc->mTRNL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'ORNT':
      desc->mORNT = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'GTRN':
      desc->mGTRN = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'GORN':
      desc->mGORN = CParticleDataFactory::GetVectorElement(in);
      break;
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
