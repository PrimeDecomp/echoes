#include "Kyoto/Particles/CParticleSwooshDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

const CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,
                                            const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CSwooshDescription* desc = CParticleSwooshDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

// Force function ordering
static void hack() { TObjOwnerDerivedFromIObj< CSwooshDescription >::GetNewDerivedObject(nullptr); }

CSwooshDescription* CParticleSwooshDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                 CSimplePool* pool) {
  rstl::vector< CAssetId > ids;
  return CreateGeneratorDescription(in, pool);
}

CSwooshDescription* CParticleSwooshDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                           CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'SWSH') {
    return nullptr;
  }
  CSwooshDescription* desc = rs_new CSwooshDescription();
  CreateWPSM(desc, in, pool);
  return desc;
}

bool CParticleSwooshDataFactory::CreateWPSM(CSwooshDescription* desc, CInputStream& in,
                                            CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  CGlobalRandom globalRandom(random);

  while (!done) {
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'PSLT':
      desc->mPSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'TIME':
      desc->mTIME = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LRAD':
      desc->mLRAD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RRAD':
      desc->mRRAD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LLRD':
      desc->mLLRD = CParticleDataFactory::GetBool(in);
      break;
    case 'LENG':
      desc->mLENG = CParticleDataFactory::GetIntElement(in);
      break;
    case 'COLR':
      desc->mCOLR = CParticleDataFactory::GetColorElement(in);
      break;
    case 'SIDE':
      desc->mSIDE = CParticleDataFactory::GetIntElement(in);
      break;
    case 'CROS':
      desc->mCROS = CParticleDataFactory::GetBool(in);
      break;
    case 'SROT':
      desc->mSROT = CParticleDataFactory::GetBool(in);
      break;
    case 'IROT':
      desc->mIROT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'ROTM':
      desc->mROTM = CParticleDataFactory::GetRealElement(in);
      break;
    case 'POFS':
      desc->mPOFS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'IVEL':
      desc->mIVEL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'NPOS':
      desc->mNPOS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'VELM':
      desc->mVELM = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLS1':
      desc->mVLS1 = CParticleDataFactory::GetBool(in);
      break;
    case 'VLM2':
      desc->mVLM2 = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLS2':
      desc->mVLS2 = CParticleDataFactory::GetBool(in);
      break;
    case 'SPLN':
      desc->mSPLN = CParticleDataFactory::GetIntElement(in);
      break;
    case 'WIRE':
      desc->mWIRE = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXR':
      desc->mTEXR = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'TSPN':
      desc->mTSPN = CParticleDataFactory::GetIntElement(in);
      break;
    case 'AALP':
      desc->mAALP = CParticleDataFactory::GetBool(in);
      break;
    case 'ZBUF':
      desc->mZBUF = CParticleDataFactory::GetBool(in);
      break;
    case 'ORNT':
      desc->mORNT = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXW':
      desc->mTEXW = CParticleDataFactory::GetBool(in);
      break;
    case 'CLTX':
      desc->mCLTX = CParticleDataFactory::GetBool(in);
      break;
    case 'CRND':
      desc->mCRND = CParticleDataFactory::GetBool(in);
      break;
    case 'DFLG':
      desc->mDFLG = CParticleDataFactory::GetBitflag(in);
      break;
    case '_END':
      done = true;
      break;
    default:
      return false;
    }
  }

  return true;
}
