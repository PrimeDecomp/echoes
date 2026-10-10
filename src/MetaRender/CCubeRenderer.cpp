#include "MetaRender/CCubeRenderer.hpp"
#include "Collision/CollisionUtil.hpp"
#include "MetaRender/SModelRenderData.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CDrawablePlaneObject.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "WorldFormat/CAreaRenderOctTree.hpp"
#include "WorldFormat/CMetroidModelInstance.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include "dolphin/os/OSCache.h"
#include <string.h>

// Reconstructed against the G2ME01 renderer; remaining compiler differences keep this NonMatching.
CCubeRenderer* CCubeRenderer::sRenderer = nullptr;

static CModelFlags skNormalFlag = CModelFlags::Normal();
static CModelFlags skNormalFlagNoUpdate = CModelFlags::Normal().DepthCompareUpdate(true, false);

namespace Buckets {
typedef rstl::reserved_vector< CDrawable, 512 > DrawableList;
typedef rstl::reserved_vector< CDrawable*, 128 > Bucket;
typedef rstl::reserved_vector< Bucket, 50 > BucketList;
typedef rstl::reserved_vector< CDrawablePlaneObject, 8 > PlaneList;
typedef rstl::reserved_vector< ushort, 8 > PlaneBucketList;

static DrawableList* sData;
static BucketList* sBuckets;
static PlaneList* sPlaneObjectData;
static PlaneBucketList* sPlaneObjectBucket;
static rstl::reserved_vector< ushort, 50 > sBucketIndex;
static const rstl::pair< float, float > skWorstMinMaxDistance(99999.f, -99999.f);
static rstl::pair< float, float > sMinMaxDistance = skWorstMinMaxDistance;

void Shutdown();
void Init(void* workspace);
uint GetWorkspaceSize();
void Clear();
void Sort();
void Insert(const CVector3f& pos, const CAABox& bounds, EDrawableType type, const void* data,
            const CPlane& plane, ushort extraSort, bool alpha);
void InsertPlaneObject(float closeDistance, float farDistance, const CAABox& bounds,
                       bool invertTest, const CPlane& plane, bool zOnly, EDrawableType type,
                       const void* data);
} // namespace Buckets

template < bool Special, bool Alpha >
void CCubeRenderer::DrawGeometry(int areaId, const char* name, const SGeometryTag&,
                                 const SGeometryTag&, const SGeometryTag&, const SGeometryTag&,
                                 const SGeometryTag&) {
  SetupRendererStates(true);
  uchar lastAlpha = 0;
  if (Alpha) {
    gpRender->SetDestinationAlpha(0);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  }
  SetMaterialMode(mRequestedMaterialMode);

  CAreaListItem* area = nullptr;
  rstl::list< CAreaListItem >::iterator areaIt = FindArea(areaId);
  if (areaIt != mAreaListItems.end()) {
    area = &*areaIt;
  }
  if (area) {
    const rstl::vector< CMetroidModelInstance >* geometry = area->GetModelVector();
    const rstl::vector< rstl::auto_ptr< CCubeModel > >* models = area->GetModelList();
    for (int modelIndex = 0; modelIndex < geometry->size(); ++modelIndex) {
      const CMetroidModelInstance& instance = (*geometry)[modelIndex];
      const CCubeModel* model = (*models)[modelIndex].get();
      const SModelSurfaceOrder& order = area->mModelSurfaceOrders[modelIndex];
      const ushort* begin = order.mSurfaceIndices.get() + (Special ? order.mSortedEnd : 0);
      const ushort* end =
          order.mSurfaceIndices.get() + (Special ? order.mTotalCount : order.mOpaqueEnd);
      model->TryLockTextures();
      model->SetArraysCurrent();
      const ushort* records = instance.GetSurfaceRecords();
      for (int lightSet = 0; lightSet < mLightSets.size(); ++lightSet) {
        for (const ushort* surface = begin; surface != end; ++surface) {
          ushort areaIndex = records[*surface * 2 + 2];
          if (lightSet != area->mLightSetIndices[areaIndex]) {
            continue;
          }
          const CCubeSurface firstSurface(instance.GetSurfaces()[*surface]);
          ActivateLightsForModel(mLightSets[lightSet]);
          if (Alpha) {
            const uchar alpha = area->mPVSAlpha[areaIndex];
            if (alpha != lastAlpha) {
              CGX::SetDstAlpha(true, alpha);
              lastAlpha = alpha;
            }
          }
          model->DrawSurface(firstSurface, CModelFlags(CModelFlags::kT_Opaque, 1.f));
          for (++surface; surface != end; ++surface) {
            areaIndex = records[*surface * 2 + 2];
            if (lightSet == area->mLightSetIndices[areaIndex]) {
              const CCubeSurface nextSurface(instance.GetSurfaces()[*surface]);
              if (Alpha) {
                const uchar alpha = area->mPVSAlpha[areaIndex];
                if (alpha != lastAlpha) {
                  CGX::SetDstAlpha(true, alpha);
                  lastAlpha = alpha;
                }
              }
              model->DrawSurface(nextSurface, CModelFlags(CModelFlags::kT_Opaque, 1.f));
            }
          }
          break;
        }
      }
    }
  }
  if (Alpha) {
    GXSetAlphaUpdate(GX_FALSE);
  }
  SetMaterialMode(0);
  SetupCGraphicsStates();
}

uint Buckets::GetWorkspaceSize() {
  return sizeof(DrawableList) + sizeof(BucketList) + sizeof(PlaneList) + sizeof(PlaneBucketList) +
         4;
}

void Buckets::Init(void* workspace) {
  uchar* data = reinterpret_cast< uchar* >((reinterpret_cast< size_t >(workspace) + 3) & ~3);
  sData = new (data) DrawableList;
  data += sizeof(DrawableList);
  sBuckets = new (data) BucketList;
  data += sizeof(BucketList);
  sPlaneObjectData = new (data) PlaneList;
  data += sizeof(PlaneList);
  sPlaneObjectBucket = new (data) PlaneBucketList;
  sBuckets->resize(50, Bucket());
  sMinMaxDistance = skWorstMinMaxDistance;
}

void Buckets::Shutdown() {
  sData = nullptr;
  sBuckets = nullptr;
  sPlaneObjectData = nullptr;
  sPlaneObjectBucket = nullptr;
}

void Buckets::Insert(const CVector3f& pos, const CAABox& bounds, EDrawableType type,
                     const void* data, const CPlane& plane, ushort extraSort, bool alpha) {
  DrawableList& list = *sData;
  if (list.size() == list.capacity()) {
    return;
  }

  float distance = plane.GetHeight(pos);
  list.push_back(CDrawable(type, extraSort, distance, bounds, data, alpha));
  sMinMaxDistance.first = rstl::min_val(distance, sMinMaxDistance.first);
  sMinMaxDistance.second = rstl::max_val(distance, sMinMaxDistance.second);
  __dcbt(&list.back() + 1, 0);
}

void Buckets::InsertPlaneObject(float closeDistance, float farDistance, const CAABox& bounds,
                                bool invertTest, const CPlane& plane, bool zOnly,
                                EDrawableType type, const void* data) {
  PlaneList& planes = *sPlaneObjectData;
  if (planes.size() == planes.capacity()) {
    return;
  }

  planes.push_back(CDrawablePlaneObject(type, closeDistance, farDistance, bounds, invertTest, plane,
                                        zOnly, data));
}

namespace Buckets {
struct planeSorter {
  bool operator()(ushort a, ushort b) const {
    const CDrawablePlaneObject& planeA = (*sPlaneObjectData)[a];
    const CDrawablePlaneObject& planeB = (*sPlaneObjectData)[b];
    return planeA.GetDistance() < planeB.GetDistance();
  }
};

struct sorter {
  bool operator()(const CDrawable* a, const CDrawable* b) const {
    const float distanceA = a->GetDistance();
    const float distanceB = b->GetDistance();
    if (distanceA == distanceB) {
      return a->GetExtraSort() > b->GetExtraSort();
    }
    return distanceA > distanceB;
  }
};
} // namespace Buckets

void Buckets::Sort() {
  const float negativeMin = -sMinMaxDistance.first;
  const float delta = rstl::max_val(sMinMaxDistance.second - sMinMaxDistance.first, 1.f);
  float pitch = 1.f / (delta * (1.f / 49.f));
  short index = 0;
  for (CDrawablePlaneObject* plane = sPlaneObjectData->begin(); plane != sPlaneObjectData->end();
       ++plane, ++index) {
    if (sPlaneObjectBucket->size() < sPlaneObjectBucket->capacity()) {
      sPlaneObjectBucket->push_back(index);
    }
  }
  int precision = 50;
  {
    PlaneBucketList& planeBuckets = *sPlaneObjectBucket;
    if (planeBuckets.size() != 0) {
      rstl::sort(planeBuckets.begin(), planeBuckets.end(), planeSorter());
      precision = 50 / (planeBuckets.size() + 1);
      pitch = 1.f / (delta * (1.f / static_cast< float >(precision - 2)));
      short position = 0;
      for (ushort* bucket = planeBuckets.begin(); bucket != planeBuckets.end();
           ++bucket, ++position) {
        (*sPlaneObjectData)[*bucket].SetBucketIndex(precision * (position + 1));
      }
    }
  }
  PlaneBucketList& planeBuckets = *sPlaneObjectBucket;
  PlaneList& planeData = *sPlaneObjectData;
  for (CDrawable* drawable = sData->begin(); drawable != sData->end(); ++drawable) {
    int slot = -1;
    const float relativeDistance = negativeMin + drawable->GetDistance();
    if (planeBuckets.size() == 0) {
      slot = rstl::max_val(rstl::min_val(49, static_cast< int >(relativeDistance * pitch)), 1);
    } else {
      slot = rstl::max_val(
          rstl::min_val(precision - 2, static_cast< int >(relativeDistance * pitch)), 0);
      for (ushort* bucket = planeBuckets.begin(); bucket != planeBuckets.end(); ++bucket) {
        CDrawablePlaneObject& plane = planeData[*bucket];
        bool partial;
        bool full;
        if (plane.IsOptimalPlane()) {
          partial = drawable->GetBounds().GetMaxPoint().GetZ() > plane.GetPlane().GetConstant()
                        ? true
                        : false;
          full = drawable->GetBounds().GetMinPoint().GetZ() > plane.GetPlane().GetConstant()
                     ? true
                     : false;
        } else {
          partial = plane.GetPlane().GetHeight(drawable->GetBounds().ClosestPointAlongVector(
                        plane.GetPlane().GetNormal())) > 0.f
                        ? true
                        : false;
          full = plane.GetPlane().GetHeight(drawable->GetBounds().FurthestPointAlongVector(
                     plane.GetPlane().GetNormal())) > 0.f
                     ? true
                     : false;
        }
        bool continueTest;
        if (drawable->IsAlpha()) {
          continueTest = plane.IsViewInFront() ? !partial : full;
        } else if (plane.IsViewInFront()) {
          continueTest = !(partial && full);
        } else {
          continueTest = partial || full;
        }
        if (!continueTest) {
          break;
        }
        slot += precision;
      }
    }
    if (slot == -1) {
      slot = 49;
    }
    Bucket& bucket = (*sBuckets)[slot];
    if (bucket.size() < bucket.capacity()) {
      bucket.push_back(drawable);
    }
  }
  for (int i = sBuckets->size() - 1; i >= 0; --i) {
    Bucket& bucket = (*sBuckets)[i];
    sBucketIndex.push_back(i);
    if (bucket.size() != 0) {
      rstl::sort(bucket.begin(), bucket.end(), sorter());
    }
  }
  for (ushort* plane = planeBuckets.end() - 1; plane != planeBuckets.begin() - 1; --plane) {
    CDrawablePlaneObject& planeObject = planeData[*plane];
    Bucket& bucket = (*sBuckets)[planeObject.GetBucketIndex()];
    if (bucket.size() < bucket.capacity()) {
      bucket.push_back(&planeObject);
    }
  }
}

void Buckets::Clear() {
  sData->clear();
  sBucketIndex.clear();
  sPlaneObjectData->clear();
  sPlaneObjectBucket->clear();
  for (Bucket* it = sBuckets->begin(); it != sBuckets->end(); ++it) {
    it->clear();
  }
  sMinMaxDistance = skWorstMinMaxDistance;
}

CCubeRenderer::SModelSurfaceOrder::SModelSurfaceOrder(const CCubeModel& model)
: mSurfaceIndices(), mOpaqueEnd(0), mSortedEnd(0), mTotalCount(0) {
  const rstl::vector< void* >& surfaces = model.GetModelInstance().Surfaces();
  mSurfaceIndices = rs_new ushort[surfaces.size()];
  ushort* index = mSurfaceIndices.get();
  int count = 0;
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCubeSurface surface(surfaces[i]);
    const uint flags = model.GetMaterial(surface).GetFlags();
    if ((flags & 0x1010) == 0) {
      *index++ = i;
      ++count;
    }
  }
  mOpaqueEnd = count;
  index = mSurfaceIndices.get() + count;
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCubeSurface surface(surfaces[i]);
    const uint flags = model.GetMaterial(surface).GetFlags();
    if ((flags & kStateFlag_DepthSorting) != 0) {
      *index++ = i;
      ++count;
    }
  }
  mSortedEnd = count;
  index = mSurfaceIndices.get() + count;
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCubeSurface surface(surfaces[i]);
    const uint flags = model.GetMaterial(surface).GetFlags();
    if ((flags & 0x1000) != 0) {
      *index++ = i;
      ++count;
    }
  }
  mTotalCount = count;
}

CCubeRenderer::CAreaListItem::CAreaListItem(
    const rstl::vector< CMetroidModelInstance >* geometry, const CAreaRenderOctTree* octTree,
    const rstl::vector< SAreaSurface >* surfaces, const rstl::vector< uint >* ambientLightIds,
    const rstl::vector< signed char >* ambientLightIndices,
    const rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > >& textures,
    const rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > >& models, int areaId)
: mGeometry(geometry)
, mOctTree(octTree)
, mSurfaces(surfaces)
, mAmbientLightIds(ambientLightIds)
, mAmbientLightIndices(ambientLightIndices)
, mTextures(textures)
, mModels(models)
, mAreaId(areaId) {
  mPVSAlpha.resize(mSurfaces->size() - 1, 0);
  mModelSurfaceOrders.reserve(models->size());
  for (int i = 0; i < models->size(); ++i) {
    mModelSurfaceOrders.push_back_unsafe(SModelSurfaceOrder(*(*models)[i]));
  }
}

CCubeRenderer::CCubeRenderer(IObjectStore& store, COsContext& context, CMemorySys& memory,
                             IFactory& factory)
: mFactory(factory)
, mObjStore(store)
, mFont(1.f)
, mPrimVertCount(0)
, mFrustumPlanes(CTransform4f::Identity(), 1.5707964f, 1.f, 1.f, false, 100.f)
, mDrawableCallback(nullptr)
, mViewPlane(0.f, CUnitVector3f(0.f, 1.f, 0.f, CUnitVector3f::kN_Yes))
, mPVSMode(0)
, mBlackTex(kTF_RGB565, 4, 4, 1)
, mReflectionRamp(kTF_IA8, 32, 32, 1)
, mFogVolumeRamp(kTF_I8, 256, 256, 1)
, mSphereRamp(kTF_I8, 32, 32, 1)
, mAlphaMaskRamp(kTF_I4, 16, 16, 1)
, mScanRamp(kTF_I4, 8, 8, 1)
, mRandom(20)
, mReflectionAge(2)
, mPrimColor(CColor::White())
, mPrimNormal(CVector3f::Forward())
, mWorldLightColor(static_cast< uchar >(255), static_cast< uchar >(0), static_cast< uchar >(255),
                   static_cast< uchar >(255))
, mSilhouetteMaskCountdown(0)
, mBigRing(store.GetObj("TXTR_BigRing"))
, mDarkWorldCloud(store.GetObj("TXTR_DarkWorldCloud"))
, mScanSweepBar(store.GetObj("TXTR_ScanSweepBar"))
, mFlatSphere(store.GetObj("CMDL_FlatSphere"))
, mFlatSphereLow(store.GetObj("CMDL_FlatSphereLow"))
, mFlatCylinder(store.GetObj("CMDL_FlatCylinder"))
, mFlatCylinderLow(store.GetObj("CMDL_FlatCylinderLow"))
, mDarkLightWorldPalette(ClonePalette(store.GetObj("TXTR_DarkLightworldPalette")))
, mReflectionDirty(false)
, mDrawWireframe(false)
, mRequestRGBA6(false)
, mCurrentRGBA6(false)
, mPreserveDestinationAlpha(false)
, mDisableFog(false)
, mPersistRGBA6(false)
, mRenderingSilhouette(false)
, mCurrentMaterialMode(0)
, mRequestedMaterialMode(0) {
  memset(mBlackTex.Lock(), 0, 32);
  mBlackTex.UnLock();
  GenerateReflectionTex();
  GenerateFogVolumeRampTex();
  GenerateSphereRampTex();
  GenerateAlphaMaskRampTex();
  GenerateScanRampTex();
  sRenderer = this;
  Buckets::Shutdown();
}

CGraphicsPalette* CCubeRenderer::ClonePalette(const TLockedToken< CTexture >& texture) {
  const CGraphicsPalette* palette = texture->GetPalette();
  CGraphicsPalette* result =
      rs_new CGraphicsPalette(palette->GetFormat(), palette->GetEntryCount());
  void* dst = result->Lock();
  memcpy(dst, palette->GetPaletteData(), static_cast< int >(result->GetEntryCount()) * 16 / 8);
  result->UnLock();
  return result;
}

void CCubeRenderer::GenerateReflectionTex() {
  const float radius = 14.f;
  const float halfScale = 128.f;
  ushort* data = static_cast< ushort* >(mReflectionRamp.Lock());
  int texel = 0;
  for (int yBlock = 0; yBlock < 8; ++yBlock) {
    for (int xBlock = 0; xBlock < 8; ++xBlock) {
      for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
          float fx = 0.f;
          float fy = 0.f;
          CVector2f vec(static_cast< float >(xBlock * 4 + (x - 14)),
                        static_cast< float >(yBlock * 4 + (y - 14)));
          const float mag = vec.Magnitude();
          if (mag <= radius) {
            vec.Normalize();
            vec *= (radius - mag) / radius;
            fx = vec.GetX();
            fy = vec.GetY();
          }
          const float scaledX = halfScale * fx + halfScale;
          const int ix = static_cast< int >(CMath::Clamp(0.f, scaledX, 255.f));
          const float scaledY = halfScale * fy + halfScale;
          const int iy = static_cast< int >(CMath::Clamp(0.f, scaledY, 255.f));
          data[texel++] = static_cast< ushort >((iy & 0xff) | ((ix & 0xff) << 8));
        }
      }
    }
  }
  mReflectionRamp.UnLock();
}

void CCubeRenderer::GenerateFogVolumeRampTex() {
  uchar* data = static_cast< uchar* >(mFogVolumeRamp.Lock());
  memset(data, 0xff, 0x10000);
  for (int y = 0, yOff = 0; y < 2048; ++y, yOff += 32) {
    const int tileXBase = (y % 32) * 8;
    const int tileYBase = (y / 32) * 4;
    for (int x = 0; x < 32; ++x) {
      const int tileX = tileXBase + (x & 7);
      const int tileY = tileYBase + (x >> 3);
      const uint tmp = static_cast< uint >((tileY << 16) | (tileX << 8) | 0x7f);
      const double t = static_cast< double >(tmp) / 16777215.0;
      const double a = (-(150.0 / (t * (750.f - 0.2f) - 750.0)) - 0.2f) * 3.0 / (750.f - 0.2f);
      const float value = CMath::Clamp< float >(0.f, a, 1.f);
      data[yOff + x] = CCast::ToUint8(0.5f * (value * value + value) * 255.f);
    }
  }
  mFogVolumeRamp.UnLock();
}

void CCubeRenderer::GenerateSphereRampTex() {
  const int height = 32;
  const int width = 32;
  const float halfRes = (height - 1) / 2.f;
  uchar* data = static_cast< uchar* >(mSphereRamp.Lock());
  for (int y = 0, start = 0; y < height; ++y, start += width) {
    for (int x = 0; x < width; ++x) {
      float fx = static_cast< float >(((y % 4) << 3) + (x & 7));
      float fy = static_cast< float >(((y / 4) << 2) + (x >> 3));
      fx = fx / halfRes - 1.f;
      fy = fy / halfRes - 1.f;
      const float mag = CMath::SqrtF(fx * fx + fy * fy);
      const float value = CMath::Clamp(0.f, 1.f - mag * mag, 1.f);
      data[start + x] = static_cast< uchar >(value * 255.f);
    }
  }
  mSphereRamp.UnLock();
}

