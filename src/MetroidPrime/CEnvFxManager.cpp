#include "MetroidPrime/CEnvFxManager.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "rstl/auto_ptr.hpp"
#include <float.h>

// The target stores the largest finite single-precision value directly.
static const float skMaximumBlockingHeight = FLT_MAX;
static float g_SnowForces[256][2];
static float g_DarkWorldForceSpeed = 5.f;
static float g_DarkWorldRiseSpeed = 4.f;
static float g_DarkWorldImpulseScale = 2.5f;
static float g_DarkWorldImpulseRate = 20.f;
static int g_TrailPeriod = 8;
static int g_TrailSecondaryAxis = 2;
static float g_TrailPrimaryScale;
static float g_TrailDecayRate;
static int g_TrailPrimaryAxis;
static const CColor skTrailColor6(0.6f, 0.71f, 0.48f, 0.175f);
static const CColor skTrailColor7(0.f, 0.f, 1.f, 0.175f);

// Empty no-argument hook called at the start of the CEnvFxManager constructor.
extern "C" void fn_80168498() {}

CEnvFxManagerGrid::CEnvFxManagerGrid(const CVector2i& position, const CVector2i& extent,
                                     const rstl::vector< CVectorFixed8_8 >& initialParticles,
                                     int reserve)
: mBlockDirty(true)
, mPosition(position)
, mExtent(extent)
, mBlock(false, skMaximumBlockingHeight)
, mParticles(initialParticles) {
  mParticles.reserve(reserve);
}

CEnvFxManager::CEnvFxManager()
: mParticleBounds(CVector3f(-63.5f, -63.5f, -63.5f), CVector3f(63.5f, 63.5f, 63.5f))
, mFocusCellPosition(CVector3f::Zero())
, mEnableSplash(false)
, mFirstSnowForce(0.f)
, mLastBlockedGridIdx(-1)
, mFxDensity(0.f)
, mTargetFxDensity(0.f)
, mMaxDensityDeltaSpeed(0.f)
, mRainSoundFade(1.f)
, mSnowflakeTextureMipBlanked(false)
, mTxtrEnvGradient(TLockedToken< CTexture >(gpSimplePool->GetObj("TXTR_EnvGradient")))
, mEnvRainSplash(TLockedToken< CGenDescription >(gpSimplePool->GetObj("PART_EnvRainSplash")))
, mRainSoundActive(false)
, mRainSoundsStopped(false)
, mTxtrSnowFlake(TLockedToken< CTexture >(gpSimplePool->GetObj("TXTR_SnowFlake")))
, mUnderwaterFlake(TLockedToken< CTexture >(gpSimplePool->GetObj("TXTR_UnderwaterFlake")))
, mDarkWorldParticleTexture(gpSimplePool->GetObj("TXTR_DarkworldParticleTexture"))
, mPreviousFxType(kEFX_None) {
  fn_80168498();
  CRandom16 random(0);
  for (int i = 0; i < 4; ++i) {
    mEnvRainSplashIds.push_back(kInvalidUniqueId);
  }

  for (int row = 0; row < 8; ++row) {
    for (int column = 0; column < 8; ++column) {
      mGrids.push_back(CEnvFxManagerGrid(CVector2i(column * 0x800, row * 0x800),
                                         CVector2i(0x800, 0x800), rstl::vector< CVectorFixed8_8 >(),
                                         0xab));
    }
  }

  for (int i = 15; i >= 0; --i) {
    mSnowZDeltas.push_back(CVector3f(0.f, 0.f, random.Range(-2.f, -4.f)));
  }
}

void CEnvFxManagerGrid::RenderRainParticles(const CTransform4f& camXf) {
  int count = mParticles.size();
  float absDot = CMath::AbsF(CVector3f::Dot(camXf.GetUp(), CVector3f::Up()));
  CGX::Begin(GX_LINES, GX_VTXFMT6, count * 2);
  short zOffset = static_cast< short >(512.f * (1.f - absDot) + 256.f);
  for (int i = 0; i < count; ++i) {
    CVectorFixed8_8 particle = mParticles[i];
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord1s16(10);
    GXPosition3s16(particle.mX, particle.mY, particle.mZ + zOffset);
    GXTexCoord1s16(0);
  }
  CGX::End();
}

void CEnvFxManagerGrid::RenderSnowParticles(const CTransform4f& camXf) {
  short zx = real_to_fixed8_8(0.2f * camXf.Get02());
  short zy = real_to_fixed8_8(0.2f * camXf.Get12());
  short zz = real_to_fixed8_8(0.2f * camXf.Get22());
  short xx = real_to_fixed8_8(0.2f * camXf.Get00());
  short xy = real_to_fixed8_8(0.2f * camXf.Get10());
  short xz = real_to_fixed8_8(0.2f * camXf.Get20());
  const int count = mParticles.size();
  CGX::Begin(GX_QUADS, GX_VTXFMT6, count * 4);
  const CVectorFixed8_8* particles = mParticles.data();
  for (int i = count - 1; i >= 0; --i) {
    CVectorFixed8_8 particle = particles[i];
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 0);
    particle.mX += zx;
    particle.mY += zy;
    particle.mZ += zz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 2);
    particle.mX += xx;
    particle.mY += xy;
    particle.mZ += xz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(2, 2);
    particle.mX -= zx;
    particle.mY -= zy;
    particle.mZ -= zz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(2, 0);
  }
  CGX::End();
}

void CEnvFxManagerGrid::RenderDriftingParticles(const CTransform4f& camXf) {
  const short zx = real_to_fixed8_8(0.2f * camXf.Get02());
  const short zy = real_to_fixed8_8(0.2f * camXf.Get12());
  const short zz = real_to_fixed8_8(0.2f * camXf.Get22());
  const short xx = real_to_fixed8_8(0.2f * camXf.Get00());
  const short xy = real_to_fixed8_8(0.2f * camXf.Get10());
  const short xz = real_to_fixed8_8(0.2f * camXf.Get20());
  const int count = mParticles.size();
  CGX::Begin(GX_QUADS, GX_VTXFMT6, count * 4);
  const CVectorFixed8_8* particles = mParticles.data();
  for (int i = count - 1; i >= 0; --i) {
    CVectorFixed8_8 particle = particles[i];
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 0);
    particle.mX += zx;
    particle.mY += zy;
    particle.mZ += zz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 2);
    particle.mX += xx;
    particle.mY += xy;
    particle.mZ += xz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(2, 2);
    particle.mX -= zx;
    particle.mY -= zy;
    particle.mZ -= zz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(2, 0);
  }
  CGX::End();
}

