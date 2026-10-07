#include "MetroidPrime/ScriptObjects/CScriptDestructibleBarrier.hpp"


#include "Collision/CollisionUtil.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDestructibleBarrier.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "REL/REL_Setup.h"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/COBBTreeGroup.hpp"

CBarrierChunkGrid::CBarrierChunkGrid(const CVector3i& dims, const CVector3f& chunkSize,
                                     float chunkHealth)
: mDims(dims)
, mChunkSize(chunkSize)
, mInvChunkSize(1.f / chunkSize.GetX(), 1.f / chunkSize.GetY(), 1.f / chunkSize.GetZ())
, mBounds(CVector3f::Zero(), CVector3f(dims.GetX() * chunkSize.GetX(),
                                       dims.GetY() * chunkSize.GetY(),
                                       dims.GetZ() * chunkSize.GetZ()))
, mNumDestroyed(0)
, mChunkHealth(chunkHealth)
, mLayerCounts(dims.GetZ(), 0)
, mRowCounts(dims.GetZ() * dims.GetY(), dims[0])
, mChunkHealths(dims.GetZ() * (dims.GetY() * dims.GetX()), chunkHealth) {
  const int layerCount = dims.GetX() * dims.GetY();
  for (int i = 0; i < mLayerCounts.size(); ++i) {
    mLayerCounts[i] = layerCount;
  }
}

rstl::optional_object< SBarrierSection > CBarrierChunkGrid::SplitOffTop() {
  int split = -1;
  const int dimX = mDims.GetX();
  const int dimY = mDims.GetY();
  int top = mDims.GetZ();
  CBarrierChunkGrid* grid;
  for (int z = top - 1; z > 0; --z) {
    if (mLayerCounts[z - 1] == dimX * dimY) {
      continue;
    }
    if (mLayerCounts[z] == 0) {
      top = z;
      continue;
    }
    int connections = 0;
    const float* upper = &mChunkHealths[dimX * (z * mDims.GetY())];
    const float* lower = &mChunkHealths[dimX * ((z - 1) * dimY)];
    for (int y = 0; y < dimY; ++y) {
      for (int x = 0; x < dimX; ++x) {
        if (*upper > 0.f && *lower > 0.f) {
          ++connections;
          break;
        }
        ++upper;
        ++lower;
      }
    }
    if (connections == 0) {
      split = z;
      break;
    }
  }

  if (split != -1) {
    grid = rs_new CBarrierChunkGrid(*this);
    for (int z = 0; z < mDims.GetZ(); ++z) {
      if (z < split) {
        grid->mLayerCounts[z] = 0;
        grid->mRowCounts[z] = 0;
        float* health = &grid->mChunkHealths[grid->mDims.GetX() * (z * grid->mDims.GetY())];
        for (int y = 0; y < grid->mDims.GetY(); ++y) {
          grid->mRowCounts[y + z * grid->mDims.GetY()] = 0;
          for (int x = 0; x < grid->mDims.GetX(); ++x) {
            *health++ = 0.f;
          }
        }
      } else {
        mLayerCounts[z] = 0;
        mRowCounts[z] = 0;
        float* health = &mChunkHealths[mDims.GetX() * (z * mDims.GetY())];
        for (int y = 0; y < mDims.GetY(); ++y) {
          mRowCounts[y + z * mDims.GetY()] = 0;
          for (int x = 0; x < mDims.GetX(); ++x) {
            if (*health > 0.f) {
              ++mNumDestroyed;
              *health = 0.f;
            }
            ++health;
          }
        }
      }
    }

    CVector3f center = mBounds.GetCenterPoint();
    CVector3f spawnPos = center;
    spawnPos.SetZ(split * mChunkSize.GetZ());
    const CVector3f pivot =
        spawnPos + CVector3f(0.f, 0.f, 0.5f * (top - split - 1) * mChunkSize.GetZ());
    SBarrierSection section(grid, spawnPos, pivot, split);
    return section;
  }
  return rstl::optional_object< SBarrierSection >();
}

rstl::optional_object< CVector3i > CBarrierChunkGrid::GetChunkAt(const CVector3f& pos,
                                                                float radius) const {
  CVector3f closest = pos;
  if (CollisionUtil::AABoxPointSqrDist(pos, mBounds, &closest) > radius * radius) {
    return rstl::optional_object< CVector3i >();
  }
  const CVector3f& local = CVector3f::ByElementMultiply(closest, mInvChunkSize);
  return CVector3i(rstl::min_val(rstl::max_val(int(local.GetX()), 0), mDims.GetX() - 1),
                   rstl::min_val(rstl::max_val(int(local.GetY()), 0), mDims.GetY() - 1),
                   rstl::min_val(rstl::max_val(int(local.GetZ()), 0), mDims.GetZ() - 1));
}

