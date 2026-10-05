#include "MetroidPrime/CEnvFxManager.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "rstl/auto_ptr.hpp"
#include "Collision/CCollidableAABox.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CRayCastResult.hpp"
#include "MetroidPrime/CGameCollision.hpp"

// The target stores the largest finite single-precision value directly.
static const float skMaximumBlockingHeight = 3.402823466e+38F;
static float g_SnowForces[256][2];

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
  int count = mParticles.size();
  short zx = real_to_fixed8_8(0.2f * camXf.Get02());
  short zy = real_to_fixed8_8(0.2f * camXf.Get12());
  short zz = real_to_fixed8_8(0.2f * camXf.Get22());
  short xx = real_to_fixed8_8(0.2f * camXf.Get00());
  short xy = real_to_fixed8_8(0.2f * camXf.Get10());
  short xz = real_to_fixed8_8(0.2f * camXf.Get20());
  CGX::Begin(GX_QUADS, GX_VTXFMT6, count * 4);
  for (int i = count - 1; i >= 0; --i) {
    CVectorFixed8_8 particle = mParticles[i];
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
  const int count = mParticles.size();
  const short zx = real_to_fixed8_8(0.2f * camXf.Get02());
  const short zy = real_to_fixed8_8(0.2f * camXf.Get12());
  const short zz = real_to_fixed8_8(0.2f * camXf.Get22());
  const short xx = real_to_fixed8_8(0.2f * camXf.Get00());
  const short xy = real_to_fixed8_8(0.2f * camXf.Get10());
  const short xz = real_to_fixed8_8(0.2f * camXf.Get20());
  CGX::Begin(GX_QUADS, GX_VTXFMT6, count * 4);
  for (int i = count - 1; i >= 0; --i) {
    CVectorFixed8_8 particle = mParticles[i];
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
  const CColor baseColor = type == kEFX_Unknown6 ? CColor(0.6f, 0.71f, 0.48f, 0.175f)
                                                    : CColor(0.f, 0.f, 1.f, 0.175f);
  const int period = type == kEFX_Unknown6 ? 2 : 8;
  for (int trail = 0; trail < mParticles.size() / 8; ++trail) {
    const int frame = mTrailFrames[trail];
    const int base = trail * 8;
    int segment = (frame / period) & 7;
    const short fraction = static_cast< short >(
        256.f * static_cast< float >(frame % period) / static_cast< float >(period));
    const float lifetime = mParticleLifetimes[trail];
    float fade = 1.f;
    if (lifetime < 0.2f) {
      fade = lifetime / 0.2f;
    } else if (lifetime > 0.8f) {
      fade = (1.f - lifetime) / 0.2f;
    }
    const CColor color = baseColor.WithAlphaModulatedBy(fade);
    GXSetTevColor(GX_TEVREG1, color.GetGXColor());

    CVectorFixed8_8 position = mParticles[base + segment];
    uchar alpha = 0x7f;
    CGX::Begin(GX_LINESTRIP, GX_VTXFMT6, 8);
    for (int point = 0; point < 8; ++point) {
      segment = (segment + 7) & 7;
      const CVectorFixed8_8& delta = mParticles[base + segment];
      const CVectorFixed8_8 next(position.mX + delta.mX, position.mY + delta.mY,
                                 position.mZ + delta.mZ);
      if (point == 0 || point == 6) {
        if (point == 6) {
          GXPosition3s16(position.mX, position.mY, position.mZ);
          GXTexCoord2u8(0, alpha);
          alpha -= 15;
        }
        GXPosition3s16(next.mX - ((delta.mX * fraction) >> 8),
                       next.mY - ((delta.mY * fraction) >> 8),
                       next.mZ - ((delta.mZ * fraction) >> 8));
      } else {
        GXPosition3s16(position.mX, position.mY, position.mZ);
      }
      GXTexCoord2u8(0, alpha);
      alpha -= 15;
      position = next;
      if (point == 6) {
        break;
      }
    }
    CGX::End();
  }
}

void CEnvFxManagerGrid::RenderUnderwaterParticles(const CTransform4f& camXf) {
  int count = mParticles.size();
  short zx = real_to_fixed8_8(0.5f * camXf.Get02());
  short zy = real_to_fixed8_8(0.5f * camXf.Get12());
  short zz = real_to_fixed8_8(0.5f * camXf.Get22());
  short xx = real_to_fixed8_8(0.5f * camXf.Get00());
  short xy = real_to_fixed8_8(0.5f * camXf.Get10());
  short xz = real_to_fixed8_8(0.5f * camXf.Get20());
  CGX::Begin(GX_QUADS, GX_VTXFMT6, count * 4);
  for (int i = count - 1; i >= 0; --i) {
    CVectorFixed8_8 particle = mParticles[i];
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
  if (mParticles.empty() || !mBlock.first) {
    return false;
  }
  const float gridX = fixed8_8_to_real(mPosition.GetX());
  const float gridY = fixed8_8_to_real(mPosition.GetY());
  const CTransform4f gridXf = xf * CTransform4f::Translate(gridX, gridY, 0.f);
  gpRender->SetModelMatrix(gridXf);

  if (type == kEFX_Snow || type == kEFX_Rain || type == kEFX_DarkWorld ||
      type == kEFX_Unknown5 || type == kEFX_Unknown6 || type == kEFX_Unknown7) {
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
    const float brightness = rstl::max_val(0.f, 1.5f * lifetime - 0.5f);
    const float remaining = 1.f - lifetime;
    const float remainingSquared = remaining * remaining;
    const uchar red = static_cast< uchar >(255.f * brightness);
    const uchar green = static_cast< uchar >(red * lifetime);
    const uchar alpha = static_cast< uchar >(255.f * (1.f - remainingSquared * remainingSquared));
    CGX::SetTevKColor(GX_KCOLOR0, CColor(red, green, red, alpha).GetGXColor());

    const CVectorFixed8_8& particle = mParticles[i];
    const CVectorFixed8_8& offset = offsets[i & 15];
    const CVectorFixed8_8& up = upDeltas[i & 15];
    const CVectorFixed8_8& right = rightDeltas[i & 15];
    const short x = particle.mX + offset.mX;
    const short y = particle.mY + offset.mY;
    const short z = particle.mZ + offset.mZ;
    CGX::Begin(GX_QUADS, GX_VTXFMT6, 4);
    GXPosition3s16(x, y, z);
    GXTexCoord2u8(0, 0);
    GXPosition3s16(x + up.mX, y + up.mY, z + up.mZ);
    GXTexCoord2u8(0, 2);
    GXPosition3s16(x + up.mX + right.mX, y + up.mY + right.mY, z + up.mZ + right.mZ);
    GXTexCoord2u8(2, 2);
    GXPosition3s16(x + right.mX, y + right.mY, z + right.mZ);
    GXTexCoord2u8(2, 0);
    CGX::End();
  }
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Black().GetGXColor());
}

CVector3f CEnvFxManager::GetParticleBoundsToWorldScale() const {
  return (mParticleBounds.GetMaxPoint() - mParticleBounds.GetMinPoint()) / 127.f;
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
    CHUDBillboardEffect* effect = new CHUDBillboardEffect(
        rstl::optional_object< TToken< CGenDescription > >(*mEnvRainSplash),
        rstl::optional_object< TToken< CElectricDescription > >(), id, true,
        rstl::string_l("VisorRainSplashes"),
        CHUDBillboardEffect::GetNearClipDistance(mgr, playerIndex),
        CHUDBillboardEffect::GetScaleForPOV(mgr), playerIndex, CColor::White(),
        CVector3f::One(), CVector3f::Zero(), false);
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
  const CCameraManager* cameraManager = mgr.GetCameraManager(0);
  const CTransform4f camXf = cameraManager->GetCurrentCameraTransform(mgr, true);
  const EEnvFxType type = static_cast< EEnvFxType >(mgr.GetWorld()->GetNeededEnvFx());

  if (cameraManager->GetCurrentCamera(mgr, true)->GetFluidCount() != 0) {
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
                CMath::Limit(densityDelta, dt * mMaxDensityDeltaSpeed / 11000.f);

  const CVector3f scale = GetParticleBoundsToWorldScale();
  const CVector3f inverseScale(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
  const CVector3f forwardPoint = camXf.GetTranslation() + 23.8125f * camXf.GetForward();
  const CVector3f cellBase(forwardPoint.GetX() - CMath::ModF(forwardPoint.GetX(), 7.9375f),
                           forwardPoint.GetY() - CMath::ModF(forwardPoint.GetY(), 7.9375f),
                           forwardPoint.GetZ());
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
  int maxParticleCount = 0;
  switch (type) {
  case kEFX_Snow:
  case kEFX_Unknown5:
    maxParticleCount = 0x1c98;
    break;
  case kEFX_Rain:
    maxParticleCount = 11000;
    break;
  case kEFX_UnderwaterFlake:
    maxParticleCount = 0xfeb;
    break;
  case kEFX_DarkWorld:
    maxParticleCount = 0x2ee;
    break;
  case kEFX_Unknown6:
  case kEFX_Unknown7:
    maxParticleCount = 0x1c90;
    break;
  default:
    break;
  }
  maxParticleCount /= 64;
  int cellParticleCount = static_cast< int >(mFxDensity * maxParticleCount);
  const bool trails = type == kEFX_Unknown6 || type == kEFX_Unknown7;
  if (trails) {
    maxParticleCount &= ~7;
    cellParticleCount &= ~7;
  }

  static uint seed = 0;
  CRandom16 random(seed);
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.mBlock.first) {
      continue;
    }
    rstl::vector< CVectorFixed8_8 >& particles = grid.mParticles;
    if (particles.size() < cellParticleCount) {
      particles.reserve(maxParticleCount);
      if (type == kEFX_DarkWorld) {
        grid.mParticleLifetimes.reserve(maxParticleCount);
      } else if (trails) {
        grid.mParticleLifetimes.reserve(maxParticleCount / 8);
        grid.mTrailFrames.reserve(maxParticleCount / 8);
      }
      while (particles.size() < cellParticleCount) {
        const short x = static_cast< short >(random.Range(
            0.f, static_cast< float >(grid.mExtent.GetX()) - (trails ? 20.f : 0.f)));
        const short y =
            static_cast< short >(random.Range(0.f, static_cast< float >(grid.mExtent.GetY())));
        short z;
        if (type == kEFX_DarkWorld) {
          z = real_to_fixed8_8((invXf * CVector3f(0.f, 0.f, grid.mBlock.second)).GetZ());
        } else if (trails) {
          z = static_cast< short >(random.Range(20.f, 16363.f));
        } else {
          z = real_to_fixed8_8(random.Range(0.f, 63.f));
        }
        particles.push_back(CVectorFixed8_8(x, y, z));
        if (type == kEFX_DarkWorld) {
          grid.mParticleLifetimes.push_back(random.Float());
        } else if (trails) {
          grid.mParticleLifetimes.push_back(1.f);
          grid.mTrailFrames.push_back(8 * (type == kEFX_Unknown7 ? 8 : 2) * random.Range(0, 100));
          for (int point = 1; point < 8; ++point) {
            particles.push_back(CVectorFixed8_8());
          }
        }
      }
    } else {
      particles.resize(cellParticleCount);
      if (type == kEFX_DarkWorld) {
        grid.mParticleLifetimes.resize(cellParticleCount);
      } else if (trails) {
        grid.mParticleLifetimes.resize(cellParticleCount / 8);
        grid.mTrailFrames.resize(cellParticleCount / 8);
      }
    }
  }
  seed = random.GetSeed();
}

void CEnvFxManager::CalculateSnowForces(const CVectorFixed8_8& zVec,
                                        rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                                        EEnvFxType type, const CVector3f& inverseScale, float dt) {
  if (type != kEFX_Snow && type != kEFX_DarkWorld && type != kEFX_Unknown5) {
    return;
  }

  CRandom16 random(99);
  CVector3f accumulated = CVector3f::Zero();
  CVectorFixed8_8 previous;
  float phase = 0.f;
  const float speed = type == kEFX_DarkWorld ? 5.f : 1.f;
  for (int i = 255; i >= 0; --i) {
    float forceX = g_SnowForces[i][0];
    float forceY = g_SnowForces[i][1];
    if (type == kEFX_Unknown5) {
      forceX = CMath::FastSinR(phase) + 0.2f * random.Range(-1.f, 1.f);
      forceY = CMath::FastCosR(phase) + 0.2f * random.Range(-1.f, 1.f);
    }
    accumulated += CVector3f(inverseScale.GetX() * dt * speed * forceX,
                              inverseScale.GetY() * dt * speed * forceY, 0.f);
    const CVectorFixed8_8 current(real_to_fixed8_8(accumulated.GetX()),
                                   real_to_fixed8_8(accumulated.GetY()),
                                   real_to_fixed8_8(accumulated.GetZ()));
    snowForces.push_back(CVectorFixed8_8(current.mX - previous.mX, current.mY - previous.mY,
                                         current.mZ - previous.mZ));
    previous = current;
    phase += 0.024543693f;
  }
  snowForces[0].mX -= real_to_fixed8_8(accumulated.GetX());
  snowForces[0].mY -= real_to_fixed8_8(accumulated.GetY());
  snowForces[0].mZ -= real_to_fixed8_8(accumulated.GetZ());

  if (type == kEFX_DarkWorld) {
    const int forceIndex = static_cast< int >(20.f * CGraphics::GetSecondsMod900()) % 255;
    const CVectorFixed8_8 impulse = snowForces[forceIndex];
    const int impulseScale = static_cast< int >(256.f * 2.5f);
    const short zVelocity = real_to_fixed8_8(4.f * dt * inverseScale.GetZ());
    for (int i = 0; i < snowForces.size(); ++i) {
      snowForces[i].mX += static_cast< short >((impulse.mX * impulseScale) >> 8) + zVec.mX;
      snowForces[i].mY += static_cast< short >((impulse.mY * impulseScale) >> 8) + zVec.mY;
      snowForces[i].mZ += static_cast< short >((impulse.mZ * impulseScale) >> 8) + zVelocity + zVec.mZ;
    }
    return;
  }
  const float zDeltaTime = type == kEFX_Unknown5 ? 0.f : dt;
  for (int i = 0; i < snowForces.size(); ++i) {
    const CVector3f delta = CVector3f::ByElementMultiply(
        inverseScale, zDeltaTime * mSnowZDeltas[i & 15]);
    snowForces[i].mX += real_to_fixed8_8(delta.GetX()) + zVec.mX;
    snowForces[i].mY += real_to_fixed8_8(delta.GetY()) + zVec.mY;
    snowForces[i].mZ += real_to_fixed8_8(delta.GetZ()) + zVec.mZ;
  }
}

void CEnvFxManager::UpdateBlockedGrids(CStateManager& mgr, EEnvFxType type,
                                       const CTransform4f& camXf, const CTransform4f& xf,
                                       const CTransform4f& invXf) {
  const CPlayer* player = mgr.GetPlayer(0);
  const CVector3f playerPos = player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed
                                  ? camXf.GetTranslation()
                                  : player->GetBallPosition();
  const CVector3f localPos = invXf * playerPos;
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
        const CVector2i gridPos = grid.GetStart();
        const CVector3f localGrid(fixed8_8_to_real(gridPos.GetX()),
                                  fixed8_8_to_real(gridPos.GetY()), 0.f);
        const CVector3f start = xf * localGrid + 500.f * CVector3f::Up();
        const CVector3f down = CVector3f::Down();
        if (type == kEFX_Unknown6 || type == kEFX_Unknown7) {
          bool visible = CGameCollision::RayStaticLineOfSightTest(mgr, start, down, 1000.f,
                                                                   filter);
          if (visible) {
            if (!blockListBuilt) {
              BuildBlockObjectList(blockList, mgr);
              blockListBuilt = true;
            }
            for (int j = 0; j < blockList.size(); ++j) {
              const CScriptTrigger* trigger =
                  TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(blockList[j]));
              if (trigger == nullptr) {
                continue;
              }
              const rstl::optional_object< CAABox > bounds = trigger->GetTouchBounds();
              if (bounds) {
                const CCollidableAABox box(*bounds, CMaterialList(kMT_Trigger));
                const CInternalRayCastStructure ray(start, down, 1000.f,
                                                    CTransform4f::Identity(), filter);
                if (box.CastRayInternal(ray).IsValid()) {
                  visible = false;
                  break;
                }
              }
            }
          }
          grid.SetVisibility(rstl::pair< bool, float >(visible, visible ? -10000.f : 0.f));
        } else {
          CRayCastResult best =
              CGameCollision::RayStaticIntersection(mgr, start, down, 1000.f, filter);
          if (best.IsValid()) {
            if (!blockListBuilt) {
              BuildBlockObjectList(blockList, mgr);
              blockListBuilt = true;
            }
            for (int j = 0; j < blockList.size(); ++j) {
              const CScriptTrigger* trigger =
                  TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(blockList[j]));
              if (trigger == nullptr) {
                continue;
              }
              const rstl::optional_object< CAABox > bounds = trigger->GetTouchBounds();
              if (bounds) {
                const CCollidableAABox box(*bounds, CMaterialList(kMT_Trigger));
                const CInternalRayCastStructure ray(start, down, 1000.f,
                                                    CTransform4f::Identity(), filter);
                const CRayCastResult hit = box.CastRayInternal(ray);
                if (hit.IsValid() && hit.GetTime() < best.GetTime()) {
                  best = hit;
                }
              }
            }
          }
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
    uint force = static_cast< uint >(mFirstSnowForce);
    if (!grid.mBlock.first) {
      continue;
    }

    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      CVectorFixed8_8& particle = grid.mParticles[j];
      particle += snowForces[force];
      particle.mZ &= 0x3fff;
      force = (force + 1) & 0xff;
    }
  }
}