void CEnvFxManagerGrid::RenderParticleTrails(EEnvFxType type) {
  const int count = mParticles.size() / 8;
  const int* frames = mTrailFrames.data();
  const float* lifetimes = mParticleLifetimes.data();
  const CVectorFixed8_8* particles = mParticles.data();
  for (int trail = 0; trail < count; ++trail) {
    const int frame = frames[trail];
    const int base = trail * 8;
    int segment = (frame / g_TrailPeriod) % 8;
    const short fraction = real_to_fixed8_8(static_cast< float >(frame % g_TrailPeriod) /
                                            static_cast< float >(g_TrailPeriod));
    CColor color = type == kEFX_Unknown6 ? skTrailColor6 : skTrailColor7;
    CVectorFixed8_8 position = particles[base + segment];
    uchar alpha = 0x7f;
    const float lifetime = lifetimes[trail];
    if (lifetime < 0.2f) {
      color.SetAlpha(color.GetAlpha() * lifetime / 0.2f);
    } else if (lifetime > 0.8f) {
      color.SetAlpha(color.GetAlpha() * (1.f - lifetime) / 0.2f);
    }
    GXSetTevColor(GX_TEVREG1, color.GetGXColor());
    CGX::Begin(GX_LINESTRIP, GX_VTXFMT6, 8);
    for (int point = 0; point < 8; ++point) {
      if (--segment < 0) {
        segment = 7;
      }
      const CVectorFixed8_8 next = position + particles[base + segment];
      if (point == 0) {
        const CVectorFixed8_8 interpolated = next + (position - next) * fraction;
        GXPosition3s16(interpolated.mX, interpolated.mY, interpolated.mZ);
      } else if (point == 6) {
        GXPosition3s16(position.mX, position.mY, position.mZ);
        GXTexCoord2u8(0, alpha);
        alpha -= 15;
        ++point;
        const CVectorFixed8_8 interpolated = next + (position - next) * fraction;
        GXPosition3s16(interpolated.mX, interpolated.mY, interpolated.mZ);
      } else {
        GXPosition3s16(position.mX, position.mY, position.mZ);
      }
      GXTexCoord2u8(0, alpha);
      alpha -= 15;
      position = next;
    }
    CGX::End();
  }
}

void CEnvFxManagerGrid::RenderUnderwaterParticles(const CTransform4f& camXf) {
  short zx = real_to_fixed8_8(0.5f * camXf.Get02());
  short zy = real_to_fixed8_8(0.5f * camXf.Get12());
  short zz = real_to_fixed8_8(0.5f * camXf.Get22());
  short xx = real_to_fixed8_8(0.5f * camXf.Get00());
  short xy = real_to_fixed8_8(0.5f * camXf.Get10());
  short xz = real_to_fixed8_8(0.5f * camXf.Get20());
  const int count = mParticles.size();
  CGX::Begin(GX_QUADS, GX_VTXFMT6, count * 4);
  const CVectorFixed8_8* particles = mParticles.data();
  for (int i = count - 1; i >= 0; --i) {
    CVectorFixed8_8 particle = particles[i];
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 0);
    particle.mX += zx;
    particle.mY += zy;
    particle.mZ += zz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 2);
    particle.mX += xx;
    particle.mY += xy;
    particle.mZ += xz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(2, 2);
    particle.mX -= zx;
    particle.mY -= zy;
    particle.mZ -= zz;
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(2, 0);
  }
  CGX::End();
}

bool CEnvFxManagerGrid::SetupRender(const CTransform4f& xf, const CTransform4f& invXf,
                                    const CTransform4f& camXf, float density, EEnvFxType type) {
  if (mParticles.empty()) {
    return false;
  }
  if (!mBlock.first) {
    return false;
  }
  const float gridX = fixed8_8_to_real(mPosition.GetX());
  const float gridY = fixed8_8_to_real(mPosition.GetY());
  const CTransform4f gridXf = xf * CTransform4f::Translate(gridX, gridY, 0.f);
  gpRender->SetModelMatrix(gridXf);

  if (type == kEFX_Snow || type == kEFX_Rain || type == kEFX_DarkWorld || type == kEFX_Unknown5 ||
      type == kEFX_Unknown6 || type == kEFX_Unknown7) {
    const CVector3f localUp = invXf * (mBlock.second * CVector3f::Up());
    float texMtx[2][4] = {{0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 10.f, 0.f}};
    texMtx[1][3] = -(10.f * localUp.GetZ() + 0.5f);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX5, GX_MTX2x4);
  }
  return true;
}

void CEnvFxManagerGrid::Render(const CTransform4f& xf, const CTransform4f& invXf,
                               const CTransform4f& camXf, float density, EEnvFxType type) {
  if (!SetupRender(xf, invXf, camXf, density, type)) {
    return;
  }

  switch (type) {
  case kEFX_Snow:
    RenderSnowParticles(camXf);
    break;
  case kEFX_Rain:
    RenderRainParticles(camXf);
    break;
  case kEFX_UnderwaterFlake:
    RenderUnderwaterParticles(camXf);
    break;
  case kEFX_Unknown5:
    RenderDriftingParticles(camXf);
    break;
  case kEFX_Unknown6:
  case kEFX_Unknown7:
    RenderParticleTrails(type);
    break;
  default:
    break;
  }
}

void CEnvFxManagerGrid::RenderDarkWorldParticles(const CTransform4f& xf, const CTransform4f& invXf,
                                                 const CTransform4f& camXf, float density,
                                                 const CVectorFixed8_8* offsets,
                                                 const CVectorFixed8_8* upDeltas,
                                                 const CVectorFixed8_8* rightDeltas) {
  if (!SetupRender(xf, invXf, camXf, density, kEFX_DarkWorld)) {
    return;
  }
  CGX::SetTevKColor(GX_KCOLOR0, CColor::White().GetGXColor());
  for (int i = mParticles.size() - 1; i >= 0; --i) {
    const float lifetime = mParticleLifetimes[i];
    float brightness = 1.5f * lifetime - 0.5f;
    if (brightness < 0.f) {
      brightness = 0.f;
    }
    const float remaining = 1.f - lifetime;
    const float remainingSquared = remaining * remaining;
    const uchar red = static_cast< uchar >(255.f * brightness);
    const uchar green = static_cast< uchar >(red * lifetime);
    const uchar alpha = static_cast< uchar >(255.f * -(remainingSquared * remainingSquared - 1.f));
    GXSetTevKColor(GX_KCOLOR0, CColor(red, green, red, alpha).GetGXColor());

    CGX::Begin(GX_QUADS, GX_VTXFMT6, 4);
    const CVectorFixed8_8 particle = mParticles[i] + offsets[i & 15];
    const CVectorFixed8_8& up = upDeltas[i & 15];
    const CVectorFixed8_8& right = rightDeltas[i & 15];
    GXPosition3s16(particle.mX, particle.mY, particle.mZ);
    GXTexCoord2u8(0, 0);
    int x = particle.mX + up.mX;
    int y = particle.mY + up.mY;
    int z = particle.mZ + up.mZ;
    GXPosition3s16(x, y, z);
    GXTexCoord2u8(0, 2);
    x += right.mX;
    y += right.mY;
    z += right.mZ;
    GXPosition3s16(x, y, z);
    GXTexCoord2u8(2, 2);
    x -= up.mX;
    y -= up.mY;
    z -= up.mZ;
    GXPosition3s16(x, y, z);
    GXTexCoord2u8(2, 0);
    CGX::End();
  }
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Black().GetGXColor());
}