bool CBarrierChunkGrid::ApplyDamage(CStateManager& mgr, const CVector3f& pos, float damage,
                                    float radius,
                                    const rstl::optional_object< ChunkCallback >& callback) {
  bool destroyed = false;
  const rstl::optional_object< CVector3i > start = GetChunkAt(pos, radius);
  if (!start) {
    return false;
  }

  CVector3i cur = *start;
  CVector3i prev = *start;
  const int steps =
      rstl::min_val(rstl::max_val(int(damage / mChunkHealth), 4), 20);
  for (int i = 0; i < steps; ++i) {
    const rstl::pair< bool, float > result =
        DamageChunk(cur.GetX(), cur.GetY(), cur.GetZ(), damage);
    damage = result.second;
    destroyed |= result.first;
    if (damage <= 0.f) {
      break;
    }
    if (!result.first) {
      cur = *start;
    } else if (callback) {
      (*callback)(cur);
    }
    do {
      const int axis = mgr.Random()->Next() % 3;
      const int dir = (mgr.Random()->Next() & 2) == 0 ? -1 : 1;
      cur[axis] += dir;
    } while (cur == prev);
    prev = cur;
    cur[0] = rstl::min_val(rstl::max_val(cur[0], 0), mDims.GetX() - 1);
    cur[1] = rstl::min_val(rstl::max_val(cur[1], 0), mDims.GetY() - 1);
    cur[2] = rstl::min_val(rstl::max_val(cur[2], 0), mDims.GetZ() - 1);
  }
  return destroyed;
}

rstl::pair< bool, float > CBarrierChunkGrid::DamageChunk(int x, int y, int z, float damage) {
  float& health = mChunkHealths[x + (y + z * mDims.GetY()) * mDims.GetX()];
  const float remaining = damage - health;
  if (health > 0.f) {
    health -= damage;
    if (health <= 0.f) {
      health = 0.f;
      --mLayerCounts[z];
      --mRowCounts[y + z * mDims.GetY()];
      ++mNumDestroyed;
      return rstl::pair< bool, float >(true, remaining);
    }
  }
  return rstl::pair< bool, float >(false, remaining);
}

int CBarrierChunkGrid::SplitBox(const CVector3i& min, const CVector3i& size, CVector3i& minA,
                                CVector3i& sizeA, CVector3i& minB, CVector3i& sizeB) {
  if (size.GetY() > 1) {
    minA = min;
    sizeA = size;
    sizeA[1] = sizeA[1] / 2;
    minB = minA;
    minB[1] = minB[1] + sizeA[1];
    sizeB = size;
    sizeB[1] = size.GetY() - sizeA[1];
    return 1;
  }
  if (size.GetZ() > 1) {
    minA = min;
    sizeA = size;
    sizeA[2] = sizeA[2] / 2;
    minB = minA;
    minB[2] = minB[2] + sizeA[2];
    sizeB = size;
    sizeB[2] = size.GetZ() - sizeA[2];
    return 2;
  }
  return 0;
}

void CBarrierChunkGrid::AddBoxGeometry(COBBTree::SIndexData& data,
                                       rstl::vector< ushort >& surfaces, const CVector3f& center,
                                       const CVector3f& extent) const {
  const COBBTree::SIndexData& cube =
      COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitCube)->GetIndexData();
  const ushort vertBase = data.mVertices.size();
  const ushort edgeBase = data.mEdges.size();
  const ushort triBase = data.mSurfaceIndices.size() / 3;

  if (data.mMaterials.size() == 0) {
    data.mMaterials = cube.mMaterials;
  }

  data.mEdgeMaterials.reserve(data.mEdgeMaterials.size() + cube.mEdgeMaterials.size());
  data.mEdgeMaterials.insert(data.mEdgeMaterials.end(), cube.mEdgeMaterials.begin(),
                             cube.mEdgeMaterials.end());

  data.mEdges.reserve(data.mEdges.size() + cube.mEdges.size());
  for (int i = 0; i < cube.mEdges.size(); ++i) {
    data.mEdges.push_back_unsafe(CCollisionEdge(vertBase + cube.mEdges[i].GetVertIndex1(),
                                                vertBase + cube.mEdges[i].GetVertIndex2()));
  }

  data.x60_.reserve(data.x60_.size() + cube.x60_.size());
  for (int i = 0; i < cube.x60_.size(); ++i) {
    data.x60_.push_back_unsafe(triBase + cube.x60_[i]);
  }

  data.mSurfaceIndices.reserve(data.mSurfaceIndices.size() + cube.mSurfaceIndices.size());
  for (int i = 0; i < cube.mSurfaceIndices.size(); ++i) {
    data.mSurfaceIndices.push_back_unsafe(edgeBase + cube.mSurfaceIndices[i]);
  }

  data.mSurfaceMaterials.reserve(data.mSurfaceMaterials.size() + cube.mSurfaceMaterials.size());
  data.mSurfaceMaterials.insert(data.mSurfaceMaterials.end(), cube.mSurfaceMaterials.begin(),
                                cube.mSurfaceMaterials.end());

  data.mVertMaterials.reserve(data.mVertMaterials.size() + cube.mVertMaterials.size());
  data.mVertMaterials.insert(data.mVertMaterials.end(), cube.mVertMaterials.begin(),
                             cube.mVertMaterials.end());

  data.mVertices.reserve(data.mVertices.size() + cube.mVertices.size());
  for (int i = 0; i < cube.mVertices.size(); ++i) {
    data.mVertices.push_back_unsafe(cube.mVertices[i] * extent + center);
  }

  surfaces.reserve(surfaces.size() + 12);
  for (int i = 0; i < 12; ++i) {
    surfaces.push_back_unsafe(triBase + i);
  }
}