void CEnvFxManager::UpdateDriftingParticles(
    float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
    const CTransform4f& invXf) {
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    uint force = static_cast< uint >(mFirstSnowForce);
    if (!grid.mBlock.first) {
      continue;
    }
    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      CVectorFixed8_8& particle = grid.mParticles[j];
      particle += snowForces[force];
      particle.mZ &= 0x3fff;
      force = (force + 1) & 0xff;
    }
  }
}

void CEnvFxManager::UpdateParticleTrails(float dt, const CVectorFixed8_8& zVec) {
  static const float kSecondaryOffsets[8] = {-0.6f, 1.2f, -0.9f, 0.3f,
                                              -0.4f, 0.9f, -2.f, 0.4f};
  static const float kPrimaryOffsets[7] = {0.3f, 0.9f, 0.5f, 0.8f, 0.4f, 0.7f, 0.2f};
  static uint seed = 0;
  CRandom16 random(seed);
  const int period = mPreviousFxType == kEFX_Unknown7 ? 8 : 2;
  const int primaryAxis = mPreviousFxType == kEFX_Unknown7 ? 0 : 2;
  const int secondaryAxis = mPreviousFxType == kEFX_Unknown7 ? 2 : 0;
  const float primaryScale = mPreviousFxType == kEFX_Unknown7 ? 1.5f : 1.f;

  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.mBlock.first) {
      continue;
    }
    for (int trail = 0; trail < grid.mParticles.size() / 8; ++trail) {
      float& lifetime = grid.mParticleLifetimes[trail];
      int& frame = grid.mTrailFrames[trail];
      lifetime -= dt / 3.f;
      ++frame;
      const int base = trail * 8;
      if (lifetime > 0.f) {
        if (frame % period == 0) {
          const int current = ((frame - 1) / period) & 7;
          const int next = (current + 1) & 7;
          CVectorFixed8_8& currentPoint = grid.mParticles[base + current];
          CVectorFixed8_8& nextPoint = grid.mParticles[base + next];
          nextPoint = currentPoint;
          if (frame % (period * 2) == 0) {
            const short delta = real_to_fixed8_8(
                primaryScale * kPrimaryOffsets[trail % 7]);
            if (primaryAxis == 0) {
              nextPoint.mX += delta;
            } else {
              nextPoint.mZ += delta;
            }
          } else {
            const short delta = real_to_fixed8_8(
                kSecondaryOffsets[(frame / (period * 2)) & 7]);
            if (secondaryAxis == 0) {
              nextPoint.mX += delta;
            } else {
              nextPoint.mZ += delta;
            }
          }
          currentPoint.mX -= nextPoint.mX;
          currentPoint.mY -= nextPoint.mY;
          currentPoint.mZ -= nextPoint.mZ;
        }
      } else {
        lifetime = 1.f;
        frame = period * random.Range(0, 100);
        const int current = (frame / period) & 7;
        for (int point = 0; point < 8; ++point) {
          grid.mParticles[base + point] = CVectorFixed8_8();
        }
        grid.mParticles[base + current] = CVectorFixed8_8(
            static_cast< short >(random.Range(0.f, grid.mExtent.GetX() - 20.f)),
            static_cast< short >(random.Range(0.f, static_cast< float >(grid.mExtent.GetY()))),
            static_cast< short >(random.Range(20.f, 16363.f)));
      }
      CVectorFixed8_8& point = grid.mParticles[base + ((frame / period) & 7)];
      point.mZ = (point.mZ + zVec.mZ) & 0x3fff;
    }
  }
  seed = random.GetSeed();
}

