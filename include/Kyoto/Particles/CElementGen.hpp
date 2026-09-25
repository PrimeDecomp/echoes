#ifndef _CELEMENTGEN
#define _CELEMENTGEN

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

class CGenDescription;
class CModVectorElement;

class CElementGen : public CParticleGen {
public:
  enum EModelOrientationType {
    kMOT_Normal,
    kMOT_One,
  };
  enum EOptionalSystemFlags {
    kOSF_None,
    kOSF_One,
    kOSF_Two,
  };
  enum LightType {
    kLT_None = 0,
    kLT_Custom = 1,
    kLT_Directional = 2,
    kLT_Spot = 3,
  };
  struct CParticle {
    int mEndFrame;
    CVector3f mPos;
    CVector3f mPrevPos;
    CVector3f mVel;
    int mStartFrame;
    float mLineLengthOrSize;
    float mLineWidthOrRota;
    CColor mColor;
  };

  CElementGen(TToken< CGenDescription >, EModelOrientationType = kMOT_Normal,
              EOptionalSystemFlags = kOSF_One);
  ~CElementGen() override;

  virtual const bool Update(double) override;
  virtual void Render() override;
  virtual void SetOrientation(const CTransform4f& orientation) override;
  virtual void SetTranslation(const CVector3f& translation) override;
  virtual void SetGlobalOrientation(const CTransform4f& orientation) override;
  virtual void SetGlobalTranslation(const CVector3f& translation) override;
  virtual void SetGlobalScale(const CVector3f& scale) override;
  virtual void SetLocalScale(const CVector3f& scale) override;
  virtual void SetParticleEmission(bool emission) override;
  virtual void SetModulationColor(const CColor& col) override;
  virtual void SetGeneratorRate(float rate) override {}

  virtual const CTransform4f& GetOrientation() const override;
  virtual const CVector3f& GetTranslation() const override;
  virtual const CTransform4f& GetGlobalOrientation() const override;
  virtual const CVector3f& GetGlobalTranslation() const override;
  virtual const CVector3f& GetGlobalScale() const override;
  virtual bool GetParticleEmission() const override;
  virtual const CColor& GetModulationColor() const override;

  virtual int GetActiveParticleCount() override;
  virtual bool IsSystemDeletable() override;
  virtual rstl::optional_object< CAABox > GetBounds() override;
  virtual int GetParticleCount() override;
  virtual bool SystemHasLight() override;
  virtual CLight GetLight() override;
  virtual void DestroyParticles() override;
  virtual uint Get4CharId() const override;

  int GetEmitterTime() const override;
  int GetSystemCount();
  void EndLifetime();

  int GetCumulativeParticleCount() const { return mCumulativeParticles; }
  bool IsIndirectTextured()
      const; // { return x2c_loadedGenDesc->x54_x40_TEXR && x2c_loadedGenDesc->x58_x44_TIND; }
  float GetExternalVar(int index) const;

  static void Initialize();
  static void ShutDown();

  void SetGlobalOrientAndTrans(const CTransform4f& xf);
  void SetLeaveLightsEnabledForModelRender(bool b) { mModelsUseLights = b; }

  static void SetSubtractBlend(bool subtract) { sSubtractBlend = subtract; }

public:
  TLockedToken< CGenDescription > mGenDesc;
  CGenDescription* mLoadedGenDesc;
  EModelOrientationType mOrientType;
  rstl::vector< CParticle > mParticles;
  rstl::vector< CMatrix3f > mParentMatrices;
  rstl::vector< float[8] > mAdvValues;
  int mInternalStartFrame;
  int mCurFrame;
  double mCurSeconds;
  float mTimeDeltaScale;
  int mPrevFrame;
  bool mParticleEmission;
  float mGeneratorRemainder;
  int mMAXP;
  ushort mRandomSeed;
  float mGeneratorRate;
  float mExternalVars[16];
  CVector3f mTranslation;
  CVector3f mGlobalTranslation;
  CVector3f mPOFS;
  CVector3f mGlobalScale;
  CTransform4f mGlobalScaleTransform;
  CTransform4f mGlobalScaleTransformInverse;
  CVector3f mLocalScale;
  CTransform4f mLocalScaleTransform;
  CTransform4f mLocalScaleTransformInverse;
  CTransform4f mOrientation;
  CMatrix3f mOrientationInverse;
  CTransform4f mGlobalOrientation;
  uint mActiveParticleCount;
  uint mCumulativeParticles;
  uint mRecursiveParticleCount;
  int mPSLT;
  bool mTranslationDirty : 1;
  bool mLIT_ : 1;
  bool mAAPH : 1;
  bool mZBUF : 1;
  bool mZTest : 1;
  bool mORNT : 1;
  bool mMBLR : 1;
  bool mLINE : 1;
  bool mFXLL : 1;
  bool mWarmedUp : 1;
  bool mModelsUseLights : 1;
  bool mEnableOPTS : 1;
  bool mEnableADV : 1;
  int mMBSP;
  GXLightID mBackupLightActive;
  bool mHasVMD[4];
  CRandom16 mRandState;
  CModVectorElement* mVELSources[4];
  rstl::vector< rstl::auto_ptr< CParticleGen > > mActivePartChildren;
  int mCSSD;
  int mSISY;
  int mPISY;
  int mSSSD;
  CVector3f mSSPO;
  int mSESD;
  CVector3f mSEPO;
  float x2c4_;
  float x2c8_;
  CVector3f mAabbMin;
  CVector3f mAabbMax;
  float mMaxSize;
  CAABox mSystemBounds;
  LightType mLightType;
  CColor mLCLR;
  float mLINT;
  CVector3f mLOFF;
  CVector3f mLDIR;
  EFalloffType mFalloffType;
  float mLFOR;
  float mLSLA;
  CColor mModuColor;

  static bool sSubtractBlend;

public:
  // Hypothesis: the sbss flag read by the G2ME01 CElementGen render paths.
  static bool sMoveRedToAlphaBuffer;
};
CHECK_SIZEOF(CElementGen, 0x338)

#endif // _CELEMENTGEN
