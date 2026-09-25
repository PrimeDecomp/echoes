#ifndef _CCUBERENDERER
#define _CCUBERENDERER

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/Text/CFont.hpp"
#include "MetaRender/IRenderer.hpp"
#include "Weapons/IWeaponRenderer.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "types.h"

class CCubeModel;
class CCubeSurface;

class CCubeRenderer : public IRenderer, public IWeaponRenderer, public TOneStatic< CCubeRenderer > {
public:
  // Guessed name.
  struct SModelSurfaceOrder {
    explicit SModelSurfaceOrder(const CCubeModel& model);

    rstl::auto_ptr< ushort > mSurfaceIndices;
    int mOpaqueEnd;
    int mSortedEnd;
    int mTotalCount;
  };

  class CAreaListItem {
  public:
    CAreaListItem(const rstl::vector< CMetroidModelInstance >* geometry,
                  const CAreaRenderOctTree* octTree, const rstl::vector< SAreaSurface >* surfaces,
                  const rstl::vector< uint >* ambientLightIds,
                  const rstl::vector< signed char >* ambientLightIndices,
                  const rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > >& textures,
                  const rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > >& models,
                  int areaId);

    const rstl::vector< CMetroidModelInstance >* mGeometry;
    const CAreaRenderOctTree* mOctTree;
    const rstl::vector< SAreaSurface >* mSurfaces;
    const rstl::vector< uint >* mAmbientLightIds;
    const rstl::vector< signed char >* mAmbientLightIndices;
    rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > > mTextures;
    rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > > mModels;
    int mAreaId;
    rstl::vector< uchar > mLightSetIndices;
    rstl::vector< uchar > mPVSAlpha;
    rstl::vector< SModelSurfaceOrder > mModelSurfaceOrders;
  };

  class CFogVolumeListItem {
  public:
    CFogVolumeListItem(const CTransform4f& xf, const CColor& color, const CAABox& bounds,
                       const TLockedToken< CModel >* model, const CSkinnedModel* skinnedModel);

    CTransform4f mTransform;
    CColor mColor;
    CAABox mBounds;
    rstl::optional_object< TLockedToken< CModel > > mModel;
    const CSkinnedModel* mSkinnedModel;
  };

  CCubeRenderer(IObjectStore& store, COsContext& context, CMemorySys& memory, IFactory& factory);