void CEnvFxManager::UpdateDarkWorldParticles(
    float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
    const CTransform4f& invXf) {
  const int firstForce = static_cast< int >(mFirstSnowForce);
  const float lifetimeDelta = dt / 3.f;
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.mBlock.first) {
      continue;
    }
    CRandom16 random(99);
    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      float& lifetime = grid.mParticleLifetimes[j];
      lifetime -= lifetimeDelta;
      CVectorFixed8_8& particle = grid.mParticles[j];
      if (lifetime > 0.f) {
        const CVectorFixed8_8& force = snowForces[(firstForce + random.Next()) & 0xff];
        const float elapsed = 1.f - lifetime;
        const float elapsedSquared = elapsed * elapsed;
        const float growth = 2.f * elapsedSquared * elapsedSquared;
        const short forceScale = real_to_fixed8_8(1.f + growth);
        particle.mX += (force.mX * forceScale) >> 8;
        particle.mY += (force.mY * forceScale) >> 8;
        particle.mZ += (force.mZ * forceScale) >> 8;
        particle.mZ += real_to_fixed8_8(dt * growth);
      } else {
        lifetime = 1.f;
        const short z = real_to_fixed8_8(
            (invXf * CVector3f(0.f, 0.f, grid.mBlock.second)).GetZ());
        const short y = static_cast< short >(
            random.Range(0.f, static_cast< float >(grid.mExtent.GetY())));
        const short x = static_cast< short >(
            random.Range(0.f, static_cast< float >(grid.mExtent.GetX())));
        particle = CVectorFixed8_8(x, y, z);
      }
    }
  }
}

