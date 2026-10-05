#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/SModelRenderData.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CDrawablePlaneObject.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "WorldFormat/CMetroidModelInstance.hpp"
#include "rstl/math.hpp"

#include "dolphin/os/OSCache.h"
#include <string.h>

// NonMatching scaffold: unfinished rendering passes are explicitly marked below.
CCubeRenderer* CCubeRenderer::sRenderer = nullptr;
IWeaponRenderer* IWeaponRenderer::sWeaponRenderer = nullptr;

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
void CCubeRenderer::DrawGeometry(int areaId) {
  // TODO: traverse visible surfaces, selecting the material and alpha pass.
}

uint Buckets::GetWorkspaceSize() {
  return sizeof(DrawableList) + sizeof(BucketList) + sizeof(PlaneList) + sizeof(PlaneBucketList) +
         4;
}

void Buckets::Init(void* workspace) {
  uchar* data = reinterpret_cast< uchar* >((reinterpret_cast< uint >(workspace) + 3) & ~3);
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
  if (sData->size() == sData->capacity()) {
    return;
  }

  const float distance = plane.GetHeight(pos);
  sData->push_back(CDrawable(type, extraSort, distance, bounds, data, alpha));
  sMinMaxDistance.first = rstl::min_val(distance, sMinMaxDistance.first);
  sMinMaxDistance.second = rstl::max_val(distance, sMinMaxDistance.second);
}

void Buckets::InsertPlaneObject(float closeDistance, float farDistance, const CAABox& bounds,
                                bool invertTest, const CPlane& plane, bool zOnly,
                                EDrawableType type, const void* data) {
  if (sPlaneObjectData->size() == sPlaneObjectData->capacity()) {
    return;
  }

  sPlaneObjectData->push_back(CDrawablePlaneObject(type, closeDistance, farDistance, bounds,
                                                   invertTest, plane, zOnly, data));
}

void Buckets::Sort() {
  // TODO: depth buckets, plane intersections and per-bucket sorting.
}

void Buckets::Clear() {
  sData->clear();
  sBucketIndex.clear();
  sPlaneObjectData->clear();
  sPlaneObjectBucket->clear();
  for (int i = 0; i < sBuckets->size(); ++i) {
    (*sBuckets)[i].clear();
  }
  sMinMaxDistance = skWorstMinMaxDistance;
}