COBBTree::CNode* CBarrierChunkGrid::BuildBoxNode(COBBTree::SIndexData& data, const CVector3i& min,
                                                 const CVector3i& size) const {
  const CVector3f extent = GetExtent(size);
  const CVector3f center =
      CVector3f(min.GetX() * mChunkSize.GetX(), min.GetY() * mChunkSize.GetY(),
                min.GetZ() * mChunkSize.GetZ()) +
      0.5f * extent;
  rstl::vector< ushort > surfaces;
  AddBoxGeometry(data, surfaces, center, extent);
  CTransform4f xf = CTransform4f::Translate(center);
  return rs_new COBBTree::CNode(xf, extent, nullptr, nullptr,
                                rs_new COBBTree::CLeafData(surfaces));
}

COBBTree::CNode* CBarrierChunkGrid::BuildRowNode(COBBTree::SIndexData& data,
                                                 const CVector3i& row) const {
  rstl::vector< ushort > surfaces;
  const float* health =
      &mChunkHealths[row.GetY() * mDims.GetX() + mDims.GetX() * (row.GetZ() * mDims.GetY())];
  bool alive = *health > 0.f;
  int start = alive ? 0 : 0x7fffffff;
  int end = -1;
  CVector3i runMin(0, row.GetY(), row.GetZ());
  CVector3i runSize(0, 1, 1);
  for (int x = 0; x <= mDims.GetX(); ++x) {
    bool cur = x == mDims.GetX() ? !alive : *health > 0.f;
    if (cur != alive) {
      alive = cur;
      if (cur) {
        runMin[0] = x;
        runSize[0] = 0;
        start = rstl::min_val(x, start);
      } else {
        end = rstl::max_val(x, end);
        const CVector3f extent = GetExtent(runSize);
        const CVector3f center =
            CVector3f(runMin.GetX() * mChunkSize.GetX(), runMin.GetY() * mChunkSize.GetY(),
                      runMin.GetZ() * mChunkSize.GetZ()) +
            0.5f * extent;
        AddBoxGeometry(data, surfaces, center, extent);
      }
    }
    ++runSize[0];
    ++health;
  }

  if (end == -1) {
    return nullptr;
  }

  const CVector3f extent = CVector3f(float(end - start), 1.f, 1.f) * mChunkSize;
  const CVector3f center =
      CVector3f(start * mChunkSize.GetX(), row.GetY() * mChunkSize.GetY(),
                row.GetZ() * mChunkSize.GetZ()) +
      0.5f * extent;
  CTransform4f xf = CTransform4f::Translate(center);
  return rs_new COBBTree::CNode(xf, extent, nullptr, nullptr,
                                rs_new COBBTree::CLeafData(surfaces));
}

COBBTree::CNode* CBarrierChunkGrid::BuildNode(COBBTree::SIndexData& data, const CVector3i& min,
                                              const CVector3i& size) const {
  CVector3i minA = CVector3i::Zero();
  CVector3i sizeA = CVector3i::Zero();
  CVector3i minB = CVector3i::Zero();
  CVector3i sizeB = CVector3i::Zero();
  if (SplitBox(min, size, minA, sizeA, minB, sizeB) == 0) {
    return BuildRowNode(data, min);
  }
  const CVector3f extent = GetExtent(size);
  const CVector3f center =
      CVector3f(min.GetX() * mChunkSize.GetX(), min.GetY() * mChunkSize.GetY(),
                min.GetZ() * mChunkSize.GetZ()) +
      0.5f * extent;
  CTransform4f xf = CTransform4f::Translate(center);
  return rs_new COBBTree::CNode(xf, extent, BuildNode(data, minA, sizeA),
                                BuildNode(data, minB, sizeB), nullptr);
}

rstl::auto_ptr< CCollidableOBBTreeGroup >
CBarrierChunkGrid::BuildCollision(const CMaterialList& material) const {
  COBBTree::CNode::SetAllocator(nullptr);
  COBBTree::SIndexData data;
  const COBBTree::CNode* root = BuildNode(data, CVector3i::Zero(), mDims);
  rstl::auto_ptr< COBBTree > tree(rs_new COBBTree(data, root));
  return rs_new CCollidableOBBTreeGroup(rs_new COBBTreeGroup(tree), material);
}