CVector3f CEnvFxManager::GetParticleBoundsToWorldScale() const {
  return (1.f / 127.f) * (mParticleBounds.GetMaxPoint() - mParticleBounds.GetMinPoint());
}

void CEnvFxManager::MoveWrapCells(EEnvFxType type, int moveX, int moveY) {
  if (moveX == 0 && moveY == 0) {
    return;
  }

  const bool moveAll = CMath::AbsD(static_cast< double >(moveX)) >= 1.0 ||
                       CMath::AbsD(static_cast< double >(moveY)) >= 1.0;
  rstl::reserved_vector< rstl::pair< bool, float >, 64 > visibility;
  for (int i = 0; i < 64; ++i) {
    visibility.push_back(mGrids[i].GetVisibility());
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      CEnvFxManagerGrid& grid = mGrids[row * 8 + col];
      const int sourceCol = col - moveX;
      const int sourceRow = row - moveY;
      if (!moveAll && sourceCol >= 0 && sourceCol < 8 && sourceRow >= 0 && sourceRow < 8) {
        grid.SetVisibility(visibility[sourceRow * 8 + sourceCol]);
      } else {
        grid.SetDirty(true);
      }
      grid.SetStart(CVector2i((grid.GetStart().GetX() + (moveX << 11)) & 0x3fff,
                              (grid.GetStart().GetY() + (moveY << 11)) & 0x3fff));
    }
  }
}

void CEnvFxManager::AsyncLoadResources(CStateManager& mgr) {
  for (int playerIndex = 0; playerIndex < mgr.GetNumPlayers(); ++playerIndex) {
    const TUniqueId id = mgr.AllocateUniqueId();
    mEnvRainSplashIds[playerIndex] = id;
    CHUDBillboardEffect* effect =
        new CHUDBillboardEffect(rstl::optional_object< TToken< CGenDescription > >(*mEnvRainSplash),
                                rstl::optional_object< TToken< CElectricDescription > >(), id, true,
                                rstl::string_l("VisorRainSplashes"),
                                CHUDBillboardEffect::GetNearClipDistance(mgr, playerIndex),
                                CHUDBillboardEffect::GetScaleForPOV(mgr), playerIndex,
                                CColor::White(), CVector3f::One(), CVector3f::Zero(), false);
    effect->SetRunIndefinitely(true);
    mgr.AddObject(effect);
  }
}

void CEnvFxManager::Initialize() {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName("DUMB_SnowForces");
  rstl::auto_ptr< CInputStream > stream(
      gpResourceFactory->GetResLoader().LoadNewResourceSync(*tag, nullptr));
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < 2; ++j) {
      g_SnowForces[i][j] = stream->ReadFloat();
    }
  }
}

void CEnvFxManager::Cleanup() {
  mEnvRainSplashIds.clear();
  mRainSoundActive = false;
  mLeftRainSound.Clear();
  mRightRainSound.Clear();
}

void CEnvFxManager::ClearParticles() {
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    grid.mParticles = rstl::vector< CVectorFixed8_8 >();
    grid.mParticleLifetimes = rstl::vector< float >();
    grid.mTrailFrames = rstl::vector< int >();
  }
}

void CEnvFxManager::Update(float dt, CStateManager& mgr) {
  if (gpMain->IsMaxSpeed()) {
    return;
  }
  const CTransform4f camXf = mgr.GetCameraManager(0)->GetCurrentCameraTransform(mgr, true);
  const EEnvFxType type = static_cast< EEnvFxType >(mgr.GetWorld()->GetNeededEnvFx());
  switch (type) {
  case kEFX_Unknown6:
    g_TrailPeriod = 2;
    g_TrailPrimaryScale = 6.f;
    g_TrailDecayRate = 1.f / 3.f;
    g_TrailPrimaryAxis = 2;
    g_TrailSecondaryAxis = 0;
    break;
  case kEFX_Unknown7:
    g_TrailPeriod = 8;
    g_TrailPrimaryScale = 1.5f;
    g_TrailDecayRate = 1.f / 3.f;
    g_TrailPrimaryAxis = 0;
    g_TrailSecondaryAxis = 2;
    break;
  }

  if (mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true)->GetFluidCount() != 0) {
    mLastBlockedGridIdx = -1;
    mEnableSplash = false;
    SetSplashEffectRate(0.f, mgr);
  }
  UpdateRainSounds(dt, mgr);
  UpdateVisorSplash(mgr, dt, camXf);

  if (type != mPreviousFxType) {
    if (mPreviousFxType != kEFX_None) {
      ClearParticles();
      AreaLoaded();
    }
    mPreviousFxType = type;
    if (type == kEFX_None) {
      return;
    }
  }

  const float densityDelta = mTargetFxDensity - mFxDensity;
  mFxDensity += rstl::min_val(1.f, CMath::AbsF(densityDelta) / 0.15f) *
                CMath::Limit(densityDelta, dt * (mMaxDensityDeltaSpeed / 11000.f));

  const CVector3f scale = GetParticleBoundsToWorldScale();
  const CVector3f inverseScale(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
  const CVector3f forwardPoint = camXf.GetTranslation() + 23.8125f * camXf.GetForward();
  const CVector3f cellBase =
      forwardPoint - CVector3f(CMath::ModF(forwardPoint.GetX(), 7.9375f),
                               CMath::ModF(forwardPoint.GetY(), 7.9375f), 0.f);
  const CVector3f delta = mFocusCellPosition - cellBase;
  mFocusCellPosition = cellBase;
  MoveWrapCells(type, static_cast< int >(delta.GetX() / 7.9375f),
                static_cast< int >(delta.GetY() / 7.9375f));

  CVectorFixed8_8 zVec(0, 0, real_to_fixed8_8(delta.GetZ() * inverseScale.GetZ()));
  if (type == kEFX_UnderwaterFlake) {
    zVec.mZ += real_to_fixed8_8(0.5f * dt);
  } else if (type == kEFX_DarkWorld) {
    zVec.mZ += static_cast< short >(-10.f * dt);
  }
  rstl::reserved_vector< CVectorFixed8_8, 256 > snowForces;
  CalculateSnowForces(zVec, snowForces, type, inverseScale, dt);

  const CTransform4f xf = GetParticleBoundsToWorldTransform();
  const CTransform4f invXf = xf.GetInverse();
  UpdateBlockedGrids(mgr, type, camXf, xf, invXf);
  CreateNewParticles(type, invXf);
  mPreviousFxType = type;

  switch (type) {
  case kEFX_Snow:
    UpdateSnowParticles(snowForces);
    break;
  case kEFX_Rain:
    UpdateRainParticles(zVec, inverseScale, dt);
    break;
  case kEFX_UnderwaterFlake:
    UpdateUnderwaterParticles(zVec);
    break;
  case kEFX_DarkWorld:
    UpdateDarkWorldParticles(dt, snowForces, invXf);
    break;
  case kEFX_Unknown5:
    UpdateDriftingParticles(dt, snowForces, invXf);
    break;
  case kEFX_Unknown6:
  case kEFX_Unknown7:
    UpdateParticleTrails(dt, zVec);
    break;
  default:
    break;
  }

  mFirstSnowForce = CMath::ModF(
      ((type == kEFX_Snow || type == kEFX_DarkWorld || type == kEFX_Unknown5) ? 1.f : 0.125f) +
          mFirstSnowForce,
      256.f);
}

