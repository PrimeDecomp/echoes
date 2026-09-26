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
  // CParticleGen
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
  int GetSystemCount() override;
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
  TLockedToken< CSwooshDescription > mDesc;
  int mCurFrame;
  int mPSLT;
  double mCurTime;
  CVector3f mTranslation;
  CTransform4f mOrientation;
  CTransform4f mInvOrientation;
  CVector3f mGlobalTranslation;
  CTransform4f mGlobalOrientation;
  CVector3f mGlobalScale;
  CTransform4f mScaleXf;
  CTransform4f mInvScaleXf;
  CVector3f mLocalScale;
  int mCurParticle;
  rstl::vector< SSwooshData > mSwooshes;
  rstl::vector< CVector3f > mP0;
  rstl::vector< CVector3f > mP1;
  rstl::vector< CVector3f > mP2;
  rstl::vector< CVector3f > mP3;
  int mParticleCount;
  int mSPLN;
  int mLENG;
  int mSIDE;
  GXPrimitive mPrim;
  CRandom16 mRand;
  float x1cc_;
  float x1d0_;
  float mTSPNFloat;
  bool mEmitting : 1;
  bool mAALP : 1;
  bool mForceOneUpdate : 1;
  bool mRenderGaps : 1;
  bool mLLRD : 1;
  bool mVLS1 : 1;
  bool mVLS2 : 1;
  bool mConstantTex : 1;
  bool mConstantUv : 1;
  float mUMin;
  float mVMin;
  float mUMax;
  float mVMax;
  CTexture* mTex;
  float mUvSpan;
  int mTSPNInt;
  CVector3f mAabbMin;
  CVector3f mAabbMax;
  float mMaxRadius;
  CColor mModuColor;
};

CHECK_SIZEOF(CParticleSwoosh, 0x218)
NESTED_CHECK_SIZEOF(CParticleSwoosh, SSwooshData, 0x80)

#endif // _CPARTICLESWOOSH
