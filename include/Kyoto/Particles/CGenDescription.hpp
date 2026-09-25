#ifndef _CGENDESCRIPTION
#define _CGENDESCRIPTION

#include "types.h"

#include "Kyoto/CToken.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CElectricDescription;
class CModel;
class CSpawnSystemKeyframeData;
class CSwooshDescription;

class CGenDescription {
public:
  typedef rstl::optional_object< TCachedToken< CModel > > TParticleModel;
  typedef rstl::optional_object< TCachedToken< CGenDescription > > TChildGeneratorDesc;
  typedef rstl::optional_object< TCachedToken< CSwooshDescription > > TSwooshGeneratorDesc;
  typedef rstl::optional_object< CToken > TElectricGeneratorDesc;

  CGenDescription();
  ~CGenDescription();

  CIntElement* mPSLT;
  CIntElement* mPSWT;
  CRealElement* mPSTS;
  CVectorElement* mPOFS;
  CIntElement* mSEED;
  CRealElement* mLENG;
  CRealElement* mWIDT;
  CIntElement* mMAXP;
  CRealElement* mGRTE;
  CColorElement* mCOLR;
  CIntElement* mLTME;
  CEmitterElement* mEMTR;
  bool mLINE : 1;
  bool mFXLL : 1;
  bool mAAPH : 1;
  bool mZBUF : 1;
  bool mSORT : 1;
  bool mLIT_ : 1;
  bool mORNT : 1;
  bool mRSOP : 1;
  bool mMBLR : 1;
  bool mPMAB : 1;
  bool mPMUS : 1;
  bool mPMOO : 1;
  bool mVMD1 : 1;
  bool mVMD2 : 1;
  bool mVMD3 : 1;
  bool mVMD4 : 1;
  bool mCIND : 1;
  bool mINDM : 1;
  bool mOPTS : 1;
  bool mRDOP : 1;
  bool mVMPC : 1;
  CIntElement* mMBSP;
  CRealElement* mSIZE;
  CRealElement* mROTA;
  CUVElement* mTEXR;
  CUVElement* mTIND;
  CIntElement* mXTAD;
  CRealElement* mFXBR;
  CVectorElement* mFXBO;
  TParticleModel mPMDL;
  CVectorElement* mPMOP;
  CVectorElement* mPMRT;
  CVectorElement* mPMSC;
  CColorElement* mPMCL;
  CVectorElement* mPMOV;
  CModVectorElement* mVEL1;
  CModVectorElement* mVEL2;
  CModVectorElement* mVEL3;
  CModVectorElement* mVEL4;
  TChildGeneratorDesc mICTS;
  CIntElement* mNCSY;
  CIntElement* mCSSD;
  TChildGeneratorDesc mIDTS;
  CIntElement* mNDSY;
  TChildGeneratorDesc mIITS;
  CIntElement* mPISY;
  CIntElement* mSISY;
  CSpawnSystemKeyframeData* mKSSM;
  TSwooshGeneratorDesc mSSWH;
  CIntElement* mSSSD;
  CVectorElement* mSSPO;
  TElectricGeneratorDesc mSELC;
  CIntElement* mSESD;
  CVectorElement* mSEPO;
  CIntElement* mLTYP;
  CColorElement* mLCLR;
  CRealElement* mLINT;
  CVectorElement* mLOFF;
  CVectorElement* mLDIR;
  CIntElement* mLFOT;
  CRealElement* mLFOR;
  CRealElement* mLSLA;
  CRealElement* mADV1;
  CRealElement* mADV2;
  CRealElement* mADV3;
  CRealElement* mADV4;
  CRealElement* mADV5;
  CRealElement* mADV6;
  CRealElement* mADV7;
  CRealElement* mADV8;
  CRealElement* mADV9;
  CVectorElement* mVAV1;
  CVectorElement* mVAV2;
  CVectorElement* mVAV3;
  int mDFLG;
};
CHECK_SIZEOF(CGenDescription, 0x150)

#endif // _CGENDESCRIPTION