void CBarrierChunkGrid::Render(const CTransform4f& xfIn, const CModelFlags& flags,
                               const CModel* left, const CModel* center, const CModel* right,
                               const CModel* fullRow, const CModel* fullLayer) const {
  const float halfX = 0.5f * mChunkSize.GetX();
  const float halfY = 0.5f * mChunkSize.GetY();
  const CVector3f origin = xfIn.GetTranslation();
  CTransform4f xf = xfIn;
  const int dimX = mDims.GetX();
  const int layerSize = dimX * mDims.GetY();
  const int dimZ = mDims.GetZ();
  for (int z = 0; z < dimZ; ++z) {
    const float zOff = z * mChunkSize.GetZ();
    if (layerSize == mLayerCounts[z] && fullLayer) {
      xf.SetTranslation(xf.Rotate(CVector3f(halfX, halfY, 0.f) + CVector3f(0.f, 0.f, zOff)) + origin);
      CGraphics::SetModelMatrix(xf);
      fullLayer->Draw(flags);
      continue;
    }
    const int dimY = mDims.GetY();
    for (int y = 0; y < dimY; ++y) {
      const float yOff = y * mChunkSize.GetY();
      if (mDims.GetX() == mRowCounts[y + z * dimY] && fullRow) {
        xf.SetTranslation(xf.Rotate(CVector3f(halfX, halfY, 0.f) + CVector3f(0.f, yOff, zOff)) + origin);
        CGraphics::SetModelMatrix(xf);
        fullRow->Draw(flags);
        continue;
      }
      const int rowX = mDims.GetX();
      const float* health = &mChunkHealths[(y + z * dimY) * rowX];
      for (int x = 0; x < rowX; ++x, ++health) {
        if (*health > 0.f) {
          xf.SetTranslation(xf.Rotate(CVector3f(halfX, halfY, 0.f) + CVector3f(x * mChunkSize.GetX(), yOff, zOff)) +
                            origin);
          CGraphics::SetModelMatrix(xf);
          if (x == 0) {
            left->Draw(flags);
          } else if (x == mDims.GetX() - 1) {
            right->Draw(flags);
          } else {
            center->Draw(flags);
          }
        }
      }
    }
  }
}

CScriptDestructibleBarrier::CScriptDestructibleBarrier(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CActorParameters& actParms, const CHealthInfo& healthInfo,
    const CDamageVulnerability& dVuln, const CVector3i& dims, const CVector3f& chunkSize,
    const CModelData& leftModel, const CModelData& centerModel, const CModelData& rightModel,
    const CModelData& fullRowModel, const CModelData& fullLayerModel,
    const CModelData& baseModel, CAssetId particle1, int particle1Count, CAssetId particle2,
    int particle2Count, CAssetId particle3, int particle3Count, CAssetId particle4,
    int particle4Count, int sfxChunkGenerated, int sfxChunkDestroyed, int sfxMoveDown,
    int sfxMoveUp, int sfxStop, bool startLowered, float lowerPercent, float lowerDelay,
    float moveSpeed)
: CPhysicsActor(uid, name, info, 0, xf, leftModel,
                CMaterialList(kMT_Unknown59, kMT_Immovable, kMT_Occluder), CAABox::MakeNullBox(),
                SMoverData(1.f), actParms, CPhysicsActor::skDefaultStepData)
, mHealthInfo(CHealthInfo(100000.f, healthInfo.GetKnockBackResistance()))
, mChunkHealthInfo(healthInfo)
, mDamageVulnerability(dVuln)
, mGrid(dims, chunkSize, healthInfo.GetInitialHP())
, mCollision(mGrid.BuildCollision(GetMaterialList()))
, mDims(dims)
, mChunkSize(chunkSize)
, mLeftModel(leftModel)
, mCenterModel(centerModel)
, mRightModel(rightModel)
, mFullRowModel(fullRowModel)
, mFullLayerModel(fullLayerModel)
, mBaseModel(baseModel)
, mParticle1(particle1 != kInvalidAssetId
                 ? rs_new CElementGen(TToken< CGenDescription >(
                                          gpSimplePool->GetObj(SObjectTag('PART', particle1))),
                                      CElementGen::kMOT_Normal, CElementGen::kOSF_One)
                 : nullptr)
, mParticle1Count(particle1Count)
, mParticle2(particle2 != kInvalidAssetId
                 ? rs_new CElementGen(TToken< CGenDescription >(
                                          gpSimplePool->GetObj(SObjectTag('PART', particle2))),
                                      CElementGen::kMOT_Normal, CElementGen::kOSF_One)
                 : nullptr)
, mParticle2Count(particle2Count)
, mParticle3(particle3 != kInvalidAssetId
                 ? rs_new CElementGen(TToken< CGenDescription >(
                                          gpSimplePool->GetObj(SObjectTag('PART', particle3))),
                                      CElementGen::kMOT_Normal, CElementGen::kOSF_One)
                 : nullptr)
, mParticle3Count(particle3Count)
, mParticle4(particle4 != kInvalidAssetId
                 ? rs_new CElementGen(TToken< CGenDescription >(
                                          gpSimplePool->GetObj(SObjectTag('PART', particle4))),
                                      CElementGen::kMOT_Normal, CElementGen::kOSF_One)
                 : nullptr)
