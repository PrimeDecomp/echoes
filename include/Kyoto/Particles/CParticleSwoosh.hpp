#ifndef _CPARTICLESWOOSH
#define _CPARTICLESWOOSH

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/vector.hpp"

class CSwooshDescription;

class CParticleSwoosh : public CParticleGen {
  friend class CParticleElectric;

public:
  struct SSwooshData {
    bool mActive;
    float mLeftRad;
    float mRightRad;
    CVector3f mTranslation;
    CVector3f mOffset;
    CVector3f mUseOffset;
    float mInitialRot;
    float mRotm;
    CTransform4f mOrientation;
    int mFrame;
    CColor mColor;
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
  uchar x20_pad[0x140];
  int mCurParticle;
  rstl::vector< SSwooshData > mSwooshes;
  uchar x174_pad[0x64];
  bool mEmitting : 1;
  bool x1d8_25_ : 1;
  bool mForceOneUpdate : 1;
  uchar x1d9_pad[0x3f];
};
CHECK_SIZEOF(CParticleSwoosh, 0x218)
NESTED_CHECK_SIZEOF(CParticleSwoosh, SSwooshData, 0x80)

#endif // _CPARTICLESWOOSH
