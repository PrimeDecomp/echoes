#include "Kyoto/Particles/CParticleElectricDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/CVParamTransfer.hpp"

const CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in,
                                              const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.x0_obj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CElectricDescription* desc = CParticleElectricDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

CElectricDescription* CParticleElectricDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                     CSimplePool* pool) {
  rstl::vector< CAssetId > resources;
  return CreateGeneratorDescription(in, pool);
}

CElectricDescription* CParticleElectricDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                               CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'ELSM') {
    return nullptr;
  }
  CElectricDescription* desc = rs_new CElectricDescription();
  CreateELSM(desc, in, pool);
  LoadELSMTokens(desc);
  return desc;
}

void CParticleElectricDataFactory::LoadELSMTokens(CElectricDescription* desc) {
  if (desc->x48_SSWH) {
    desc->x48_SSWH->ForceCache();
  }
  if (desc->x58_GPSM) {
    desc->x58_GPSM->ForceCache();
  }
  if (desc->x68_EPSM) {
    desc->x68_EPSM->ForceCache();
  }
}

bool CParticleElectricDataFactory::CreateELSM(CElectricDescription* desc, CInputStream& in,
                                              CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'LIFE':
      desc->x0_LIFE = CParticleDataFactory::GetIntElement(in);
      break;
    case 'SLIF':
      desc->x4_SLIF = CParticleDataFactory::GetIntElement(in);
      break;
    case 'GRAT':
      desc->x8_GRAT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SCNT':
      desc->xc_SCNT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'SSEG':
      desc->x10_SSEG = CParticleDataFactory::GetIntElement(in);
      break;
    case 'COLR':
      desc->x14_COLR = CParticleDataFactory::GetColorElement(in);
      break;
    case 'IEMT':
      desc->x18_IEMT = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'FEMT':
      desc->x1c_FEMT = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'AMPL':
      desc->x20_AMPL = CParticleDataFactory::GetRealElement(in);
      break;
    case 'AMPD':
      desc->x24_AMPD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SSWH': {
      const FourCC childId = CParticleDataFactory::GetClassID(in);
      if (childId != 'NONE') {
        const CAssetId id = in.ReadInt32();
        if (id != 0) {
          desc->x48_SSWH = TCachedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
        }
      }
      break;
    }
    case 'GPSM': {
      rstl::vector< CAssetId > resources;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, resources);
      if (child) {
        desc->x58_GPSM = TCachedToken< CGenDescription >(*child);
      }
      break;
    }
    case 'EPSM': {
      rstl::vector< CAssetId > resources;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, resources);
      if (child) {
        desc->x68_EPSM = TCachedToken< CGenDescription >(*child);
      }
      break;
    }
    case 'LWD1':
      desc->x28_LWD1 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LWD2':
      desc->x2c_LWD2 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LWD3':
      desc->x30_LWD3 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LCL1':
      desc->x34_LCL1 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'LCL2':
      desc->x38_LCL2 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'LCL3':
      desc->x3c_LCL3 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'ZERY':
      desc->x78_ZERY = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXR':
      desc->x40_TEXR = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case '_END':
      done = true;
      break;
    case 'DFLG':
      desc->x44_DFLG = CParticleDataFactory::GetBitflag(in);
      break;
    default:
      return false;
    }
  }
  return true;
}