void CCubeRenderer::GenerateAlphaMaskRampTex() {
  uchar* data = static_cast< uchar* >(mAlphaMaskRamp.Lock());
  for (uint y = 0; y < 16; ++y) {
    const uchar value = y < 8 ? 0 : 0xff;
    for (int x = 0; x < 8; ++x) {
      data[y * 8 + x] = value;
    }
  }
  mAlphaMaskRamp.UnLock();
}

void CCubeRenderer::GenerateScanRampTex() {
  uchar* data = static_cast< uchar* >(mScanRamp.Lock());
  memset(data, 0xff, 8);
  memset(data + 8, 0, 8);
  memset(data + 16, 0xff, 8);
  memset(data + 24, 0, 8);
  DCFlushRange(data, 32);
  mScanRamp.UnLock();
}

CCubeRenderer::~CCubeRenderer() {
  sRenderer = nullptr;
  Buckets::Shutdown();
  if (mSilhouetteMask.get()) {
    mSilhouetteMask->ScheduleDeletion();
  }
}

void CCubeRenderer::AddStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry,
                                      const CAreaRenderOctTree* octTree,
                                      const rstl::vector< SAreaSurface >* surfaces,
                                      const rstl::vector< uint >* ambientLightIds,
                                      const rstl::vector< signed char >* ambientLightIndices,
                                      int areaId) {
  if (FindStaticGeometry(geometry) == mAreaListItems.end()) {
    rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > > models =
        rs_new rstl::vector< rstl::auto_ptr< CCubeModel > >();
    rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > > textures =
        rs_new rstl::vector< TCachedToken< CTexture > >();
    if (!geometry->empty()) {
      CCubeModel::MakeTexturesFromMats(geometry->front().GetMaterialPointer(), *textures, mObjStore,
                                       false);
      models->reserve(geometry->size());
      for (int i = 0; i < geometry->size(); ++i) {
        const CMetroidModelInstance& instance = (*geometry)[i];
        models->push_back(rs_new CCubeModel(
            const_cast< rstl::vector< void* >* >(&instance.GetSurfaces()), textures.get(),
            instance.GetMaterialPointer(), instance.GetVertexPointer(), instance.GetNormalPointer(),
            instance.GetColorPointer(), instance.GetTCPointer(), instance.GetPackedTCPointer(),
            instance.GetBoundingBox(), instance.GetFlags(), false, i));
      }
    }
    mAreaListItems.push_back(CAreaListItem(geometry, octTree, surfaces, ambientLightIds,
                                           ambientLightIndices, textures, models, areaId));
    GXInvalidateVtxCache();
  }
}

rstl::list< CCubeRenderer::CAreaListItem >::iterator
CCubeRenderer::FindStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) {
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    if (area->mGeometry == geometry) {
      return area;
    }
  }
  return mAreaListItems.end();
}

void CCubeRenderer::RemoveStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) {
  rstl::list< CAreaListItem >::iterator area = FindStaticGeometry(geometry);
  if (area != mAreaListItems.end()) {
    mAreaListItems.erase(area);
  }
}

void CCubeRenderer::SetModelMatrix(const CTransform4f& xf) { CGraphics::SetModelMatrix(xf); }

void CCubeRenderer::SetWorldViewpoint(const CTransform4f& xf) {
  CGraphics::SetViewPointMatrix(xf);
  const CVector3f normal = xf.GetForward();
  mViewPlane.SetFrom(CVector3f::Dot(normal, xf.GetTranslation()), normal);
}

void CCubeRenderer::BeginScene() {
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;
  CGraphics::SetUseVideoFilter(true);
  CGraphics::SetViewport(0, 0, width, height);
  CGraphics::SetClearColor(CColor(static_cast< uchar >(0), 0, 0, 0));
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetPerspective(75.f, 1.3333334f, 1.f, 4096.f);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::TickRenderTimings();
  if (mSilhouetteMaskCountdown != 0) {
    --mSilhouetteMaskCountdown;
    if (mSilhouetteMaskCountdown == 0) {
      mSilhouetteMask->ScheduleDeletion();
      mSilhouetteMask = nullptr;
    }
  }
  mCurrentRGBA6 = mRequestRGBA6;
  if (!mPersistRGBA6) {
    mRequestRGBA6 = false;
  }
  GXSetPixelFmt(mCurrentRGBA6 ? GX_PF_RGBA6_Z24 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  if (mPreserveDestinationAlpha) {
    mPreserveDestinationAlpha = false;
  } else {
    GXSetAlphaUpdate(GX_TRUE);
  }
  CGX::SetDstAlpha(true, 0);
  CGraphics::BeginScene();
}

void CCubeRenderer::EndScene() {
  mPersistRGBA6 = !CGraphics::IsBeginSceneClearFb();
  CGraphics::EndScene();
  if (mReflectionAge >= 2) {
    mReflectionTex = nullptr;
  } else {
    ++mReflectionAge;
  }
  CGraphics::SetClearColor(CColor(0));
}

void CCubeRenderer::AddParticleGen(const CParticleGen& gen) {
  const rstl::optional_object< CAABox > bounds = const_cast< CParticleGen& >(gen).GetBounds();
  if (bounds) {
    const CVector3f closest = bounds->ClosestPointAlongVector(mViewPlane.GetNormal());
    Buckets::Insert(closest, *bounds, kDT_Particle, &gen, mViewPlane, 0, true);
  }
}

void CCubeRenderer::AddParticleGen(const CParticleGen& gen, const CVector3f& pos,
                                   const CAABox& bounds) {
  Buckets::Insert(pos, bounds, kDT_Particle, &gen, mViewPlane, 0, true);
}

void CCubeRenderer::AddPlaneObject(const void* obj, const CAABox& bounds, const CPlane& plane,
                                   int type) {
  static const CVector3f sOptimalPlane(0.f, 0.f, 1.f);
  CVector3f closestPoint = bounds.ClosestPointAlongVector(mViewPlane.GetNormal());
  float closestDist = mViewPlane.GetHeight(closestPoint);
  CVector3f furthestPoint = bounds.FurthestPointAlongVector(mViewPlane.GetNormal());
  float furthestDist = mViewPlane.GetHeight(furthestPoint);
  if (closestDist < 0.f && furthestDist < 0.f) {
    return;
  }

  bool zOnly;
  if (plane.GetNormal() == sOptimalPlane) {
    zOnly = true;
  } else {
    zOnly = false;
  }

  bool invertTest;
  if (zOnly) {
    if (CGraphics::GetViewMatrix().GetTranslation().GetZ() >= plane.GetConstant()) {
      invertTest = true;
    } else {
      invertTest = false;
    }
  } else if (plane.GetHeight(CGraphics::GetViewMatrix().GetTranslation()) >= 0.f) {
    invertTest = true;
  } else {
    invertTest = false;
  }

  Buckets::InsertPlaneObject(closestDist, furthestDist, bounds, invertTest, plane, zOnly,
                             EDrawableType(type + 2), obj);
}

void CCubeRenderer::AddDrawable(const void* obj, const CVector3f& pos, const CAABox& bounds,
                                int mode, EDrawableSorting sorting) {
  if (sorting == kDS_UnsortedCallback) {
    mDrawableCallback(obj, mDrawableCallbackUserData, mode);
  } else {
    Buckets::Insert(pos, bounds, static_cast< EDrawableType >(mode + kDT_Actor), obj, mViewPlane, 0,
                    sorting == kDS_AlphaSortedCallback);
  }
}

void CCubeRenderer::SetupRendererStates(const bool depthWrite) {
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetAmbientColor(CColor(0));
  CGraphics::SetDepthWriteMode(true, kE_LEqual, depthWrite);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_Or, kAF_Always, 0);
  CCubeMaterial::ResetCachedMaterials();
  GXSetTevColor(GX_TEVREG1, mWorldLightColor.GetGXColor());
}

void CCubeRenderer::SetupCGraphicsStates() {
  const GXColor white = {255, 255, 255, 255};
  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CTevCombiners::ResetStates();
  CGraphics::SetAmbientColor(CColor(0.4f, 0.4f, 0.4f, 1.f));
  CGX::SetChanMatColor(CGX::Channel0, white);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CCubeMaterial::EnsureTevsDirect();
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_Or, kAF_Always, 0);
}

void CCubeRenderer::AddWorldSurface(short modelIndex, ushort surfaceIndex, uint blend,
                                    const CAABox& bounds) {
  const ushort extraSort = blend == 0x50004;
  const uint surface = (static_cast< uint >(modelIndex) << 16) | surfaceIndex;
  const CVector3f& closest = bounds.ClosestPointAlongVector(mViewPlane.GetNormal());
  Buckets::Insert(closest, bounds, kDT_WorldSurface, reinterpret_cast< const void* >(surface),
                  mViewPlane, extraSort, false);
}

void CCubeRenderer::DrawRenderBucketsDebug() {}

void CCubeRenderer::RenderBucketItems(const CAreaListItem* area, bool alpha) {
  const rstl::reserved_vector< ushort, 50 >& bucketIndices = Buckets::sBucketIndex;
  const Buckets::BucketList& buckets = *Buckets::sBuckets;
  int lastType = -1;
  const CCubeModel* currentModel = nullptr;
  uchar lastLightSet = 255;
  if (alpha) {
    GXSetAlphaUpdate(GX_FALSE);
  }
  for (rstl::reserved_vector< ushort, 50 >::const_iterator bucketIt = bucketIndices.begin();
       bucketIt != bucketIndices.end(); ++bucketIt) {
    const Buckets::Bucket& bucket = buckets[*bucketIt];
    for (Buckets::Bucket::const_iterator it = bucket.begin(); it != bucket.end(); ++it) {
      const CDrawable* drawable = *it;
      const int type = drawable->GetType();
      if (lastType == kDT_WorldSurface && type != kDT_WorldSurface) {
        SetupCGraphicsStates();
        SetMaterialMode(0);
        if (alpha) {
          CGX::SetDstAlpha(false, 0);
          GXSetAlphaUpdate(GX_FALSE);
        }
      }
      switch (type) {
      case kDT_Particle:
        const_cast< CParticleGen* >(static_cast< const CParticleGen* >(drawable->GetData()))
            ->Render();
        break;
      case kDT_WorldSurface: {
        if (lastType != type) {
          SetupRendererStates(false);
          currentModel = nullptr;
          SetMaterialMode(mRequestedMaterialMode);
          lastLightSet = 255;
        }
        const uint packed = reinterpret_cast< uint >(drawable->GetData());
        const short modelIndex = packed >> 16;
        const ushort surfaceIndex = packed;
        const CMetroidModelInstance& instance = (*area->mGeometry)[modelIndex];
        const CCubeSurface surface(instance.GetSurfaces()[surfaceIndex]);
        const CCubeModel* model = surface.GetParent();
        if (model != currentModel) {
          model->SetArraysCurrent();
          currentModel = model;
        }
        const ushort areaIndex = instance.GetSurfaceAreaIndex(surfaceIndex);
        const uchar lightSet = area->mLightSetIndices[areaIndex];
        if (lightSet != lastLightSet) {
          ActivateLightsForModel(mLightSets[lightSet]);
          lastLightSet = lightSet;
        }
        if (alpha) {
          const uchar destinationAlpha = area->mPVSAlpha[areaIndex];
          const bool enabled = destinationAlpha != 0;
          CGX::SetDstAlpha(enabled, destinationAlpha);
          GXSetAlphaUpdate(enabled);
        }
        model->DrawSurface(surface, skNormalFlagNoUpdate);
        break;
      }
      default:
        if (mDrawableCallback != nullptr) {
          mDrawableCallback(drawable->GetData(), mDrawableCallbackUserData,
                            drawable->GetType() - kDT_Actor);
        }
        break;
      }
      lastType = type;
    }
  }
  if (alpha) {
    CGX::SetDstAlpha(false, 0);
    GXSetAlphaUpdate(GX_FALSE);
  }
  SetMaterialMode(0);
}

void CCubeRenderer::DrawSortedGeometry(int mode, int areaId) {
  SetupRendererStates(true);
  const CAreaListItem* area = nullptr;
  rstl::list< CAreaListItem >::iterator it = FindArea(areaId);
  if (it != mAreaListItems.end()) {
    area = &*it;
  }
  if (area != nullptr) {
    const rstl::vector< CMetroidModelInstance >* geometry = area->GetModelVector();
    const rstl::vector< rstl::auto_ptr< CCubeModel > >* models = area->GetModelList();
    for (int modelIndex = 0; modelIndex < geometry->size(); ++modelIndex) {
      const CMetroidModelInstance& instance = (*geometry)[modelIndex];
      const CCubeModel* model = (*models)[modelIndex].get();
      const SModelSurfaceOrder& order = area->mModelSurfaceOrders[modelIndex];
      const ushort* begin = order.mSurfaceIndices.get() + order.mOpaqueEnd;
      const ushort* end = order.mSurfaceIndices.get() + order.mSortedEnd;
      const ushort* records = instance.GetSurfaceRecords();
      const rstl::vector< void* >& surfaces = model->GetModelInstance().Surfaces();
      for (const ushort* index = begin; index != end; ++index) {
        const uint surfaceIndex = *index;
        if (area->mLightSetIndices[records[surfaceIndex * 2 + 2]] != 255) {
          const CCubeSurface surface(surfaces[surfaceIndex]);
          const CCubeMaterial material = model->GetMaterial(surface);
          AddWorldSurface(modelIndex, surfaceIndex, material.GetCompressedBlend(),
                          surface.GetBounds());
        }
      }
    }
  }
  Buckets::Sort();
  RenderBucketItems(area, mode == 1);
  SetupCGraphicsStates();
  DrawRenderBucketsDebug();
  Buckets::Clear();
}

void CCubeRenderer::EvaluateModelLights(uchar* lights, const CAABox& bounds, const uint* overlaps,
                                        int wordCount, uint modelIndex) {
  lights[0] = lights[1] = lights[2] = lights[3] = 63;
  if (mDynamicLights.empty()) {
    return;
  }
  int ids[4];
  float distances[4] = {-1.f, -1.f, -1.f, -1.f};
  int count = 0;
  for (int i = 0; i < mDynamicLights.size() && count < 4; ++i, overlaps += wordCount) {
    const CLight& light = mDynamicLights[i];
    if (light.GetType() == kLT_Hard || !CAreaRenderOctTree::TestBit(overlaps, modelIndex)) {
      continue;
    }
    const CSphere sphere(light.GetPosition(), light.GetRadius());
    const float distance = CollisionUtil::AABoxSphereIntersectionRadius(bounds, sphere);
    bool replaced = false;
    for (int j = 0; j < count; ++j) {
      if (ids[j] == light.GetId() && light.GetId() != 0) {
        if (distance >= 0.f && distance < distances[j]) {
          distances[j] = distance;
          replaced = true;
          lights[j] = i;
        }
        break;
      }
    }
    if (!replaced) {
      distances[count] = distance;
      if (distances[count] >= 0.f) {
        lights[count] = i;
        ids[count] = light.GetId();
        ++count;
      }
    }
  }
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      if (lights[j] < lights[i]) {
        const uchar index = lights[i];
        lights[i] = lights[j];
        lights[j] = index;
      }
    }
  }
}

IRenderer* AllocateRenderer(IObjectStore& store, COsContext& context, CMemorySys& memory,
                            IFactory& factory) {
  CCubeRenderer* renderer = rs_new CCubeRenderer(store, context, memory, factory);
  IWeaponRenderer::SetRenderer(renderer);
  return renderer;
}

void CCubeRenderer::PrimColor(float r, float g, float b, float a) { mPrimColor.Set(r, g, b, a); }

void CCubeRenderer::PrimColor(const CColor& color) { mPrimColor = color; }

void CCubeRenderer::BeginPrimitive(EPrimitiveType primitive, int count) {
  const GXVtxDescList desc[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_NRM, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  mPrimVertCount = count;
  CGX::SetVtxDescv(desc);
  CGX::Begin(GXPrimitive(primitive), GX_VTXFMT0, count);
}

void CCubeRenderer::BeginLines(int count) { CCubeRenderer::BeginPrimitive(kPT_Lines, count); }

void CCubeRenderer::BeginLineStrip(int count) {
  CCubeRenderer::BeginPrimitive(kPT_LineStrip, count);
}

void CCubeRenderer::BeginTriangles(int count) {
  CCubeRenderer::BeginPrimitive(kPT_Triangles, count);
}

void CCubeRenderer::BeginTriangleStrip(int count) {
  CCubeRenderer::BeginPrimitive(kPT_TriangleStrip, count);
}

void CCubeRenderer::BeginTriangleFan(int count) {
  CCubeRenderer::BeginPrimitive(kPT_TriangleFan, count);
}

void CCubeRenderer::PrimVertex(const CVector3f& vertex) {
  --mPrimVertCount;
  GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
  GXNormal3f32(mPrimNormal.GetX(), mPrimNormal.GetY(), mPrimNormal.GetZ());
  GXColor1u32(mPrimColor.GetColor_u32());
}

void CCubeRenderer::PrimNormal(const CVector3f& normal) { mPrimNormal = normal; }

void CCubeRenderer::EndPrimitive() {
  while (mPrimVertCount != 0) {
    PrimVertex(CVector3f::Zero());
  }
  CGX::End();
}

void CCubeRenderer::SetAmbientColor(const CColor& color) { CGraphics::SetAmbientColor(color); }

void CCubeRenderer::SetPerspective(float fovy, float width, float height, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, (width / height) * CGraphics::GetPixelAspectRatio(), znear, zfar);
}

void CCubeRenderer::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, aspect, znear, zfar);
}

rstl::pair< CVector2f, CVector2f > CCubeRenderer::SetViewportOrtho(bool centered, float znear,
                                                                   float zfar) {
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;
  const float left = centered ? static_cast< float >(-(width / 2)) : 0.f;
  const float top = centered ? static_cast< float >(-(height / 2)) : 0.f;
  const float right = static_cast< float >(centered ? width / 2 : width);
  const float bottom = static_cast< float >(centered ? height / 2 : height);
  CGraphics::SetOrtho(left, right, bottom, top, znear, zfar);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  return rstl::pair< CVector2f, CVector2f >(CVector2f(left, top), CVector2f(right, bottom));
}

void CCubeRenderer::SetViewport(int left, int top, int width, int height) {
  CGraphics::SetViewport(left, top, width, height);
  CGraphics::SetScissor(left, top, width, height);
}

void CCubeRenderer::SetDepthReadWrite(bool read, bool update) {
  CGraphics::SetDepthWriteMode(read, kE_LEqual, update);
}

void CCubeRenderer::SetBlendMode_AdditiveAlpha() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_AlphaBlended() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_NoColorWrite() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_Zero, kBF_One, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_ColorMultiply() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_Zero, kBF_SrcColor, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_InvertDst() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_InvDstColor, kBF_Zero, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_InvertSrc() {
  CGraphics::SetBlendMode(kBM_Logic, kBF_One, kBF_Zero, kLO_InvCopy);
}

void CCubeRenderer::SetBlendMode_Replace() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
}

void CCubeRenderer::SetBlendMode_AdditiveDestColor() {
  CGraphics::SetBlendMode(kBM_Blend, kBF_DstColor, kBF_One, kLO_Clear);
}

float CCubeRenderer::GetFPS() { return CGraphics::GetFPS(); }

void CCubeRenderer::SetDrawableCallback(TDrawableCallback callback, const void* context) {
  mDrawableCallback = callback;
  mDrawableCallbackUserData = context;
}

void CCubeRenderer::SetDebugOption(EDebugOption option, int value) {
  switch (option) {
  case kDO_PVSMode:
    mPVSMode = value != 0;
    break;
  case kDO_PVSState:
    mPVSState = value;
    break;
  case kDO_FogDisabled:
    mDisableFog = value != 0;
    break;
  }
}

CTexture* CCubeRenderer::GetRealReflection() {
  mReflectionAge = 0;
  if (mReflectionTex.null()) {
    return &mBlackTex;
  }
  return mReflectionTex.get();
}