void CEnvFxManager::CreateNewParticles(EEnvFxType type, const CTransform4f& invXf) {
  const int totalParticleCount = type == kEFX_Snow              ? 0x1c98
                                 : type == kEFX_Rain            ? 11000
                                 : type == kEFX_UnderwaterFlake ? 0xfeb
                                 : type == kEFX_DarkWorld       ? 0x2ee
                                 : type == kEFX_Unknown5        ? 0x1c98
                                 : type == kEFX_Unknown6        ? 0x1c90
                                 : type == kEFX_Unknown7        ? 0x1c90
                                                                : 0;
  const int perCell = totalParticleCount / 64;
  int cellParticleCount = static_cast< int >(mFxDensity * perCell);
  int maxParticleCount = perCell;
  if (type == kEFX_Unknown6 || type == kEFX_Unknown7) {
    maxParticleCount -= maxParticleCount % 8;
    cellParticleCount -= cellParticleCount % 8;
  }

  static uint seed = 0;
  CRandom16 random(seed);
  const bool darkWorld = type == kEFX_DarkWorld;
  const bool leavingDarkWorld = type != kEFX_DarkWorld && mPreviousFxType == kEFX_DarkWorld;
  const bool trails = type == kEFX_Unknown6 || type == kEFX_Unknown7;
  const bool leavingTrails = (type != kEFX_Unknown6 && mPreviousFxType == kEFX_Unknown6) ||
                             (type != kEFX_Unknown7 && mPreviousFxType == kEFX_Unknown7);
  if (leavingDarkWorld || leavingTrails) {
    for (int i = 0; i < mGrids.size(); ++i) {
      mGrids[i].mParticleLifetimes = rstl::vector< float >();
    }
    if (leavingTrails) {
      for (int i = 0; i < mGrids.size(); ++i) {
        mGrids[i].mTrailFrames = rstl::vector< int >();
      }
    }
  }
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.GetVisibility().first) {
      continue;
    }
    rstl::vector< CVectorFixed8_8 >& particles = grid.mParticles;
    rstl::vector< float >& lifetimes = grid.mParticleLifetimes;
    rstl::vector< int >& trailFrames = grid.mTrailFrames;
    if (cellParticleCount > particles.size() ||
        ((trails || darkWorld) && cellParticleCount > lifetimes.size())) {
      if (cellParticleCount > particles.capacity() ||
          ((trails || darkWorld) && cellParticleCount > lifetimes.capacity())) {
        particles.reserve(maxParticleCount);
        if (darkWorld) {
          lifetimes.reserve(maxParticleCount);
        }
        if (trails) {
          lifetimes.reserve(maxParticleCount / 8);
          trailFrames.reserve(maxParticleCount / 8);
        }
      }
      const int remaining = cellParticleCount - particles.size();
      for (int j = 0; j < remaining; ++j) {
        // X is left uninitialized on the dark-world path in retail.
        float x;
        int z;
        if (darkWorld) {
          z = static_cast< int >(256.f *
                                 (invXf * CVector3f(0.f, 0.f, grid.GetVisibility().second)).GetZ());
        } else if (trails) {
          x = random.Range(0.f, static_cast< float >(grid.mExtent.GetX()) - 20.f);
          z = static_cast< int >(random.Range(20.f, 16363.f));
        } else {
          x = random.Range(0.f, static_cast< float >(grid.mExtent.GetX()));
          z = static_cast< int >(256.f * random.Range(0.f, 63.f));
        }
        const int y = random.Range(0.f, static_cast< float >(grid.mExtent.GetY()));
        particles.push_back_unsafe(CVectorFixed8_8(static_cast< int >(x), y, z));
        if (darkWorld) {
          lifetimes.push_back_unsafe(random.Float());
        } else if (trails) {
          lifetimes.push_back_unsafe(1.f);
          trailFrames.push_back_unsafe(g_TrailPeriod * random.Range(0, 100) * 8);
        }
        if (trails) {
          for (int point = 1; point < 8; ++point) {
            particles.push_back_unsafe(CVectorFixed8_8());
            ++j;
          }
        }
      }
    } else {
      particles.resize(cellParticleCount);
      if (darkWorld) {
        lifetimes.resize(cellParticleCount);
      } else if (trails) {
        lifetimes.resize(cellParticleCount / 8);
        trailFrames.resize(cellParticleCount / 8);
      }
    }
  }
  seed = random.GetSeed();
}

