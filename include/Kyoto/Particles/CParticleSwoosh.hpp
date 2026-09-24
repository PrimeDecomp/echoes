#ifndef _CPARTICLESWOOSH
#define _CPARTICLESWOOSH

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/vector.hpp"

class CSwooshDescription;
class CTexture;

#include "dolphin/gx/GXEnum.h"

class CParticleSwoosh : public CParticleGen {
  friend class CParticleElectric;

public:
  struct SSwooshData {
    // Prime-derived record layout; internal field names remain cross-game hypotheses.

    bool mActive;
    float mLeftRad;
    float mRightRad;
    CVector3f mTranslation;    // Updated by system's velocity sources or user code
    CVector3f mOffset;         // Updated by POFS once per system update (also resets x24_useOffset)
    CVector3f mUseOffset;      // Combination of POFS and NPOS, once per particle instance
    float mInitialRot;         // Rotation bias once per system update
    float mRotm;               // Rotation bias once per particle instance
    CTransform4f mOrientation; // Updated by user code
    int mFrame;                // Frame index of evaluated data
    CColor mColor;             // Updated by COLR
    int mStartFrame;
    CVector3f mVelocity;
  };
  CParticleSwoosh(TToken< CSwooshDescription > desc, int leng);
  ~CParticleSwoosh() override;

  const bool Update(double dt) override;
  void Render() override;
  void SetOrientation(const CTransform4f& orientation) override;
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
  void SetParticleEmission(bool emission) override;
  void SetModulationColor(const CColor& col) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override;
  bool GetParticleEmission() const override;
  const CColor& GetModulationColor() const override;
  int GetEmitterTime() const override;
  int GetActiveParticleCount() override;
  bool IsSystemDeletable() override;
  rstl::optional_object< CAABox > GetBounds() override;
  int GetParticleCount() override;
  bool SystemHasLight() override;
  CLight GetLight() override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  // Recomputes the bounds from every active swoosh; name is a hypothesis.
  void UpdateAllBounds();

  void SetWarmUp() { mForceOneUpdate = true; }
  const int GetSwooshCount() const { return mSwooshes.size(); }

private:
  // Offsets corroborated by the Echoes constructor/getters; internal effect names follow Prime.
  TLockedToken< CSwooshDescription > x20_desc;
  int x2c_curFrame;
  int x30_PSLT;
  double x38_curTime;
  CVector3f x40_translation;
  CTransform4f x4c_orientation;
  CTransform4f x7c_invOrientation;
  CVector3f xac_globalTranslation;
  CTransform4f xb8_globalOrientation;
  CVector3f xe8_globalScale;
  CTransform4f xf4_scaleXf;
  CTransform4f x124_invScaleXf;
  CVector3f x154_localScale;
  int mCurParticle;
  rstl::vector< SSwooshData > mSwooshes;
  rstl::vector< CVector3f > x174_p0;
  rstl::vector< CVector3f > x184_p1;
  rstl::vector< CVector3f > x194_p2;
  rstl::vector< CVector3f > x1a4_p3;
  int x1b4_particleCount;
  int x1b8_SPLN;
  int x1bc_LENG;
  int x1c0_SIDE;
  GXPrimitive x1c4_prim;
  CRandom16 x1c8_rand;
  float x1cc_;
  float x1d0_;
  float x1d4_TSPN;
  bool mEmitting : 1;
  bool x1d8_25_AALP : 1;
  bool mForceOneUpdate : 1;
  bool x1d8_27_renderGaps : 1;
  bool x1d8_28_LLRD : 1;
  bool x1d8_29_VLS1 : 1;
  bool x1d8_30_VLS2 : 1;
  bool x1d8_31_constantTex : 1;
  bool x1d9_24_constantUv : 1;
  float x1dc_uMin;
  float x1e0_vMin;
  float x1e4_uMax;
  float x1e8_vMax;
  CTexture* x1ec_tex;
  float x1f0_uvSpan;
  int x1f4_TSPN;
  CVector3f x1f8_aabbMin;
  CVector3f x204_aabbMax;
  float x210_maxRadius;
  CColor x214_moduColor;
};

CHECK_SIZEOF(CParticleSwoosh, 0x218)
NESTED_CHECK_SIZEOF(CParticleSwoosh, SSwooshData, 0x80)

#endif // _CPARTICLESWOOSH