CCubeRenderer::SModelSurfaceOrder::SModelSurfaceOrder(const CCubeModel& model)
: mSurfaceIndices(), mOpaqueEnd(0), mSortedEnd(0), mTotalCount(0) {
  // TODO: partition the model's surface indices by material flags.
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
  mPVSAlpha.resize(surfaces->size() - 1, 0);
  mModelSurfaceOrders.reserve(mModels->size());
  for (int i = 0; i < mModels->size(); ++i) {
    mModelSurfaceOrders.push_back(SModelSurfaceOrder(*(*mModels)[i]));
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
, mViewPlane(0.f, CUnitVector3f(CVector3f::Forward(), CUnitVector3f::kN_Yes))
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
  memcpy(result->Lock(), palette->GetPaletteData(), result->GetEntryCount() * sizeof(ushort));
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
  for (int y = 0; y < height; ++y) {
    const int start = y * width;
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
      CCubeModel::MakeTexturesFromMats(geometry->front().GetMaterialPointer(), *textures,
                                     mObjStore, false);
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
  mViewPlane = CPlane(xf.GetTranslation(), CUnitVector3f(xf.GetForward(), CUnitVector3f::kN_No));
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
  if (mReflectionAge < 2) {
    ++mReflectionAge;
  } else {
    mReflectionTex = nullptr;
  }
  CGraphics::SetClearColor(CColor(static_cast< uchar >(0), 0, 0, 0));
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
  const CVector3f closest = bounds.ClosestPointAlongVector(mViewPlane.GetNormal());
  const float closeDistance = mViewPlane.GetHeight(closest);
  const CVector3f furthest = bounds.FurthestPointAlongVector(mViewPlane.GetNormal());
  const float farDistance = mViewPlane.GetHeight(furthest);
  if (closeDistance >= 0.f || farDistance >= 0.f) {
    const bool zOnly = plane.GetNormal() == sOptimalPlane;
    const CVector3f viewPosition = CGraphics::GetViewMatrix().GetTranslation();
    const bool invertTest = zOnly ? !(viewPosition.GetZ() < plane.GetConstant())
                                 : !(plane.GetHeight(viewPosition) < 0.f);
    Buckets::InsertPlaneObject(closeDistance, farDistance, bounds, invertTest, plane, zOnly,
                              static_cast< EDrawableType >(type + kDT_Actor), obj);
  }
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

void CCubeRenderer::SetupRendererStates(bool depthWrite) {
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
  const uint surface = (static_cast< uint >(static_cast< ushort >(modelIndex)) << 16) | surfaceIndex;
  const CVector3f closest = bounds.ClosestPointAlongVector(mViewPlane.GetNormal());
  Buckets::Insert(closest, bounds, kDT_WorldSurface, reinterpret_cast< const void* >(surface),
                  mViewPlane, blend == 0x50004 ? 1 : 0, false);
}

void CCubeRenderer::DrawRenderBucketsDebug() {}

void CCubeRenderer::RenderBucketItems(const CAreaListItem* area, bool alpha) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawSortedGeometry(int mode, int areaId) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::EvaluateModelLights(uchar* lights, const CAABox& bounds, const uint* overlaps,
                                        int wordCount, uint modelIndex) {
  // TODO: reconstruct this rendering pass.
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

void CCubeRenderer::BeginLines(int count) { BeginPrimitive(kPT_Lines, count); }

void CCubeRenderer::BeginLineStrip(int count) { BeginPrimitive(kPT_LineStrip, count); }

void CCubeRenderer::BeginTriangles(int count) { BeginPrimitive(kPT_Triangles, count); }

void CCubeRenderer::BeginTriangleStrip(int count) { BeginPrimitive(kPT_TriangleStrip, count); }

void CCubeRenderer::BeginTriangleFan(int count) { BeginPrimitive(kPT_TriangleFan, count); }

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
  CGraphics::SetPerspective(fovy, width / height, znear, zfar);
}

void CCubeRenderer::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  CGraphics::SetPerspective(fovy, aspect, znear, zfar);
}

rstl::pair< CVector2f, CVector2f > CCubeRenderer::SetViewportOrtho(bool centered, float znear,
                                                                   float zfar) {
  const CViewport& vp = CGraphics::GetViewport();
  const float left = static_cast< float >(centered ? -vp.mWidth / 2 : 0);
  const float top = static_cast< float >(centered ? -vp.mHeight / 2 : 0);
  const float right = static_cast< float >(centered ? vp.mWidth / 2 : vp.mWidth);
  const float bottom = static_cast< float >(centered ? vp.mHeight / 2 : vp.mHeight);
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
  return mReflectionTex.get() ? mReflectionTex.get() : &mBlackTex;
}

void CCubeRenderer::CacheReflection(void (*callback)(void*, const CVector3f&), void* context,
                                    bool clear) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawSpaceWarp(const CVector3f& point, float strength) {
  if (point.GetZ() < 1.f) {
    _DrawSpaceWarp(point, strength);
  }
}

void CCubeRenderer::_DrawSpaceWarp(const CVector3f& point, float strength) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::SetWireframeFlags(int flags) {
  CCubeModel::SetDrawingOccluders((flags & 1) != 0);
  mDrawWireframe = (flags & 2) != 0;
}

void CCubeRenderer::SetWorldFog(ERglFogMode mode, float start, float end, const CColor& color) {
  CGraphics::SetFog(mode, start, end, color);
}

int CCubeRenderer::GetStaticWorldDataSize() {
  int size = 0;
  for (rstl::list< CAreaListItem >::const_iterator area = mAreaListItems.begin();
       area != mAreaListItems.end(); ++area) {
    size += area->mTextures->size() * sizeof(TCachedToken< CTexture >);
  }
  return size;
}

void CCubeRenderer::DrawFogFan(const CVector3f* vertices, int count) {
  if (count < 3) {
    return;
  }
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast< ushort >(count));
  for (int i = 0; i < count; ++i) {
    GXPosition3f32(vertices[i].GetX(), vertices[i].GetY(), vertices[i].GetZ());
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
    const CVector3f max = -worldBounds.GetMaxPoint();
    const CPlane planes[7] = {
        CPlane(min.GetX(), CVector3f::Right()),
        CPlane(max.GetX(), CVector3f::Left()),
        CPlane(min.GetY(), CVector3f::Forward()),
        CPlane(max.GetY(), CVector3f::Back()),
        CPlane(min.GetZ(), CVector3f::Up()),
        CPlane(max.GetZ(), CVector3f::Down()),
        CPlane(CVector3f::Dot(view.GetTranslation(), forward) + 0.2f + 0.1f, forward),
    };
    CGraphics::SetModelMatrix(CTransform4f::Identity());
    const CVector3f dimensions = worldBounds.GetMaxPoint() - worldBounds.GetMinPoint();
    const float maxExtent = rstl::max_val(rstl::max_val(dimensions.GetZ(), dimensions.GetY()),
                                        dimensions.GetX());
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
    mFogVolumes.push_back(
        CFogVolumeListItem(CGraphics::GetModelMatrix(), color, bounds, model, skinnedModel));
  }
}

void CCubeRenderer::ReallyRenderFogVolume(const CColor& color, const CAABox& bounds,
                                          const CModel* model, const CSkinnedModel* skinnedModel) {
  // TODO: reconstruct this rendering pass.
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
    CGraphics::SetModelMatrix(fog->mTransform);
    ReallyRenderFogVolume(fog->mColor, fog->mBounds, fog->mModel ? **fog->mModel : nullptr,
                          fog->mSkinnedModel);
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
  const CVector3f translation = -rotatedBounds.GetMinPoint();
  const CVector3f dimensions = rotatedBounds.GetMaxPoint() - rotatedBounds.GetMinPoint();
  xf = (CTransform4f::Scale(5.f / dimensions.GetX(), 5.f / dimensions.GetY(),
                            5.f / dimensions.GetZ()) * CTransform4f::Translate(translation)) * xf;
  const CAABox transformedBounds = bounds.GetTransformedAABox(xf);
  (void)transformedBounds;
  const float y = -(1.f - amount) * 6.f + 1.f;
  const float x = -0.85f * amount - 0.15f;
  const float post0[3][4] = {
      {1.f, 1.f, 0.f, amount}, {0.f, 0.f, 1.f, y}, {0.f, 0.f, 0.f, 1.f},
  };
  const float post1[3][4] = {
      {1.f, 1.f, 0.f, x}, {0.f, 0.f, 1.f, y}, {0.f, 0.f, 0.f, 1.f},
  };
  GXLoadTexMtxImm(xf.GetCStyleMatrix(), GX_TEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(post0, GX_PTTEXMTX0, GX_MTX3x4);
  GXLoadTexMtxImm(post1, GX_PTTEXMTX1, GX_MTX3x4);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX0);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTTEXMTX1);
  CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetZMode(true, GX_LEQUAL, true);
  model.DrawFlat(CModelFlags(CModelFlags::kT_Opaque, CColor::White()), true, true);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void CCubeRenderer::DrawModelFlat(const SModelRenderData& model, const CModelFlags& flags,
                                  bool unsortedOnly) {
  const char blendMode = static_cast< char >(flags.GetTrans());
  if (blendMode < 7) {
    if (blendMode < 5) {
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    } else {
      CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    }
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
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
  // TODO: reconstruct this rendering pass.
}

bool CCubeRenderer::DrawScanSurface(const CAreaListItem& area, const CCubeModel& model,
                                    const CCubeSurface& surface, uint lightSet, bool alpha) {
  // TODO: select the surface material and scan-pass drawing state.
  return false;
}

void CCubeRenderer::DrawScanRing(float radius, float thickness, float alpha, float fade,
                                 float scanTime, int areaId) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::SetGXRegister1Color(const CColor& color) {
  GXSetTevColor(GX_TEVREG1, color.GetGXColor());
}

void CCubeRenderer::SetWorldLightFadeLevel(float level) {
  const uchar value = static_cast< uchar >(level * 255.f);
  mWorldLightColor = CColor(value, value, value, static_cast< uchar >(255));
}

uchar CCubeRenderer::FindOrAddLightSet(uint lightSet) {
  for (int i = 0; i < mLightSets.size(); ++i) {
    if (mLightSets[i] == lightSet) {
      return static_cast< uchar >(i);
    }
  }
  if (mLightSets.size() == mLightSets.capacity()) {
    return 0;
  }
  mLightSets.push_back(lightSet);
  return static_cast< uchar >(mLightSets.size() - 1);
}

void CCubeRenderer::FindOverlappingWorldModels(rstl::vector< uint >& models, const CAABox& bounds) {
  // TODO: reconstruct this rendering pass.
}

int CCubeRenderer::DrawOverlappingWorldModelShadows(int areaId, rstl::vector< uint >& models,
                                                    const CAABox& bounds) {
  // TODO: draw the selected model surfaces and return their count.
  return 0;
}

void CCubeRenderer::DrawWorldModelShadow(const CAABox& bounds) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawOverlappingWorldModelIDs(int areaId, rstl::vector< uint >& models,
                                                 const CAABox& bounds) {
  // TODO: reconstruct this rendering pass.
}

void* CCubeRenderer::GetRenderToTexBuffer(int index) {
  return static_cast< uchar* >(CGraphics::GetDolphinSpareBuffer()) +
         (static_cast< uint >(index * CGraphics::GetSpareBufferSize()) >> 4);
}

void CCubeRenderer::CopyScreenTex(uint divisor, bool half, void* dest, GXTexFmt format,
                                  bool clear) const {
  const CViewport& viewport = CGraphics::GetViewport();
  GXSetTexCopySrc(viewport.mLeft, viewport.mTop + viewport.mHeight - viewport.mHeight / divisor,
                 viewport.mWidth / divisor, viewport.mHeight / divisor);
  const uint width = half ? viewport.mWidth / 2 : viewport.mWidth;
  const uint height = half ? viewport.mHeight / 2 : viewport.mHeight;
  GXSetTexCopyDst(width / divisor, height / divisor, format, half);
  const CColor clearColor = CGraphics::GetClearColor();
  CGraphics::SetClearColor(CColor(0));
  GXSetColorUpdate(false);
  GXCopyTex(dest ? dest : CGraphics::GetDolphinSpareBuffer(), clear);
  GXSetColorUpdate(true);
  GXPixModeSync();
  CGraphics::SetClearColor(clearColor);
}

void CCubeRenderer::DoPhazonSuitIndirectAlphaBlur(float scale, float amount) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::ReallyDrawPhazonSuitEffect(const CColor& color, const CTexture& texture) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::ReallyDrawPhazonSuitIndirectEffect(const CColor& color, const CTexture& texture,
                                                       const CTexture& indirectTexture, float scale,
                                                       float offset, float alpha,
                                                       const CColor& additiveColor) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::RenderSilhouette(
    float blur, const CColor& color,
    const rstl::optional_object< TCachedToken< CTexture > >& texture, float scale, float offset,
    float alpha, const CColor& additiveColor) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::AllocatePhazonSuitMaskTexture() {
  mRequestRGBA6 = true;
  if (!mSilhouetteMask.get()) {
    const CViewport& viewport = CGraphics::GetViewport();
    mSilhouetteMask = rs_new CTexture(kTF_I8, viewport.mWidth >> 2, viewport.mHeight >> 2, 1);
  }
  mSilhouetteMaskCountdown = 2;
}

float CCubeRenderer::GetRandomInterpolation(float time, float period, int seed) {
  const float scaledTime = time / period;
  const float fraction = scaledTime - static_cast< int >(scaledTime);
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
  float fraction = scaledTime - static_cast< int >(scaledTime);
  if (fraction >= 0.5f) {
    fraction = -(fraction - 1.f);
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
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
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
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::fn_802679DC(const void* unused, const SModelRenderData& model,
                              const CModelFlags& flags) {
  const uint otherFlags = flags.GetOtherFlags();
  CGX::SetZMode(true, (otherFlags & CModelFlags::kF_DepthCompare) ? GX_LEQUAL : GX_ALWAYS,
                (otherFlags & CModelFlags::kF_DepthUpdate) != 0);
  model.DrawFlat(flags, true, true);
}

void CCubeRenderer::LoadEnvironmentTextureMatrix(uint matrix, uint postMatrix,
                                                 const CTransform4f& xf, bool alternate) {
  CTransform4f textureTransform = xf * CGraphics::GetModelMatrix();
  textureTransform.SetTranslation(CVector3f::Zero());
  GXLoadTexMtxImm(textureTransform.GetCStyleMatrix(), matrix, GX_MTX3x4);
  static const float environmentMatrix[3][4] = {
      {0.5f, 0.f, 0.f, 0.5f}, {0.f, 0.f, 0.5f, 0.5f}, {0.f, 0.f, 0.f, 1.f},
  };
  static const float alternateMatrix[3][4] = {
      {2.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 2.f, 0.f}, {0.f, 0.f, 0.f, 1.f},
  };
  GXLoadTexMtxImm(alternate ? alternateMatrix : environmentMatrix, postMatrix, GX_MTX3x4);
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
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::GetScreenMipInfo(int width, int height, int mipCount, GXTexFmt format,
                                     int* size, int* mipWidth, int* mipHeight) {
  int total = 0;
  for (int i = 0; i < mipCount; ++i) {
    width >>= 1;
    height >>= 1;
    total += GXGetTexBufferSize(width, height, format, GX_FALSE, 0);
  }
  if (size) {
    *size = total;
  }
  if (mipWidth) {
    *mipWidth = width;
  }
  if (mipHeight) {
    *mipHeight = height;
  }
}

void CCubeRenderer::SetupScreenCopyStates() {
  CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  static const GXVtxDescList desc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
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
  GenerateScreenMipmaps(mipCount, alpha ? GX_CTF_A8 : GX_CTF_R8, GX_TF_I8, 0, 0,
                        viewport.mWidth, viewport.mHeight);
}

void CCubeRenderer::SetMaterialMode(int mode) {
  mCurrentMaterialMode = mode;
  if (mode == 1) {
    CCubeMaterial::UseThermalTevs();
  } else if (mode == 0) {
    CCubeMaterial::UseNormalTevs();
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
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawDarkWorldTransition(const CColor& color0, const CColor& color1,
                                            const CColor& color2, const CColor& color3,
                                            const CVector2i& offset, const CVector2i& sourceSize,
                                            const CVector2i& targetSize) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawDarkWorldFilter(float amount) {
  // TODO: reconstruct this rendering pass.
}

uint CCubeRenderer::PackLightSet(const uchar* lights, float ambient) {
  const uint level = CMath::ClampI(0, static_cast< int >(63.f * ambient), 63);
  return (lights[0] & 63) | ((lights[1] & 63) << 6) | ((lights[2] & 63) << 12) |
         ((lights[3] & 63) << 18) | (level << 24);
}

void CCubeRenderer::UnpackLightSet(uint lightSet, uchar* lights, float* ambient,
                                   uchar* quantizedAmbient) {
  for (int i = 0; i < 4; ++i) {
    lights[i] = (lightSet >> (6 * i)) & 63;
  }

  const uchar level = (lightSet >> 24) & 63;
  if (ambient) {
    *ambient = level * (1.f / 63.f);
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
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawUnsortedGeometry(int areaId) { DrawGeometry< false, false >(areaId); }

void CCubeRenderer::DrawUnsortedGeometryAlpha(int areaId) { DrawGeometry< false, true >(areaId); }

void CCubeRenderer::DrawSpecialGeometry(int areaId) { DrawGeometry< true, false >(areaId); }

void CCubeRenderer::DrawSpecialGeometryAlpha(int areaId) { DrawGeometry< true, true >(areaId); }

void CCubeRenderer::DrawAreaModel(int areaId, int modelId, const CModelFlags& flags) {
  // TODO: reconstruct this rendering pass.
}

CAABox CCubeRenderer::GetAreaModelBounds(int areaId, int modelId) {
  // TODO: find the area's model and return its bounds.
  return CAABox(CVector3f::Zero(), CVector3f::Zero());
}

void CCubeRenderer::DrawVisibleAreaGeometry(int areaId, const CPVSVisSet& pvs,
                                            const CFrustumPlanes& frustum, const CAABox& bounds) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::ActivateLightsForModel(uint lightSet) {
  // TODO: reconstruct this rendering pass.
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
    for (int i = 0; i < visible.size(); ++i) {
      area->mPVSAlpha[visible[i].first] = visible[i].second << 2;
    }
  }
}

void CCubeRenderer::DisablePVS(int areaId) {
  rstl::list< CAreaListItem >::iterator area = FindArea(areaId);
  if (area != mAreaListItems.end()) {
    for (int i = 0; i < area->mPVSAlpha.size(); ++i) {
      area->mPVSAlpha[i] = 0;
    }
  }
}

void CCubeRenderer::DrawModelWithTextureMask(const SModelRenderData& model, const CTexture& texture,
                                             const CVector3f& origin, const CColor& color,
                                             float scale) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::CopyTextureRegion(void* dest, int format, int left, int top, int width,
                                      int height) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawDarkWorldCloud(float time, const CVector3f& scale, const CColor& color) {
  // TODO: reconstruct this rendering pass.
}

void CCubeRenderer::DrawModelNoise(const SModelRenderData& model, const CColor& color,
                                   bool additive) {
  void* noise = reinterpret_cast< void* >(((mRandom.Next() + 31) & ~31) + 0x8000);
  CGraphics::LoadDolphinSpareTexture(96, 96, GX_TF_IA4, noise, CGraphics::kSpareBufferTexMapID);
  CGX::SetBlendMode(GX_BM_BLEND, additive ? GX_BL_ONE : GX_BL_SRCALPHA,
                    additive ? GX_BL_ONE : GX_BL_INVSRCALPHA, GX_LO_CLEAR);
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
  model.DrawFlat(CModelFlags(CModelFlags::kT_Opaque, CColor::White()), true, false);
}

uint GetRendererWorkspaceSize() { return Buckets::GetWorkspaceSize(); }

void SetRendererWorkspace(void* workspace) { Buckets::Init(workspace); }

void ReleaseRendererWorkspace() { Buckets::Shutdown(); }

void CCubeRenderer::DrawString(const char* text, int x, int y) {
  mFont.DrawString(text, x, y, CColor::White());
}