void CEnvFxManager::CalculateSnowForces(const CVectorFixed8_8& zVec,
                                        rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                                        EEnvFxType type, const CVector3f& inverseScale, float dt) {
  if (type != kEFX_DarkWorld && type != kEFX_Snow && type != kEFX_Unknown5) {
    return;
  }

  CVector3f accumulated = CVector3f::Zero();
  CVector3f previous = accumulated;
  CRandom16 random(99);
  const float (*forces)[2] = g_SnowForces;
  if (forces != nullptr) {
    float phase = 0.f;
    const float speed = type == kEFX_DarkWorld ? g_DarkWorldForceSpeed : 1.f;
    for (int i = 255; i >= 0; --i) {
      float forceX;
      float forceY;
      if (type == kEFX_DarkWorld) {
        // Retail samples these values before replacing them with the static forces below.
        const float randomX = random.Range(-1.f, 1.f);
        forceX = CMath::FastSinR(phase) + 0.2f * randomX;
        const float randomY = random.Range(-1.f, 1.f);
        forceY = CMath::FastCosR(phase) + 0.2f * randomY;
      }
      if (type == kEFX_Unknown5) {
        forceX = CMath::FastSinR(phase) + 0.2f * random.Range(-1.f, 1.f);
        forceY = CMath::FastCosR(phase) + 0.2f * random.Range(-1.f, 1.f);
      } else {
        forceX = forces[i][0];
        forceY = forces[i][1];
      }
      const float scaledDt = dt * speed;
      accumulated += CVector3f::ByElementMultiply(
          inverseScale, CVector3f(forceX * scaledDt, forceY * scaledDt, 0.f));
      snowForces.push_back(CVectorFixed8_8::FromCVector3f(accumulated - previous));
      previous = accumulated;
      phase += 0.024543693f;
    }
    snowForces[0] = CVectorFixed8_8::FromCVector3f(snowForces[0].ToCVector3f() - accumulated);
  }

  if (type == kEFX_DarkWorld) {
    const int forceIndex = static_cast< int >(
        CMath::FastFmod(g_DarkWorldImpulseRate * CGraphics::GetSecondsMod900(), 255.f));
    const CVectorFixed8_8 velocity = CVectorFixed8_8::FromCVector3f(
        CVector3f::ByElementMultiply(inverseScale, CVector3f(0.f, 0.f, g_DarkWorldRiseSpeed * dt)));
    const CVectorFixed8_8 delta =
        snowForces[forceIndex] * real_to_fixed8_8(g_DarkWorldImpulseScale) + velocity;
    for (int i = 0; i < snowForces.size(); ++i) {
      snowForces[i] = snowForces[i] + delta + zVec;
    }
    return;
  }
  const float zDeltaTime = type == kEFX_Unknown5 ? 0.f : dt;
  for (int i = 0; i < snowForces.size(); ++i) {
    const CVector3f delta =
        CVector3f::ByElementMultiply(inverseScale, zDeltaTime * mSnowZDeltas[i & 15]);
    const CVectorFixed8_8 fixedDelta = CVectorFixed8_8::FromCVector3f(delta);
    snowForces[i] = snowForces[i] + fixedDelta + zVec;
  }
}

void CEnvFxManager::UpdateBlockedGrids(CStateManager& mgr, EEnvFxType type,
                                       const CTransform4f& camXf, const CTransform4f& xf,
                                       const CTransform4f& invXf) {
  const CPlayer* player = mgr.GetPlayer(0);
  const CVector3f playerPos = player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed
                                  ? camXf.GetTranslation()
                                  : player->GetBallPosition();
  const CVector3f& localPos = CVector3f(invXf * playerPos);
  const CVector2i localPlayerPos(real_to_fixed8_8(localPos.GetX()),
                                 real_to_fixed8_8(localPos.GetY()));
  mLastBlockedGridIdx = -1;
  mEnableSplash = false;

  rstl::reserved_vector< TUniqueId, 1024 > blockList;
  bool blockListBuilt = false;
  int blockedGrids = 0;
  for (int i = 0; i < mGrids.size(); ++i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (blockedGrids < 8 && grid.IsDirty()) {
      if (type == kEFX_UnderwaterFlake || type == kEFX_Unknown5) {
        grid.SetVisibility(rstl::pair< bool, float >(true, -skMaximumBlockingHeight));
      } else {
        const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Solid, kMT_Trigger),
            CMaterialList(kMT_ProjectilePassthrough, kMT_SeeThrough));
        const CVector2i& gridPos = CVector2i(grid.GetStart() + grid.GetSize() * 0);
        const CVector3f localGrid(fixed8_8_to_real(gridPos.GetX()),
                                  fixed8_8_to_real(gridPos.GetY()), 0.f);
        const CVector3f start = xf * localGrid + 500.f * CVector3f::Up();
        const CVector3f down = CVector3f::Down();
        CRayCastResult best = CRayCastResult::MakeInvalid();
        const bool trails = type >= kEFX_Unknown6 && type <= kEFX_Unknown7;
        if (trails) {
          bool visible = CGameCollision::RayStaticLineOfSightTest(mgr, start, down, 1000.f, filter);
          if (visible) {
            if (!blockListBuilt) {
              BuildBlockObjectList(blockList, mgr);
              blockListBuilt = true;
            }
            for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = blockList.begin();
                 it != blockList.end(); ++it) {
              const CScriptTrigger* trigger =
                  TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(*it));
              if (trigger == nullptr) {
                continue;
              }
              const rstl::optional_object< CAABox > bounds = trigger->GetTouchBounds();
              if (!bounds) {
                continue;
              }
              {
                const CCollidableAABox box(*bounds, CMaterialList(kMT_Trigger));
                const CRayCastResult hit =
                    box.CastRay(start, down, 1000.f, filter, CTransform4f::Identity());
                if (hit.IsValid()) {
                  visible = false;
                  break;
                }
              }
            }
          }
          grid.SetVisibility(rstl::pair< bool, float >(visible, visible ? -10000.f : 0.f));
        } else {
          best = CGameCollision::RayStaticIntersection(mgr, start, down, 1000.f, filter);
          const CMaterialFilter& floorOrTrigger =
              CMaterialFilter::MakeInclude(CMaterialList(kMT_Trigger, kMT_Floor));
          if (!floorOrTrigger.Passes(best.GetMaterial())) {
            best = CRayCastResult(CRayCastResult::kI_Invalid);
          }
          if (best.IsValid()) {
            if (!blockListBuilt) {
              BuildBlockObjectList(blockList, mgr);
              blockListBuilt = true;
            }
            for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = blockList.begin();
                 it != blockList.end(); ++it) {
              const CScriptTrigger* trigger =
                  TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(*it));
              if (trigger == nullptr) {
                continue;
              }
              const rstl::optional_object< CAABox > bounds = trigger->GetTouchBounds();
              if (!bounds) {
                continue;
              }
              {
                const CCollidableAABox box(*bounds, CMaterialList(kMT_Trigger));
                const CRayCastResult hit =
                    box.CastRay(start, down, 1000.f, filter, CTransform4f::Identity());
                if (hit.IsValid() && hit.GetTime() < best.GetTime()) {
                  best = hit;
                }
              }
            }
          }
        }
        if (!trails) {
          grid.SetVisibility(rstl::pair< bool, float >(best.IsValid(), best.GetPoint().GetZ()));
        }
        ++blockedGrids;
      }
      grid.SetDirty(false);
    }

    const CVector2i end = grid.GetStart() + grid.GetSize();
    if (localPlayerPos.GetX() >= grid.GetStart().GetX() &&
        localPlayerPos.GetY() >= grid.GetStart().GetY() && localPlayerPos.GetX() < end.GetX() &&
        localPlayerPos.GetY() < end.GetY() && grid.GetVisibility().first &&
        grid.GetVisibility().second <= playerPos.GetZ()) {
      mEnableSplash = true;
      mLastBlockedGridIdx = i;
    }
  }
}