, mParticle4Count(particle4Count)
, mSfxChunkGenerated(sfxChunkGenerated)
, mSfxChunkDestroyed(sfxChunkDestroyed)
, mSfxMoveDown(sfxMoveDown)
, mSfxMoveUp(sfxMoveUp)
, mSfxStop(sfxStop)
, mLowerPercent(lowerPercent)
, mLowerDelay(lowerDelay)
, mMoveSpeed(moveSpeed)
, mTimeSinceDamage(-1.f)
, mTimeSinceDamage2(-1.f)
, mLowerOffset(0.f)
, mHeight(mChunkSize.GetZ() * mDims.GetZ())
, mRenderXf(xf)
, mInvRenderXf(CTransform4f::Identity())
, mTouchBounds(CAABox::MakeNullBox())
, mMoveSfx()
, mState(0)
, mTargetState(0) {
  mTouchedByPlayer = false;
  mHasTransparency = false;
  mPlayedSfx = false;
  SetMovable(false);
  UpdateTransforms();
  ModelData()->SetRenderUnsortedParts(false);
  if (startLowered) {
    mLowerOffset = mHeight;
    mState = 2;
    mTargetState = 2;
  }
  mHasTransparency = mHasTransparency |
                     (mLeftModel.IsNull() ? false
                                          : !mLeftModel.IsDefinitelyOpaque(CModelData::kWM_Normal));
  mHasTransparency =
      mHasTransparency |
      (mRightModel.IsNull() ? false : !mRightModel.IsDefinitelyOpaque(CModelData::kWM_Normal));
  mHasTransparency =
      mHasTransparency |
      (mCenterModel.IsNull() ? false : !mCenterModel.IsDefinitelyOpaque(CModelData::kWM_Normal));
  mHasTransparency = mHasTransparency |
                     (mFullRowModel.IsNull()
                          ? false
                          : !mFullRowModel.IsDefinitelyOpaque(CModelData::kWM_Normal));
  mHasTransparency = mHasTransparency |
                     (mFullLayerModel.IsNull()
                          ? false
                          : !mFullLayerModel.IsDefinitelyOpaque(CModelData::kWM_Normal));
  mHasTransparency =
      mHasTransparency |
      (mBaseModel.IsNull() ? false : !mBaseModel.IsDefinitelyOpaque(CModelData::kWM_Normal));
}

CScriptDestructibleBarrier::~CScriptDestructibleBarrier() {}

void CScriptDestructibleBarrier::UpdateTransforms() {
  mRenderXf = GetTransform();
  CVector3f center = mGrid.GetBounds().GetCenterPoint();
  const CVector3f& offset = CVector3f(center.GetX(), center.GetY(), mLowerOffset);
  mRenderXf.SetTranslation(mRenderXf.GetTranslation() - offset);
  mInvRenderXf = mRenderXf.GetQuickInverse();
  mTouchBounds = mGrid.GetBounds().GetTransformedAABox(mRenderXf);
  SetOtherBounds(mTouchBounds);
  SetRenderBounds(mTouchBounds);
  mTouchBounds = CAABox(mTouchBounds.GetMinPoint() - CVector3f::One(),
                        mTouchBounds.GetMaxPoint() + CVector3f::One());
  const CTransform4f rot = GetTransform().GetRotation();
  if (mParticle1.get()) {
    mParticle1->SetOrientation(rot);
  }
  if (mParticle2.get()) {
    mParticle2->SetOrientation(rot);
  }
  if (mParticle3.get()) {
    mParticle3->SetOrientation(rot);
  }
  if (mParticle4.get()) {
    mParticle4->SetOrientation(rot);
  }
}

rstl::optional_object< CAABox > CScriptDestructibleBarrier::GetTouchBounds() const {
  if (GetActive()) {
    return mTouchBounds;
  }
  return rstl::optional_object< CAABox >();
}

void CScriptDestructibleBarrier::Touch(CActor& actor, CStateManager& mgr) {
  if (TCastToPtr< CPlayer >(&actor)) {
    mTouchedByPlayer = true;
  }
}

void CScriptDestructibleBarrier::PreThink(float dt, CStateManager& mgr) {}