void CCubeRenderer::CacheReflection(void (*callback)(void*, const CVector3f&), void* context,
                                    bool clear) {
  if (mReflectionDirty) {
    mReflectionDirty = false;
    mReflectionAge = 0;
    if (!mReflectionTex.get()) {
      mReflectionTex = rs_new CTexture(kTF_RGB565, 128, 128, 1);
    }
    const CViewport& viewport = CGraphics::GetViewport();
    const int left = viewport.mLeft;
    const int top = viewport.mTop;
    const int width = viewport.mWidth;
    const int height = viewport.mHeight;
    const int captureTop = CGraphics::GetRenderMode().efbHeight - 256;
    CGraphics::SetViewport(0, captureTop, 256, 256);
    CGraphics::SetScissor(0, captureTop, 256, 256);
    void* buffer = CGraphics::GetDolphinSpareBuffer();
    GXSetTexCopySrc(0, 0, 256, 256);
    GXSetTexCopyDst(128, 128, GX_TF_RGB565, true);
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXCopyTex(buffer, true);
    callback(context, CCubeMaterial::GetViewingReflection());
    void* reflection = const_cast< void* >(mReflectionTex->GetConstBitMapData(0));
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXCopyTex(reflection, clear);
    CGraphics::SetViewport(left, top, width, height);
    CGraphics::SetScissor(left, top, width, height);
  }
}

void CCubeRenderer::DrawSpaceWarp(const CVector3f& point, float strength) {
  if (point.GetZ() >= 1.f) {
    return;
  }
  _DrawSpaceWarp(point, strength);
}

void CCubeRenderer::_DrawSpaceWarp(const CVector3f& point, float strength) {
  const int vpLeft = CGraphics::GetViewport().mLeft;
  const int vpTop = CGraphics::GetViewport().mTop;
  const int vpWidth = CGraphics::GetViewport().mWidth;
  const int vpHeight = CGraphics::GetViewport().mHeight;

  CVector3f projectedPoint = point;
  float& screenY = projectedPoint[1];
  float& screenX = projectedPoint[0];
  screenX = static_cast< float >(vpLeft) +
            (static_cast< float >(vpWidth / 2) * screenX + static_cast< float >(vpWidth / 2));
  screenY = static_cast< float >(vpTop) +
            (static_cast< float >(vpHeight / 2) * -screenY + static_cast< float >(vpHeight / 2));

  CVector2i center(static_cast< int >(screenX) & ~3, static_cast< int >(screenY) & ~3);
  CVector2i minPoint = center - CVector2i(0x60, 0x60);
  CVector2i maxPoint = center + CVector2i(0x60, 0x60);

  int& minX = minPoint[0];
  int& maxX = maxPoint[0];
  CVector2f uv1min(0.f, 0.f);
  float& uMin = uv1min[0];
  CVector2f uv1max(1.f, 1.f);
  float& uMax = uv1max[0];

  const int alignedLeft = vpLeft & ~3;
  const int alignedTop = vpTop & ~3;
  const int alignedRight = (3 + (vpLeft + vpWidth)) & ~3;
  const int alignedBottom = (3 + (vpTop + vpHeight)) & ~3;

  if (minX < alignedLeft) {
    uv1min[0] = 0.0052083335f * static_cast< float >(alignedLeft - minPoint.GetX());
    minPoint.SetX(alignedLeft);
  }

  if (minPoint[1] < alignedTop) {
    uv1min[1] = 0.0052083335f * static_cast< float >(alignedTop - minPoint.GetY());
    minPoint.SetY(alignedTop);
  }

  if (maxX > alignedRight) {
    uv1max[0] = 1.f - 0.0052083335f * static_cast< float >(maxPoint.GetX() - alignedRight);
    maxPoint.SetX(alignedRight);
  }

  if (maxPoint[1] > alignedBottom) {
    uv1max[1] = 1.f - 0.0052083335f * static_cast< float >(maxPoint.GetY() - alignedBottom);
    maxPoint.SetY(alignedBottom);
  }

  CVector2i dimensions = maxPoint - minPoint;
  int& sizeX = dimensions[0];
  int& sizeY = dimensions[1];
  if (sizeX <= 0 || sizeY <= 0) {
    return;
  }

  GXFogType fogType;
  float fogStartZ;
  float fogEndZ;
  float fogNearZ;
  float fogFarZ;
  GXColor fogColor;

  CGX::GetFog(&fogType, &fogStartZ, &fogEndZ, &fogNearZ, &fogFarZ, &fogColor);
  CGX::SetFog(GX_FOG_NONE, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);

  void* spareBuffer = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopySrc(minPoint.GetX(), minPoint.GetY(), sizeX, sizeY);
  GXSetTexCopyDst(sizeX, sizeY, GX_TF_RGBA8, false);
  GXCopyTex(spareBuffer, false);
  GXPixModeSync();

  CGraphics::LoadDolphinSpareTexture(sizeX, sizeY, GX_TF_RGBA8, 0, CGraphics::kSpareBufferTexMapID);

  mReflectionRamp.Load(GX_TEXMAP1, CTexture::kCM_Clamp);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);

  const float indScale = static_cast< float >(0.5 * strength);
  float indMtx[2][3] = {
      {indScale, 0.f, 0.f},
      {0.f, indScale, 0.f},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, -1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(2);
  CGX::SetNumChans(0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

  static const GXVtxDescList vtxDescrs[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescrs);

  CGraphics::CProjectionState backupProj(CGraphics::GetProjectionState());
  CTransform4f backupViewMtx(CGraphics::GetViewMatrix());

  CGraphics::SetOrtho(static_cast< float >(vpLeft), static_cast< float >(vpWidth + vpLeft),
                      static_cast< float >(vpTop), static_cast< float >(vpHeight + vpTop), -4096.f,
                      4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  CGX::SetZMode(false, GX_ALWAYS, false);
  GXSetCullMode(GX_CULL_NONE);

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

  GXPosition3f32(static_cast< float >(minPoint.GetX()), 0.5f,
                 static_cast< float >(minPoint.GetY()));
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(uMin, uv1min[1]);

  GXPosition3f32(static_cast< float >(minPoint.GetX()), 0.5f,
                 static_cast< float >(maxPoint.GetY()));
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(uMin, uv1max[1]);

  GXPosition3f32(static_cast< float >(maxPoint.GetX()), 0.5f,
                 static_cast< float >(maxPoint.GetY()));
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(uMax, uv1max[1]);

  GXPosition3f32(static_cast< float >(maxPoint.GetX()), 0.5f,
                 static_cast< float >(minPoint.GetY()));
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(uMax, uv1min[1]);

  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);

  CGraphics::SetProjectionState(backupProj);
  CGraphics::SetViewPointMatrix(backupViewMtx);

  CGX::SetFog(fogType, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor);
}

void CCubeRenderer::SetWireframeFlags(int flags) {
  CCubeModel::SetModelWireframe((flags & 1) != 0);
  mDrawWireframe = (flags & 2) != 0;
}

void CCubeRenderer::SetWorldFog(ERglFogMode mode, float start, float end, const CColor& color) {
  if (mDisableFog) {
    mode = static_cast< ERglFogMode >(0);
  }
  CGraphics::SetFog(mode, start, end, color);
}

int CCubeRenderer::GetStaticWorldDataSize() {
  int size = 0;
  rstl::list< CAreaListItem >::const_iterator it = mAreaListItems.begin();
  rstl::list< CAreaListItem >::const_iterator end = mAreaListItems.end();
  for (; it != end; ++it) {
    const rstl::vector< TCachedToken< CTexture > >* const textures = it->mTextures.get();
    if (textures) {
      size += textures->size() * sizeof(TCachedToken< CTexture >);
    }
  }
  return size;
}

void CCubeRenderer::DrawFogFan(const CVector3f* vertices, int count) {
  if (count < 3) {
    return;
  }
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast< ushort >(count));
  for (int i = 0; i < count; ++i) {
    const CVector3f& v = vertices[i];
    GXPosition3f32(v.GetX(), v.GetY(), v.GetZ());
  }
  CGX::End();
}

void CCubeRenderer::DrawFogFans(const CPlane* planes, int planeCount, const CVector3f* vertices,
                                int vertexCount, int front, int back) {
  if (back == front) {
    DrawFogFans(planes, planeCount, vertices, vertexCount, front, back + 1);
  } else if (back == planeCount) {
    DrawFogFan(vertices, vertexCount);
  } else {
    rstl::reserved_vector< CVector3f, 20 > clippedVertices;
    rstl::reserved_vector< bool, 20 > clippedFlags;
    const CPlane& plane = planes[back];
    for (int i = 0; i < vertexCount; ++i) {
      clippedFlags.push_back(!plane.IsFacing(vertices[i]));
    }
    for (int i = 0; i < vertexCount; ++i) {
      const int next = i != vertexCount - 1 ? i + 1 : 0;
      const int clippedMask = clippedFlags[i] | (clippedFlags[next] << 1);
      if ((clippedMask & 1) == 0) {
        clippedVertices.push_back(vertices[i]);
      }
      if (clippedMask == 1 || clippedMask == 2) {
        const float t = plane.ClipLineSegment(vertices[i], vertices[next]);
        if (t > 0.f && t < 1.f) {
          clippedVertices.push_back(CVector3f::Lerp(vertices[i], vertices[next], t));
        }
      }
    }
    if (clippedVertices.size() >= 3) {
      DrawFogFans(planes, planeCount, clippedVertices.data(), clippedVertices.size(), front,
                  back + 1);
    }
  }
}

void CCubeRenderer::DrawFogSlices(const CPlane* planes, int planeCount, int planeIndex,
                                  const CVector3f& center, float extent) {
  static const int edges[3][2] = {{1, 2}, {0, 2}, {0, 1}};
  const CPlane& plane = planes[planeIndex];
  rstl::reserved_vector< CVector3d, 4 > doubleCorners;
  rstl::reserved_vector< CVector3f, 4 > corners;
  int axis = 0;
  if (fabs(plane.GetNormal().GetY()) > fabs(plane.GetNormal().GetX())) {
    axis = 1;
  }
  if (fabs(plane.GetNormal().GetZ()) > fabs(plane.GetNormal()[axis])) {
    axis = 2;
  }
  const CVector3d projectedCenter(center - plane.GetHeight(center) * plane.GetNormal());
  float axisSign = plane.GetNormal()[axis] < 0.f ? 1.f : -1.f;
  if (axis == 1) {
    axisSign = -axisSign;
  }
  CVector3d offsetA(0.0, 0.0, 0.0);
  CVector3d offsetB(0.0, 0.0, 0.0);
  offsetA[edges[axis][0]] = extent;
  offsetB[edges[axis][1]] = extent * axisSign;
  doubleCorners.push_back(projectedCenter - offsetA - offsetB);
  doubleCorners.push_back(projectedCenter + offsetA - offsetB);
  doubleCorners.push_back(projectedCenter + offsetA + offsetB);
  doubleCorners.push_back(projectedCenter - offsetA + offsetB);
  for (int i = 0; i < 4; ++i) {
    const CVector3d normal(plane.GetNormal());
    const CVector3d& corner = doubleCorners[i];
    const double height = corner.GetX() * normal.GetX() + corner.GetY() * normal.GetY() +
                          corner.GetZ() * normal.GetZ() - plane.GetConstant();
    const CVector3d projected = corner - height * normal;
    corners.push_back(CVector3f(projected.GetX(), projected.GetY(), projected.GetZ()));
  }
  DrawFogFans(planes, planeCount, corners.data(), doubleCorners.size(), planeIndex, 0);
}

void CCubeRenderer::RenderFogVolumeModel(const CAABox& bounds, const CModel* model,
                                         const CTransform4f& modelView, CTransform4f view,
                                         const CSkinnedModel* skinnedModel) {
  if (model == nullptr && skinnedModel == nullptr) {
    const CAABox transformedBounds = bounds.GetTransformedAABox(modelView);
    const CAABox worldBounds = transformedBounds;
    static const GXVtxDescList desc[] = {{GX_VA_POS, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
    CGX::SetVtxDescv(desc);
    const CUnitVector3f forward(view.GetForward());
    const CVector3f& min = worldBounds.GetMinPoint();
    const CVector3f& max = worldBounds.GetMaxPoint();
    const CPlane planes[7] = {
        CPlane(min.GetX(), CVector3f::Right()),
        CPlane(-max.GetX(), CVector3f::Left()),
        CPlane(min.GetY(), CVector3f::Forward()),
        CPlane(-max.GetY(), CVector3f::Back()),
        CPlane(min.GetZ(), CVector3f::Up()),
        CPlane(-max.GetZ(), CVector3f::Down()),
        CPlane(CVector3f::Dot(view.GetTranslation(), forward) + 0.2f + 0.1f, forward),
    };
    CGraphics::SetModelMatrix(CTransform4f::Identity());
    const CVector3f dimensions = worldBounds.GetMaxPoint() - worldBounds.GetMinPoint();
    const float maxExtent =
        rstl::max_val(rstl::max_val(dimensions.GetZ(), dimensions.GetY()), dimensions.GetX());
    const float sliceExtent = maxExtent * 2.f;
    for (int i = 0; i < 7; ++i) {
      DrawFogSlices(planes, 7, i, worldBounds.GetCenterPoint(), sliceExtent);
    }
  } else if (skinnedModel == nullptr) {
    model->Touch(0);
    model->DolphinDrawFlat(CModel::kDF_All);
  }
}

void CCubeRenderer::RenderFogVolume(const CColor& color, const CAABox& bounds,
                                    const TLockedToken< CModel >* model,
                                    const CSkinnedModel* skinnedModel) {
  if (!mDisableFog) {
    mFogVolumes.push_back(CFogVolumeListItem(CGraphics::GetModelMatrix(), CColor(color), bounds,
                                             model, skinnedModel));
  }
}

void CCubeRenderer::ReallyRenderFogVolume(const CColor& color, const CAABox& bounds,
                                          const CModel* model, const CSkinnedModel* skinnedModel) {
  static const int skEdges[12][2] = {
      {0, 1}, {1, 3}, {3, 2}, {2, 0}, {4, 5}, {5, 7},
      {7, 6}, {6, 4}, {0, 4}, {1, 5}, {3, 7}, {2, 6},
  };
  static const GXVtxDescList vtxDescrs[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };

  const int copyWidth = CGraphics::GetRenderMode().fbWidth / 2;
  const int copyHeight = CGraphics::GetRenderMode().xfbHeight / 2;
  void* copyBufA = CGraphics::GetDolphinSpareBuffer();
  void* copyBufB = static_cast< uchar* >(copyBufA) + copyWidth * copyHeight * 2;

  const int vpLeft = CGraphics::GetViewport().mLeft;
  const int vpTop = CGraphics::GetViewport().mTop;
  const int vpWidth = CGraphics::GetViewport().mWidth;
  const int vpHeight = CGraphics::GetViewport().mHeight;

  int maxChunkW = copyWidth;
  int maxChunkH = copyHeight;
  int chunkH = maxChunkH;
  int chunkW = maxChunkW;
  int drawLeft = 0;
  int drawTop = 0;
  bool recalcChunk = true;

  float uv0MinX = 0.f;
  float uv0MinY = 0.f;
  float uv0MaxX = 0.f;
  float uv0MaxY = 0.f;

  const CTransform4f modelXf(CGraphics::GetModelMatrix());
  const CTransform4f viewXf(CGraphics::GetViewMatrix());
  const CMatrix4f projMtx = CGraphics::GetPerspectiveProjectionMatrix();

  CVector2i minBounds(vpWidth, vpHeight);
  CVector2i maxBounds(0, 0);

  int& minY = minBounds[1];
  bool allOutside = true;
  rstl::reserved_vector< CVector3f, 8 > clipPts;
  rstl::reserved_vector< float, 8 > clipWs;
  for (int i = 0; i < 8; ++i) {
    const CVector3f worldPoint = modelXf * bounds.GetPoint(i);

    const CVector3f rotated = viewXf.TransposeRotate(CVector3f(worldPoint.GetX() - viewXf.Get03(),
                                                               worldPoint.GetY() - viewXf.Get13(),
                                                               worldPoint.GetZ() - viewXf.Get23()));

    const CVector3f projected = projMtx * rotated;
    clipPts.push_back(projected);
    clipWs.push_back(projMtx.MultiplyGetW(rotated));
  }

  for (int i = 0; i < 20; ++i) {
    CVector3f ndc;

    if (i < 8) {
      ndc = clipPts[i] / clipWs[i];
    } else {
      const int edge = i - 8;
      const int idxA = skEdges[edge][0];
      const CVector3f pA = clipPts[idxA];
      const int idxB = skEdges[edge][1];

      const float wA = clipWs[idxA];
      const float wB = clipWs[idxB];
      const CVector3f pB = clipPts[idxB];

      if ((pA.GetZ() / wA > 1.f) != (pB.GetZ() / wB > 1.f)) {
        const float t = -(wA - 1.f) / (wB - wA);
        if (t > 0.f && t < 1.f) {
          const float invW = 1.f / (wA + t * (wB - wA));
          ndc = invW * (pA + t * (pB - pA));
        } else {
          continue;
        }
      } else {
        continue;
      }
    }

    if (!(ndc.GetZ() <= 1.001f)) {
      continue;
    }

    const int scrX = static_cast< int >(static_cast< float >(vpWidth) * ndc.GetX() * 0.5f +
                                        static_cast< float >(vpWidth / 2));
    const int scrY = static_cast< int >(static_cast< float >(vpHeight) * -ndc.GetY() * 0.5f +
                                        static_cast< float >(vpHeight / 2));

    const int p0 = rstl::max_val(scrX, 0) & ~3;
    const int p1 = rstl::max_val(scrY, 0) & ~3;
    const int p2 = rstl::min_val(scrX + 3, vpWidth - 4) & ~3;
    const int p3 = rstl::min_val(scrY + 3, vpHeight - 4) & ~3;

    minBounds[0] = rstl::min_val(minBounds[0], p0);
    minY = rstl::min_val(minY, p1);
    maxBounds[0] = rstl::max_val(maxBounds[0], p2);
    maxBounds[1] = rstl::max_val(maxBounds[1], p3);

    allOutside = false;
  }

  int drawRight = vpWidth;
  int drawBottom = vpHeight;
  if (!allOutside) {
    maxChunkW = rstl::min_val(maxBounds.GetX() - minBounds.GetX(), maxChunkW);
    maxChunkH = rstl::min_val(maxBounds.GetY() - minBounds.GetY(), maxChunkH);

    drawRight = rstl::min_val(maxBounds[0], vpWidth);
    drawBottom = rstl::min_val(maxBounds[1], vpHeight);
    drawLeft = minBounds.GetX();
    drawTop = minBounds.GetY();
  }

  if (maxChunkW <= 0 || maxChunkH <= 0) {
    return;
  }

  if (((drawTop + vpTop) & 1) != 0) {
    --drawTop;
  }
  if (((drawLeft + vpLeft) & 1) != 0) {
    --drawLeft;
  }

  bool oldVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);

  const float texOffsetX = 0.5f / static_cast< float >(mFogVolumeRamp.GetWidth());
  const float texOffsetY = 0.5f / static_cast< float >(mFogVolumeRamp.GetHeight());
  float fogTexMtx[2][4] = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
  };
  fogTexMtx[0][3] = texOffsetX;
  fogTexMtx[1][3] = texOffsetY;
  GXLoadTexMtxImm(fogTexMtx, GX_TEXMTX0, GX_MTX2x4);

  const CAABox modelAabb(modelXf * bounds.GetMinPoint() - CVector3f(1.f, 1.f, 1.f),
                         modelXf * bounds.GetMaxPoint() + CVector3f(1.f, 1.f, 1.f));

  bool doDoublePass = modelAabb.PointInside(CGraphics::GetViewMatrix().GetTranslation()) &&
                      (model != nullptr || skinnedModel != nullptr);
  if (doDoublePass) {
    mRequestRGBA6 = true;
    if (!IsRGBA6Current()) {
      doDoublePass = false;
    }
  }

  CGX::SetIndTexMtxSTPointFive(GX_ITM_0, 1);
  const int passCount = static_cast< int >(doDoublePass) + 1;

  for (int y = drawTop; y < drawBottom; y += chunkH) {
    if (drawBottom - y < chunkH) {
      chunkH = drawBottom - y;
      recalcChunk = true;
    }
    if (chunkW != maxChunkW) {
      chunkW = maxChunkW;
      recalcChunk = true;
    }

    const int copyTop = y + vpTop;

    for (int x = drawLeft; x < drawRight; x += chunkW) {
      CGraphics::SetModelMatrix(modelXf);

      if (drawRight - x < chunkW) {
        chunkW = drawRight - x;
        recalcChunk = true;
      }

      if (recalcChunk) {
        uv0MaxX = static_cast< float >(chunkW - 1) / static_cast< float >(chunkW);
        uv0MaxY = static_cast< float >(chunkH - 1) / static_cast< float >(chunkH);

        GXSetTexCopyDst(static_cast< u16 >(chunkW), static_cast< u16 >(chunkH), GX_TF_Z16,
                        GX_FALSE);

        uv0MinX = 0.5f / static_cast< float >(chunkW);
        uv0MinY = 0.5f / static_cast< float >(chunkH);
      }

      const int copyLeft = x + vpLeft;

      GXSetTexCopySrc(static_cast< u16 >(copyLeft), static_cast< u16 >(copyTop),
                      static_cast< u16 >(chunkW), static_cast< u16 >(chunkH));
      GXSetScissor(copyLeft, copyTop, chunkW, chunkH);

      CGX::SetZMode(true, GX_LEQUAL, true);
      CGX::SetNumTevStages(1);
      CGX::SetNumTexGens(1);
      CGX::SetNumChans(0);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
      mFogVolumeRamp.Load(GX_TEXMAP2, CTexture::kCM_Clamp);
      GXSetCullMode(GX_CULL_BACK);
      CGX::SetDstAlpha(true, 0xff);
      RenderFogVolumeModel(bounds, model, modelXf, CGraphics::GetViewMatrix(), skinnedModel);

      if (doDoublePass) {
        CGX::SetZMode(false, GX_ALWAYS, false);
        RenderFogVolumeModel(bounds, model, modelXf, CGraphics::GetViewMatrix(), skinnedModel);
        CGX::SetZMode(true, GX_LEQUAL, true);
      }

      CGX::SetDstAlpha(true, 0);
      GXCopyTex(copyBufA, GX_FALSE);
      GXPixModeSync();
      CGraphics::LoadDolphinSpareTexture(chunkW, chunkH, GX_TF_IA8, copyBufA, GX_TEXMAP0);

      GXSetCullMode(GX_CULL_FRONT);
      RenderFogVolumeModel(bounds, model, modelXf, CGraphics::GetViewMatrix(), skinnedModel);

      if (doDoublePass) {
        CGX::SetZMode(true, GX_GREATER, false);
        RenderFogVolumeModel(bounds, model, modelXf, CGraphics::GetViewMatrix(), skinnedModel);
        CGX::SetZMode(true, GX_LEQUAL, true);
      }

      GXCopyTex(copyBufB, GX_FALSE);
      GXPixModeSync();
      CGraphics::LoadDolphinSpareTexture(chunkW, chunkH, GX_TF_IA8, copyBufB, GX_TEXMAP1);

      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
      CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
      GXInvalidateTexAll();

      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false,
                          GX_PTIDENTITY);
      CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
      CGX::SetNumTexGens(2);
      CGX::SetNumChans(0);

      const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
      CGX::SetVtxDescv(vtxDescrs);
      const CTransform4f oldView(CGraphics::GetViewMatrix());

      CGraphics::SetOrtho(0.f, static_cast< float >(vpWidth), 0.f, static_cast< float >(vpHeight),
                          -4096.f, 4096.f);
      CGraphics::SetViewPointMatrix(CTransform4f::Identity());
      CGraphics::SetModelMatrix(CTransform4f::Identity());

      CGX::SetZMode(false, GX_ALWAYS, false);
      GXSetCullMode(GX_CULL_NONE);
      GXSetAlphaUpdate(GX_FALSE);

      const int right = x + chunkW;
      const int bottom = y + chunkH;
      for (int pass = 0; pass < passCount; ++pass) {
        if (pass == 0) {
          CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0,
                              GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
          CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_ITF_8, GX_ITB_NONE, GX_ITM_0,
                              GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
          GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
          GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD0, GX_TEXMAP0);
          CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
          CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
          CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

          CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_TEXA);
          CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
          CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
          CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);
          CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
          CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);

          CGX::SetNumIndStages(2);
          CGX::SetNumTevStages(2);
          CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
        } else if (pass == 1) {
          CGX::SetTevDirect(GX_TEVSTAGE1);
          GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);

          CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
          CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
          CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

          CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_APREV, GX_CC_CPREV, GX_CC_ZERO);
          CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
          CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
          CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);

          CGX::SetNumIndStages(1);
          CGX::SetNumTevStages(2);
          CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_CLEAR);
        }

        CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

        GXPosition3f32(static_cast< float >(x), 0.5f, static_cast< float >(y));
        GXTexCoord2f32(uv0MinX, uv0MinY);

        GXPosition3f32(static_cast< float >(x), 0.5f, static_cast< float >(bottom));
        GXTexCoord2f32(uv0MinX, (uv0MinY + uv0MaxY));

        GXPosition3f32(static_cast< float >(right), 0.5f, static_cast< float >(bottom));
        GXTexCoord2f32((uv0MinX + uv0MaxX), (uv0MinY + uv0MaxY));

        GXPosition3f32(static_cast< float >(right), 0.5f, static_cast< float >(y));
        GXTexCoord2f32((uv0MinX + uv0MaxX), uv0MinY);

        CGX::End();
      }

      GXSetAlphaUpdate(GX_TRUE);
      CGraphics::SetViewPointMatrix(oldView);
      CGX::SetNumIndStages(0);
      CGX::SetTevDirect(GX_TEVSTAGE0);
      CGX::SetTevDirect(GX_TEVSTAGE1);
      GXSetCullMode(GX_CULL_FRONT);
      CGraphics::SetProjectionState(oldProjection);
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    }
  }

  GXSetScissor(vpLeft, vpTop, vpWidth, vpHeight);
  CGraphics::SetUseVideoFilter(oldVideoFilter);
}

