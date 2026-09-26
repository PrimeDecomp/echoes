#ifndef _CWEAPONDESCRIPTION
#define _CWEAPONDESCRIPTION

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CVectorElement;
class CModVectorElement;
class CIntElement;
class CRealElement;
class CColorElement;
class CUVElement;
class CModel;
class CGenDescription;
class CSwooshDescription;
class CCollisionResponseData;

class CWeaponDescription {
public:
  typedef rstl::optional_object< TLockedToken< CModel > > TParticleModel;
  typedef rstl::optional_object< TLockedToken< CGenDescription > > TChildGeneratorDesc;
  typedef rstl::optional_object< TLockedToken< CSwooshDescription > > TSwooshGeneratorDesc;
  typedef rstl::optional_object< TLockedToken< CCollisionResponseData > > TCollisionResponseDesc;

  CWeaponDescription();
  ~CWeaponDescription();

  const TCollisionResponseDesc& GetCollisionResponse() const { return mCOLR; }
  const TParticleModel& GetOHEF() const { return mOHEF; }

  CVectorElement* mIORN;
  CVectorElement* mIVEC;
  CVectorElement* mPSOV;
  CModVectorElement* mPSVM;
  CIntElement* mPSLT;
  CVectorElement* mPSCL;
  CColorElement* mPCOL;
  CVectorElement* mPOFS;
  CVectorElement* mOFST;
  CUVElement* mB1TX;
  CVectorElement* mB1PO;
  CColorElement* mB1CL;
  CRealElement* mB1SE;
  CRealElement* mB1RT;
  CUVElement* mB2TX;
  CVectorElement* mB2PO;
  CColorElement* mB2CL;
  CRealElement* mB2SE;
  CRealElement* mB2RT;
  CUVElement* mTTEX;
  CVectorElement* mTLPO;
  CColorElement* mTSCL;
  CColorElement* mTECL;
  CRealElement* mTSZE;
  CRealElement* mTLEN;
  bool mAPSO : 1;
  bool mF60H : 1;
  bool mSVBD : 1;
  bool mNDTT : 1;
  bool mHOMG : 1;
  bool mDP1C : 1;
  bool mAP11 : 1;
  bool mSPS1 : 1;
  bool mDP2C : 1;
  bool mAP21 : 1;
  bool mSPS2 : 1;
  bool mAS11 : 1;
  bool mAS12 : 1;
  bool mAS13 : 1;
  bool mEELT : 1;
  bool mEWTR : 1;
  bool mLWTR : 1;
  bool mSWTR : 1;
  bool mFC60 : 1;
  bool mRWPE : 1;
  bool mVMD2 : 1;
  bool mRB1A : 1;
  bool mRB2A : 1;
  bool mRTLA : 1;
  bool mBHBT : 1;
  CRealElement* mTRAT;
  TChildGeneratorDesc mAPSM;
  TChildGeneratorDesc mAPS2;
  TSwooshGeneratorDesc mASW1;
  TSwooshGeneratorDesc mASW2;
  TSwooshGeneratorDesc mASW3;
  TParticleModel mOHEF;
  TCollisionResponseDesc mCOLR;
  int mPJFX;
  CRealElement* mRNGE;
  CRealElement* mFOFF;
};
CHECK_SIZEOF(CWeaponDescription, 0xe8)

#endif // _CWEAPONDESCRIPTION