void CEnvFxManager::UpdateRainParticles(const CVectorFixed8_8& zVec, const CVector3f& inverseScale,
                                        float dt) {
  const short deltaZ = zVec.GetZ() + real_to_fixed8_8(-40.f * dt * inverseScale.GetZ());
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    CEnvFxManagerGrid& grid = mGrids[i];
    if (!grid.mBlock.first) {
      continue;
    }
    for (int j = grid.mParticles.size() - 1; j >= 0; --j) {
      grid.mParticles[j].mZ = (grid.mParticles[j].mZ + deltaZ) & 0x3fff;
    }
  }
}

void CEnvFxManager::UpdateUnderwaterParticles(const CVectorFixed8_8& zVec) {
  for (int i = mGrids.size() - 1; i >= 0; --i) {
    rstl::vector< CVectorFixed8_8 >& particles = mGrids[i].mParticles;
    for (int j = particles.size() - 1; j >= 0; --j) {
      particles[j].mZ = (particles[j].mZ + zVec.GetZ()) & 0x3fff;
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
        TCastToPtr< CHUDBillboardEffect >(mgr.GetObjectByIdFromListAll(mEnvRainSplashIds[i]));
    if (effect != nullptr) {
      mgr.SetActorAreaId(*effect, mgr.GetNextAreaId());
    }
  }

  const float upness = CVector3f::Dot(camXf.GetForward(), CVector3f::Up());
  const float splashRate = mEnableSplash ? mFxDensity * rstl::max_val(0.f, upness) : 0.f;
  float forwardRate = 0.f;
  if (mEnableSplash && upness >= -0.1f) {
    const CPlayer* player = mgr.GetPlayer(0);
    const CVector3f localVelocity =
        player->GetTransform().TransposeRotate(player->GetVelocityWR());
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
        TCastToPtr< CHUDBillboardEffect >(mgr.GetObjectByIdFromListAll(mEnvRainSplashIds[i]));
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
  const CCameraManager* cameraManager = mgr.GetCameraManager(0);
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
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);

  BlankFirstSnowflakeMip(***mTxtrSnowFlake);
  (*mTxtrSnowFlake)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  (*mTxtrEnvGradient)->Load(GX_TEXMAP1, CTexture::kCM_Clamp);
}

void CEnvFxManager::SetupDriftingParticleTevs(CStateManager& mgr) {
  mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true);
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
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Blue().GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  BlankFirstSnowflakeMip(***mTxtrSnowFlake);
  (*mTxtrSnowFlake)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE,
                      GX_PTIDENTITY);
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
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevKColor(GX_KCOLOR0, CColor::White().GetGXColor());
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
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  BlankFirstSnowflakeMip(***mUnderwaterFlake);
  (*mUnderwaterFlake)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE,
                      GX_PTIDENTITY);
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
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevKColor(GX_KCOLOR0, CColor(1.f, 1.f, 1.f, 0.15f).GetGXColor());
  (*mTxtrEnvGradient)->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
}