void CCubeRenderer::SetRequestedMaterialMode(int mode) {
  mRequestedMaterialMode = mode;
  if (mode == 0) {
    SetMaterialMode(0);
  }
}

namespace {
struct fog_sorter {
  bool operator()(const CCubeRenderer::CFogVolumeListItem& first,
                  const CCubeRenderer::CFogVolumeListItem& second) const {
    const CTransform4f& view = CGraphics::GetViewMatrix();
    const CVector3f position = view.GetTranslation();
    const CAABox firstBounds = first.mBounds.GetTransformedAABox(first.mTransform);
    const CAABox secondBounds = second.mBounds.GetTransformedAABox(second.mTransform);
    const bool insideFirst = firstBounds.PointInside(
        CVector3f(position.GetX(), position.GetY(), firstBounds.GetMinPoint().GetZ()));
    const bool insideSecond = secondBounds.PointInside(
        CVector3f(position.GetX(), position.GetY(), secondBounds.GetMinPoint().GetZ()));
    if (insideFirst != insideSecond) {
      return insideFirst;
    }
    const CVector3f forward = view.GetForward();
    const float firstDistance =
        CVector3f::Dot(forward, firstBounds.FurthestPointAlongVector(forward));
    const float secondDistance =
        CVector3f::Dot(forward, secondBounds.FurthestPointAlongVector(forward));
    return firstDistance < secondDistance;
  }
};
} // namespace

void CCubeRenderer::PostRenderFogs() {
  mFogVolumes.sort(fog_sorter());
  for (rstl::list< CFogVolumeListItem >::iterator fog = mFogVolumes.begin();
       fog != mFogVolumes.end(); ++fog) {
    const CFogVolumeListItem& item = *fog;
    CGraphics::SetModelMatrix(item.mTransform);
    ReallyRenderFogVolume(item.mColor, item.mBounds, item.mModel ? **item.mModel : nullptr,
                          item.mSkinnedModel);
  }
  mFogVolumes.clear();
}

CCubeRenderer::CFogVolumeListItem::CFogVolumeListItem(const CTransform4f& xf, const CColor& color,
                                                      const CAABox& bounds,
                                                      const TLockedToken< CModel >* model,
                                                      const CSkinnedModel* skinnedModel)
: mTransform(xf)
, mColor(color)
, mBounds(bounds)
, mModel(model ? rstl::optional_object< TLockedToken< CModel > >(*model)
               : rstl::optional_object_null())
, mSkinnedModel(skinnedModel) {}

void CCubeRenderer::DrawModelDisintegrate(const SModelRenderData& model, const CTexture& texture,
                                          const CColor& color, float amount) {
  texture.Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  CGX::SetNumIndStages(0);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(2);
  CGX::SetNumChans(0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());

  const CAABox& bounds = model.GetAABB();
  CTransform4f xf = CTransform4f::RotateX(CRelAngle::FromRadians(-0.7853982f));
  const CAABox rotatedBounds = bounds.GetTransformedAABox(xf);
  const CVector3f dimensions = rotatedBounds.GetMaxPoint() - rotatedBounds.GetMinPoint();
  xf = (CTransform4f::Scale(5.f / dimensions.GetX(), 5.f / dimensions.GetY(),
                            5.f / dimensions.GetZ()) *
        CTransform4f::Translate(-rotatedBounds.GetMinPoint())) *
       xf;
  const CAABox transformedBounds = bounds.GetTransformedAABox(xf);
  (void)transformedBounds;
  const float y = -(1.f - amount) * 6.f + 1.f;
  const float x = -0.85f * amount - 0.15f;
  const float post0[3][4] = {
      {1.f, 1.f, 0.f, amount},
      {0.f, 0.f, 1.f, y},
      {0.f, 0.f, 0.f, 1.f},
  };
  const float post1[3][4] = {
      {1.f, 1.f, 0.f, x},
      {0.f, 0.f, 1.f, y},
      {0.f, 0.f, 0.f, 1.f},
  };
  GXLoadTexMtxImm(xf.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(post0, GX_PTTEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(post1, GX_PTTEXMTX1, GX_MTX3x4);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX1);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetZMode(true, GX_LEQUAL, true);
  model.DrawFlat(CModelFlags(CModelFlags::kT_Opaque, 1.f), true, true);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void CCubeRenderer::DrawModelFlat(const SModelRenderData& model, const CModelFlags& flags,
                                  bool unsortedOnly) {
  const char blendMode = static_cast< char >(flags.GetTrans());
  if (blendMode > 6) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else if (blendMode > 4) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }
  const uint otherFlags = flags.GetOtherFlags();
  CGX::SetZMode(true, (otherFlags & CModelFlags::kF_DepthCompare) ? GX_LEQUAL : GX_ALWAYS,
                (otherFlags & CModelFlags::kF_DepthUpdate) != 0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(0);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKColor(GX_KCOLOR0, flags.GetColorRef().GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, false, GX_PTIDENTITY);
  model.DrawFlat(flags, true, !unsortedOnly);
}

void CCubeRenderer::DrawScreenFilter(const CColor& color0, const CColor& color1,
                                     const CColor& color2) {
  SetupRendererStates(true);
  const CViewport& viewport = CGraphics::GetViewport();
  const int width = viewport.mWidth;
  const int height = viewport.mHeight;
  const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
  const CTransform4f oldView(CGraphics::GetViewMatrix());
  int mipWidth;
  int mipHeight;
  int mipSize = 0;
  CGX::SetDstAlpha(true, 0);
  GXSetAlphaUpdate(false);
  GXPixModeSync();
  GenerateScreenMipmaps(1, false);
  GetScreenMipInfo(width, height, 1, GX_TF_I8, &mipSize, &mipWidth, &mipHeight);
  GXTexObj texture;
  GXInitTexObj(&texture, CGraphics::GetDolphinSpareBuffer(), mipWidth, mipHeight, GX_TF_I8,
               GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  GXLoadTexObj(&texture, GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP0);
  const GXVtxDescList threeTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_TEX0, GX_DIRECT}, {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(threeTexDesc);
  CGraphics::SetFog(kRFM_None, 0.f, 1.f, CColor::Black());
  CGraphics::SetOrtho(0.f, width, 0.f, height, -4096.f, 4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  const CColor full = color1;
  GXColor halfColor = {0, 0, 0, 255};
  halfColor.r = full.GetRedu8() / 2;
  halfColor.g = full.GetGreenu8() / 2;
  halfColor.b = full.GetBlueu8() / 2;
  CGX::SetTevKColor(GX_KCOLOR0, full.GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR1, halfColor);
  CGX::SetTevKColor(GX_KCOLOR2, color0.GetGXColor());
  GXColor quarterColor = {0, 0, 0, 255};
  quarterColor.r = full.GetRedu8() / 4;
  quarterColor.g = full.GetGreenu8() / 4;
  quarterColor.b = full.GetBlueu8() / 4;
  CGX::SetTevKColor(GX_KCOLOR3, quarterColor);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K0);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
  const GXColor comparison = {8, 8, 8, 8};
  GXSetTevColor(GX_TEVREG0, comparison);
  CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_CPREV, GX_CC_C0, GX_CC_CPREV, GX_CC_KONST);
  CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, true,
                     GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
  CGX::SetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_8_8);
  CGX::SetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetNumTevStages(4);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(3);
  const float offsetU[3] = {2.f / mipWidth, -2.f / mipWidth, 0.f};
  const float offsetV[3] = {2.f / mipHeight, -2.f / mipHeight, 0.f};
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetZMode(false, GX_ALWAYS, false);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.f, 0.f);
  GXTexCoord2f32(offsetU[0], offsetV[0]);
  GXTexCoord2f32(offsetU[1], offsetV[1]);
  GXTexCoord2f32(offsetU[2], offsetV[2]);
  GXPosition3f32(0.f, 0.f, height);
  GXTexCoord2f32(offsetU[0], 1.f + offsetV[0]);
  GXTexCoord2f32(offsetU[1], 1.f + offsetV[1]);
  GXTexCoord2f32(offsetU[2], 1.f + offsetV[2]);
  GXPosition3f32(width, 0.f, height);
  GXTexCoord2f32(1.f + offsetU[0], 1.f + offsetV[0]);
  GXTexCoord2f32(1.f + offsetU[1], 1.f + offsetV[1]);
  GXTexCoord2f32(1.f + offsetU[2], 1.f + offsetV[2]);
  GXPosition3f32(width, 0.f, 0.f);
  GXTexCoord2f32(1.f + offsetU[0], offsetV[0]);
  GXTexCoord2f32(1.f + offsetU[1], offsetV[1]);
  GXTexCoord2f32(1.f + offsetU[2], offsetV[2]);
  CGX::End();

  mScanRamp.Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTevKColor(GX_KCOLOR0, color2.GetGXColor());
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetNumTevStages(1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_CLEAR);
  const GXVtxDescList oneTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(oneTexDesc);
  const float rampWidth = width * 0.125f;
  const float rampHeight = height * 0.125f;
  CGX::SetDstAlpha(true, 0);
  GXSetAlphaUpdate(true);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(0.f, 0.f, height);
  GXTexCoord2f32(0.f, rampHeight);
  GXPosition3f32(width, 0.f, height);
  GXTexCoord2f32(rampWidth, rampHeight);
  GXPosition3f32(width, 0.f, 0.f);
  GXTexCoord2f32(rampWidth, 0.f);
  CGX::End();
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGraphics::SetProjectionState(oldProjection);
  CGraphics::SetViewPointMatrix(oldView);
  SetupCGraphicsStates();
}

int CCubeRenderer::DrawScanSurface(int areaSurfaceIndex, const CCubeModel& model,
                                   CMetroidModelInstance::CSurfaceGroups groups, ushort group,
                                   bool intersects, int prevResult) {
  const ushort count = groups.GetSurfaceCount(group);
  const ushort* indices = groups.GetSurfaceIndices(group);
  int result = 1;
  if (intersects) {
    result = 2;
  }
  const int n = count;
  for (int i = 0; i < n; ++i) {
    const CCubeSurface surface(model.GetModelInstance().Surfaces()[indices[i]]);
    if (!(model.GetMaterialByIndex(surface.GetMaterialIndex()).GetFlags() &
          kStateFlag_DepthSorting)) {
      model.DrawSurfaceFlat(surface);
    }
  }
  return result;
}

// Guessed name
void CCubeRenderer::DrawEchoVisorGeometry(float pulsePhase, float bigRingScale,
                                          float bigRingFadeStart, float auraSmallSize,
                                          float auraBigSize, int areaId) {
  SetupRendererStates(true);
  mRequestRGBA6 = true;
  CGX::SetDstAlpha(false, 0);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_Or, kAF_Always, 0);
  mBigRing->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  mSphereRamp.Load(GX_TEXMAP1, CTexture::kCM_Clamp);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_4_8);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetNumTevStages(2);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(2);

  CTransform4f ringMatrix = CTransform4f::Identity();
  CTransform4f volumeMatrix = CTransform4f::Identity();
  CAABox echoBounds = CAABox::MakeNullBox();
  int drawResult = 0;
  if (pulsePhase > bigRingFadeStart) {
    GXColor ringColor = {0, 0, 0, 0};
    ringColor.a =
        CCast::ToUint8(255.f * (1.f - (pulsePhase - bigRingFadeStart) / (1.f - bigRingFadeStart)));
    GXSetTevColor(GX_TEVREG1, ringColor);
  } else {
    const GXColor opaque = {0, 0, 0, 255};
    GXSetTevColor(GX_TEVREG1, opaque);
  }
  const CVector3f viewPos = CGraphics::GetViewMatrix().GetTranslation();
  const float ringScale = 1.f / (1.f + pulsePhase * bigRingScale);
  const float extent = auraBigSize * (1.f - pulsePhase) + auraSmallSize * pulsePhase;
  const float volumeScale = 1.f / extent;
  ringMatrix = CTransform4f(ringScale, 0.f, 0.f, ringScale * -viewPos.GetX() + 0.5f, 0.f, ringScale,
                            0.f, ringScale * -viewPos.GetY() + 0.5f, 0.f, 0.f, 0.f, 1.f);
  volumeMatrix =
      CTransform4f(volumeScale, 0.f, 0.f, volumeScale * -viewPos.GetX() + 0.5f, 0.f, volumeScale,
                   0.f, volumeScale * -viewPos.GetY() + 0.5f, 0.f, 0.f, 0.f, 1.f);
  const CVector3f spread(extent, extent, 4096.f);
  echoBounds = CAABox(viewPos - spread, viewPos + spread);
  GXLoadTexMtxImm(ringMatrix.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX2x4);
  GXLoadTexMtxImm(volumeMatrix.GetCStyleMatrix(), GX_TEXMTX1, GX_MTX2x4);
  GXLoadTexMtxImm(volumeMatrix.GetCStyleMatrix(), GX_TEXMTX2, GX_MTX2x4);
  CGX::SetNumIndStages(0);
  CGX::SetNumChans(0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  GXSetColorUpdate(true);

  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    if (areaId != -1 && areaId != area->mAreaId) {
      continue;
    }
    const rstl::vector< SAreaSurface >& surfaces = *area->GetSurfaces();
    const rstl::vector< CMetroidModelInstance >& geometry = *area->GetModelVector();
    const rstl::vector< rstl::auto_ptr< CCubeModel > >& models = *area->GetModelList();
    for (int i = 0; i < surfaces.size(); ++i) {
      const SAreaSurface& surface = surfaces[i];
      const short modelIndex = surface.mModelIndex;
      const short groupIndex = surface.mSurfaceGroupIndex;
      if (modelIndex == -1 || groupIndex == -1) {
        continue;
      }
      const int surfaceIndex = i - 1;
      if (area->mLightSetIndices[surfaceIndex] == 255) {
        continue;
      }
      const CCubeModel* model = models[modelIndex].get();
      const CMetroidModelInstance::CSurfaceGroups groups = geometry[modelIndex].GetSurfaceGroups();
      model->SetArraysCurrent();
      drawResult = DrawScanSurface(surfaceIndex, *model, groups, groupIndex,
                                   echoBounds.DoBoundsOverlap(surface.mBounds), drawResult);
    }
    GXSetColorUpdate(true);
    GXSetAlphaUpdate(true);
    CGX::SetDstAlpha(true, 0);
  }
  SetupCGraphicsStates();
}

void CCubeRenderer::SetGXRegister1Color(const CColor& color) {
  GXSetTevColor(GX_TEVREG1, color.GetGXColor());
}

void CCubeRenderer::SetWorldLightFadeLevel(float level) {
  const uchar value = CCast::ToUint8(level * 255.f);
  mWorldLightColor = CColor(value, value, value, static_cast< uchar >(255));
}

uchar CCubeRenderer::FindOrAddLightSet(uint lightSet) {
  for (int i = 0; i < mLightSets.size(); ++i) {
    if (mLightSets[i] == lightSet) {
      return static_cast< uchar >(i);
    }
  }
  if (mLightSets.size() < mLightSets.capacity()) {
    mLightSets.push_back(lightSet);
    return static_cast< uchar >(mLightSets.size() - 1);
  }
  return 0;
}

