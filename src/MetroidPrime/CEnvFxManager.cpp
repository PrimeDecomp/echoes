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
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
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
  // TODO: Draw the billboards for environment effect 5.
}

void CEnvFxManagerGrid::RenderParticleTrails(EEnvFxType type) {
  // TODO: Interpolate the eight-point histories and fade their line strips.
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
  // TODO: Draw lifetime-faded quads using the sixteen precomputed corner offsets.
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
  // TODO: Resize and seed per-grid particles, lifetimes and trail histories for this effect.
}

void CEnvFxManager::CalculateSnowForces(const CVectorFixed8_8& zVec,
                                        rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                                        EEnvFxType type, const CVector3f& inverseScale, float dt) {
  // TODO: Build the force cycle, with separate dark-world and effect-5 motion.
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
  // The target retains the dt and transform arguments, but uses the snow update body.
  UpdateSnowParticles(snowForces);
}

void CEnvFxManager::UpdateParticleTrails(float dt, const CVectorFixed8_8& zVec) {
  // TODO: Advance normalized lifetimes, frame counters and eight-point trail histories.
}

void CEnvFxManager::UpdateDarkWorldParticles(
    float dt, rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
    const CTransform4f& invXf) {
  // TODO: Apply lifetime-dependent forces and respawn particles at the blocking height.
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
  // TODO: Configure snow texture, fog, blending and ceiling clipping.
}

void CEnvFxManager::SetupDriftingParticleTevs(CStateManager& mgr) {
  // TODO: Configure the effect-5 snow-texture variant.
}

void CEnvFxManager::SetupDarkWorldTevs() {
  // TODO: Configure additive dark-world particle rendering.
}

void CEnvFxManager::SetupUnderwaterTevs(const CTransform4f& invXf, CStateManager& mgr) {
  // TODO: Configure underwater texture blending and water-surface clipping.
}

void CEnvFxManager::SetupDefaultTevSwapMode() {
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
}

void CEnvFxManager::SetupRainTevs() {
  // TODO: Configure rain line rendering and the environment gradient.
}

void CEnvFxManager::SetupParticleTrailTevs(CStateManager& mgr) {
  // TODO: Configure fog, line width, gradient texture and additive trail blending.
}

void CEnvFxManager::Render(const CStateManager& mgr) {
  // TODO: Select the effect setup and render the grids in camera space.
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