  // IRenderer
  ~CCubeRenderer() override;
  void AddStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry,
                         const CAreaRenderOctTree* octTree,
                         const rstl::vector< SAreaSurface >* surfaces,
                         const rstl::vector< uint >* ambientLightIds,
                         const rstl::vector< signed char >* ambientLightIndices,
                         int areaId) override;
  void EnablePVS(int areaId, const rstl::vector< rstl::pair< int, int > >& visible) override;
  void DisablePVS(int areaId) override;
  // Guessed name
  void PrepareWorldRendering(
      const rstl::pair< int, const CPVSVisSet* >* pvsSets, int pvsCount,
      const CFrustumPlanes& frustum,
      const rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 >* areaFrusta,
      const rstl::vector< CLight >& lights, const rstl::pair< int, float >* ambientLights,
      int ambientLightCount) override;
  void RemoveStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) override;
  void DrawUnsortedGeometry(int areaId) override;
  void DrawSortedGeometry(int mode, int areaId) override;
  // Guessed name
  void DrawSpecialGeometry(int areaId) override;
  // Guessed name
  void DrawScanRing(float radius, float thickness, float alpha, float fade, float scanTime,
                    int areaId) override;
  // Guessed name
  void DrawUnsortedGeometryAlpha(int areaId) override;
  // Guessed name
  void DrawSpecialGeometryAlpha(int areaId) override;
  // Guessed name
  void DrawAreaModel(int areaId, int modelId, const CModelFlags& flags) override;
  void PostRenderFogs() override;
  void SetModelMatrix(const CTransform4f& xf) override;
  // Also overrides IWeaponRenderer.
  void AddParticleGen(const CParticleGen& gen) override;
  void AddParticleGen(const CParticleGen& gen, const CVector3f& pos, const CAABox& bounds) override;
  void AddPlaneObject(const void* obj, const CAABox& bounds, const CPlane& plane,
                      int type) override;
  void AddDrawable(const void* obj, const CVector3f& pos, const CAABox& bounds, int mode,
                   EDrawableSorting sorting) override;
  void SetDrawableCallback(TDrawableCallback callback, const void* context) override;
  void SetWorldViewpoint(const CTransform4f& xf) override;
  void SetPerspective(float fovy, float width, float height, float znear, float zfar) override;
  void SetPerspective(float fovy, float aspect, float znear, float zfar) override;
  rstl::pair< CVector2f, CVector2f > SetViewportOrtho(bool centered, float znear,
                                                      float zfar) override;
  void SetViewport(int left, int top, int width, int height) override;
  void SetDepthReadWrite(bool read, bool update) override;
  void SetBlendMode_AdditiveAlpha() override;
  void SetBlendMode_AlphaBlended() override;
  void SetBlendMode_NoColorWrite() override;
  void SetBlendMode_ColorMultiply() override;
  void SetBlendMode_InvertDst() override;
  void SetBlendMode_InvertSrc() override;
  void SetBlendMode_Replace() override;
  void SetBlendMode_AdditiveDestColor() override;
  void SetDebugOption(EDebugOption option, int value) override;
  void BeginScene() override;
  void EndScene() override;
  void BeginPrimitive(EPrimitiveType primitive, int count) override;
  void BeginLines(int count) override;
  void BeginLineStrip(int count) override;
  void BeginTriangles(int count) override;
  void BeginTriangleStrip(int count) override;
  void BeginTriangleFan(int count) override;
  void PrimVertex(const CVector3f& vertex) override;
  void PrimNormal(const CVector3f& normal) override;
  void PrimColor(float r, float g, float b, float a) override;
  void PrimColor(const CColor& color) override;
  void EndPrimitive() override;
  void SetAmbientColor(const CColor& color) override;
  void DrawString(const char* text, int x, int y) override;
  float GetFPS() override;
  void CacheReflection(void (*callback)(void*, const CVector3f&), void* context,
                       bool clear) override;
  void DrawSpaceWarp(const CVector3f& point, float strength) override;
  void DrawModelDisintegrate(const CModel& model, const CTexture& texture, const CColor& color,
                             float amount) override;
  void DrawModelFlat(const CModel& model, const CModelFlags& flags, bool unsortedOnly) override;
  // Guessed name
  void DrawModelProjectedShadow(const CModel& model, const CTexture& texture,
                                const CVector3f& direction, const CColor& color,
                                float scale) override;
  // Guessed name
  void DrawModelNoise(const CModel& model, const CColor& color, bool additive) override;
  bool EnableSilhouetteRender() override;
  // TODO: identify the unused argument and original method name.
  void fn_802679DC(const void* unused, const CModel& model, const CModelFlags& flags) override;
  // Guessed name
  void DrawSilhouetteNoise(const SSilhouetteNoise& noise) override;
  void SetWireframeFlags(int flags) override;
  void SetWorldFog(ERglFogMode mode, float start, float end, const CColor& color) override;
  void RenderFogVolume(const CColor& color, const CAABox& bounds,
                       const TLockedToken< CModel >* model,
                       const CSkinnedModel* skinnedModel) override;
  // Guessed name
  void SetRequestedMaterialMode(int mode) override;
  // Guessed name; corroborated by the Wii dark-world sphere caller.
  void DrawDarkWorldVolume(const CVector3f& pos, const CVector3f& scale, uchar mix, uchar alpha,
                           bool inside, float lod, const CVector2f& scroll1,
                           const CVector2f& scroll2, const CVector2f& texScale1,
                           const CVector2f& texScale2, const CTexture& environment,
                           const CTexture& cloud1, const CTexture& cloud2, CColor color,
                           CColor additiveColor, bool cylinder, bool additive) override;
  // Guessed name
  void DrawDarkWorldFilter(float amount) override;
  // Guessed name
  void DrawScanVisor(float scanTime, float width, float height, const CColor& color,
                     const CColor& scanColor, const CColor& maskColor, const CColor* palette,
                     int paletteSize, const CVector3f& scanRange) override;
  // Guessed name
  void DrawScreenFilter(const CColor& color0, const CColor& color1, const CColor& color2) override;
  int GetStaticWorldDataSize() override;
  void SetGXRegister1Color(const CColor& color) override;
  void SetWorldLightFadeLevel(float level) override;
  CAABox GetAreaModelBounds(int areaId, int modelId) override;
  void SetDestinationAlpha(int alpha) override;
  void DisableDestinationAlpha() override;
  bool IsRGBA6Current() const override { return mCurrentRGBA6; }
  // Guessed name
  void DrawDarkWorldTransition(const CColor& color0, const CColor& color1, const CColor& color2,
                               const CColor& color3, const CVector2i& offset,
                               const CVector2i& sourceSize, const CVector2i& targetSize) override;
  // Guessed name
  void CopyTextureRegion(void* dest, int format, int left, int top, int width, int height) override;
  // Guessed name
  void DrawDarkWorldCloud(float time, const CVector3f& scale, const CColor& color) override;

  // IWeaponRenderer
  // The destructor and AddParticleGen(const CParticleGen&) override both bases.

  rstl::list< CAreaListItem >::iterator FindArea(int areaId);
  rstl::list< CAreaListItem >::const_iterator FindArea(int areaId) const;
  rstl::list< CAreaListItem >::iterator
  FindStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry);
  void ActivateLightsForModel(uint lightSet);
  // Guessed name
  void DrawVisibleAreaGeometry(int areaId, const CPVSVisSet& pvs, const CFrustumPlanes& frustum,
                               const CAABox& bounds);
  // Guessed name
  static void UnpackLightSet(uint lightSet, uchar* lights, float* ambient, uchar* quantizedAmbient);
  // Guessed name
  static uint PackLightSet(const uchar* lights, float ambient);
  // Guessed name
  void SetMaterialMode(int mode);
  // Guessed name
  static void GenerateScreenMipmaps(int mipCount, bool depth);
  // Guessed name
  static void* GenerateScreenMipmaps(int mipCount, GXTexFmt copyFormat, GXTexFmt loadFormat,
                                     int left, int top, int width, int height);
  // Guessed name
  static void DrawTexturedScreenQuad(int left, int top, int width, int height);
  // Guessed name
  static void SetupScreenCopyStates();
  // Guessed name
  static void GetScreenMipInfo(int width, int height, int mipCount, GXTexFmt format, int* size,
                               int* mipWidth, int* mipHeight);
  // Guessed name
  static void LoadScrollingTextureMatrix(uint matrix, const CVector2f& scroll,
                                         const CVector2f& scale);
  // Guessed name
  static void LoadEnvironmentTextureMatrix(uint matrix, uint postMatrix, const CTransform4f& xf,
                                           bool alternate);
  // Guessed name
  static void PopulateNoiseTexCoords(float time, rstl::reserved_vector< CVector2f, 9 >& coords);
  // Guessed name
  static float GetRandomInterpolation(float time, float period, int seed);
  void AllocatePhazonSuitMaskTexture();
  void RenderSilhouette(float blur, const CColor& color,
                        const rstl::optional_object< TCachedToken< CTexture > >& texture,
                        float scale, float offset, float alpha, const CColor& additiveColor);
  void ReallyDrawPhazonSuitIndirectEffect(const CColor& color, const CTexture& texture,
                                          const CTexture& indirectTexture, float scale,
                                          float offset, float alpha, const CColor& additiveColor);
  void ReallyDrawPhazonSuitEffect(const CColor& color, const CTexture& texture);
  void DoPhazonSuitIndirectAlphaBlur(float scale, float amount);
  void CopyScreenTex(uint divisor, bool half, void* dest, GXTexFmt format, bool clear) const;
  static void* GetRenderToTexBuffer(int index);
  void DrawOverlappingWorldModelIDs(int areaId, rstl::vector< uint >& models, const CAABox& bounds);
  // Guessed name
  void DrawWorldModelShadow(const CAABox& bounds);
  int DrawOverlappingWorldModelShadows(int areaId, rstl::vector< uint >& models,
                                       const CAABox& bounds);
  void FindOverlappingWorldModels(rstl::vector< uint >& models, const CAABox& bounds);
  // Guessed name
  uchar FindOrAddLightSet(uint lightSet);
  // Guessed name
  bool DrawScanSurface(const CAreaListItem& area, const CCubeModel& model,
                       const CCubeSurface& surface, uint lightSet, bool alpha);
  void ReallyRenderFogVolume(const CColor& color, const CAABox& bounds, const CModel* model,
                             const CSkinnedModel* skinnedModel);
  static void RenderFogVolumeModel(const CAABox& bounds, const CModel* model,
                                   const CTransform4f& modelView, CTransform4f view,
                                   const CSkinnedModel* skinnedModel);
  static void DrawFogSlices(const CPlane* planes, int planeCount, int planeIndex,
                            const CVector3f& center, float extent);
  static void DrawFogFans(const CPlane* planes, int planeCount, const CVector3f* vertices,
                          int vertexCount, int front, int back);
  static void DrawFogFan(const CVector3f* vertices, int count);
  void _DrawSpaceWarp(const CVector3f& point, float strength);
  CTexture* GetRealReflection();
  // Guessed name
  void EvaluateModelLights(uchar* lights, const CAABox& bounds, const uint* overlaps, int wordCount,
                           uint modelIndex);
  void RenderBucketItems(const CAreaListItem* area, bool alpha);
  void DrawRenderBucketsDebug();
  // Guessed name
  void AddWorldSurface(short modelIndex, ushort surfaceIndex, uint blend, const CAABox& bounds);
  void SetupCGraphicsStates();
  void SetupRendererStates(bool depthWrite);
  // Guessed name
  void GenerateScanRampTex();
  // Guessed name
  void GenerateAlphaMaskRampTex();
  void GenerateSphereRampTex();
  void GenerateFogVolumeRampTex();
  void GenerateReflectionTex();
  // Guessed name
  CGraphicsPalette* ClonePalette(const TLockedToken< CTexture >& texture);

  bool GetReflectionFlag() const { return mReflectionDirty; }
  void SetReflectionFlag() { mReflectionDirty = true; }
  const CTexture& GetAlphaMaskRamp() const { return mAlphaMaskRamp; }
  int GetMaterialMode() const { return mCurrentMaterialMode; }
  static CCubeRenderer* That() { return sRenderer; }