void CScriptDestructibleBarrier::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const float destroyedPercent = 100.f * mGrid.GetNumDestroyed() /
                                 (mDims.GetX() * mDims.GetY() * mDims.GetZ());

  SBarrierSection* it = mSections.begin();
  while (it != mSections.end()) {
    if (mgr.GetObjectById(it->mActorId) == nullptr) {
      it = mSections.erase(it);
    } else {
      ++it;
    }
  }

  if (mParticle1.get()) {
    mParticle1->Update(dt);
  }
  if (mParticle2.get()) {
    mParticle2->Update(dt);
  }
  if (mParticle3.get()) {
    mParticle3->Update(dt);
  }
  if (mParticle4.get()) {
    mParticle4->Update(dt);
  }

  if (mTargetState != mState) {
    if (mState == 1) {
      if (!mMoveSfx && destroyedPercent < 100.f) {
        mMoveSfx = CSfxManager::AddEmitter(mSfxMoveDown, GetTranslation(),
                                           GetCurrentAreaId().Value(), true, true);
      }
      mLowerOffset += mMoveSpeed * dt;
      mLowerOffset = rstl::min_val(mLowerOffset, mHeight);
      if (mLowerOffset == mHeight) {
        if (mMoveSfx) {
          CSfxManager::SfxStop(mMoveSfx);
          mMoveSfx = CSfxHandle();
        }
        CSfxManager::SfxStart(mSfxStop, CAudioSys::kMaxVolume, 64, CSfxManager::kAllAreas);
        mState = 2;
        mGrid = CBarrierChunkGrid(mDims, mChunkSize, mChunkHealthInfo.GetInitialHP());
        mCollision = mGrid.BuildCollision(GetMaterialList());
        mTimeSinceDamage = -1.f;
        mTimeSinceDamage2 = -1.f;
        SendScriptMsgs(kSS_Down, mgr, kInvalidUniqueId, kSM_None);
      }
    } else {
      mState = 3;
      if (!mMoveSfx) {
        mMoveSfx = CSfxManager::AddEmitter(mSfxMoveUp, GetTranslation(),
                                           GetCurrentAreaId().Value(), true, true);
      }
      if (!mTouchedByPlayer) {
        mLowerOffset = mLowerOffset - mMoveSpeed * dt;
        mLowerOffset = rstl::max_val(mLowerOffset, 0.f);
      }
      if (0.f == mLowerOffset) {
        if (mMoveSfx) {
          CSfxManager::SfxStop(mMoveSfx);
          mMoveSfx = CSfxHandle();
        }
        CSfxManager::AddEmitter(mSfxStop, GetTranslation(), GetCurrentAreaId().Value(), true,
                                false);
        mState = 0;
      }
    }
    UpdateTransforms();
  }

  if (mState == 0 && destroyedPercent > mLowerPercent) {
    mState = 1;
    mTargetState = 0;
  }

  if (mState == 0 && mLowerDelay > 0.f) {
    if (mTimeSinceDamage >= 0.f) {
      mTimeSinceDamage += dt;
    }
    if (mTimeSinceDamage2 >= 0.f) {
      mTimeSinceDamage2 += dt;
    }
    if (mTimeSinceDamage > mLowerDelay) {
      mState = 1;
      mTargetState = 0;
    }
  }

  mTouchedByPlayer = false;
  mPlayedSfx = false;
}

CHealthInfo* CScriptDestructibleBarrier::HealthInfo() { return &mHealthInfo; }

const CDamageVulnerability* CScriptDestructibleBarrier::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

void CScriptDestructibleBarrier::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Reset:
    if (mTargetState == 0 && mState == 0) {
      mTargetState = 2;
      mState = 1;
    } else if (mTargetState == 2) {
      mTargetState = 0;
    }
    break;
  case kSM_AreaLoaded:
    break;
  case kSM_Delete:
    if (mMoveSfx) {
      CSfxManager::SfxStop(mMoveSfx);
    }
    break;
  case kSM_Damage:
    TakeDamage(mgr);
    break;
  }
  CPhysicsActor::AcceptScriptMsg(mgr, msg);
}

static inline CTransform4f GetSpawnXf(const CTransform4f& xf, const SBarrierSection& section) {
  return xf * CTransform4f::Translate(section.mSpawnPos);
}

