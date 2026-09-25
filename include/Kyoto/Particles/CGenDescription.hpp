#ifndef _CGENDESCRIPTION
#define _CGENDESCRIPTION

#include "types.h"

#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CModel;
class CElectricDescription;
class CSwooshDescription;
class CSpawnSystemKeyframeData;

class CGenDescription {
public:
  typedef rstl::optional_object< TCachedToken< CModel > > TParticleModel;
  typedef rstl::optional_object< TCachedToken< CGenDescription > > TChildGeneratorDesc;
  typedef rstl::optional_object< TCachedToken< CSwooshDescription > > TSwooshGeneratorDesc;
  typedef TCachedToken< CElectricDescription > TElectricGeneratorDesc;

  CGenDescription();
  ~CGenDescription();

private:
  rstl::single_ptr< CIntElement > mPSLT;
  rstl::single_ptr< CIntElement > mPSWT;
  rstl::single_ptr< CRealElement > mPSTS;
  rstl::single_ptr< CVectorElement > mPOFS;
  rstl::single_ptr< CIntElement > mSEED;
  rstl::single_ptr< CRealElement > mLENG;
  rstl::single_ptr< CRealElement > mWIDT;
  rstl::single_ptr< CIntElement > mMAXP;
  rstl::single_ptr< CRealElement > mGRTE;
  rstl::single_ptr< CColorElement > mCOLR;
  rstl::single_ptr< CIntElement > mLTME;
  rstl::single_ptr< CEmitterElement > mEMTR;
  bool mSORT : 1;
  bool mMBLR : 1;
  bool mLINE : 1;
  bool mLIT_ : 1;
  bool mAAPH : 1;
  bool mZBUF : 1;
  bool mFXLL : 1;
  bool mPMAB : 1;
  bool mVMD4 : 1;
  bool mVMD3 : 1;
  bool mVMD2 : 1;
  bool mVMD1 : 1;
  bool mOPTS : 1;
  bool mPMUS : 1;
  bool mPMOO : 1;
  bool mCIND : 1;
  bool mORNT : 1;
  bool mRSOP : 1;
  rstl::single_ptr< CIntElement > mMBSP;
  rstl::single_ptr< CRealElement > mSIZE;
  rstl::single_ptr< CRealElement > mROTA;
  rstl::single_ptr< CUVElement > mTEXR;
  rstl::single_ptr< CUVElement > mTIND;
  TParticleModel mPMDL;
  rstl::single_ptr< CVectorElement > mPMOP;
  rstl::single_ptr< CVectorElement > mPMRT;
  rstl::single_ptr< CVectorElement > mPMSC;
  rstl::single_ptr< CColorElement > mPMCL;
  rstl::single_ptr< CModVectorElement > mVEL1;
  rstl::single_ptr< CModVectorElement > mVEL2;
  rstl::single_ptr< CModVectorElement > mVEL3;
  rstl::single_ptr< CModVectorElement > mVEL4;
  TChildGeneratorDesc mICTS;
  rstl::single_ptr< CIntElement > mNCSY;
  rstl::single_ptr< CIntElement > mCSSD;
  TChildGeneratorDesc mIDTS;
  rstl::single_ptr< CIntElement > mNDSY;
  TChildGeneratorDesc mIITS;
  rstl::single_ptr< CIntElement > mPISY;
  rstl::single_ptr< CIntElement > mSISY;
  rstl::single_ptr< CSpawnSystemKeyframeData > mKSSM;
  TSwooshGeneratorDesc mSSWH;
  rstl::single_ptr< CIntElement > mSSSD;
  rstl::single_ptr< CVectorElement > mSSPO;
  TElectricGeneratorDesc mSELC;
  rstl::single_ptr< CIntElement > mSESD;
  rstl::single_ptr< CVectorElement > mSEPO;
  rstl::single_ptr< CIntElement > mLTYP;
  rstl::single_ptr< CColorElement > mLCLR;
  rstl::single_ptr< CRealElement > mLINT;
  rstl::single_ptr< CVectorElement > mLOFF;
  rstl::single_ptr< CVectorElement > mLDIR;
  rstl::single_ptr< CIntElement > mLFOT;
  rstl::single_ptr< CRealElement > mLFOR;
  rstl::single_ptr< CRealElement > mLSLA;
  rstl::single_ptr< CRealElement > mADV1;
  rstl::single_ptr< CRealElement > mADV2;
  rstl::single_ptr< CRealElement > mADV3;
  rstl::single_ptr< CRealElement > mADV4;
  rstl::single_ptr< CRealElement > mADV5;
  rstl::single_ptr< CRealElement > mADV6;
  rstl::single_ptr< CRealElement > mADV7;
  rstl::single_ptr< CRealElement > mADV8;
};
CHECK_SIZEOF(CGenDescription, 0x12c)

#endif // _CGENDESCRIPTION