void CEnvFxManager::UpdateSnowParticles(rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces) {
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    int force = static_cast< int >(mFirstSnowForce);
    if (!grid.GetVisibility().first) {
      continue;
    }

    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      CVectorFixed8_8& particle = grid.mParticles[j];
      CVectorFixed8_8 next = particle + snowForces[force];
      next.mZ &= 0x3fff;
      particle = next;
      force = (force + 1) & 0xff;
    }
  }
}

void CEnvFxManager::UpdateDriftingParticles(
    float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
    const CTransform4f& invXf) {
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    int force = static_cast< int >(mFirstSnowForce);
    if (!grid.GetVisibility().first) {
      continue;
    }
    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      CVectorFixed8_8& particle = grid.mParticles[j];
      CVectorFixed8_8 next = particle + snowForces[force];
      next.mZ &= 0x3fff;
      particle = next;
      force = (force + 1) & 0xff;
    }
  }
}

void CEnvFxManager::UpdateParticleTrails(float dt, const CVectorFixed8_8& zVec) {
  static const float kSecondaryOffsets[8] = {-0.6f, 1.2f, -0.9f, 0.3f, -0.4f, 0.9f, -2.f, 0.4f};
  static const float kPrimaryOffsets[7] = {0.3f, 0.9f, 0.5f, 0.8f, 0.4f, 0.7f, 0.2f};
  static uint seed = 0;
  CRandom16 random(seed);
  const short zDelta = zVec.mZ;

  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.GetVisibility().first) {
      continue;
    }
    const int count = grid.mParticles.size() / 8;
    for (int trail = 0; trail < count; ++trail) {
      grid.mParticleLifetimes[trail] -= dt * g_TrailDecayRate;
      ++grid.mTrailFrames[trail];
      const int base = trail * 8;
      if (grid.mParticleLifetimes[trail] <= 0.f) {
        grid.mParticleLifetimes[trail] = 1.f;
        grid.mTrailFrames[trail] = g_TrailPeriod * random.Range(0, 100);
        const int current = (grid.mTrailFrames[trail] / g_TrailPeriod) % 8;
        const short z = static_cast< short >(random.Range(20.f, 16363.f));
        const short y =
            static_cast< short >(random.Range(0.f, static_cast< float >(grid.mExtent.GetY())));
        const short x = static_cast< short >(random.Range(0.f, grid.mExtent.GetX() - 20.f));
        grid.mParticles[base + current] = CVectorFixed8_8(x, y, z);
        for (int point = 1; point < 8; ++point) {
          grid.mParticles[base + ((current + point) % 8)] = CVectorFixed8_8();
        }
      } else if (grid.mTrailFrames[trail] % g_TrailPeriod == 0) {
        const int current = ((grid.mTrailFrames[trail] - 1) / g_TrailPeriod) % 8;
        const int next = (current + 1) % 8;
        grid.mParticles[base + next] = grid.mParticles[base + current];
        if (grid.mTrailFrames[trail] % (g_TrailPeriod * 2) == 0) {
          const short delta = real_to_fixed8_8(g_TrailPrimaryScale * kPrimaryOffsets[trail % 7]);
          grid.mParticles[base + next][g_TrailPrimaryAxis] += delta;
        } else {
          const short delta = real_to_fixed8_8(
              kSecondaryOffsets[(grid.mTrailFrames[trail] / (g_TrailPeriod * 2)) % 8]);
          grid.mParticles[base + next][g_TrailSecondaryAxis] += delta;
        }
        grid.mParticles[base + current] =
            grid.mParticles[base + current] - grid.mParticles[base + next];
      }
      CVectorFixed8_8& point =
          grid.mParticles[base + ((grid.mTrailFrames[trail] / g_TrailPeriod) % 8)];
      point.mZ = (zDelta + point.mZ) & 0x3fff;
    }
  }
  seed = random.GetSeed();
}

void CEnvFxManager::UpdateDarkWorldParticles(
    float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
    const CTransform4f& invXf) {
  const int firstForce = static_cast< int >(mFirstSnowForce);
  const float lifetimeDelta = (1.f / 3.f) * dt;
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.GetVisibility().first) {
      continue;
    }
    CRandom16 random(99);
    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      grid.mParticleLifetimes[j] -= lifetimeDelta;
      if (grid.mParticleLifetimes[j] <= 0.f) {
        grid.mParticleLifetimes[j] = 1.f;
        const short z =
            real_to_fixed8_8((invXf * CVector3f(0.f, 0.f, grid.GetVisibility().second)).GetZ());
        const short y =
            static_cast< short >(random.Range(0.f, static_cast< float >(grid.mExtent.GetY())));
        const short x =
            static_cast< short >(random.Range(0.f, static_cast< float >(grid.mExtent.GetX())));
        grid.mParticles[j] = CVectorFixed8_8(x, y, z);
      } else {
        const CVectorFixed8_8& force = snowForces[(firstForce + random.Next()) & 0xff];
        const float elapsed = 1.f - grid.mParticleLifetimes[j];
        const float growth = 2.f * (elapsed * (elapsed * (elapsed * elapsed)));
        const short forceScale = real_to_fixed8_8(1.f + growth);
        CVectorFixed8_8 particle = grid.mParticles[j] + force * forceScale;
        particle.mZ += real_to_fixed8_8(dt * growth);
        grid.mParticles[j] = particle;
      }
    }
  }
}

void CEnvFxManager::UpdateRainParticles(const CVectorFixed8_8& zVec, const CVector3f& inverseScale,
                                        float dt) {
  const short deltaZ = zVec.GetZ() + real_to_fixed8_8(-40.f * dt * inverseScale.GetZ());
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.GetVisibility().first) {
      continue;
    }
    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      CVectorFixed8_8& particle = grid.mParticles[j];
      particle.mZ = (deltaZ + particle.mZ) & 0x3fff;
    }
  }
}

void CEnvFxManager::UpdateUnderwaterParticles(const CVectorFixed8_8& zVec) {
  const short zDelta = zVec.GetZ();
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    for (int j = mGrids[i].mParticles.size() - 1; j >= 0; --j) {
      mGrids[i].mParticles[j].mZ = (zDelta + mGrids[i].mParticles[j].mZ) & 0x3fff;
    }
  }
}

