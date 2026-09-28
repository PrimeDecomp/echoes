#include "MetroidPrime/CDecalManager.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/math.hpp"

rstl::reserved_vector< CDecalManager::SDecal, 64 > CDecalManager::mDecalPool;
rstl::reserved_vector< int, 64 > CDecalManager::mActiveIndexList;
int CDecalManager::mFreeIndex;
bool CDecalManager::mPoolInitialized = false;
float CDecalManager::mDeltaTimeSinceLastDecalCreation;
int CDecalManager::mLastDecalCreatedIndex;
CAssetId CDecalManager::mLastDecalCreatedAssetId;

namespace {
const CMaterialList skImplicitWorldMaterials(kMT_Unknown59, kMT_Unknown60);

// Guessed name for the world-triangle collection callback.
class CDecalTriangleCollector : public IDisplayListTriangleCallback {
public:
  CDecalTriangleCollector(const CVector3f& center, const CVector3f& halfExtent,
                          const CVector3f* positions, rstl::vector< CCollisionSurface >& surfaces);

  // IDisplayListTriangleCallback
  bool OnTriangle(const CDisplayListReader& reader, const uchar* a, const uchar* b,
                  const uchar* c) override;

private:
  CVector3f mCenter;
  CVector3f mHalfExtent;
  const CVector3f* mPositions;
  rstl::vector< CCollisionSurface >& mSurfaces;
};
CHECK_SIZEOF(CDecalTriangleCollector, 0x24)

bool CDecalTriangleCollector::OnTriangle(const CDisplayListReader& reader, const uchar* a,
                                         const uchar* b, const uchar* c) {
  const CVector3f vertA = mPositions[reader.GetVertexIndex(a, GX_VA_POS)];
  const CVector3f vertB = mPositions[reader.GetVertexIndex(b, GX_VA_POS)];
  const CVector3f vertC = mPositions[reader.GetVertexIndex(c, GX_VA_POS)];
  if (CollisionUtil::TriBoxOverlap(mCenter, mHalfExtent, vertA, vertB, vertC)) {
    mSurfaces.push_back(CCollisionSurface(vertA, vertB, vertC, ~u64(0)));
  }
  return true;
}
} // namespace

void CDecalManager::Initialize() {
  if (mPoolInitialized) {
    return;
  }
  mDecalPool.clear();
  for (int i = 0; i < mDecalPool.capacity(); ++i) {
    mDecalPool.push_back(SDecal(rstl::optional_object_null(), TAreaId(0), i - 1));
  }
  mFreeIndex = mDecalPool.capacity() - 1;
  mPoolInitialized = true;
  mDeltaTimeSinceLastDecalCreation = 0.f;
  mLastDecalCreatedIndex = -1;
  mLastDecalCreatedAssetId = kInvalidAssetId;
}

void CDecalManager::ShutDown() {
  mActiveIndexList.clear();
  mDecalPool.clear();
  mPoolInitialized = false;
}

void CDecalManager::Reinitialize() {
  if (!mPoolInitialized) {
    Initialize();
  }
  for (int i = 0; i < mDecalPool.capacity(); ++i) {
    mDecalPool[i] = SDecal(rstl::optional_object_null(), TAreaId(0), i - 1);
  }
  mActiveIndexList.clear();
  mFreeIndex = mDecalPool.capacity() - 1;
}

namespace {
CDecalTriangleCollector::CDecalTriangleCollector(const CVector3f& center,
                                                 const CVector3f& halfExtent,
                                                 const CVector3f* positions,
                                                 rstl::vector< CCollisionSurface >& surfaces)
: mCenter(center), mHalfExtent(halfExtent), mPositions(positions), mSurfaces(surfaces) {
  mSurfaces.reserve(64);
}
} // namespace