void CScriptDestructibleBarrier::TakeDamage(CStateManager& mgr) {
  const float damage = GetHealthInfo()->GetInitialHP() - GetHealthInfo()->GetHP();
  HealthInfo()->SetHP(HealthInfo()->GetInitialHP());
  if (const CActor* actor = TCastToConstPtr< CActor >(
          mgr.GetObjectById(GetHealthInfo()->GetLastDamageSource()))) {
    bool destroyed;
    if (const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(
            mgr.GetObjectById(GetHealthInfo()->GetLastDamageSource()))) {
      const CDamageInfo& dInfo = proj->GetCurrentDamageInfo();
      const float minSize = rstl::min_val(
          mChunkSize.GetX(), rstl::min_val(mChunkSize.GetY(), mChunkSize.GetZ()));
      const CVector3f hitPos =
          proj->GetTranslation() + (0.5f * minSize) * proj->GetTransform().GetForward();
      const CVector3f localPos = mInvRenderXf * hitPos;
      destroyed = mGrid.ApplyDamage(
          mgr, localPos, dInfo.GetDamage(*GetDamageVulnerability()), 0.1f,
          rstl::optional_object< CBarrierChunkGrid::ChunkCallback >(
              TFunctor1FromMethod< CScriptDestructibleBarrier, const CVector3i& >::Make(
                  *this, &CScriptDestructibleBarrier::OnChunkDestroyed)));
      destroyed |= mGrid.ApplyDamage(
          mgr, localPos, dInfo.GetRadiusDamage(*GetDamageVulnerability()), dInfo.GetRadius(),
          rstl::optional_object< CBarrierChunkGrid::ChunkCallback >(
              TFunctor1FromMethod< CScriptDestructibleBarrier, const CVector3i& >::Make(
                  *this, &CScriptDestructibleBarrier::OnChunkDestroyed)));
    } else {
      const CVector3f localPos = mInvRenderXf * actor->GetTranslation();
      destroyed = mGrid.ApplyDamage(
          mgr, localPos, damage, 20.f,
          rstl::optional_object< CBarrierChunkGrid::ChunkCallback >(
              TFunctor1FromMethod< CScriptDestructibleBarrier, const CVector3i& >::Make(
                  *this, &CScriptDestructibleBarrier::OnChunkDestroyed)));
    }

    if (destroyed) {
      TEditorId generatorId = kInvalidEditorId;
      rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
      for (; conn != GetConnectionList().end(); ++conn) {
        if (conn->state == kSS_GRNT) {
          generatorId = conn->objId;
          break;
        }
      }

      for (;;) {
        rstl::optional_object< SBarrierSection > section = mGrid.SplitOffTop();
        if (!section) {
          break;
        }
        if (mSections.size() != 4 && generatorId != kInvalidEditorId) {
          CScriptObjectLoaderHelper::SGeneratedObject generated =
              mgr.ScriptObjectLoaderHelper().GenerateScriptObject(generatorId, mgr);
          if (generated.mUniqueId != kInvalidUniqueId) {
            if (CActor* sectionActor = TCastToPtr< CActor >(generated.mEntity)) {
              sectionActor->SetTransform(GetSpawnXf(mRenderXf, *section));
            }
            section->mActorId = generated.mUniqueId;
            mSections.push_back(*section);
            mgr.SendScriptMsg(generated.mEntity, GetUniqueId(), kSM_Activate, kInvalidUniqueId);
          }
        }
        if (!mPlayedSfx) {
          CSfxManager::AddEmitter(mSfxChunkGenerated, GetTranslation(),
                                  GetCurrentAreaId().Value(), true, false);
          mPlayedSfx = true;
        }
      }

      mCollision = mGrid.BuildCollision(GetMaterialList());
      if (mTimeSinceDamage < 0.f) {
        mTimeSinceDamage = 0.f;
      }
      mTimeSinceDamage2 = 0.f;
      if (!mPlayedSfx) {
        CSfxManager::AddEmitter(mSfxChunkDestroyed, GetTranslation(), GetCurrentAreaId().Value(),
                                true, false);
        mPlayedSfx = true;
      }
    }
  }
}

void CScriptDestructibleBarrier::OnChunkDestroyed(const CVector3i& chunk) {
  const CVector3f pos = mRenderXf * CVector3f(mChunkSize.GetX() * chunk.GetX(),
                                              mChunkSize.GetY() * chunk.GetY(),
                                              mChunkSize.GetZ() * chunk.GetZ());
  if (mParticle1.get()) {
    mParticle1->SetTranslation(pos);
    mParticle1->ForceParticleCreation(mParticle1Count);
  }
  if (mParticle2.get()) {
    mParticle2->SetTranslation(pos);
    mParticle2->ForceParticleCreation(mParticle2Count);
  }
  if (mParticle3.get()) {
    mParticle3->SetTranslation(pos);
    mParticle3->ForceParticleCreation(mParticle3Count);
  }
  if (mParticle4.get()) {
    mParticle4->SetTranslation(pos);
    mParticle4->ForceParticleCreation(mParticle4Count);
  }
}

const CCollisionPrimitive* CScriptDestructibleBarrier::GetCollisionPrimitive() const {
  if (mCollision.get() == nullptr) {
    return CPhysicsActor::GetCollisionPrimitive();
  }
  return mCollision.get();
}

CTransform4f CScriptDestructibleBarrier::GetPrimitiveTransform() const { return mRenderXf; }

void CScriptDestructibleBarrier::SetDamageVulnerability(const CDamageVulnerability& dVuln) {
  mDamageVulnerability = dVuln;
}

CVector3f CScriptDestructibleBarrier::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

CVector3f CScriptDestructibleBarrier::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (GetTouchBounds()) {
    return GetTouchBounds()->GetCenterPoint();
  }
  return CPhysicsActor::GetAimPosition(mgr, dt);
}

void CScriptDestructibleBarrier::PreRenderAllViewports(CStateManager& mgr) {
  if (GetRenderBoundsDirty()) {
    UpdatePortalSystemState(mgr);
    SetRenderBoundsDirty(false);
  }
}

void CScriptDestructibleBarrier::PreRender(CStateManager& mgr) {
  CAABox bounds = mGrid.GetBounds().GetTransformedAABox(mRenderXf);
  for (const SBarrierSection* it = mSections.begin(); it != mSections.end(); ++it) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(it->mActorId))) {
      const CTransform4f xf = actor->GetTransform() * CTransform4f::Translate(-it->mPivot);
      bounds.Include(mGrid.GetBounds().GetTransformedAABox(xf));
    }
  }
  if (mParticle1.get()) {
    const rstl::optional_object< CAABox > particleBounds = mParticle1->GetBounds();
    if (particleBounds) {
      bounds.Include(*particleBounds);
    }
  }
  if (mParticle2.get()) {
    const rstl::optional_object< CAABox > particleBounds = mParticle2->GetBounds();
    if (particleBounds) {
      bounds.Include(*particleBounds);
    }
  }
  if (mParticle3.get()) {
    const rstl::optional_object< CAABox > particleBounds = mParticle3->GetBounds();
    if (particleBounds) {
      bounds.Include(*particleBounds);
    }
  }
  if (mParticle4.get()) {
    const rstl::optional_object< CAABox > particleBounds = mParticle4->GetBounds();
    if (particleBounds) {
      bounds.Include(*particleBounds);
    }
  }
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  CActor::PreRender(mgr);
}

