#ifndef _CENVFXMANAGER
#define _CENVFXMANAGER

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CGenDescription;
class CStateManager;
class CTexture;
class CTransform4f;

enum EEnvFxType {
  kEFX_None,
  kEFX_Snow,
  kEFX_Rain,
  kEFX_UnderwaterFlake,
  kEFX_DarkWorld, // Guessed name
  kEFX_Unknown5,
  kEFX_Unknown6,
  kEFX_Unknown7,
};

class CVectorFixed8_8 {
public:
  CVectorFixed8_8() : mX(0), mY(0), mZ(0) {}
  CVectorFixed8_8(short x, short y, short z) : mX(x), mY(y), mZ(z) {}

  short GetX() const { return mX; }
  short GetY() const { return mY; }
  short GetZ() const { return mZ; }

  short& operator[](int index) { return (&mX)[index]; }

  CVectorFixed8_8& operator+=(const CVectorFixed8_8& other) {
    mX += other.mX;
    mY += other.mY;
    mZ += other.mZ;
    return *this;
  }

  CVectorFixed8_8 operator+(const CVectorFixed8_8& other) const {
    return CVectorFixed8_8(mX + other.mX, mY + other.mY, mZ + other.mZ);
  }

  CVectorFixed8_8 operator-(const CVectorFixed8_8& other) const {
    return CVectorFixed8_8(mX - other.mX, mY - other.mY, mZ - other.mZ);
  }

  CVectorFixed8_8 operator*(short scale) const {
    return CVectorFixed8_8((mX * scale) >> 8, (mY * scale) >> 8, (mZ * scale) >> 8);
  }

  static CVectorFixed8_8 FromCVector3f(const CVector3f& value);
  CVector3f ToCVector3f() const;

  short mX;
  short mY;
  short mZ;
};
CHECK_SIZEOF(CVectorFixed8_8, 0x6)

inline short real_to_fixed8_8(float value) {
  return static_cast< short >(static_cast< int >(256.f * value));
}

inline float fixed8_8_to_real(short value) { return (1.f / 256.f) * static_cast< float >(value); }

inline CVectorFixed8_8 CVectorFixed8_8::FromCVector3f(const CVector3f& value) {
  short x = real_to_fixed8_8(value.GetX());
  short y = real_to_fixed8_8(value.GetY());
  short z = real_to_fixed8_8(value.GetZ());
  return CVectorFixed8_8(x, y, z);
}

inline CVector3f CVectorFixed8_8::ToCVector3f() const {
  return CVector3f(fixed8_8_to_real(mX), fixed8_8_to_real(mY), fixed8_8_to_real(mZ));
}

class CEnvFxManagerGrid {
  friend class CEnvFxManager;

public:
  CEnvFxManagerGrid(const CVector2i& position, const CVector2i& extent,
                    const rstl::vector< CVectorFixed8_8 >& initialParticles, int reserve);

  void Render(const CTransform4f& xf, const CTransform4f& invXf, const CTransform4f& camXf,
              float density, EEnvFxType type);
  // Guessed name
  void RenderDarkWorldParticles(const CTransform4f& xf, const CTransform4f& invXf,
                                const CTransform4f& camXf, float density,
                                const CVectorFixed8_8* offsets, const CVectorFixed8_8* upDeltas,
                                const CVectorFixed8_8* rightDeltas);

  void SetDirty(bool dirty) { mBlockDirty = dirty; }
  bool IsDirty() const { return mBlockDirty; }
  const CVector2i& GetStart() const { return mPosition; }
  const CVector2i& GetSize() const { return mExtent; }
  void SetStart(const CVector2i& start) { mPosition = start; }
  rstl::pair< bool, float > GetVisibility() const { return mBlock; }
  void SetVisibility(rstl::pair< bool, float > block) { mBlock = block; }
  rstl::vector< CVectorFixed8_8 >& Particles() { return mParticles; }
  const rstl::vector< CVectorFixed8_8 >& Particles() const { return mParticles; }

private:
  // Guessed name
  bool SetupRender(const CTransform4f& xf, const CTransform4f& invXf, const CTransform4f& camXf,
                   float density, EEnvFxType type);
  void RenderRainParticles(const CTransform4f& camXf);
  void RenderSnowParticles(const CTransform4f& camXf);
  void RenderUnderwaterParticles(const CTransform4f& camXf);
  // Guessed names; effect 5 uses billboards, effects 6 and 7 use eight-point trails.
  void RenderDriftingParticles(const CTransform4f& camXf);
  void RenderParticleTrails(EEnvFxType type);

