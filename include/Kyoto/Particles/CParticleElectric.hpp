#ifndef _CPARTICLEELECTRIC
#define _CPARTICLEELECTRIC

#include "Kyoto/Particles/CParticleGen.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CElectricDescription;
class CElementGen;
class CParticleSwoosh;
struct SUVElementSet;

class CParticleElectric : public CParticleGen {
public:
  explicit CParticleElectric(TToken< CElectricDescription > desc);

  ~CParticleElectric() override;
  const bool Update(double) override;
  void Render() override;
  void SetOrientation(const CTransform4f& orientation) override;
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
  void SetParticleEmission(bool emission) override;
  void SetModulationColor(const CColor& col) override;
  void SetGeneratorRate(float rate) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override;
  bool GetParticleEmission() const override { return mEmitting; }
  const CColor& GetModulationColor() const override;
  float GetGeneratorRate() const override;
  int GetEmitterTime() const override;
  int GetSystemCount() override;
  bool IsSystemDeletable() override;
  rstl::optional_object< CAABox > GetBounds() override;
  int GetParticleCount() override;
  bool SystemHasLight() override;
  CLight GetLight() override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  class CLineManager {
    friend class CParticleElectric;
    rstl::vector< CVector3f, rstl::aligned_allocator > mVerts;
    float mWidth1;
    float mWidth2;
    float mWidth3;
    CColor mColor1;
    CColor mColor2;
    CColor mColor3;
    CAABox mBounds;

  public:
    CLineManager()
    : mWidth1(1.f)
    , mWidth2(2.f)
    , mWidth3(3.f)
    , mColor1(CColor::White())
    , mColor2(CColor::White())
    , mColor3(CColor::White())
    , mBounds(CAABox::MakeMaxInvertedBox()) {}
  };

  class CParticleElectricManager {
    friend class CParticleElectric;
    uint mIdx;
    uint mSLIF;
    uint mStartFrame;
    uint mEndFrame;
    int mGPSMIdx;
    int mEPSMIdx;
    CVector3f mInitialPos;
    CVector3f mInitialVel;
    CVector3f mFinalPos;
    CVector3f mFinalVel;

  public:
    CParticleElectricManager(uint idx, uint slif, uint startFrame);
  };

  void ForceParticleCreation(int count);
  void SetOverrideIPos(const CVector3f& vec) { mOverrideIPos = vec; }
  void SetOverrideIVel(const CVector3f& vec) { mOverrideIVel = vec; }
  void SetOverrideFPos(const CVector3f& vec) { mOverrideFPos = vec; }
  void SetOverrideFVel(const CVector3f& vec) { mOverrideFVel = vec; }
  static void SetGlobalSeed(ushort seed) { sSeed = seed; }
  static ushort GetGlobalSeed() { return sSeed; }

private:
  void SetupLineGXMaterial();
  void DrawLineStrip(const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
                     const CColor& color);
  void DrawTexturedLineStrip(const rstl::vector< CVector3f, rstl::aligned_allocator >& verts,
                             float width, const CColor& color, const SUVElementSet& uvs);
  void RenderLines();
  void RenderSwooshes();
  void UpdateCachedTransform();
  void UpdateLine(int idx, int frame);
  void UpdateElectricalEffects();
  void CalculateFractal(int start, int end, float ampl, float ampd);
  void CalculatePoints();
  void CreateNewParticles(int count);
  void AddElectricalEffects();
  void BuildBounds();

  TLockedToken< CElectricDescription > mElecDesc;
  int mCurrentFrame;
  int mLIFE;
  double mCurTime;
  CVector3f mTranslation;
  CTransform4f mOrientation;
  CTransform4f mInvOrientation;
  CVector3f mGlobalTranslation;
  CTransform4f mGlobalOrientation;
  CVector3f mGlobalScale;
  CVector3f mLocalScale;
  CTransform4f mCachedXf;
  float x130_;
  float mRenderTime;
  CVector3f mMinBounds;
  CVector3f mMaxBounds;
  float x150_;
  CRandom16 mRandState;
  int mSSEG;
  int mSCNT;
  float mGenRem;
  float mGeneratorRate;
  int x168_;
  CAABox mSystemBounds;
  rstl::optional_object< CVector3f > mOverrideIPos;
  rstl::optional_object< CVector3f > mOverrideIVel;
  rstl::optional_object< CVector3f > mOverrideFPos;
  rstl::optional_object< CVector3f > mOverrideFVel;
  CColor mModuColor;
  rstl::reserved_vector< bool, 32 > mAllocated;
  rstl::reserved_vector< rstl::auto_ptr< CParticleSwoosh >, 32 > mSwooshGenerators;
  rstl::reserved_vector< rstl::auto_ptr< CLineManager >, 32 > mLineManagers;
  rstl::list< CParticleElectricManager > mElectricManagers;
  rstl::vector< rstl::auto_ptr< CElementGen > > mGPSMGenerators;
  rstl::vector< rstl::auto_ptr< CElementGen > > mEPSMGenerators;
  rstl::vector< CVector3f, rstl::aligned_allocator > mCalculatedVerts;
  rstl::vector< float > mFractalMags;
  rstl::vector< CVector3f > mFractalOffsets;
  bool mEmitting : 1;
  bool mHaveGPSM : 1;
  bool mHaveEPSM : 1;
  bool mHaveSSWH : 1;
  bool mHaveLWD : 1;
  bool mTransformDirty : 1;
  bool mTranslationDirty : 1;
  bool mOrientationDirty : 1;
  bool mGlobalScaleDirty : 1;
  bool mLocalScaleDirty : 1;

  static ushort sSeed;
};
CHECK_SIZEOF(CParticleElectric, 0x460)
NESTED_CHECK_SIZEOF(CParticleElectric, CLineManager, 0x40)
NESTED_CHECK_SIZEOF(CParticleElectric, CParticleElectricManager, 0x48)

#endif // _CPARTICLEELECTRIC