void CEnvFxManager::UpdateVisorSplash(CStateManager& mgr, float dt, const CTransform4f& camXf) {
  const EEnvFxType fxType = static_cast< EEnvFxType >(mgr.GetWorld()->GetNeededEnvFx());
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    if (mEnvRainSplashIds[i] == kInvalidUniqueId) {
      continue;
    }
    CHUDBillboardEffect* effect =
        TCastToPtr< CHUDBillboardEffect >(mgr.ObjectById(mEnvRainSplashIds[i]));
    if (effect != nullptr) {
      mgr.SetActorAreaId(*effect, mgr.GetNextAreaId());
    }
  }

  const float upness = CVector3f::Dot(camXf.GetForward(), CVector3f::Up());
  const float splashRate = mEnableSplash ? mFxDensity * rstl::max_val(0.f, upness) : 0.f;
  float forwardRate = 0.f;
  if (mEnableSplash && upness >= -0.1f) {
    const CPlayer* player = mgr.GetPlayer(0);
    const CVector3f localVelocity = player->GetTransform().TransposeRotate(player->GetVelocityWR());
    if (localVelocity.CanBeNormalized()) {
      const float speed = localVelocity.Magnitude();
      forwardRate = rstl::min_val(1.f, speed / 60.f) *
                    CVector3f::Dot(localVelocity / speed, CVector3f::Forward());
    }
  }

  const float additionalRate = fxType == kEFX_Rain ? splashRate + forwardRate : 0.f;
  SetSplashEffectRate(mBaseSplashRate + additionalRate, mgr);
  mBaseSplashRate = 0.f;
}

void CEnvFxManager::SetSplashEffectRate(float rate, CStateManager& mgr) {
  for (int i = 0; i < mEnvRainSplashIds.size(); ++i) {
    CHUDBillboardEffect* effect =
        TCastToPtr< CHUDBillboardEffect >(mgr.ObjectById(mEnvRainSplashIds[i]));
    if (effect != nullptr && effect->IsElementGen()) {
      effect->GetParticleGen()->SetGeneratorRate(rate);
    }
  }
}

CTransform4f CEnvFxManager::GetParticleBoundsToWorldTransform() const {
  return CTransform4f::Translate(mFocusCellPosition) *
         CTransform4f::Translate(CVector3f(-31.75f, -31.75f, -31.75f)) *
         CTransform4f::Scale(GetParticleBoundsToWorldScale());
}

void CEnvFxManager::BlankFirstSnowflakeMip(CTexture& tex) {
  if (!mSnowflakeTextureMipBlanked) {
    void* data = tex.Lock();
    int size = tex.GetWidth() * tex.GetHeight() * tex.GetBitsPerPixel() / 8;
    uchar* ptr = static_cast< uchar* >(data);
    for (int i = 0; i < size; ++i) {
      ptr[i] = 0;
    }
    tex.UnLock();
    mSnowflakeTextureMipBlanked = true;
  }
}

void CEnvFxManager::SetupSnowTevs(CStateManager& mgr) {
  const CCameraManager* cameraManager = mgr.GetCurrentRenderCameraManager();
  const CGameCamera* camera = cameraManager->GetCurrentCamera(mgr, true);
  CColor color = CColor::White();
  if (camera->GetFluidCount() != 0) {
    gpRender->SetWorldFog(kRFM_PerspExp, 0.f, 35.f, CColor::Black());
    color = CColor(1.f, 1.f, 1.f, 0.5f);
  } else {
    gpRender->SetWorldFog(kRFM_PerspLin, 52.f, 57.f, CColor::Black());
  }

  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(2);
  CGX::SetNumTevStages(2);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);

  BlankFirstSnowflakeMip(***mTxtrSnowFlake);
  (*mTxtrSnowFlake)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  (*mTxtrEnvGradient)->Load(GX_TEXMAP1, CTexture::kCM_Clamp);
}

void CEnvFxManager::SetupDriftingParticleTevs(CStateManager& mgr) {
  mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true);
  const CColor color = CColor::Blue();
  gpRender->SetWorldFog(kRFM_PerspLin, 52.f, 57.f, CColor::Black());
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(2);
  CGX::SetNumTevStages(2);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  BlankFirstSnowflakeMip(***mTxtrSnowFlake);
  (*mTxtrSnowFlake)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  (*mTxtrEnvGradient)->Load(GX_TEXMAP1, CTexture::kCM_Clamp);
}

void CEnvFxManager::SetupDarkWorldTevs() {
  gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  const CColor& white = CColor::White();
  CGX::SetTevKColor(GX_KCOLOR0, white.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  mDarkWorldParticleTexture->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  CGX::SetNumTevStages(1);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
}

void CEnvFxManager::SetupUnderwaterTevs(const CTransform4f& invXf, CStateManager& mgr) {
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(2);
  CGX::SetNumTevStages(2);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  BlankFirstSnowflakeMip(***mUnderwaterFlake);
  (*mUnderwaterFlake)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);

  float waterTop = skMaximumBlockingHeight;
  const CObjectList& objects = mgr.GetObjectListById(kOL_All);
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    const CEntity* entity = objects[i];
    const CScriptWater* water = TCastToConstPtr< CScriptWater >(entity);
    if (water != nullptr) {
      const rstl::optional_object< CAABox > bounds = water->GetTouchBounds();
      if (bounds) {
        waterTop = rstl::min_val(waterTop, bounds->GetMaxPoint().GetZ());
      }
    }
  }
  const CVector3f localWaterTop = invXf * (waterTop * CVector3f::Up());
  float texMtx[2][4] = {{0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, -10.f, 0.f}};
  texMtx[1][3] = -(-10.f * localWaterTop.GetZ() + 0.5f);
  GXLoadTexMtxImm(texMtx, GX_TEXMTX5, GX_MTX2x4);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ONE, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP1, GX_TEV_SWAP1);
  (*mTxtrEnvGradient)->Load(GX_TEXMAP1, CTexture::kCM_Clamp);
}

void CEnvFxManager::SetupDefaultTevSwapMode() {
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
}

void CEnvFxManager::SetupRainTevs() {
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_CLR0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
  CGX::SetLineWidth(6, GX_MAX_TEXOFFSET);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetNumChans(1);
  CGX::SetChanCtrl(CGX::Channel0, GX_TRUE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  const CColor color = CColor(1.f, 1.f, 1.f, 0.15f);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  (*mTxtrEnvGradient)->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
}

void CEnvFxManager::SetupParticleTrailTevs(CStateManager& mgr) {
  const CGameCamera* camera = mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true);
  CColor color = CColor::White();
  if (camera->GetFluidCount() != 0) {
    gpRender->SetWorldFog(kRFM_PerspExp, 0.f, 35.f, CColor::Black());
    color = CColor(1.f, 1.f, 1.f, 0.5f);
  } else {
    gpRender->SetWorldFog(kRFM_PerspLin, 52.f, 57.f, CColor::Black());
  }
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(desc);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 8);
  CGX::SetLineWidth(12, GX_MAX_TEXOFFSET);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
  (*mTxtrEnvGradient)->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
}

