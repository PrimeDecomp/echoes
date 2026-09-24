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
    int x0_endFrame;
    CVector3f x4_pos;
    CVector3f x10_prevPos;
    CVector3f x1c_vel;
    int x28_startFrame;
    float x2c_lineLengthOrSize;
    float x30_lineWidthOrRota;
    CColor x34_color;
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

  int GetCumulativeParticleCount() const { return x258_cumulativeParticles; }
  bool IsIndirectTextured()
      const; // { return x2c_loadedGenDesc->x54_x40_TEXR && x2c_loadedGenDesc->x58_x44_TIND; }
  float GetExternalVar(int index) const;

  static void Initialize();
  static void ShutDown();

  void SetGlobalOrientAndTrans(const CTransform4f& xf);
  void SetLeaveLightsEnabledForModelRender(bool b) { x265_26_modelsUseLights = b; }

  static void SetSubtractBlend(bool subtract) { sSubtractBlend = subtract; }

public:
  TLockedToken< CGenDescription > x20_genDesc;
  CGenDescription* x2c_loadedGenDesc;
  EModelOrientationType x30_orientType;
  rstl::vector< CParticle > x34_particles;
  rstl::vector< CMatrix3f > x44_parentMatrices;
  rstl::vector< float[8] > x54_advValues;
  int x64_internalStartFrame;
  int x68_curFrame;
  double x70_curSeconds;
  float x78_timeDeltaScale;
  int x7c_prevFrame;
  bool x80_particleEmission;
  float x84_generatorRemainder;
  int x88_MAXP;
  ushort x8c_randomSeed;
  float x90_generatorRate;
  float x94_externalVars[16];
  CVector3f xd4_translation;
  CVector3f xe0_globalTranslation;
  CVector3f xec_POFS;
  CVector3f xf8_globalScale;
  CTransform4f x104_globalScaleTransform;
  CTransform4f x134_globalScaleTransformInverse;
  CVector3f x164_localScale;
  CTransform4f x170_localScaleTransform;
  CTransform4f x1a0_localScaleTransformInverse;
  CTransform4f x1d0_orientation;
  CMatrix3f x200_orientationInverse;
  CTransform4f x224_globalOrientation;
  uint x254_activeParticleCount;
  uint x258_cumulativeParticles;
  uint x25c_recursiveParticleCount;
  int x260_PSLT;
  bool x264_24_translationDirty : 1;
  bool x264_25_LIT_ : 1;
  bool x264_26_AAPH : 1;
  bool x264_27_ZBUF : 1;
  bool x264_28_zTest : 1;
  bool x264_29_ORNT : 1;
  bool x264_30_MBLR : 1;
  bool x264_31_LINE : 1;
  bool x265_24_FXLL : 1;
  bool x265_25_warmedUp : 1;
  bool x265_26_modelsUseLights : 1;
  bool x265_27_enableOPTS : 1;
  bool x265_28_enableADV : 1;
  int x268_MBSP;
  GXLightID x26c_backupLightActive;
  bool x270_hasVMD[4];
  CRandom16 x274_randState;
  CModVectorElement* x278_VELSources[4];
  rstl::vector< rstl::auto_ptr< CParticleGen > > x288_activePartChildren;
  int x298_CSSD;
  int x29c_SISY;
  int x2a0_PISY;
  int x2a4_SSSD;
  CVector3f x2a8_SSPO;
  int x2b4_SESD;
  CVector3f x2b8_SEPO;
  float x2c4_;
  float x2c8_;
  CVector3f x2cc_aabbMin;
  CVector3f x2d8_aabbMax;
  float x2e4_maxSize;
  CAABox x2e8_systemBounds;
  LightType x300_lightType;
  CColor x304_LCLR;
  float x308_LINT;
  CVector3f x30c_LOFF;
  CVector3f x318_LDIR;
  EFalloffType x324_falloffType;
  float x328_LFOR;
  float x32c_LSLA;
  CColor x330_moduColor;

  static bool sSubtractBlend;

public:
  // Hypothesis: the sbss flag read by the G2ME01 CElementGen render paths.
  static bool sMoveRedToAlphaBuffer;
};
CHECK_SIZEOF(CElementGen, 0x338)

#endif // _CELEMENTGEN