void CCubeRenderer::FindOverlappingWorldModels(rstl::vector< uint >& models, const CAABox& bounds) {
  int wordCount = 0;
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    if (area->mOctTree != nullptr) {
      wordCount += area->mOctTree->GetBitmapWordCount();
    }
  }
  if (wordCount == 0) {
    models = rstl::vector< uint >();
  } else {
    if (wordCount != models.capacity()) {
      models = rstl::vector< uint >();
    } else {
      models.clear();
    }
    models.resize(wordCount, 0);
    int offset = 0;
    for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
         area != mAreaListItems.end(); ++area) {
      const CAreaRenderOctTree* octTree = area->mOctTree;
      const rstl::vector< SAreaSurface >* surfaces = area->GetSurfaces();
      if (octTree != nullptr) {
        octTree->FindOverlappingModels(&models[offset], bounds);
        int surfaceBase = 0;
        for (uint word = 0; word < octTree->GetBitmapWordCount(); ++word) {
          uint* words = models.data();
          if (words[offset + word] != 0) {
            for (int bit = 0; bit < 32; ++bit) {
              if ((words[offset + word] & (1 << bit)) != 0 &&
                  !(*surfaces)[surfaceBase + bit + 1].mBounds.DoBoundsOverlap(bounds)) {
                words[offset + word] &= ~(1 << bit);
              }
            }
          }
          surfaceBase += 32;
        }
        offset += octTree->GetBitmapWordCount();
      }
    }
  }
}

int CCubeRenderer::DrawOverlappingWorldModelShadows(int alphaVal, rstl::vector< uint >& models,
                                                    const CAABox& bounds) {
  SetupRendererStates(true);
  const CModelFlags flags = CModelFlags::Normal();
  bool hadModel = false;
  CGX::SetDstAlpha(true, static_cast< uchar >(alphaVal << 2));
  int offset = 0;
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    const CAreaRenderOctTree* octTree = area->mOctTree;
    const rstl::vector< SAreaSurface >* surfaces = area->GetSurfaces();
    short currentId = static_cast< short >(alphaVal);
    short lastBank = -1;
    rstl::reserved_vector< rstl::pair< short, short >, 64 > ids;
    if (octTree == nullptr) {
      continue;
    }
    const uint* words = models.data();
    for (uint word = 0; word < octTree->GetBitmapWordCount(); ++word) {
      const uint bits = words[offset + word];
      if (bits == 0) {
        continue;
      }
      for (int bit = 0; bit < 32; ++bit) {
        if ((bits & (1 << bit)) == 0) {
          continue;
        }
        const SAreaSurface& areaSurface = (*surfaces)[word * 32 + bit + 1];
        const int modelIndex = areaSurface.mModelIndex;
        const int groupIndex = areaSurface.mSurfaceGroupIndex;
        if (modelIndex == -1 || groupIndex == -1) {
          continue;
        }
        const CCubeModel* model = (*area->mModels)[modelIndex].get();
        const CMetroidModelInstance& instance = (*area->mGeometry)[modelIndex];
        const CMetroidModelInstance::CSurfaceGroups groups = instance.GetSurfaceGroups();
        CCubeMaterial::KillCachedViewDepState();
        model->SetArraysCurrent();
        const ushort count = groups.GetSurfaceCount(groupIndex);
        const ushort* indices = groups.GetSurfaceIndices(groupIndex);
        for (ushort surfaceIndex = 0; surfaceIndex < count; ++surfaceIndex) {
          const CCubeSurface surface(instance.GetSurfaces()[indices[surfaceIndex]]);
          const CCubeMaterial material = model->GetMaterial(surface);
          const short bank = surface.GetShadowBank();
          short id = currentId;
          if (bank != lastBank) {
            int idIndex = 0;
            for (; idIndex < ids.size(); ++idIndex) {
              if (ids[idIndex].first == bank) {
                id = ids[idIndex].second;
                break;
              }
            }
            if (idIndex == ids.size() && idIndex != ids.capacity() && alphaVal <= 64) {
              id = static_cast< short >(alphaVal);
              ids.push_back(rstl::pair< short, short >(bank, alphaVal));
              ++alphaVal;
            }
            lastBank = bank;
            currentId = id;
            CGX::SetDstAlpha(true, id << 2);
          }
          if (!material.IsFlagSet(kStateFlag_DepthSorting) &&
              surface.GetBounds().DoBoundsOverlap(bounds)) {
            model->DrawSurface(surface, flags);
          }
        }
        hadModel = true;
        if (alphaVal >= 64) {
          SetupCGraphicsStates();
          return alphaVal;
        }
      }
    }
    offset += octTree->GetBitmapWordCount();
  }
  SetupCGraphicsStates();
  return alphaVal + (hadModel ? 1 : 0);
}

void CCubeRenderer::DrawWorldModelShadow(const CAABox& bounds) {
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    const CAreaRenderOctTree* octTree = area->mOctTree;
    const rstl::vector< SAreaSurface >* surfaces = area->GetSurfaces();
    if (octTree == nullptr) {
      continue;
    }
    rstl::vector< uint > models;
    octTree->FindOverlappingModels(models, bounds);
    for (uint word = 0; word < octTree->GetBitmapWordCount(); ++word) {
      const uint bits = models[word];
      if (bits == 0) {
        continue;
      }
      for (int bit = 0; bit < 32; ++bit) {
        if ((bits & (1 << bit)) == 0) {
          continue;
        }
        const SAreaSurface& areaSurface = (*surfaces)[word * 32 + bit + 1];
        const int modelIndex = areaSurface.mModelIndex;
        const int groupIndex = areaSurface.mSurfaceGroupIndex;
        if (modelIndex == -1 || groupIndex == -1) {
          continue;
        }
        const CCubeModel* model = (*area->mModels)[modelIndex].get();
        const CMetroidModelInstance& instance = (*area->mGeometry)[modelIndex];
        const CMetroidModelInstance::CSurfaceGroups groups = instance.GetSurfaceGroups();
        CCubeMaterial::KillCachedViewDepState();
        model->SetArraysCurrent();
        const ushort count = groups.GetSurfaceCount(groupIndex);
        const ushort* indices = groups.GetSurfaceIndices(groupIndex);
        for (ushort surfaceIndex = 0; surfaceIndex < count; ++surfaceIndex) {
          const CCubeSurface surface(instance.GetSurfaces()[indices[surfaceIndex]]);
          const CCubeMaterial material = model->GetMaterial(surface);
          if (!material.IsFlagSet(kStateFlag_DepthSorting) &&
              surface.GetBounds().DoBoundsOverlap(bounds)) {
            CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
            CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
          }
        }
      }
    }
  }
}

void CCubeRenderer::DrawOverlappingWorldModelIDs(int alphaVal, rstl::vector< uint >& models,
                                                 const CAABox& bounds) {
  GXColor color = {0, 0, 0, 0};
  int offset = 0;
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    const CAreaRenderOctTree* octTree = area->mOctTree;
    const rstl::vector< SAreaSurface >* surfaces = area->GetSurfaces();
    short currentId = static_cast< short >(alphaVal);
    short lastBank = -1;
    rstl::reserved_vector< rstl::pair< short, short >, 64 > ids;
    if (octTree == nullptr) {
      continue;
    }
    const uint* words = models.data();
    for (uint word = 0; word < octTree->GetBitmapWordCount(); ++word) {
      const uint bits = words[offset + word];
      if (bits == 0) {
        continue;
      }
      for (int bit = 0; bit < 32; ++bit) {
        if ((bits & (1 << bit)) == 0) {
          continue;
        }
        const SAreaSurface& areaSurface = (*surfaces)[word * 32 + bit + 1];
        const int modelIndex = areaSurface.mModelIndex;
        const int groupIndex = areaSurface.mSurfaceGroupIndex;
        if (modelIndex == -1 || groupIndex == -1) {
          continue;
        }
        const CCubeModel* model = (*area->mModels)[modelIndex].get();
        const CMetroidModelInstance& instance = (*area->mGeometry)[modelIndex];
        const CMetroidModelInstance::CSurfaceGroups groups = instance.GetSurfaceGroups();
        CCubeMaterial::KillCachedViewDepState();
        model->SetArraysCurrent();
        const ushort count = groups.GetSurfaceCount(groupIndex);
        const ushort* indices = groups.GetSurfaceIndices(groupIndex);
        for (ushort surfaceIndex = 0; surfaceIndex < count; ++surfaceIndex) {
          const CCubeSurface surface(instance.GetSurfaces()[indices[surfaceIndex]]);
          const CCubeMaterial material = model->GetMaterial(surface);
          const short bank = surface.GetShadowBank();
          short id = currentId;
          if (bank != lastBank) {
            int idIndex = 0;
            for (; idIndex < ids.size(); ++idIndex) {
              if (ids[idIndex].first == bank) {
                id = ids[idIndex].second;
                break;
              }
            }
            if (idIndex == ids.size() && idIndex != ids.capacity() && alphaVal <= 64) {
              id = static_cast< short >(alphaVal);
              ids.push_back(rstl::pair< short, short >(bank, alphaVal));
              ++alphaVal;
            }
            lastBank = bank;
            color.a = id << 2;
            currentId = id;
            CGX::SetTevKColor(GX_KCOLOR0, color);
          }
          if (!material.IsFlagSet(kStateFlag_DepthSorting) &&
              surface.GetBounds().DoBoundsOverlap(bounds)) {
            CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
            CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
          }
        }

        if (alphaVal >= 64) {
          return;
        }
      }
    }
    offset += octTree->GetBitmapWordCount();
  }
}

void* CCubeRenderer::GetRenderToTexBuffer(int index) {
  return static_cast< uchar* >(CGraphics::GetDolphinSpareBuffer()) +
         (static_cast< uint >(index * CGraphics::GetSpareBufferSize()) >> 4);
}

void CCubeRenderer::CopyScreenTex(uint divisor, bool half, void* dest, GXTexFmt format,
                                  bool clear) const {
  const CViewport& viewport = CGraphics::GetViewport();
  int width = viewport.mWidth;
  int height = viewport.mHeight;
  GXSetTexCopySrc(viewport.mLeft, viewport.mTop + height - height / divisor, width / divisor,
                  height / divisor);
  const int copyWidth = half ? width / 2 : width;
  const int copyHeight = half ? height / 2 : height;
  GXSetTexCopyDst(copyWidth / divisor, copyHeight / divisor, format, half);
  const CColor clearColor = CGraphics::GetClearColor();
  CGraphics::SetClearColor(CColor(0));
  GXSetColorUpdate(false);
  GXCopyTex(dest ? dest : CGraphics::GetDolphinSpareBuffer(), clear);
  GXSetColorUpdate(true);
  GXPixModeSync();
  CGraphics::SetClearColor(clearColor);
}