private:
  template < bool Special, bool Alpha >
  void DrawGeometry(int areaId);

  IFactory& mFactory;
  IObjectStore& mObjStore;
  CFont mFont;
  int mPrimVertCount;
  rstl::list< CAreaListItem > mAreaListItems;
  CFrustumPlanes mFrustumPlanes;
  TDrawableCallback mDrawableCallback;
  const void* mDrawableCallbackUserData;
  CPlane mViewPlane;
  uchar mPVSMode;
  int mPVSState;
  CTexture mBlackTex;
  rstl::single_ptr< CTexture > mReflectionTex;
  CTexture mReflectionRamp;
  CTexture mFogVolumeRamp;
  CTexture mSphereRamp;
  CTexture mAlphaMaskRamp;
  CTexture mScanRamp;
  CRandom16 mRandom;
  rstl::list< CFogVolumeListItem > mFogVolumes;
  int mReflectionAge;
  CColor mPrimColor;
  CVector3f mPrimNormal;
  CColor mWorldLightColor;
  rstl::vector< CLight > mDynamicLights;
  rstl::reserved_vector< uint, 96 > mLightSets;
  int mSilhouetteMaskCountdown;
  rstl::single_ptr< CTexture > mSilhouetteMask;
  TLockedToken< CTexture > mBigRing;
  TLockedToken< CTexture > mDarkWorldCloud;
  TLockedToken< CTexture > mScanSweepBar;
  TLockedToken< CModel > mFlatSphere;
  TLockedToken< CModel > mFlatSphereLow;
  TLockedToken< CModel > mFlatCylinder;
  TLockedToken< CModel > mFlatCylinderLow;
  rstl::single_ptr< CGraphicsPalette > mDarkLightWorldPalette;
  bool mReflectionDirty : 1;
  bool mDrawWireframe : 1;
  bool mRequestRGBA6 : 1;
  bool mCurrentRGBA6 : 1;
  bool mPreserveDestinationAlpha : 1;
  bool mDisableFog : 1;
  bool mPersistRGBA6 : 1;
  bool mRenderingSilhouette : 1;
  int mCurrentMaterialMode;
  int mRequestedMaterialMode;

  static CCubeRenderer* sRenderer;
};

CHECK_SIZEOF(CCubeRenderer, 0x560)
NESTED_CHECK_SIZEOF(CCubeRenderer, SModelSurfaceOrder, 0x14)
NESTED_CHECK_SIZEOF(CCubeRenderer, CAreaListItem, 0x58)
NESTED_CHECK_SIZEOF(CCubeRenderer, CFogVolumeListItem, 0x60)

extern CCubeRenderer* gpRender;

#endif // _CCUBERENDERER