void CEnvFxManager::SetupParticleTrailTevs(CStateManager& mgr) {
  const CGameCamera* camera = mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true);
  if (camera->GetFluidCount() != 0) {
    gpRender->SetWorldFog(kRFM_PerspExp, 0.f, 35.f, CColor::Black());
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
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_ZERO);
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
    CVectorFixed8_8 offsets[16];
    CVectorFixed8_8 upDeltas[16];
    CVectorFixed8_8 rightDeltas[16];
    CRandom16 random(99);
    for (int i = 0; i < 16; ++i) {
      random.Next();
      const float size = random.Range(0.05f, 0.7f);
      const CVector3f up = camXf.Rotate(CVector3f(0.f, 0.f, size));
      const CVector3f right = camXf.Rotate(CVector3f(size, 0.f, 0.f));
      const CVector3f offset = -0.5f * (up + right);
      offsets[i] = CVectorFixed8_8(real_to_fixed8_8(offset.GetX()),
                                   real_to_fixed8_8(offset.GetY()),
                                   real_to_fixed8_8(offset.GetZ()));
      upDeltas[i] = CVectorFixed8_8(real_to_fixed8_8(up.GetX()), real_to_fixed8_8(up.GetY()),
                                    real_to_fixed8_8(up.GetZ()));
      rightDeltas[i] = CVectorFixed8_8(real_to_fixed8_8(right.GetX()),
                                       real_to_fixed8_8(right.GetY()),
                                       real_to_fixed8_8(right.GetZ()));
    }
    for (int i = 0; i < mGrids.size(); ++i) {
      mGrids[i].RenderDarkWorldParticles(xf, invXf, camXf, mFxDensity, offsets, upDeltas,
                                         rightDeltas);
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
  if (density < 0.1f) {
    return static_cast< int >(74.f * (density / 0.1f));
  }
  return static_cast< int >(21.f * (density / 0.9f) + 74.f);
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
      mLeftRainSound = CSfxManager::AddEmitter(0x2841, CVector3f::Zero(),
                                               CSfxManager::kAllAreas, true, true,
                                               CSfxManager::kMaxPriority);
      mRightRainSound = CSfxManager::AddEmitter(0x2842, CVector3f::Zero(),
                                                CSfxManager::kAllAreas, true, true,
                                                CSfxManager::kMaxPriority);
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
    if (trigger != nullptr &&
        (trigger->GetTriggerFlags() & kTFL_BlockEnvironmentalEffects) != 0) {
      list.push_back(entity->GetUniqueId());
    }
  }
}

void CEnvFxManager::AreaLoaded() {
  for (int i = 0; i < mGrids.size(); ++i) {
    mGrids[i].SetDirty(true);
  }
}
