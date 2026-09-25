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
  virtual rstl::optional_object<CAABox> GetBounds() override;
  virtual int GetParticleCount() override;
  virtual bool SystemHasLight() override;
  virtual CLight GetLight() override;
  virtual void DestroyParticles() override;

  virtual uint Get4CharId() const override;

  int GetEmitterTime() const override;
  int GetSystemCount();
  void EndLifetime();

  bool IsIndirectTextured() const; // { return x28_loadedGenDesc->x54_x40_TEXR && x28_loadedGenDesc->x58_x44_TIND; }
  float GetExternalVar(int index) const;

  static void Initialize();
  static void ShutDown();

  void SetGlobalOrientAndTrans(const CTransform4f& xf);

  static void SetSubtractBlend(bool subtract) { sSubtractBlend = subtract; }

private:
  // Echoes layout not yet mapped; Prime's members do not fit the 0x338-byte G2ME01 object.
  uchar x20_data[0x318];

  static bool sSubtractBlend;

public:
  // Hypothesis: the sbss flag read by the G2ME01 CElementGen render paths.
  static bool sMoveRedToAlphaBuffer;
};
CHECK_SIZEOF(CElementGen, 0x338)

#endif // _CELEMENTGEN