void CCubeRenderer::DoPhazonSuitIndirectAlphaBlur(float scale, float amount) {
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;

  CGraphics::SetOrtho(0.f, 1.f, 1.f, 0.f, -1.f, 1.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetDepthWriteMode(false, kE_GEqual, false);

  CopyScreenTex(1, true, GetRenderToTexBuffer(8), GX_CTF_A8, true);
  CGX::SetDstAlpha(true, 0);

  CGraphics::LoadDolphinSpareTexture(width / 2, height / 2, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const GXVtxDescList vtxDescrs[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescrs);

  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_RASC, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  GXSetColorUpdate(GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetChanMatColor(CGX::Channel0, CColor::White().GetGXColor());
  CGX::SetChanAmbColor(CGX::Channel0, CColor::Black().GetGXColor());

  const uint white = CColor::White().GetColor_u32();

  CGX::SetDstAlpha(false, 0);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  GXPosition3f32(0.f, 0.f, 0.f);
  GXColor1u32(white);
  GXTexCoord2f32(0.f, 1.f);

  GXPosition3f32(0.5f, 0.f, 0.f);
  GXColor1u32(white);
  GXTexCoord2f32(1.f, 1.f);

  GXPosition3f32(0.f, 0.f, 0.5f);
  GXColor1u32(white);
  GXTexCoord2f32(0.f, 0.f);

  GXPosition3f32(0.5f, 0.f, 0.5f);
  GXColor1u32(white);
  GXTexCoord2f32(1.f, 0.f);

  CGX::End();

  CopyScreenTex(2, true, GetRenderToTexBuffer(8), GX_CTF_A8, true);
  GXSetColorUpdate(GX_FALSE);

  CGraphics::LoadDolphinSpareTexture(width >> 2, height >> 2, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const float blurOffsets[8][2] = {
      {-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f},
      {-1.f, 0.f},  {1.f, 0.f},  {0.f, 1.f},  {0.f, -1.f},
  };

  const float blurScale = scale * (2.f / static_cast< float >(width));
  const uint blurColorA = CColor(1.f, 1.f, 1.f, 0.3f).GetColor_u32();

  for (uint i = 0; i < 8; ++i) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    CVector2f ofs(blurScale * blurOffsets[i][0], blurScale * blurOffsets[i][1]);
    float& offsetX = ofs[0];

    GXPosition3f32(offsetX, 0.f, ofs[1]);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(0.f, 1.f);

    GXPosition3f32(offsetX + 0.25f, 0.f, ofs[1]);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(1.f, 1.f);

    GXPosition3f32(offsetX, 0.f, ofs[1] + 0.25f);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(0.f, 0.f);

    GXPosition3f32(offsetX + 0.25f, 0.f, ofs[1] + 0.25f);
    GXColor1u32(blurColorA);
    GXTexCoord2f32(1.f, 0.f);

    CGX::End();
  }

  CGX::SetDstAlpha(false, 0);
  CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

  const uint whiteColor = CColor(1.f, 1.f, 1.f, 1.f).GetColor_u32();

  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  GXPosition3f32(0.f, 0.f, 0.f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(0.f, 1.f);

  GXPosition3f32(0.25f, 0.f, 0.f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(1.f, 1.f);

  GXPosition3f32(0.f, 0.f, 0.25f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(0.f, 0.f);

  GXPosition3f32(0.25f, 0.f, 0.25f);
  GXColor1u32(whiteColor);
  GXTexCoord2f32(1.f, 0.f);

  CGX::End();

  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

  CopyScreenTex(4, false, GetRenderToTexBuffer(8), GX_CTF_A8, true);
  CGX::SetDstAlpha(false, 0);
  GXSetColorUpdate(GX_FALSE);

  CGraphics::LoadDolphinSpareTexture(width >> 2, height >> 2, GX_TF_I8, GetRenderToTexBuffer(8),
                                     CGraphics::kSpareBufferTexMapID);

  const float blurScaleB = amount * (1.5f / static_cast< float >(width));
  const uint blurColorB = CColor(1.f, 1.f, 1.f, 0.35f).GetColor_u32();

  for (uint i = 0; i < 8; ++i) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    CVector2f ofs(blurScaleB * blurOffsets[i][0], blurScaleB * blurOffsets[i][1]);
    float& offsetX = ofs[0];

    GXPosition3f32(offsetX, 0.f, ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(0.f, 1.f);

    GXPosition3f32(0.25f + offsetX, 0.f, ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(1.f, 1.f);

    GXPosition3f32(offsetX, 0.f, 0.25f + ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(0.f, 0.f);

    GXPosition3f32(0.25f + offsetX, 0.f, 0.25f + ofs[1]);
    GXColor1u32(blurColorB);
    GXTexCoord2f32(1.f, 0.f);

    CGX::End();
  }
}

void CCubeRenderer::ReallyDrawPhazonSuitEffect(const CColor& color, const CTexture& texture) {
  texture.Load(CGraphics::kSpareBufferTexMapID, CTexture::kCM_Repeat);
  const GXVtxDescList descriptors[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(descriptors);
  IRenderer* renderer = this;
  renderer->SetBlendMode_AdditiveAlpha();
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_RASC, GX_CC_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetChanAmbColor(CGX::Channel0, CColor::Black().GetGXColor());
  CGX::SetDstAlpha(true, 0);
  GXSetColorUpdate(GX_TRUE);
  const uint packedColor = color.GetColor_u32();
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.f, 0.f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(1.f, 0.f, 0.f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(0.f, 0.f, 1.f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(1.f, 0.f, 1.f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
}

void CCubeRenderer::ReallyDrawPhazonSuitIndirectEffect(const CColor& color, const CTexture& texture,
                                                       const CTexture& indirectTexture, float scale,
                                                       float offset, float alpha,
                                                       const CColor& additiveColor) {
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;

  CVector2i topLeft(0, 0);
  CVector2i bottomRight(width, height);
  CVector2f uv0Min(0.f, 0.f);
  float& uv0MinX = uv0Min[0];
  CVector2f uv0Max(1.f, 1.f);
  float& uv0MaxX = uv0Max[0];

  CVector2i dim = bottomRight - topLeft;
  CVector2i halfDim = dim / 2;
  int& halfX = halfDim[0];

  if (dim.GetX() <= 0 || dim.GetY() <= 0) {
    return;
  }

  CGraphics::LoadDolphinSpareTexture(halfX, halfDim.GetY(), GX_TF_RGB565,
                                     CGraphics::GetDolphinSpareBuffer(),
                                     CGraphics::kSpareBufferTexMapID);
  indirectTexture.Load(GX_TEXMAP1, CTexture::kCM_Repeat);
  texture.Load(GX_TEXMAP2, CTexture::kCM_Repeat);

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  const CColor kColor(additiveColor.GetRed() * additiveColor.GetAlpha(),
                      additiveColor.GetGreen() * additiveColor.GetAlpha(),
                      additiveColor.GetBlue() * additiveColor.GetAlpha(),
                      0.25f * additiveColor.GetAlpha());
  CGX::SetTevKColor(GX_KCOLOR0, kColor.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);

  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0);

  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);

  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetChanAmbColor(CGX::Channel0, CColor::Black().GetGXColor());

  float indScale = scale;
  u8 scaleExp = 1;
  while (CMath::AbsF(indScale) >= 0.99f) {
    indScale *= 0.5f;
    ++scaleExp;
  }
  while (CMath::AbsF(indScale) < 0.49f) {
    indScale *= 2.f;
    --scaleExp;
  }

  float indMtx[2][3] = {
      {indScale, 0.f, offset * indScale},
      {0.f, indScale, alpha * indScale},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, scaleExp);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);

  CGX::SetNumIndStages(1);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(3);
  CGX::SetNumChans(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

  static const GXVtxDescList vtxDesc[6] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);

  const CGraphics::CProjectionState backupProjection = CGraphics::GetProjectionState();
  const CTransform4f backupView(CGraphics::GetViewMatrix());

  CGraphics::SetOrtho(0.f, static_cast< float >(width), 0.f, static_cast< float >(height), -4096.f,
                      4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  CGX::SetZMode(false, GX_ALWAYS, false);
  GXSetCullMode(GX_CULL_NONE);
  CGX::SetDstAlpha(true, 0);

  const uint colorU32 = color.GetColor_u32();
  CVector2f uv1Min(0.f, 0.f);
  float& uv1MinX = uv1Min[0];
  CVector2f uv1Max(1.f, 1.f);
  float& uv1MaxX = uv1Max[0];

  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

  GXPosition3f32(topLeft.GetX(), 0.5f, topLeft.GetY());
  GXColor1u32(colorU32);
  GXTexCoord2f32(0.01f, 0.01f);
  GXTexCoord2f32(uv0MinX, uv0Min[1]);
  GXTexCoord2f32(uv1MinX, uv1Min[1]);

  GXPosition3f32(topLeft.GetX(), 0.5f, bottomRight.GetY());
  GXColor1u32(colorU32);
  GXTexCoord2f32(0.01f, 0.99f);
  GXTexCoord2f32(uv0MinX, uv0Max[1]);
  GXTexCoord2f32(uv1MinX, uv1Max[1]);

  GXPosition3f32(bottomRight.GetX(), 0.5f, bottomRight.GetY());
  GXColor1u32(colorU32);
  GXTexCoord2f32(0.99f, 0.99f);
  GXTexCoord2f32(uv0MaxX, uv0Max[1]);
  GXTexCoord2f32(uv1MaxX, uv1Max[1]);

  GXPosition3f32(bottomRight.GetX(), 0.5f, topLeft.GetY());
  GXColor1u32(colorU32);
  GXTexCoord2f32(0.99f, 0.01f);
  GXTexCoord2f32(uv0MaxX, uv0Min[1]);
  GXTexCoord2f32(uv1MaxX, uv1Min[1]);

  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);

  CGraphics::SetProjectionState(backupProjection);
  CGraphics::SetViewPointMatrix(backupView);
}

void CCubeRenderer::RenderSilhouette(
    float blur, const CColor& color,
    const rstl::optional_object< TCachedToken< CTexture > >& texture, float scale, float offset,
    float alpha, const CColor& additiveColor) {
  if (IsRGBA6Current() && mSilhouetteMaskCountdown != 0) {
    const CTransform4f view = CGraphics::GetViewMatrix();
    const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
    if (!mSilhouetteMask.get()) {
      return;
    }
    if (mSilhouetteMask->GetWidth() != (CGraphics::GetViewport().mWidth >> 2) ||
        mSilhouetteMask->GetHeight() != (CGraphics::GetViewport().mHeight >> 2)) {
      return;
    }
    DoPhazonSuitIndirectAlphaBlur(blur, blur);
    mSilhouetteMask->SetFlag1(true);
    CopyScreenTex(4, false, mSilhouetteMask->GetBitMapData(0), GX_CTF_A8, true);
    if (texture && texture->GetObject() != nullptr) {
      ReallyDrawPhazonSuitIndirectEffect(CColor(1.f, 1.f, 1.f, 1.f), *mSilhouetteMask,
                                         *texture->GetObject(), scale, offset, alpha,
                                         additiveColor);
    } else {
      ReallyDrawPhazonSuitEffect(color, *mSilhouetteMask);
    }
    mSilhouetteMask->UnLock();
    CGraphics::SetViewPointMatrix(view);
    CGraphics::SetProjectionState(projection);
    mSilhouetteMaskCountdown = 2;
  }
  CGX::SetDstAlpha(false, 0);
}

void CCubeRenderer::AllocatePhazonSuitMaskTexture() {
  mRequestRGBA6 = true;
  if (!mSilhouetteMask.get()) {
    mSilhouetteMask = rs_new CTexture(kTF_I8, CGraphics::GetViewport().mWidth >> 2,
                                      CGraphics::GetViewport().mHeight >> 2, 1);
  }
  mSilhouetteMaskCountdown = 2;
}

// The unit scale survives inlining; MWCC only folds it when written directly.
static inline float GetFractionalPart(float value, float unit) {
  const float whole = static_cast< int >(value * unit);
  return value - whole * unit;
}

float CCubeRenderer::GetRandomInterpolation(float time, float period, int seed) {
  const float scaledTime = time / period;
  const float fraction = GetFractionalPart(scaledTime, 1.f);
  const uint frame = static_cast< uint >(scaledTime - fraction);
  CRandom16 first(seed + frame);
  CRandom16 second(seed + frame + 1);
  const float firstValue = first.Float();
  const float secondValue = second.Float();
  return firstValue * (1.f - fraction) + secondValue * fraction;
}

void CCubeRenderer::PopulateNoiseTexCoords(float time,
                                           rstl::reserved_vector< CVector2f, 9 >& coords) {
  for (int y = 0; y < 3; ++y) {
    for (int x = 0; x < 3; ++x) {
      coords.push_back(CVector2f(x * 0.5f, y * 0.5f));
    }
  }
  const float scaledTime = time / 0.1f;
  CRandom16 random(static_cast< uint >(scaledTime) + 200);
  const int index = random.Range(0, 8);
  const int axis = random.Range(0, 1);
  const float frac = GetFractionalPart(scaledTime, 1.f);
  float fraction;
  if (frac < 0.5f) {
    fraction = frac;
  } else {
    fraction = -1.f * (frac - 1.f);
  }
  if (random.Range(0, 1) != 0) {
    fraction *= -1.f;
  }
  coords[index][axis] += fraction;
}

bool CCubeRenderer::EnableSilhouetteRender() {
  mRequestRGBA6 = true;
  if (!IsRGBA6Current()) {
    return false;
  }
  mRenderingSilhouette = true;
  GXSetAlphaUpdate(true);
  GXSetColorUpdate(false);
  CGX::SetDstAlpha(false, 0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(0);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  const CColor color(static_cast< uchar >(0), 0, 0, 255);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  return true;
}

void CCubeRenderer::DrawSilhouetteNoise(const SSilhouetteNoise& noise) {
  const float time = noise.mTime;
  const CColor noiseColor = noise.mColor;
  mRenderingSilhouette = false;
  CGX::SetDstAlpha(true, 0);
  void* const spare = CGraphics::GetDolphinSpareBuffer();
  void* noiseData = reinterpret_cast< void* >(((mRandom.Next() + 31) & ~31) + 0x8000);
  CGX::SetZMode(false, GX_ALWAYS, false);
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;
  GXSetAlphaUpdate(false);
  CopyScreenTex(1, true, spare, GX_TF_RGB5A3, true);
  CGraphics::LoadDolphinSpareTexture(width / 2, height / 2, GX_TF_RGB5A3, nullptr,
                                     CGraphics::kSpareBufferTexMapID);
  CGraphics::LoadDolphinSpareTexture(96, 96, GX_TF_IA4, noiseData, GX_TEXMAP0);
  GXTexObj texture;
  GXInitTexObj(&texture, noiseData, 96, 96, GX_TF_IA4, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  GXLoadTexObj(&texture, GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP0);
  GXSetAlphaUpdate(true);
  GXSetColorUpdate(true);

  const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
  const CTransform4f oldModel(CGraphics::GetModelMatrix());
  const CTransform4f oldView(CGraphics::GetViewMatrix());
  CGraphics::SetOrtho(-1.f, 1.f, -1.f, 1.f, -4096.f, 4096.f);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGX::SetZMode(true, GX_ALWAYS, false);
  const CColor color = CColor::Add(CColor(static_cast< uchar >(4), 2, 4, 255), noiseColor);
  CGX::SetNumTevStages(1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_ONE, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetNumIndStages(1);
  const float indScale = 0.25f * GetRandomInterpolation(time, 0.5f, 0);
  const float matrix[2][3] = {{indScale, 0.f, 0.f}, {0.f, indScale, 0.f}};
  GXSetIndTexMtx(GX_ITM_0, matrix, -3);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP0);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumTexGens(2);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  static const GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_INVDSTALPHA, GX_LO_CLEAR);
  rstl::reserved_vector< CVector2f, 9 > coords;
  PopulateNoiseTexCoords(time, coords);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(-1.f, 0.5f, -1.f);
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(-1.f, 0.5f, 1.f);
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(1.f, 0.5f, 1.f);
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(1.f, 0.5f, -1.f);
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();
  CGraphics::SetProjectionState(oldProjection);
  CGraphics::SetModelMatrix(oldModel);
  CGraphics::SetViewPointMatrix(oldView);
  CGX::SetZMode(true, GX_LEQUAL, true);
}

void CCubeRenderer::fn_802679DC(const void* unused, const SModelRenderData& model,
                                const CModelFlags& flags) {
  const uint otherFlags = flags.GetOtherFlags();
  CGX::SetZMode(true, (otherFlags & CModelFlags::kF_DepthCompare) ? GX_LEQUAL : GX_ALWAYS,
                (otherFlags & CModelFlags::kF_DepthUpdate) != 0);
  model.DrawFlat(flags, true, true);
}

bool CCubeRenderer::LoadEnvironmentTextureMatrix(uint matrix, uint postMatrix,
                                                 const CTransform4f& xf, bool alternate) {
  CTransform4f textureTransform = xf.MultiplyIgnoreTranslation(CGraphics::GetModelMatrix());
  textureTransform.SetTranslation(CVector3f::Zero());
  GXLoadTexMtxImm(textureTransform.GetCStyleMatrix(), matrix, GX_MTX3x4);
  static const float environmentMatrix[3][4] = {
      {0.5f, 0.f, 0.f, 0.5f},
      {0.f, 0.f, 0.5f, 0.5f},
      {0.f, 0.f, 0.f, 1.f},
  };
  static const float alternateMatrix[3][4] = {
      {2.f, 0.f, 0.f, 1.f},
      {0.f, 0.f, 2.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  };
  GXLoadTexMtxImm(alternate ? alternateMatrix : environmentMatrix, postMatrix, GX_MTX3x4);
  return true;
}

void CCubeRenderer::LoadScrollingTextureMatrix(uint matrix, const CVector2f& scroll,
                                               const CVector2f& scale) {
  const float seconds = CGraphics::GetSecondsMod900();
  const float textureMatrix[3][4] = {
      {scale.GetX(), 0.f, 0.f, seconds * scroll.GetX()},
      {0.f, scale.GetY(), 0.f, seconds * scroll.GetY()},
      {0.f, 0.f, 0.f, 1.f},
  };
  GXLoadTexMtxImm(textureMatrix, matrix, GX_MTX3x4);
}

void CCubeRenderer::DrawDarkWorldVolume(const CVector3f& pos, const CVector3f& scale, uchar mix,
                                        uchar alpha, bool inside, float lod,
                                        const CVector2f& scroll1, const CVector2f& scroll2,
                                        const CVector2f& texScale1, const CVector2f& texScale2,
                                        const CTexture& environment, const CTexture& cloud1,
                                        const CTexture& cloud2, CColor color, CColor additiveColor,
                                        bool cylinder, bool additive) {
  CGX::SetZMode(true, GX_LEQUAL, false);
  if (inside) {
    CGraphics::SetCullMode(kCM_Back);
  }
  CGraphics::SetModelMatrix(CTransform4f::Translate(pos) * CTransform4f::Scale(scale));
  if (!additive) {
    mRequestRGBA6 = true;
    CGX::SetDstAlpha(false, 255);
    GXSetAlphaUpdate(!inside);
  }
  const int firstCloud = inside ? 0 : 1;
  if (!inside) {
    environment.Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  }
  const GXTevStageID firstStage = static_cast< GXTevStageID >(firstCloud);
  const GXTevStageID secondStage = static_cast< GXTevStageID >(firstCloud + 1);
  const GXTexMapID firstMap = static_cast< GXTexMapID >(firstCloud);
  const GXTexMapID secondMap = static_cast< GXTexMapID >(firstCloud + 1);
  const GXTexCoordID firstCoord = static_cast< GXTexCoordID >(firstCloud);
  const GXTexCoordID secondCoord = static_cast< GXTexCoordID >(firstCloud + 1);
  cloud1.Load(firstMap, CTexture::kCM_Repeat);
  cloud2.Load(secondMap, CTexture::kCM_Repeat);

  const float blend = static_cast< float >(mix) / 255.f;
  const CColor environmentColor = CColor::Lerp(color, CColor::White(), blend);
  const CColor addedColor = CColor::Lerp(CColor::Black(), additiveColor, blend);
  const CColor opacity(alpha, alpha, alpha, alpha);
  CGX::SetTevKColor(GX_KCOLOR0, environmentColor.GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR1, opacity.GetGXColor());
  GXSetTevColor(GX_TEVREG0, addedColor.GetGXColor());
  if (!inside) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_C0);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_6_8);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  }
  GXTevColorArg cloudColor;
  GXTevAlphaArg cloudAlpha;
  if (inside) {
    CGX::SetTevKColorSel(firstStage, GX_TEV_KCSEL_K1);
    CGX::SetTevKAlphaSel(firstStage, GX_TEV_KASEL_K1_A);
    cloudColor = GX_CC_KONST;
    cloudAlpha = GX_CA_KONST;
  } else {
    cloudColor = GX_CC_APREV;
    cloudAlpha = GX_CA_APREV;
  }
  CGX::SetTevColorIn(firstStage, GX_CC_ZERO, GX_CC_TEXC, cloudColor, GX_CC_ZERO);
  CGX::SetTevAlphaIn(firstStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, cloudAlpha);
  CGX::SetTevOrder(firstStage, firstCoord, firstMap, GX_COLOR_NULL);
  CGX::SetTevColorOp(firstStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG1);
  CGX::SetTevAlphaOp(firstStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevDirect(firstStage);
  const GXTevColorArg secondColor = inside ? GX_CC_C0 : GX_CC_CPREV;
  const GXTevScale secondScale = additive ? GX_CS_SCALE_4 : GX_CS_SCALE_1;
  CGX::SetTevColorIn(secondStage, GX_CC_ZERO, GX_CC_C1, GX_CC_TEXC, secondColor);
  CGX::SetTevAlphaIn(secondStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevOrder(secondStage, secondCoord, secondMap, GX_COLOR_NULL);
  CGX::SetTevColorOp(secondStage, GX_TEV_ADD, GX_TB_ZERO, secondScale, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(secondStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevDirect(secondStage);

  CTransform4f environmentTransform(CGraphics::GetViewMatrix());
  const CVector3f toVolume = pos - CGraphics::GetViewMatrix().GetTranslation();
  if (toVolume.CanBeNormalized()) {
    const CUnitVector3f direction(toVolume);
    const CQuaternion rotation =
        CQuaternion::LookAt(CUnitVector3f(environmentTransform.GetForward(), CUnitVector3f::kN_No),
                            direction, CRelAngle::FromRadians(2.f * M_PIF));
    environmentTransform = rotation.BuildTransform4f()
                               .MultiplyIgnoreTranslation(environmentTransform)
                               .GetQuickInverse();
  } else {
    environmentTransform = environmentTransform.GetQuickInverse();
  }
  if (!inside) {
    LoadEnvironmentTextureMatrix(GX_TEXMTX0, GX_PTTEXMTX0, environmentTransform, false);
  }
  LoadScrollingTextureMatrix(GX_PTTEXMTX1, scroll1, texScale1);
  LoadScrollingTextureMatrix(GX_PTTEXMTX2, scroll2, texScale2);
  if (!inside) {
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, true, GX_PTTEXMTX0);
  }
  CGX::SetTexCoordGen(firstCoord, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTTEXMTX1);
  CGX::SetTexCoordGen(secondCoord, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTTEXMTX2);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetNumTevStages(firstCloud + 2);
  CGX::SetNumTexGens(firstCloud + 2);
  if (lod > 0.5f) {
    if (cylinder) {
      mFlatCylinder->DolphinDrawFlat(CModel::kDF_Unsorted);
    } else {
      mFlatSphere->DolphinDrawFlat(CModel::kDF_Unsorted);
    }
  } else {
    if (cylinder) {
      mFlatCylinderLow->DolphinDrawFlat(CModel::kDF_Unsorted);
    } else {
      mFlatSphereLow->DolphinDrawFlat(CModel::kDF_Unsorted);
    }
  }
  if (!additive) {
    CGX::SetDstAlpha(true, 0);
  }
  if (inside) {
    CGraphics::SetCullMode(kCM_Front);
  }
}

void CCubeRenderer::GetScreenMipInfo(int width, int height, int mipCount, GXTexFmt format,
                                     int* size, int* mipWidth, int* mipHeight) {
  int w = width;
  int h = height;
  int total = 0;
  for (int i = 0; i < mipCount; ++i) {
    w &= ~1;
    w >>= 1;
    h &= ~1;
    h >>= 1;
    total += GXGetTexBufferSize(w, h, format, GX_FALSE, 0);
  }
  if (size) {
    *size = total;
  }
  if (mipWidth) {
    *mipWidth = w;
  }
  if (mipHeight) {
    *mipHeight = h;
  }
}

void CCubeRenderer::SetupScreenCopyStates() {
  CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(desc);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetNumTevStages(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetZMode(false, GX_ALWAYS, false);
}

void CCubeRenderer::DrawTexturedScreenQuad(int left, int top, int width, int height) {
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(left, 0.5f, top);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(left, 0.5f, top + height);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(left + width, 0.5f, top + height);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(left + width, 0.5f, top);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();
}

void CCubeRenderer::GenerateScreenMipmaps(int mipCount, GXTexFmt copyFormat, GXTexFmt loadFormat,
                                          int left, int top, int width, int height) {
  const CViewport viewport = CGraphics::GetViewport();
  uchar* data = static_cast< uchar* >(CGraphics::GetDolphinSpareBuffer());
  const int copyLeft = viewport.mLeft + left;
  const int copyTop = viewport.mTop + top;
  const CTransform4f view = CGraphics::GetViewMatrix();
  const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
  width &= ~1;
  height &= ~1;
  bool statesSet = false;
  for (int mip = 0; mip < mipCount; ++mip) {
    int size;
    int mipWidth;
    int mipHeight;
    GetScreenMipInfo(width, height, 1, copyFormat, &size, &mipWidth, &mipHeight);
    GXSetTexCopySrc(copyLeft, copyTop, mipWidth * 2, mipHeight * 2);
    GXSetTexCopyDst(mipWidth, mipHeight, copyFormat, true);
    GXCopyTex(data, false);
    if (mip + 1 == mipCount) {
      GXPixModeSync();
    } else {
      GXTexObj texture;
      GXInitTexObj(&texture, data, mipWidth, mipHeight, loadFormat, GX_CLAMP, GX_CLAMP, false);
      GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
      GXLoadTexObj(&texture, CGraphics::kSpareBufferTexMapID);
      CTexture::InvalidateTexmap(CGraphics::kSpareBufferTexMapID);
      GXInvalidateTexRegion(CGraphics::GetSpareTextureRegion());
      GXPixModeSync();
      if (!statesSet) {
        CGraphics::SetOrtho(0.f, viewport.mWidth, 0.f, viewport.mHeight, -4096.f, 4096.f);
        CGraphics::SetViewPointMatrix(CTransform4f::Identity());
        CGraphics::SetModelMatrix(CTransform4f::Identity());
        SetupScreenCopyStates();
        statesSet = true;
      }
      DrawTexturedScreenQuad(left, top, mipWidth, mipHeight);
      data += size;
      width = mipWidth;
      height = mipHeight;
    }
  }
  if (statesSet) {
    CGraphics::SetProjectionState(projection);
    CGraphics::SetViewPointMatrix(view);
  }
}

void CCubeRenderer::GenerateScreenMipmaps(int mipCount, bool alpha) {
  const CViewport& viewport = CGraphics::GetViewport();
  const int width = viewport.mWidth;
  const int height = viewport.mHeight;
  GenerateScreenMipmaps(mipCount, alpha ? GX_CTF_A8 : GX_CTF_R8, GX_TF_I8, 0, 0, width, height);
}

void CCubeRenderer::SetMaterialMode(int mode) {
  if (mCurrentMaterialMode == mode) {
    return;
  }
  mCurrentMaterialMode = mode;
  switch (mode) {
  case 0:
    CCubeMaterial::UseNormalTevs();
    break;
  case 1:
    CCubeMaterial::UseThermalTevs();
    break;
  }
}

void CCubeRenderer::SetDestinationAlpha(int alpha) {
  mRequestRGBA6 = true;
  GXSetAlphaUpdate(GX_TRUE);
  CGX::SetDstAlpha(GX_TRUE, alpha);
}

void CCubeRenderer::DisableDestinationAlpha() {
  GXSetAlphaUpdate(GX_FALSE);
  CGX::SetDstAlpha(GX_TRUE, 0);
}

void CCubeRenderer::DrawScanVisor(float scanTime, float width, float height, const CColor& color,
                                  const CColor& scanColor, const CColor& maskColor,
                                  const CColor* palette, int paletteSize,
                                  const CVector3f& scanRange) {
  if (!mCurrentRGBA6) {
    return;
  }
  static const GXVtxDescList twoTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  static const GXVtxDescList colorTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
  const CTransform4f oldView(CGraphics::GetViewMatrix());
  const rstl::pair< CVector2f, CVector2f > screen = SetViewportOrtho(true, -4096.f, 4096.f);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  void* const spare = CGraphics::GetDolphinSpareBuffer();
  const CViewport& viewport = CGraphics::GetViewport();
  const int vpWidth = viewport.mWidth;
  const int vpHeight = viewport.mHeight;
  GXSetTexCopySrc(viewport.mLeft, viewport.mTop, vpWidth, vpHeight);
  GXSetTexCopyDst(vpWidth, vpHeight, GX_CTF_A8, false);
  GXCopyTex(spare, false);
  CGraphicsPalette scanPalette(kPF_RGB565, paletteSize * 4);
  scanPalette.Lock();
  for (int i = 0; i < paletteSize * 4; ++i) {
    scanPalette.GetPaletteData()[i] = palette[i >> 2].ToRGB565();
  }
  scanPalette.UnLock();
  scanPalette.Load();
  CGraphics::LoadDolphinSpareTexture(vpWidth, vpHeight, GX_TF_C8, GX_TLUT0, nullptr, GX_TEXMAP0);
  mScanRamp.Load(GX_TEXMAP1, CTexture::kCM_Repeat);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXA, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_A8_GT, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  CGX::SetVtxDescv(twoTexDesc);
  CGX::SetNumTexGens(2);
  CGX::SetNumTevStages(2);
  CGX::SetNumChans(0);
  CGX::SetNumIndStages(0);
  SetDepthReadWrite(false, false);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetAlphaUpdate(true);
  CGX::SetDstAlpha(true, 0);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  GXPixModeSync();
  const float rampScale = (screen.second[1] - screen.first[1]) / mScanRamp.GetHeight();
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(screen.first[0], 0.f, screen.first[1]);
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(0.f, rampScale);
  GXPosition3f32(screen.second[0], 0.f, screen.first[1]);
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(rampScale, rampScale);
  GXPosition3f32(screen.first[0], 0.f, screen.second[1]);
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(screen.second[0], 0.f, screen.second[1]);
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(rampScale, 0.f);
  CGX::End();

  {
    const float windowLeft = ((screen.second[0] + screen.first[0]) - width) * 0.5f;
    const float windowRight = (width + (screen.second[0] + screen.first[0])) * 0.5f;
    const float windowTop = ((screen.second[1] + screen.first[1]) - height) * 0.5f;
    const float windowBottom = (height + (screen.second[1] + screen.first[1])) * 0.5f;
    const float screenWidth = screen.second[0] - screen.first[0];
    const float screenHeight = screen.second[1] - screen.first[1];
    const float maskLeft = (windowLeft - screen.first[0]) / screenWidth;
    const float maskRight = (windowRight - screen.first[0]) / screenWidth;
    const float maskTop = (windowTop - screen.first[1]) / screenHeight;
    const float maskBottom = (windowBottom - screen.first[1]) / screenHeight;
    const float rampTop = 2.f / mScanRamp.GetHeight() + windowTop / mScanRamp.GetHeight();
    const float rampBottom = rampTop + (windowBottom - windowTop) / mScanRamp.GetHeight();
    const float rampLeft = 2.f / mScanRamp.GetWidth() + windowLeft / mScanRamp.GetWidth();
    const float rampRight = rampLeft + (windowRight - windowLeft) / mScanRamp.GetWidth();
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(windowLeft, 0.f, windowTop);
    GXTexCoord2f32(maskLeft, maskBottom);
    GXTexCoord2f32(rampLeft, rampBottom);
    GXPosition3f32(windowRight, 0.f, windowTop);
    GXTexCoord2f32(maskRight, maskBottom);
    GXTexCoord2f32(rampRight, rampBottom);
    GXPosition3f32(windowLeft, 0.f, windowBottom);
    GXTexCoord2f32(maskLeft, maskTop);
    GXTexCoord2f32(rampLeft, rampTop);
    GXPosition3f32(windowRight, 0.f, windowBottom);
    GXTexCoord2f32(maskRight, maskTop);
    GXTexCoord2f32(rampRight, rampTop);
    CGX::End();
  }

  CGraphics::LoadDolphinSpareTexture(vpWidth, vpHeight, GX_TF_I8, nullptr, GX_TEXMAP0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGraphics::SetCullMode(kCM_None);
  CGX::SetVtxDescv(colorTexDesc);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumChans(1);
  CGX::SetAlphaCompare(GX_LESS, 8, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  {
    const float windowLeft = ((screen.second[0] + screen.first[0]) - width) * 0.5f;
    const float windowRight = (width + (screen.second[0] + screen.first[0])) * 0.5f;
    const float windowTop = ((screen.second[1] + screen.first[1]) - height) * 0.5f;
    const float windowBottom = (height + (screen.second[1] + screen.first[1])) * 0.5f;
    const float uvLeft = (windowLeft - screen.first[0]) / (screen.second[0] - screen.first[0]);
    const float uvRight = (windowRight - screen.first[0]) / (screen.second[0] - screen.first[0]);
    const float uvTop = (windowTop - screen.first[1]) / (screen.second[1] - screen.first[1]);
    const float uvBottom = (windowBottom - screen.first[1]) / (screen.second[1] - screen.first[1]);
    struct SScanRect {
      uint color;
      float positions[4][2];
      float coords[4][2];
    };
    SScanRect rectangles[5] = {
        {maskColor.GetColor_u32(),
         {{screen.first[0] - 1.f, windowBottom},
          {screen.second[0] + 1.f, windowBottom},
          {screen.first[0] - 1.f, screen.second[1]},
          {screen.second[0] + 1.f, screen.second[1]}},
         {{0.f, uvTop}, {1.f, uvTop}, {0.f, 0.f}, {1.f, 0.f}}},
        {maskColor.GetColor_u32(),
         {{screen.first[0] - 1.f, windowTop},
          {windowLeft, windowTop},
          {screen.first[0] - 1.f, windowBottom},
          {windowLeft, windowBottom}},
         {{0.f, uvBottom}, {uvLeft, uvBottom}, {0.f, uvTop}, {uvLeft, uvTop}}},
        {scanColor.GetColor_u32(),
         {{windowLeft, windowTop},
          {windowRight, windowTop},
          {windowLeft, windowBottom},
          {windowRight, windowBottom}},
         {{uvLeft, uvBottom}, {uvRight, uvBottom}, {uvLeft, uvTop}, {uvRight, uvTop}}},
        {maskColor.GetColor_u32(),
         {{windowRight, windowTop},
          {screen.second[0] + 1.f, windowTop},
          {windowRight, windowBottom},
          {screen.second[0] + 1.f, windowBottom}},
         {{uvRight, uvBottom}, {1.f, uvBottom}, {uvRight, uvTop}, {1.f, uvTop}}},
        {maskColor.GetColor_u32(),
         {{screen.first[0] - 1.f, screen.first[1]},
          {screen.second[0] + 1.f, screen.first[1]},
          {screen.first[0] + 1.f, windowTop},
          {screen.second[0] - 1.f, windowTop}},
         {{0.f, 1.f}, {1.f, 1.f}, {0.f, uvBottom}, {1.f, uvBottom}}},
    };
    for (uint i = 0; i < sizeof(rectangles) / sizeof(rectangles[0]); ++i) {
      const SScanRect& rectangle = rectangles[i];
      const uint inverseColor = ~rectangle.color;
      if (!(inverseColor & 0xfcfcfcfc)) {
        continue;
      }
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      for (int j = 0; j < 4; ++j) {
        GXPosition3f32(rectangle.positions[j][0], 0.f, rectangle.positions[j][1]);
        GXColor1u32(inverseColor);
        GXTexCoord2f32(rectangle.coords[j][0], rectangle.coords[j][1]);
      }
      CGX::End();
    }
  }

  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetNumTexGens(2);
  CGX::SetNumTevStages(2);
  CGX::SetNumChans(1);
  CGraphics::SetCullMode(kCM_None);
  static const GXVtxDescList colorTwoTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(colorTwoTexDesc);
  static const float sweepSpeeds[2] = {0.25f, 0.0625f};
  static const float sweepHeights[2] = {1.f, 0.33f};
  for (int i = 0; i < 2; ++i) {
    CGX::SetAlphaCompare(GX_GREATER, 4, GX_AOP_AND, GX_ALWAYS, 0);
    mScanSweepBar->Load(GX_TEXMAP1, CTexture::kCM_Repeat);
    const float phase = scanTime * sweepSpeeds[i] + scanRange.GetZ();
    const float sweepOffset =
        (phase - static_cast< float >(floor(phase))) * (screen.second[1] - screen.first[1]);
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    const CVector2f sweepMin(screen.first[0] - 1.f, screen.second[1] - sweepOffset);
    const CVector2f sweepMax(screen.second[0] + 1.f,
                             screen.second[1] -
                                 (sweepOffset - sweepHeights[i] * mScanSweepBar->GetHeight()));
    const float topV = (screen.second[1] - sweepMin[1]) / (screen.second[1] - screen.first[1]);
    const float bottomV = (screen.second[1] - sweepMax[1]) / (screen.second[1] - screen.first[1]);
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(sweepMin[0], 0.f, sweepMin[1]);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(0.f, topV);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(sweepMin[0], 0.f, sweepMax[1]);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(0.f, bottomV);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(sweepMax[0], 0.f, sweepMin[1]);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(1.f, topV);
    GXTexCoord2f32(1.f, 0.f);
    GXPosition3f32(sweepMax[0], 0.f, sweepMax[1]);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(1.f, bottomV);
    GXTexCoord2f32(1.f, 1.f);
    CGX::End();
  }
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetProjectionState(oldProjection);
  CGraphics::SetViewPointMatrix(oldView);
  CGraphics::SetCullMode(kCM_Front);
}

void CCubeRenderer::DrawDarkWorldTransition(const CColor& color0, const CColor& color1,
                                            const CColor& color2, const CColor& color3,
                                            const CVector2i& offset, const CVector2i& sourceSize,
                                            const CVector2i& targetSize) {
  static const GXVtxDescList twoTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  static const GXVtxDescList oneTexDesc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  const CVector2i captureOrigin(sourceSize.GetX() & ~1, sourceSize.GetY() & ~1);
  const CVector2i captureSize(targetSize.GetX() & ~1, targetSize.GetY() & ~1);
  const CVector2i motion(-offset.GetX(), offset.GetY());
  mRequestRGBA6 = true;
  CGX::SetTevKColor(GX_KCOLOR0, color0.GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR1, color1.GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR2, color2.GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR3, color3.GetGXColor());
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  CGX::SetTevDirect(GX_TEVSTAGE2);
  CGX::SetNumChans(0);
  const CViewport& viewport = CGraphics::GetViewport();
  const int left = viewport.mLeft;
  const int top = viewport.mTop;
  const int width = viewport.mWidth;
  const int height = viewport.mHeight;
  CGX::SetDstAlpha(false, 255);
  GXSetAlphaUpdate(false);
  const CTransform4f oldView(CGraphics::GetViewMatrix());
  const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
  CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  CGraphics::SetOrtho(0.f, width, 0.f, height, -4096.f, 4096.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  const GXTexMapID spareTexMap = CGraphics::kSpareBufferTexMapID;
  void* noise = reinterpret_cast< void* >(((mRandom.Next() + 31) & ~31) + 0x8000);
  CGraphics::LoadDolphinSpareTexture(width / 2, height, GX_TF_IA8, noise, GX_TEXMAP0);
  GXSetTexCopySrc(left, top, width, height);
  GXSetTexCopyDst(width, height, GX_CTF_R8, false);
  GXCopyTex(CGraphics::GetDolphinSpareBuffer(), false);
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_I8, nullptr, spareTexMap);
  CGraphics::SetDepthWriteMode(false, kE_Always, false);
  CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, spareTexMap, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTevStages(2);
  CGX::SetNumTexGens(2);
  CGX::SetVtxDescv(twoTexDesc);
  GXPixModeSync();
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 0.5f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(0.f, 0.5f, height);
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(width, 0.5f, height);
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(width, 0.5f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  GXSetTexCopySrc(left + captureOrigin.GetX(), top + captureOrigin.GetY(), captureSize.GetX(),
                  captureSize.GetY());
  GXSetTexCopyDst(captureSize.GetX(), captureSize.GetY(), GX_CTF_A8, false);
  GXCopyTex(CGraphics::GetDolphinSpareBuffer(), false);
  mDarkLightWorldPalette->Load();
  CGraphics::LoadDolphinSpareTexture(captureSize.GetX(), captureSize.GetY(), GX_TF_C8, GX_TLUT0,
                                     nullptr, spareTexMap);
  CGX::SetVtxDescv(oneTexDesc);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, IsRGBA6Current() ? GX_CC_TEXC : GX_CC_ZERO,
                     GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K2);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, spareTexMap, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  GXPixModeSync();
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(captureOrigin.GetX(), 0.5f, captureOrigin.GetY());
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(captureOrigin.GetX(), 0.5f, captureOrigin.GetY() + captureSize.GetY());
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(captureOrigin.GetX() + captureSize.GetX(), 0.5f,
                 captureOrigin.GetY() + captureSize.GetY());
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(captureOrigin.GetX() + captureSize.GetX(), 0.5f, captureOrigin.GetY());
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  GXSetAlphaUpdate(true);
  GXSetColorUpdate(false);
  const GXColor white = {255, 255, 255, 255};
  GXSetTevColor(GX_TEVREG0, white);
  CGraphics::LoadDolphinSpareTexture(captureSize.GetX(), captureSize.GetY(), GX_TF_I8, nullptr,
                                     CGraphics::kSpareBufferTexMapID);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_KONST, GX_CA_TEXA);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K3_A);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetVtxDescv(oneTexDesc);
  const CVector2i movedOrigin = motion + captureOrigin;
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(movedOrigin.GetX(), 0.5f, movedOrigin.GetY());
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(movedOrigin.GetX(), 0.5f, movedOrigin.GetY() + captureSize.GetY());
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(movedOrigin.GetX() + captureSize.GetX(), 0.5f,
                 movedOrigin.GetY() + captureSize.GetY());
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(movedOrigin.GetX() + captureSize.GetX(), 0.5f, movedOrigin.GetY());
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  if (motion != CVector2i(0, 0)) {
    GXSetAlphaUpdate(true);
    GXSetColorUpdate(false);
    CGX::ResetVtxDescv();
    CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    CGX::SetNumTevStages(1);
    CGX::SetNumTexGens(1);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    const float distanceX = motion.GetX();
    const float distanceY = motion.GetY();
    const CVector2i end = captureOrigin + captureSize;
    if (!close_enough(distanceX, 0.f)) {
      CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
      if (distanceX > 0.f) {
        GXPosition3f32(captureOrigin.GetX(), 0.5f, captureOrigin.GetY());
        GXPosition3f32(captureOrigin.GetX(), 0.5f, end.GetY());
        GXPosition3f32(float(captureOrigin.GetX()) + distanceX, 0.5f, end.GetY());
        GXPosition3f32(float(captureOrigin.GetX()) + distanceX, 0.5f, captureOrigin.GetY());
      } else {
        GXPosition3f32(float(end.GetX()) + distanceX, 0.5f, captureOrigin.GetY());
        GXPosition3f32(float(end.GetX()) + distanceX, 0.5f, end.GetY());
        GXPosition3f32(end.GetX(), 0.5f, end.GetY());
        GXPosition3f32(end.GetX(), 0.5f, captureOrigin.GetY());
      }
      CGX::End();
    }
    if (!close_enough(distanceY, 0.f)) {
      CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
      if (distanceY > 0.f) {
        GXPosition3f32(captureOrigin.GetX(), 0.5f, captureOrigin.GetY());
        GXPosition3f32(captureOrigin.GetX(), 0.5f, float(captureOrigin.GetY()) + distanceY);
        GXPosition3f32(end.GetX(), 0.5f, float(captureOrigin.GetY()) + distanceY);
        GXPosition3f32(end.GetX(), 0.5f, captureOrigin.GetY());
      } else {
        GXPosition3f32(captureOrigin.GetX(), 0.5f, float(end.GetY()) + distanceY);
        GXPosition3f32(captureOrigin.GetX(), 0.5f, end.GetY());
        GXPosition3f32(end.GetX(), 0.5f, end.GetY());
        GXPosition3f32(end.GetX(), 0.5f, float(end.GetY()) + distanceY);
      }
      CGX::End();
    }
  }
  CGraphics::SetProjectionState(oldProjection);
  CGraphics::SetViewPointMatrix(oldView);
  GXSetAlphaUpdate(false);
  GXSetColorUpdate(true);
  mPreserveDestinationAlpha = true;
}

void CCubeRenderer::DrawDarkWorldFilter(float amount) {
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  mRequestRGBA6 = true;
  CRandom16 random(99);
  void* noise = reinterpret_cast< void* >(((random.Next() + 31) & ~31) + 0x8000);
  const CViewport& viewport = CGraphics::GetViewport();
  const int left = viewport.mLeft;
  const int top = viewport.mTop;
  const int width = viewport.mWidth;
  const int height = viewport.mHeight;
  void* const spare = CGraphics::GetDolphinSpareBuffer();
  const CTransform4f oldView(CGraphics::GetViewMatrix());
  const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
  CGraphics::SetOrtho(0.f, width, 0.f, height, 0.f, 1000.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  CGX::SetVtxDescv(desc);
  sRenderer->mDarkWorldCloud->Load(GX_TEXMAP0, CTexture::kCM_Mirror);
  const float wave = 0.25f * CMath::FastSinR(1.5f * CGraphics::GetSecondsMod900() + 2.f) + 0.75f;
  const float amplitude = (0.5f * amount) * wave;
  const float seconds = CGraphics::GetSecondsMod900();
  const float sine = amplitude * CMath::FastSinR(seconds);
  const float cosine = amplitude * CMath::FastCosR(seconds);
  const float matrix[2][3] = {{cosine, -sine, 0.f}, {0.75f * sine, 0.75f * cosine, 0.f}};
  GXSetIndTexMtx(GX_ITM_0, matrix, -5);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP0);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_8_8);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetNumTevStages(1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumTexGens(2);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_INVDSTALPHA, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetZMode(false, GX_ALWAYS, false);
  const int alignedTop = top & ~1;
  const int copyHeight = (height - alignedTop + 1) & ~1;
  GXTexObj texture;
  GXInitTexObj(&texture, noise, 64, 64, GX_TF_IA4, GX_REPEAT, GX_REPEAT, false);
  GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  GXLoadTexObj(&texture, GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP0);
  GXSetTexCopySrc(left, alignedTop, width, copyHeight);
  GXSetTexCopyDst(width / 2, copyHeight / 2, GX_TF_RGBA8, true);
  GXCopyTex(spare, false);
  GXInitTexObj(&texture, spare, width / 2, copyHeight / 2, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  GXLoadTexObj(&texture, CGraphics::kSpareBufferTexMapID);
  CTexture::InvalidateTexmap(CGraphics::kSpareBufferTexMapID);
  GXInvalidateTexRegion(CGraphics::GetSpareTextureRegion());
  GXPixModeSync();
  const float time = CGraphics::GetSecondsMod900();
  const float scrollX = CMath::FastCosR(time / 15.f);
  const float scrollY = CMath::FastSinR(time / 20.f);
  const int topOffset = alignedTop - top;
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
  GXPosition3f32(0.f, 995.f, topOffset);
  GXTexCoord2f32(0.f, 0.f);
  GXTexCoord2f32(scrollX, scrollY);
  GXPosition3f32(0.f, 995.f, topOffset + copyHeight);
  GXTexCoord2f32(0.f, 1.f);
  GXTexCoord2f32(scrollX, 1.f + scrollY);
  GXPosition3f32(width, 995.f, topOffset + copyHeight);
  GXTexCoord2f32(1.f, 1.f);
  GXTexCoord2f32(1.f + scrollX, 1.f + scrollY);
  GXPosition3f32(width, 995.f, topOffset);
  GXTexCoord2f32(1.f, 0.f);
  GXTexCoord2f32(1.f + scrollX, scrollY);
  CGX::End();
  CGraphics::SetProjectionState(oldProjection);
  CGraphics::SetViewPointMatrix(oldView);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  SetupCGraphicsStates();
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
}

uint CCubeRenderer::PackLightSet(const uchar* lights, float ambient) {
  const uint packed = (lights[0] & 63) | ((lights[1] & 63) << 6) | ((lights[2] & 63) << 12) |
                      ((lights[3] & 63) << 18);
  const uint level = CMath::ClampI(0, static_cast< int >(63.f * ambient), 63);
  return packed | (level << 24);
}

void CCubeRenderer::UnpackLightSet(uint lightSet, uchar* lights, float* ambient,
                                   uchar* quantizedAmbient) {
  for (int i = 0; i < 4; ++i) {
    lights[i] = (lightSet >> (6 * i)) & 63;
  }

  const uchar level = (lightSet >> 24) & 63;
  if (ambient) {
    *ambient = CCast::ToReal32(level) * (1.f / 63.f);
  }
  if (quantizedAmbient) {
    *quantizedAmbient = level;
  }
}

void CCubeRenderer::PrepareWorldRendering(
    const rstl::pair< int, const CPVSVisSet* >* pvsSets, int pvsCount,
    const CFrustumPlanes& frustum,
    const rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 >* areaFrusta,
    const rstl::vector< CLight >& lights, const rstl::pair< int, float >* ambientLights,
    int ambientLightCount) {
  if (lights.size() != mDynamicLights.size()) {
    mDynamicLights = rstl::vector< CLight >();
  }
  mFrustumPlanes = frustum;
  mDynamicLights.reserve(lights.size());
  mDynamicLights.clear();
  for (int i = 0; i < lights.size(); ++i) {
    const CLight& light = lights[i];
    if (light.GetType() == kLT_Hard) {
      continue;
    }
    if (light.GetType() == kLT_Point) {
      if (!mFrustumPlanes.SphereInFrustumPlanes(
              CSphere(light.GetPosition(), light.GetRadius() * 2.f))) {
        continue;
      }
    }
    mDynamicLights.push_back_unsafe(light);
  }
  mLightSets.clear();
  mLightSets.push_back(0x00ffffff);
  const CPVSVisSet defaultPVS(kVSS_OutOfBounds);
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    const CPVSVisSet* pvs = &defaultPVS;
    if (mPVSState != 0) {
      for (int i = 0; i < pvsCount; ++i) {
        if (pvsSets[i].first == area->mAreaId) {
          pvs = pvsSets[i].second;
          break;
        }
      }
    }
    const CAreaRenderOctTree* octTree = area->mOctTree;
    int wordCount = octTree->GetBitmapWordCount();
    rstl::vector< uint > overlaps;
    overlaps.resize(wordCount * mDynamicLights.size(), 0);
    for (int i = 0; i < mDynamicLights.size(); ++i) {
      const CLight& light = mDynamicLights[i];
      const float radius = light.GetRadius();
      const CVector3f extent(radius, radius, radius);
      const CAABox lightBounds(light.GetPosition() - extent, light.GetPosition() + extent);
      octTree->FindOverlappingModels(overlaps.data() + i * wordCount, lightBounds);
    }
    rstl::reserved_vector< float, 32 > ambient;
    const rstl::vector< uint >* ambientIds = area->mAmbientLightIds;
    if (ambientIds->size() <= 32) {
      ambient.resize(ambientIds->size(), 0.f);
      for (int i = 0; i < ambientIds->size(); ++i) {
        const uint id = (*ambientIds)[i];
        for (int j = 0; j < ambientLightCount; ++j) {
          if (id == ambientLights[j].first) {
            ambient[i] = ambientLights[j].second;
            break;
          }
        }
      }
    }
    const rstl::vector< SAreaSurface >* surfaces = area->mSurfaces;
    const rstl::vector< signed char >* ambientIndices = area->mAmbientLightIndices;
    const CFrustumPlanes* areaFrustum = nullptr;
    if (areaFrusta) {
      for (rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 >::const_iterator it =
               areaFrusta->begin();
           it != areaFrusta->end(); ++it) {
        if (it->first == area->mAreaId) {
          areaFrustum = &it->second;
          break;
        }
      }
    }
    rstl::vector< uchar >& lightSetIndices = area->mLightSetIndices;
    lightSetIndices.resize(surfaces->size() - 1, static_cast< uchar >(255));
    if (mDynamicLights.size() != 0 || ambient.size() != 0) {
      for (int i = 1; i < surfaces->size(); ++i) {
        const uint surfaceIndex = i - 1;
        const CAABox& bounds = (*surfaces)[i].mBounds;
        if ((pvs->GetVisible(surfaceIndex) != kVSS_EndOfTree ? true : false) &&
            mFrustumPlanes.BoxInFrustumPlanes(bounds) &&
            (!areaFrustum || areaFrustum->BoxInFrustumPlanes(bounds))) {
          float ambientLevel = 0.f;
          uchar lightIndices[4];
          EvaluateModelLights(lightIndices, bounds,
                              overlaps.size() != 0 ? overlaps.data() : nullptr, wordCount,
                              surfaceIndex);
          if (ambientIndices->size() != 0) {
            const signed char& entry = (*ambientIndices)[i - 1];
            const int ambientIndex = entry;
            if (ambientIndex >= 0) {
              ambientLevel = ambient[ambientIndex];
            }
          }
          const uint lightSet = PackLightSet(lightIndices, ambientLevel);
          const uchar lightSetIndex = FindOrAddLightSet(lightSet);
          lightSetIndices[surfaceIndex] = lightSetIndex;
        } else {
          lightSetIndices[surfaceIndex] = 255;
        }
      }
    } else {
      for (int i = 1; i < surfaces->size(); ++i) {
        const uint surfaceIndex = i - 1;
        const CAABox& bounds = (*surfaces)[i].mBounds;
        if ((pvs->GetVisible(surfaceIndex) != kVSS_EndOfTree ? true : false) &&
            mFrustumPlanes.BoxInFrustumPlanes(bounds) &&
            (!areaFrustum || areaFrustum->BoxInFrustumPlanes(bounds))) {
          lightSetIndices[surfaceIndex] = 0;
        } else {
          lightSetIndices[surfaceIndex] = 255;
        }
      }
    }
    if (mPVSState == 2) {
      const rstl::vector< SAreaSurface >* pvsSurfaces = area->mSurfaces;
      for (int i = 1; i < pvsSurfaces->size(); ++i) {
        uchar& lightSet = lightSetIndices[i - 1];
        if (lightSet == 255) {
          lightSet = 0;
        } else {
          lightSet = 255;
        }
      }
    }
  }
}

void CCubeRenderer::DrawUnsortedGeometry(int areaId) {
  DrawGeometry< false, false >(areaId, "DrawUnsortedGeometry", SGeometryTag(), SGeometryTag(),
                               SGeometryTag(), SGeometryTag(), SGeometryTag());
}

void CCubeRenderer::DrawUnsortedGeometryAlpha(int areaId) {
  DrawGeometry< false, true >(areaId, "DrawGeometryScan", SGeometryTag(), SGeometryTag(),
                              SGeometryTag(), SGeometryTag(), SGeometryTag());
}

void CCubeRenderer::DrawSpecialGeometry(int areaId) {
  DrawGeometry< true, false >(areaId, "DrawTranslastGeometry", SGeometryTag(), SGeometryTag(),
                              SGeometryTag(), SGeometryTag(), SGeometryTag());
}

void CCubeRenderer::DrawSpecialGeometryAlpha(int areaId) {
  DrawGeometry< true, true >(areaId, "DrawGeometryScanTranslast", SGeometryTag(), SGeometryTag(),
                             SGeometryTag(), SGeometryTag(), SGeometryTag());
}

void CCubeRenderer::DrawAreaModel(int areaId, int modelId, const CModelFlags& flags) {
  rstl::list< CAreaListItem >::iterator area = FindArea(areaId);
  if (area == mAreaListItems.end()) {
    return;
  }
  const SAreaSurface& areaSurface = (*area->mSurfaces)[modelId + 1];
  const int modelIndex = areaSurface.mModelIndex;
  const int groupIndex = areaSurface.mSurfaceGroupIndex;
  const CMetroidModelInstance& instance = (*area->mGeometry)[modelIndex];
  const CCubeModel* model = (*area->mModels)[modelIndex].get();
  CCubeMaterial::ResetCachedMaterials();
  model->SetArraysCurrent();
  const CMetroidModelInstance::CSurfaceGroups groups = instance.GetSurfaceGroups();
  const ushort count = groups.GetSurfaceCount(groupIndex);
  const ushort* indices = groups.GetSurfaceIndices(groupIndex);
  for (ushort i = 0; i < count; ++i) {
    const CCubeSurface surface(instance.GetSurfaces()[indices[i]]);
    model->DrawSurface(surface, flags);
  }
}

CAABox CCubeRenderer::GetAreaModelBounds(int areaId, int modelId) {
  rstl::list< CAreaListItem >::const_iterator area =
      static_cast< const CCubeRenderer* >(this)->FindArea(areaId);
  if (area != mAreaListItems.end()) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    const SAreaSurface& areaSurface = (*area->mSurfaces)[modelId + 1];
    const int groupIndex = areaSurface.mSurfaceGroupIndex;
    const CMetroidModelInstance& model = (*area->mGeometry)[areaSurface.mModelIndex];
    const CMetroidModelInstance::CSurfaceGroups groups = model.GetSurfaceGroups();
    const ushort count = groups.GetSurfaceCount(groupIndex);
    const ushort* indices = groups.GetSurfaceIndices(groupIndex);
    for (ushort i = 0; i < count; ++i) {
      const CCubeSurface surface(model.GetSurfaces()[indices[i]]);
      const CAABox& surfaceBounds = surface.GetBounds();
      bounds.AccumulateBounds(surfaceBounds.GetMinPoint());
      bounds.AccumulateBounds(surfaceBounds.GetMaxPoint());
    }
    return bounds;
  } else {
    return CAABox::Identity();
  }
}

void CCubeRenderer::DrawVisibleAreaGeometry(int areaId, const CPVSVisSet& pvs,
                                            const CFrustumPlanes& frustum, const CAABox& bounds) {
  rstl::list< CAreaListItem >::iterator area = FindArea(areaId);
  if (area != mAreaListItems.end()) {
    const rstl::vector< CMetroidModelInstance >* geometry = area->GetModelVector();
    const rstl::vector< rstl::auto_ptr< CCubeModel > >* models = area->GetModelList();
    const rstl::vector< SAreaSurface >* surfaces = area->GetSurfaces();
    for (int i = 0; i < surfaces->size() - 1; ++i) {
      if (pvs.GetVisible(i) != kVSS_EndOfTree) {
        const SAreaSurface& areaSurface = (*surfaces)[i + 1];
        if (areaSurface.mBounds.DoBoundsOverlap(bounds) &&
            frustum.BoxInFrustumPlanes(areaSurface.mBounds)) {
          const int modelIndex = areaSurface.mModelIndex;
          const int groupIndex = areaSurface.mSurfaceGroupIndex;
          if (modelIndex != -1 && groupIndex != -1) {
            const CCubeModel* model = (*models)[modelIndex].get();
            const CMetroidModelInstance& instance = (*geometry)[modelIndex];
            const CMetroidModelInstance::CSurfaceGroups groups = instance.GetSurfaceGroups();
            model->SetArraysCurrent();
            const ushort count = groups.GetSurfaceCount(groupIndex);
            const ushort* indices = groups.GetSurfaceIndices(groupIndex);
            for (ushort j = 0; j < count; ++j) {
              const CCubeSurface surface(instance.GetSurfaces()[indices[j]]);
              const CAABox surfaceBounds = surface.GetBounds();
              if (bounds.DoBoundsOverlap(surfaceBounds) &&
                  frustum.BoxInFrustumPlanes(surfaceBounds)) {
                model->DrawSurfaceFlat(surface);
              }
            }
          }
        }
      }
    }
  }
}

void CCubeRenderer::ActivateLightsForModel(uint lightSet) {
  uchar lights[4];
  uchar ambient;
  uint count = 0;
  UnpackLightSet(lightSet, lights, nullptr, &ambient);
  for (; count < 4; ++count) {
    if (lights[count] == 63) {
      break;
    }
    CGraphics::LoadLight(static_cast< ERglLight >(count), mDynamicLights[lights[count]]);
  }
  const uchar lightState = (1 << count) - 1;
  const GXColor white = {255, 255, 255, 255};
  GXColor ambientColor;
  const uchar amb = ambient;
  ambientColor.r = amb;
  ambientColor.g = amb;
  ambientColor.b = amb;
  ambientColor.a = 255;
  CGX::SetChanAmbColor(CGX::Channel0, ambientColor);
  if (lightState != 0) {
    CGraphics::SetLightState(lightState);
    CGX::SetChanMatColor(CGX::Channel0, white);
  } else {
    CGraphics::DisableAllLights();
    const GXColor color = CGX::GetChanAmbColor(CGX::Channel0);
    CGX::SetChanMatColor(CGX::Channel0, color);
  }
  CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  const GXColor channelColor = CGX::GetChanAmbColor(CGX::Channel1);
  CGX::SetChanMatColor(CGX::Channel1, channelColor);
}

rstl::list< CCubeRenderer::CAreaListItem >::iterator CCubeRenderer::FindArea(int areaId) {
  for (rstl::list< CAreaListItem >::iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    if (area->mAreaId == areaId) {
      return area;
    }
  }
  return mAreaListItems.end();
}

rstl::list< CCubeRenderer::CAreaListItem >::const_iterator
CCubeRenderer::FindArea(int areaId) const {
  return const_cast< CCubeRenderer* >(this)->FindArea(areaId);
}

void CCubeRenderer::EnablePVS(int areaId, const rstl::vector< rstl::pair< int, int > >& visible) {
  DisablePVS(areaId);
  rstl::list< CAreaListItem >::iterator area = FindArea(areaId);
  if (area != mAreaListItems.end()) {
    for (rstl::vector< rstl::pair< int, int > >::const_iterator it = visible.begin();
         it != visible.end(); ++it) {
      area->mPVSAlpha[it->first] = it->second << 2;
    }
  }
}

void CCubeRenderer::DisablePVS(int areaId) {
  rstl::list< CAreaListItem >::iterator area = FindArea(areaId);
  if (area != mAreaListItems.end()) {
    CBasics::ZeroMemory(area->mPVSAlpha.data(), area->mPVSAlpha.size());
  }
}

void CCubeRenderer::DrawModelWithTextureMask(const SModelRenderData& model, const CTexture& texture,
                                             const CVector3f& origin, const CColor& color,
                                             float scale) {
  const CModelFlags flags = CModelFlags::AlphaBlended(color);
  const CCubeModel& instance = *model.mModel->GetModelInstance();
  CCubeMaterial::KillCachedViewDepState();
  CCubeMaterial::ResetCachedMaterials();
  const CCubeMaterial material = instance.GetMaterialByIndex(0);
  const CCubeSurface surface(instance.GetModelInstance().Surfaces().front());
  material.SetCurrent(flags, surface, instance);

  const CAABox& bounds = instance.GetBoundingBox();
  const float minY = bounds.GetMinPoint().GetY();
  const float minZ = bounds.GetMinPoint().GetZ();
  const float inverseY = 1.f / (bounds.GetMaxPoint().GetY() - minY);
  const float inverseZ = 1.f / (bounds.GetMaxPoint().GetZ() - minZ);
  const float matrix[2][4] = {
      {0.f, scale * inverseY, 0.f,
       -(scale * (minY * inverseY + inverseY * (origin.GetY() - minY)) - 0.5f)},
      {0.f, 0.f, scale * inverseZ,
       -(scale * (minZ * inverseZ + inverseZ * (origin.GetZ() - minZ)) - 0.5f)},
  };
  GXLoadTexMtxImm(matrix, GX_TEXMTX7, GX_MTX2x4);
  texture.Load(GX_TEXMAP6, CTexture::kCM_Clamp);
  const GXTevStageID stage = static_cast< GXTevStageID >(CGX::GetNumTevStages() - 1);
  const GXTexCoordID coord = static_cast< GXTexCoordID >(CGX::GetNumTexGens());
  CGX::SetTexCoordGen(coord, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX7, false, GX_PTIDENTITY);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_TEXC);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(stage);
  CGX::SetTevOrder(stage, coord, GX_TEXMAP6, GX_COLOR_NULL);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetNumTexGens(coord + 1);
  model.DrawFlat(flags, true, true);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
}

void CCubeRenderer::CopyTextureRegion(void* dest, int format, int left, int top, int width,
                                      int height) {
  ushort copyWidth = static_cast< uint >(width) < 8 ? 8 : width & 0xfffe;
  ushort copyHeight = static_cast< uint >(height) < 8 ? 8 : height & 0xfffe;
  GXSetTexCopySrc(left & 0xfffe, top & 0xfffe, copyWidth, copyHeight);
  switch (format) {
  case 0:
    GXSetTexCopyDst(copyWidth, copyHeight, GX_CTF_A8, false);
    break;
  case 1:
    GXSetTexCopyDst(copyWidth, copyHeight, GX_TF_Z16, false);
    break;
  case 2:
    GXSetTexCopyDst(copyWidth, copyHeight, GX_CTF_Z8L, false);
    break;
  default:
    return;
  }
  GXCopyTex(dest, false);
}

// The full-strength layer keeps its multiply by one: the factor only folds when written inline.
static inline void SetCloudLayerColor(GXTevKColorID id, const CColor& from, const CColor& to,
                                      float opacity, float factor) {
  CGX::SetTevKColor(id, CColor::Lerp(from, to, opacity * factor).GetGXColor());
}

void CCubeRenderer::DrawDarkWorldCloud(float time, const CVector3f& scale, const CColor& color) {
  static const GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_TEX0, GX_DIRECT}, {GX_VA_TEX1, GX_DIRECT},
      {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  GXSetAlphaUpdate(false);
  const CTransform4f oldView(CGraphics::GetViewMatrix());
  const CGraphics::CProjectionState oldProjection(CGraphics::GetProjectionState());
  const rstl::pair< CVector2f, CVector2f > screen = SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = screen.first;
  const CVector2f& rb = screen.second;
  CGX::SetVtxDescv(vtxDesc);
  mDarkWorldCloud->Load(GX_TEXMAP0, CTexture::kCM_Mirror);

  const float remaining = 1.f - scale.GetY();
  const CColor transparent(static_cast< uchar >(0), 0, 0, 0);
  const CColor fullColor = color;
  time /= 3.f;
  const float growth = remaining / 3.f;
  const float scrollX = 2.f * scale.GetX();
  const float scrollY = 2.f * scale.GetZ();
  const float extent2 = 1.f / 3.f + growth;
  const float extent1 = 2.f / 3.f + growth;
  const float extent0 = 1.f + growth;
  const float low0 = 0.5f * -extent0 + 0.5f;
  const float low1 = 0.5f * -extent1 + 0.5f;
  const float low2 = 0.5f * -extent2 + 0.5f;
  const float high0 = 0.5f * extent0 + 0.5f;
  const float high1 = 0.5f * extent1 + 0.5f;
  const float high2 = 0.5f * extent2 + 0.5f;
  SetCloudLayerColor(GX_KCOLOR0, transparent, fullColor, time, scale.GetY());
  SetCloudLayerColor(GX_KCOLOR1, transparent, fullColor, time, 1.f);
  SetCloudLayerColor(GX_KCOLOR2, transparent, fullColor, time, remaining);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K2);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K2_A);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE2);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  CGX::SetTevDirect(GX_TEVSTAGE2);
  CGX::SetNumTevStages(3);
  CGX::SetNumTexGens(3);
  CGX::SetNumChans(0);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetZMode(false, GX_ALWAYS, false);

  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(lt.GetX(), 0.f, lt.GetY());
  GXTexCoord2f32(low0 + scrollX, low0 + scrollY);
  GXTexCoord2f32(low1 + scrollX, low1 + scrollY);
  GXTexCoord2f32(low2 + scrollX, low2 + scrollY);
  GXPosition3f32(rb.GetX(), 0.f, lt.GetY());
  GXTexCoord2f32(high0 + scrollX, low0 + scrollY);
  GXTexCoord2f32(high1 + scrollX, low1 + scrollY);
  GXTexCoord2f32(high2 + scrollX, low2 + scrollY);
  GXPosition3f32(lt.GetX(), 0.f, rb.GetY());
  GXTexCoord2f32(low0 + scrollX, high0 + scrollY);
  GXTexCoord2f32(low1 + scrollX, high1 + scrollY);
  GXTexCoord2f32(low2 + scrollX, high2 + scrollY);
  GXPosition3f32(rb.GetX(), 0.f, rb.GetY());
  GXTexCoord2f32(high0 + scrollX, high0 + scrollY);
  GXTexCoord2f32(high1 + scrollX, high1 + scrollY);
  GXTexCoord2f32(high2 + scrollX, high2 + scrollY);
  CGX::End();
  CGraphics::SetProjectionState(oldProjection);
  CGraphics::SetViewPointMatrix(oldView);
}

void CCubeRenderer::DrawModelNoise(const SModelRenderData& model, const CColor& color,
                                   bool additive) {
  void* noise = reinterpret_cast< void* >(((mRandom.Next() + 31) & ~31) + 0x8000);
  CGraphics::LoadDolphinSpareTexture(96, 96, GX_TF_IA4, noise, CGraphics::kSpareBufferTexMapID);
  if (additive) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  }
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetZMode(true, GX_LEQUAL, false);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  if (additive) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  } else {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_KONST);
  }
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumIndStages(0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  model.DrawFlat(CModelFlags(CModelFlags::kT_Opaque, 1.f), true, false);
}

const CAABox& SModelRenderData::GetAABB() const {
  return mModel ? mModel->GetAABB() : mSkinnedModel->GetModel()->GetAABB();
}

void SModelRenderData::DrawFlat(const CModelFlags& flags, bool unsorted, bool sorted) const {
  if (!unsorted && !sorted) {
    return;
  }
  if (mModel) {
    CModel::EDrawFlatFlags selection;
    if (unsorted && sorted) {
      selection = CModel::kDF_All;
    } else if (!unsorted) {
      selection = CModel::kDF_Sorted;
    } else {
      selection = CModel::kDF_Unsorted;
    }
    mModel->PreDrawModel(flags);
    mModel->DolphinDrawFlat(selection);
  } else if (mSkinnedModel) {
    uint drawFlags = CSkinnedModel::kDF_Flat;
    if (unsorted) {
      drawFlags |= CSkinnedModel::kDF_Unsorted;
    }
    if (sorted) {
      drawFlags |= CSkinnedModel::kDF_Sorted;
    }
    if (mPose) {
      mSkinnedModel->DolphinDrawWithFlags(mPose, drawFlags, flags);
    } else if (mWorkspace) {
      mSkinnedModel->DolphinDrawFromWorkspace(*mWorkspace, drawFlags, flags);
    }
  }
}

uint GetRendererWorkspaceSize() { return Buckets::GetWorkspaceSize(); }

void SetRendererWorkspace(void* workspace) { Buckets::Init(workspace); }

void ReleaseRendererWorkspace() { Buckets::Shutdown(); }

void CCubeRenderer::DrawString(const char* text, int x, int y) {
  mFont.DrawString(text, x, y, CColor::White());
}
