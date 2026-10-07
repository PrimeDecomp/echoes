#include "Weapons/CProjectileWeaponDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Weapons/CWeaponDescription.hpp"

CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in,
                                              const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CWeaponDescription* desc = CProjectileWeaponDataFactory::GetGeneratorDesc(in, pool);
  return desc;
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
  bool done = false;
  CRandom16 random;
  CGlobalRandom globalRandom(random);

  while (!done) {
    switch (CParticleDataFactory::GetClassID(in)) {
    case 'IORN':
      desc->mIORN = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'IVEC':
      desc->mIVEC = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PSOV':
      desc->mPSOV = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PSVM':
      desc->mPSVM = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VMD2':
      desc->mVMD2 = CParticleDataFactory::GetBool(in);
      break;
    case 'PSLT':
      desc->mPSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'APSM': {
      rstl::vector< CAssetId > assets;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, assets);
      if (child) {
        desc->mAPSM = TLockedToken< CGenDescription >(*child);
      } else {
        desc->mAPSM = rstl::optional_object_null();
      }
      break;
    }
    case 'DP1C':
      desc->mDP1C = CParticleDataFactory::GetBool(in);
      break;
    case 'AP11':
      desc->mAP11 = CParticleDataFactory::GetBool(in);
      break;
    case 'APS2': {
      rstl::vector< CAssetId > assets;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, assets);
      if (child) {
        desc->mAPS2 = TLockedToken< CGenDescription >(*child);
      } else {
        desc->mAPS2 = rstl::optional_object_null();
      }
      break;
    }
    case 'DP2C':
      desc->mDP2C = CParticleDataFactory::GetBool(in);
      break;
    case 'AP21':
      desc->mAP21 = CParticleDataFactory::GetBool(in);
      break;
    case 'APSO':
      desc->mAPSO = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW1': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadInt32();
        desc->mASW1 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS11':
      desc->mAS11 = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW2': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadInt32();
        desc->mASW2 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS12':
      desc->mAS12 = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW3': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadInt32();
        desc->mASW3 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS13':
      desc->mAS13 = CParticleDataFactory::GetBool(in);
      break;
    case 'OHEF': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadInt32();
        desc->mOHEF = TLockedToken< CModel >(pool->GetObj(SObjectTag('CMDL', id)));
      }
      break;
    }
    case 'PSCL':
      desc->mPSCL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PCOL':
      desc->mPCOL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'COLR': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadInt32();
        desc->mCOLR = TLockedToken< CCollisionResponseData >(pool->GetObj(SObjectTag('CRSC', id)));
      }
      break;
    }
    case 'POFS':
      desc->mPOFS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'OFST':
      desc->mOFST = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'RB1A':
      desc->mRB1A = CParticleDataFactory::GetBool(in);
      break;
    case 'B1TX':
      desc->mB1TX = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'B1PO':
      desc->mB1PO = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'B1SE':
      desc->mB1SE = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B1RT':
      desc->mB1RT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B1CL':
      desc->mB1CL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'RB2A':
      desc->mRB2A = CParticleDataFactory::GetBool(in);
      break;
    case 'B2TX':
      desc->mB2TX = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'B2PO':
      desc->mB2PO = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'B2SE':
      desc->mB2SE = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B2RT':
      desc->mB2RT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B2CL':
      desc->mB2CL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'TTEX':
      desc->mTTEX = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'TLPO':
      desc->mTLPO = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'TSZE':
      desc->mTSZE = CParticleDataFactory::GetRealElement(in);
      break;
    case 'TLEN':
      desc->mTLEN = CParticleDataFactory::GetRealElement(in);
      break;
    case 'TSCL':
      desc->mTSCL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'TECL':
      desc->mTECL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'RTLA':
      desc->mRTLA = CParticleDataFactory::GetBool(in);
      break;
    case 'HOMG':
      desc->mHOMG = CParticleDataFactory::GetBool(in);
      break;
    case 'TRAT':
      desc->mTRAT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'PJFX':
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        desc->mPJFX = in.ReadInt32();
      }
      break;
    case 'RNGE':
      desc->mRNGE = CParticleDataFactory::GetRealElement(in);
      break;
    case 'FOFF':
      desc->mFOFF = CParticleDataFactory::GetRealElement(in);
      break;
    case 'EWTR':
      desc->mEWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'LWTR':
      desc->mLWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'SWTR':
      desc->mSWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'F60H':
      desc->mF60H = CParticleDataFactory::GetBool(in);
      break;
    case 'SVBD':
      desc->mSVBD = CParticleDataFactory::GetBool(in);
      break;
    case 'NDTT':
      desc->mNDTT = CParticleDataFactory::GetBool(in);
      break;
    case 'SPS1':
      desc->mSPS1 = CParticleDataFactory::GetBool(in);
      break;
    case 'SPS2':
      desc->mSPS2 = CParticleDataFactory::GetBool(in);
      break;
    case 'FC60':
      desc->mFC60 = CParticleDataFactory::GetBool(in);
      break;
    case 'RWPE':
      desc->mRWPE = CParticleDataFactory::GetBool(in);
      break;
    case 'EELT':
      desc->mEELT = CParticleDataFactory::GetBool(in);
      break;
    case 'BHBT':
      desc->mBHBT = CParticleDataFactory::GetBool(in);
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