  bool mBlockDirty : 1;
  CVector2i mPosition;              // 8.8 fixed point
  CVector2i mExtent;                // 8.8 fixed point
  rstl::pair< bool, float > mBlock; // Visibility and world-space blocking height
  rstl::vector< CVectorFixed8_8 > mParticles;
  rstl::vector< float > mParticleLifetimes; // Guessed name; normalized remaining lifetime
  rstl::vector< int > mTrailFrames;         // Guessed name; selects/interpolates trail history
};
CHECK_SIZEOF(CEnvFxManagerGrid, 0x4c)

class CEnvFxManager {
public:
  CEnvFxManager();

  void Update(float dt, CStateManager& mgr);
  void Render(const CStateManager& mgr);
  static void Initialize();
  void FadeDensity(float density, int speed);
  // Guessed names; these fade the rain audio, not the particles.
  void StopRainSounds();
  void PlayRainSounds();
  void AreaLoaded();
  void AsyncLoadResources(CStateManager& mgr);
  void Cleanup();
  // Guessed name
  void ClearParticles();

  void SetSplashRate(float rate) { mBaseSplashRate = rate; }
  bool IsSplashActive() const { return mEnableSplash; }
  float GetRainMagnitude() const { return mFxDensity; }

private:
  void SetSplashEffectRate(float rate, CStateManager& mgr);
  void UpdateRainSounds(float dt, CStateManager& mgr);
  CVector3f GetParticleBoundsToWorldScale() const;
  CTransform4f GetParticleBoundsToWorldTransform() const;
  void UpdateVisorSplash(CStateManager& mgr, float dt, const CTransform4f& camXf);
  void MoveWrapCells(EEnvFxType type, int moveX, int moveY);
  void CalculateSnowForces(const CVectorFixed8_8& zVec,
                           rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                           EEnvFxType type, const CVector3f& inverseScale, float dt);
  static void BuildBlockObjectList(rstl::reserved_vector< TUniqueId, 1024 >& list,
                                   CStateManager& mgr);
  void UpdateBlockedGrids(CStateManager& mgr, EEnvFxType type, const CTransform4f& camXf,
                          const CTransform4f& xf, const CTransform4f& invXf);
  void CreateNewParticles(EEnvFxType type, const CTransform4f& invXf);
  void UpdateSnowParticles(rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces);
  void UpdateRainParticles(const CVectorFixed8_8& zVec, const CVector3f& inverseScale, float dt);
  void UpdateUnderwaterParticles(const CVectorFixed8_8& zVec);
  // Guessed names
  void UpdateDriftingParticles(float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                               const CTransform4f& invXf);
  void UpdateParticleTrails(float dt, const CVectorFixed8_8& zVec);
  void UpdateDarkWorldParticles(float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                                const CTransform4f& invXf);
  void SetupSnowTevs(CStateManager& mgr);
  void SetupRainTevs();
  void SetupUnderwaterTevs(const CTransform4f& invXf, CStateManager& mgr);
  void SetupDefaultTevSwapMode();
  void BlankFirstSnowflakeMip(CTexture& tex);
  // Guessed names
  void SetupDriftingParticleTevs(CStateManager& mgr);
  void SetupDarkWorldTevs();
  void SetupParticleTrailTevs(CStateManager& mgr);

  CAABox mParticleBounds;
  CVector3f mFocusCellPosition;
  bool mEnableSplash;
  float mFirstSnowForce;
  int mLastBlockedGridIdx;
  float mFxDensity;
  float mTargetFxDensity;
  float mMaxDensityDeltaSpeed;
  float mRainSoundFade; // Guessed name
  bool mSnowflakeTextureMipBlanked;
  rstl::optional_object< TLockedToken< CTexture > > mTxtrEnvGradient;
  rstl::reserved_vector< CEnvFxManagerGrid, 64 > mGrids;
  float mBaseSplashRate;
  rstl::optional_object< TLockedToken< CGenDescription > > mEnvRainSplash;
  rstl::reserved_vector< TUniqueId, 4 > mEnvRainSplashIds;
  bool mRainSoundActive;
  CSfxHandle mLeftRainSound;
  CSfxHandle mRightRainSound;
  bool mRainSoundsStopped; // Guessed name
  rstl::optional_object< TLockedToken< CTexture > > mTxtrSnowFlake;
  rstl::reserved_vector< CVector3f, 16 > mSnowZDeltas;
  rstl::optional_object< TLockedToken< CTexture > > mUnderwaterFlake;
  TLockedToken< CTexture > mDarkWorldParticleTexture; // Guessed name
  EEnvFxType mPreviousFxType;                         // Guessed name
};
CHECK_SIZEOF(CEnvFxManager, 0x147c)

#endif // _CENVFXMANAGER