void CScriptDestructibleBarrier::AddToRenderer(const CStateManager& mgr) const {
  if (!mHasTransparency) {
    RenderChunks(mgr);
  }
  CActor::AddToRenderer(mgr);
}

void CScriptDestructibleBarrier::Render(const CStateManager& mgr) const {
  if (mHasTransparency) {
    RenderChunks(mgr);
  }
  if (mParticle1.get()) {
    mParticle1->Render();
  }
  if (mParticle2.get()) {
    mParticle2->Render();
  }
  if (mParticle3.get()) {
    mParticle3->Render();
  }
  if (mParticle4.get()) {
    mParticle4->Render();
  }
}

static inline const CModel* GetChunkModel(const CModelData& model) {
  return model.IsNull() ? nullptr : *model.PickStaticModel(CModelData::kWM_Normal);
}

void CScriptDestructibleBarrier::RenderChunks(const CStateManager& mgr) const {
  GetActorLights()->ActivateLights();
  if (!mBaseModel.IsNull()) {
    CTransform4f xf = GetTransform();
    CGraphics::SetModelMatrix(xf);
    mBaseModel.PickStaticModel(CModelData::kWM_Normal)->Draw(GetModelFlags());
  }
  mGrid.Render(mRenderXf, GetModelFlags(), GetChunkModel(mLeftModel), GetChunkModel(mCenterModel),
               GetChunkModel(mRightModel), GetChunkModel(mFullRowModel),
               GetChunkModel(mFullLayerModel));
  for (const SBarrierSection* it = mSections.begin(); it != mSections.end(); ++it) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(it->mActorId))) {
      const CTransform4f xf = actor->GetTransform() * CTransform4f::Translate(-it->mPivot);
      it->mGrid->Render(xf, actor->GetModelFlags(), GetChunkModel(mLeftModel),
                        GetChunkModel(mCenterModel), GetChunkModel(mRightModel),
                        GetChunkModel(mFullRowModel), GetChunkModel(mFullLayerModel));
    }
  }
}

static CModelData MakeChunkModel(CAssetId id) {
  if (id == kInvalidAssetId) {
    return CModelData::CModelDataNull();
  }
  return CModelData(CStaticRes(id, CVector3f(1.f, 1.f, 1.f)));
}

CEntity* LoadDestructibleBarrier(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDestructibleBarrier sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDestructibleBarrier.inc"

  CModelData leftModel = MakeChunkModel(sldrThis.leftModel);
  CModelData centerModel = MakeChunkModel(sldrThis.centerModel);
  CModelData rightModel = MakeChunkModel(sldrThis.rightModel);
  CModelData fullRowModel = MakeChunkModel(sldrThis.unknown_0x396660b4);
  CModelData fullLayerModel = MakeChunkModel(sldrThis.unknown_0x48e25884);
  CModelData baseModel = MakeChunkModel(sldrThis.baseModel);
  return rs_new CScriptDestructibleBarrier(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      LdrToTransform4f(sldrThis.editorProperties), LdrToActorParameters(sldrThis.actorInformation),
      LdrToHealthInfo(sldrThis.health), LdrToDamageVulnerability(sldrThis.vulnerability),
      CVector3i(sldrThis.numChunksWidth, sldrThis.numChunksDepth, sldrThis.numChunksHeight),
      sldrThis.chunkSize, leftModel, centerModel, rightModel, fullRowModel, fullLayerModel,
      baseModel, sldrThis.unknown_0x1eb90d06, sldrThis.unknown_0x9d852dfe,
      sldrThis.unknown_0x982d7fa8, sldrThis.unknown_0x2e11003d, sldrThis.unknown_0x5371ac0d,
      sldrThis.unknown_0x409d1b7c, sldrThis.unknown_0x4e749cb5, sldrThis.unknown_0x92485dfa,
      sldrThis.soundEffectOnChunkGenerated, sldrThis.soundEffectOnChunkDestroyed,
      sldrThis.soundEffectOnMoveDown, sldrThis.soundEffectOnMoveUp, sldrThis.soundEffectOnStop,
      sldrThis.unknown_0x4d3109e3, sldrThis.unknown_0x605847b9, sldrThis.unknown_0xcd9c67fe,
      sldrThis.unknown_0x0af428b4);
}

static void SetFuncPtrs() {
  static SDestructibleBarrier_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadDestructibleBarrier;
  SetSDestructibleBarrier_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSDestructibleBarrier_FuncPtrs(nullptr); }