void CEnvFxManager::Render(const CStateManager& mgr) {
  const EEnvFxType type = static_cast< EEnvFxType >(mgr.GetWorld()->GetNeededEnvFx());
  if (type == kEFX_None ||
      (mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
       mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo)) {
    return;
  }

  CGraphics::SetCullMode(kCM_None);
  gpRender->SetDepthReadWrite(true, false);
  const CTransform4f xf = GetParticleBoundsToWorldTransform();
  const CTransform4f invXf = xf.GetInverse();
  const CTransform4f camXf = mgr.GetCameraManager(0)->GetCurrentCameraTransform(mgr, true);

  switch (type) {
  case kEFX_Snow:
    SetupSnowTevs(const_cast< CStateManager& >(mgr));
    break;
  case kEFX_Rain:
    SetupRainTevs();
    break;
  case kEFX_UnderwaterFlake:
    SetupUnderwaterTevs(invXf, const_cast< CStateManager& >(mgr));
    break;
  case kEFX_DarkWorld:
    SetupDarkWorldTevs();
    break;
  case kEFX_Unknown5:
    SetupDriftingParticleTevs(const_cast< CStateManager& >(mgr));
    break;
  case kEFX_Unknown6:
  case kEFX_Unknown7:
    SetupParticleTrailTevs(const_cast< CStateManager& >(mgr));
    break;
  default:
    break;
  }

  if (type == kEFX_DarkWorld) {
    rstl::reserved_vector< CVectorFixed8_8, 16 > offsets;
    rstl::reserved_vector< CVectorFixed8_8, 16 > upDeltas;
    rstl::reserved_vector< CVectorFixed8_8, 16 > rightDeltas;
    CRandom16 random(99);
    const CTransform4f cameraRotation = camXf.GetRotation();
    for (int i = 0; i < 16; ++i) {
      random.Next();
      const float size = random.Range(0.05f, 0.7f);
      const CVector3f up = cameraRotation * CVector3f(0.f, 0.f, size);
      const CVector3f right = cameraRotation * CVector3f(size, 0.f, 0.f);
      const CVector3f offset = 0.5f * -up - 0.5f * right;
      upDeltas.push_back(CVectorFixed8_8(real_to_fixed8_8(up.GetX()), real_to_fixed8_8(up.GetY()),
                                         real_to_fixed8_8(up.GetZ())));
      rightDeltas.push_back(CVectorFixed8_8(real_to_fixed8_8(right.GetX()),
                                            real_to_fixed8_8(right.GetY()),
                                            real_to_fixed8_8(right.GetZ())));
      offsets.push_back(CVectorFixed8_8(real_to_fixed8_8(offset.GetX()),
                                        real_to_fixed8_8(offset.GetY()),
                                        real_to_fixed8_8(offset.GetZ())));
    }
    for (int i = 0; i < mGrids.size(); ++i) {
      mGrids[i].RenderDarkWorldParticles(xf, invXf, camXf, mFxDensity, offsets.data(),
                                         upDeltas.data(), rightDeltas.data());
    }
  } else {
    for (int i = 0; i < mGrids.size(); ++i) {
      mGrids[i].Render(xf, invXf, camXf, mFxDensity, type);
    }
  }
  CGraphics::SetCullMode(kCM_Front);
  if (type == kEFX_UnderwaterFlake) {
    SetupDefaultTevSwapMode();
  }
}

static int CalcRainVolume(float density) {
  float volume;
  if (density < 0.1f) {
    volume = 74.f * (density / 0.1f);
  } else {
    volume = 21.f * (density / 0.9f) + 74.f;
  }
  return static_cast< int >(volume);
}

static short CalcRainPitch(float density) { return static_cast< short >(8192.f * density); }

void CEnvFxManager::UpdateRainSounds(float dt, CStateManager& mgr) {
  if (mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain) {
    if (mRainSoundsStopped) {
      mRainSoundFade = rstl::max_val(0.f, mRainSoundFade - dt);
    } else {
      mRainSoundFade = rstl::min_val(1.f, mRainSoundFade + dt);
    }

    if (mgr.GetGameState() == CStateManager::kGS_SoftPaused) {
      return;
    }
    const CTransform4f camXf = mgr.GetCameraManager(0)->GetCurrentCameraTransform(mgr, true);
    const uchar volume = static_cast< uchar >(mRainSoundFade * CalcRainVolume(mFxDensity));
    if (!mRainSoundActive) {
      mLeftRainSound = CSfxManager::AddEmitter(0x2841, CVector3f::Zero(), CSfxManager::kAllAreas,
                                               true, true, CSfxManager::kMaxPriority);
      mRightRainSound = CSfxManager::AddEmitter(0x2842, CVector3f::Zero(), CSfxManager::kAllAreas,
                                                true, true, CSfxManager::kMaxPriority);
      mRainSoundActive = true;
    }
    CSfxManager::UpdateEmitter(mLeftRainSound, camXf.GetTranslation() - camXf.GetRight(),
                               camXf.GetRight(), volume);
    CSfxManager::UpdateEmitter(mRightRainSound, camXf.GetTranslation() + camXf.GetRight(),
                               -camXf.GetRight(), volume);
    const short pitch = CalcRainPitch(mFxDensity);
    CSfxManager::PitchBend(mLeftRainSound, pitch);
    CSfxManager::PitchBend(mRightRainSound, pitch);
  } else if (mRainSoundActive) {
    CSfxManager::RemoveEmitter(mLeftRainSound);
    CSfxManager::RemoveEmitter(mRightRainSound);
    mRainSoundActive = false;
  }
}

void CEnvFxManager::FadeDensity(float density, int speed) {
  mTargetFxDensity = density;
  mMaxDensityDeltaSpeed = speed;
}

void CEnvFxManager::StopRainSounds() { mRainSoundsStopped = true; }

void CEnvFxManager::PlayRainSounds() { mRainSoundsStopped = false; }

void CEnvFxManager::BuildBlockObjectList(rstl::reserved_vector< TUniqueId, 1024 >& list,
                                         CStateManager& mgr) {
  const CObjectList& objects = mgr.GetObjectListById(kOL_All);
  for (int index = objects.GetFirstObjectIndex(); index != -1;
       index = objects.GetNextObjectIndex(index)) {
    const CEntity* entity = objects[index];
    const CScriptTrigger* trigger = TCastToConstPtr< CScriptTrigger >(entity);
    if (trigger != nullptr && (trigger->GetTriggerFlags() & kTFL_BlockEnvironmentalEffects) != 0) {
      list.push_back(entity->GetUniqueId());
    }
  }
}

void CEnvFxManager::AreaLoaded() {
  for (int i = 0; i < mGrids.size(); ++i) {
    mGrids[i].SetDirty(true);
  }
}