void CDecalManager::GatherWorldSurfaces(const CStateManager& mgr, const CAABox& bounds,
                                        const CMaterialFilter& filter,
                                        rstl::vector< CCollisionSurface >& surfaces) {
  const CMaterialFilter worldFilter = filter.WithImplicitMaterials(skImplicitWorldMaterials);
  if (worldFilter.GetType() == CMaterialFilter::kFT_Never) {
    return;
  }
  const CWorld& world = *mgr.GetWorld();
  for (CGameArea::CConstChainIterator area = world.GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    if (area->GetOcclusionState() != CGameArea::kOS_Visible) {
      continue;
    }
    const CGameArea::CPostConstructed& post = *area->GetPostConstructed();
    if (!post.mRenderOctTree) {
      continue;
    }
    const CAreaRenderOctTree& octree = *post.mRenderOctTree;
    rstl::vector< uint > bitmap(octree.GetBitmapWordCount(), 0);
    octree.FindOverlappingModels(bitmap.data(), bounds);
    for (uint word = 0; word < octree.GetBitmapWordCount(); ++word) {
      if (bitmap[word] == 0) {
        continue;
      }
      for (int bit = 0; bit < 32; ++bit) {
        if ((bitmap[word] & (1u << bit)) == 0) {
          continue;
        }
        const SAreaSurface& areaSurface = post.mSurfaces[word * 32 + bit + 1];
        if (areaSurface.mModelIndex == -1 || areaSurface.mSurfaceGroupIndex == -1) {
          continue;
        }
        const CMetroidModelInstance& model = post.mModelInstances[areaSurface.mModelIndex];
        const ushort surfaceCount = model.GetSurfaceCountInGroup(areaSurface.mSurfaceGroupIndex);
        const ushort* indices = model.GetSurfaceIndices(areaSurface.mSurfaceGroupIndex);
        for (ushort i = 0; i < surfaceCount; ++i) {
          const CCubeSurface surface(model.GetSurfaces()[indices[i]]);
          const CAABox surfaceBounds = surface.GetBounds();
          const CCubeMaterial material = model.GetMaterialByIndex(surface.GetMaterialIndex());
          if (!worldFilter.Passes(CMaterialList(material.GetMaterialMask())) ||
              !surfaceBounds.DoBoundsOverlap(bounds)) {
            continue;
          }
          const CDisplayListReader reader(surface.GetDisplayList(), surface.GetDisplayListSize(),
                                          material.GetVertexDesc());
          CDecalTriangleCollector collector(
              bounds.GetCenterPoint(), bounds.GetHalfExtent(),
              static_cast< const CVector3f* >(model.GetVertexPointer()), surfaces);
          reader.EnumerateTriangles(collector);
        }
      }
    }
  }
}

void CDecalManager::AddDecal(const TToken< CDecalDescription >& desc, const CTransform4f& xf,
                             const CUnitVector3f& direction, CStateManager& mgr) {
  const CAssetId assetId = desc.GetTag().GetId();
  if (mLastDecalCreatedIndex != -1 && mDeltaTimeSinceLastDecalCreation < 0.75f &&
      mLastDecalCreatedAssetId == assetId) {
    const SDecal& existing = mDecalPool[mLastDecalCreatedIndex];
    if ((existing.mDecal->GetTranslation() - xf.GetTranslation()).MagSquared() < 0.01f) {
      return;
    }
  }
  if (mFreeIndex == -1) {
    RemoveFromActiveList(mActiveIndexList.begin(), mActiveIndexList[0]);
  }

  rstl::vector< CCollisionSurface > surfaces;
  {
    TToken< CDecalDescription > description(desc);
    float size1 = 0.f;
    float size2 = 0.f;
    description->mQuad1.mSZE->GetValue(0, size1);
    description->mQuad2.mSZE->GetValue(0, size2);
    const float halfSize = rstl::max_val(0.5f * size1, 0.5f * size2);
    const CVector3f halfExtent(halfSize, halfSize, halfSize);
    const CAABox bounds(xf.GetTranslation() - halfExtent, xf.GetTranslation() + halfExtent);
    GatherWorldSurfaces(mgr, bounds, CMaterialFilter::MakeInclude(CMaterialList(kMT_Unknown59)),
                        surfaces);
  }

  const int index = mFreeIndex;
  SDecal& decal = mDecalPool[index];
  mFreeIndex = decal.mNextFreeIndex;
  decal.mDecal.clear();
  decal.mDecal = CDecal(desc, xf, direction, surfaces);
  decal.mAreaId = mgr.GetNextAreaId();
  mDeltaTimeSinceLastDecalCreation = 0.f;
  mLastDecalCreatedIndex = index;
  mLastDecalCreatedAssetId = assetId;
  mActiveIndexList.push_back(index);
}

rstl::reserved_vector< int, 64 >::iterator
CDecalManager::RemoveFromActiveList(rstl::reserved_vector< int, 64 >::iterator it, int index) {
  const rstl::reserved_vector< int, 64 >::iterator next = mActiveIndexList.erase(it);
  mDecalPool[index].mNextFreeIndex = mFreeIndex;
  mFreeIndex = index;
  if (mLastDecalCreatedIndex == index) {
    mLastDecalCreatedIndex = -1;
  }
  return next;
}

void CDecalManager::Update(float dt, CStateManager& mgr) {
  mDeltaTimeSinceLastDecalCreation += dt;
  for (rstl::reserved_vector< int, 64 >::iterator it = mActiveIndexList.begin();
       it != mActiveIndexList.end();) {
    SDecal& decal = mDecalPool[*it];
    if (decal.mAreaId != mgr.GetNextAreaId() || decal.mDecal->IsDone()) {
      it = RemoveFromActiveList(it, *it);
    } else {
      decal.mDecal->Update(dt);
      ++it;
    }
  }
}

void CDecalManager::AddToRenderer(const CStateManager& mgr) {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
    return;
  }
  const rstl::reserved_vector< int, 64 >::const_iterator end = mActiveIndexList.end();
  for (rstl::reserved_vector< int, 64 >::const_iterator it = mActiveIndexList.begin(); it != end;
       ++it) {
    const CDecal& decal = *mDecalPool[*it].mDecal;
    gpRender->AddDrawable(&decal, decal.GetTranslation(),
                          CAABox(decal.GetTranslation(), decal.GetTranslation()), 2,
                          IRenderer::kDS_SortedCallback);
  }
}
