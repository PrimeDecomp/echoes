#ifndef _IRENDERER
#define _IRENDERER

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CAreaRenderOctTree;
class CMetroidModelInstance;
class CModel;
class CModelFlags;
class CParticleGen;
class CPVSVisSet;
class CSkinnedModel;
class CFrustumPlanes;
class CPlane;
class CVector2i;
class IFactory;
class IObjectStore;
class COsContext;
class CMemorySys;

// Guessed name. Entries describe area surfaces, with entry zero reserved.
struct SAreaSurface {
  CAABox mBounds;
  short mModelIndex;
  ushort mSurfaceIndex;
  uint x1c_;
};
CHECK_SIZEOF(SAreaSurface, 0x20)

// Guessed name.
struct SSilhouetteNoise {
  float mTime;
  CColor mColor;
};
CHECK_SIZEOF(SSilhouetteNoise, 8)

class IRenderer {
public:
  typedef void (*TDrawableCallback)(const void*, const void*, int);

  enum EDrawableSorting {
    kDS_SortedCallback,
    kDS_AlphaSortedCallback,
    kDS_UnsortedCallback,
  };
  enum EPrimitiveType {
    kPT_Quads = GX_QUADS,
    kPT_Triangles = GX_TRIANGLES,
    kPT_TriangleStrip = GX_TRIANGLESTRIP,
    kPT_TriangleFan = GX_TRIANGLEFAN,
    kPT_Lines = GX_LINES,
    kPT_LineStrip = GX_LINESTRIP,
    kPT_Points = GX_POINTS,
  };
  enum EDebugOption {
    kDO_PVSMode,
    kDO_PVSState,
    kDO_FogDisabled,
  };

  virtual ~IRenderer() = 0;
  virtual void AddStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry,
                                 const CAreaRenderOctTree* octTree,
                                 const rstl::vector< SAreaSurface >* surfaces,
                                 const rstl::vector< uint >* ambientLightIds,
                                 const rstl::vector< signed char >* ambientLightIndices,
                                 int areaId) = 0;
  virtual void EnablePVS(int areaId, const rstl::vector< rstl::pair< int, int > >& visible) = 0;
  virtual void DisablePVS(int areaId) = 0;
  // Guessed name
  virtual void PrepareWorldRendering(
      const rstl::pair< int, const CPVSVisSet* >* pvsSets, int pvsCount,
      const CFrustumPlanes& frustum,
      const rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 >* areaFrusta,
      const rstl::vector< CLight >& lights, const rstl::pair< int, float >* ambientLights,
      int ambientLightCount) = 0;
  virtual void RemoveStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) = 0;
  virtual void DrawUnsortedGeometry(int areaId) = 0;
  virtual void DrawSortedGeometry(int mode, int areaId) = 0;
  // Guessed name
  virtual void DrawSpecialGeometry(int areaId) = 0;
  // Guessed name
  virtual void DrawScanRing(float radius, float thickness, float alpha, float fade, float scanTime,
                            int areaId) = 0;
  // Guessed name
  virtual void DrawUnsortedGeometryAlpha(int areaId) = 0;
  // Guessed name
  virtual void DrawSpecialGeometryAlpha(int areaId) = 0;
  // Guessed name
  virtual void DrawAreaModel(int areaId, int modelId, const CModelFlags& flags) = 0;
  virtual void PostRenderFogs() = 0;
  virtual void SetModelMatrix(const CTransform4f& xf) = 0;
  virtual void AddParticleGen(const CParticleGen& gen) = 0;
  virtual void AddParticleGen(const CParticleGen& gen, const CVector3f& pos,
                              const CAABox& bounds) = 0;
  virtual void AddPlaneObject(const void* obj, const CAABox& bounds, const CPlane& plane,
                              int type) = 0;
  virtual void AddDrawable(const void* obj, const CVector3f& pos, const CAABox& bounds, int mode,
                           EDrawableSorting sorting) = 0;
  virtual void SetDrawableCallback(TDrawableCallback callback, const void* context) = 0;
  virtual void SetWorldViewpoint(const CTransform4f& xf) = 0;
  virtual void SetPerspective(float fovy, float width, float height, float znear, float zfar) = 0;
  virtual void SetPerspective(float fovy, float aspect, float znear, float zfar) = 0;
  virtual rstl::pair< CVector2f, CVector2f > SetViewportOrtho(bool centered, float znear,
                                                              float zfar) = 0;
  virtual void SetViewport(int left, int top, int width, int height) = 0;
  virtual void SetDepthReadWrite(bool read, bool update) = 0;
  virtual void SetBlendMode_AdditiveAlpha() = 0;
  virtual void SetBlendMode_AlphaBlended() = 0;
  virtual void SetBlendMode_NoColorWrite() = 0;
  virtual void SetBlendMode_ColorMultiply() = 0;
  virtual void SetBlendMode_InvertDst() = 0;
  virtual void SetBlendMode_InvertSrc() = 0;
  virtual void SetBlendMode_Replace() = 0;
  virtual void SetBlendMode_AdditiveDestColor() = 0;
  virtual void SetDebugOption(EDebugOption option, int value) = 0;
  virtual void BeginScene() = 0;
  virtual void EndScene() = 0;
  virtual void BeginPrimitive(EPrimitiveType primitive, int count) = 0;
  virtual void BeginLines(int count) = 0;
  virtual void BeginLineStrip(int count) = 0;
  virtual void BeginTriangles(int count) = 0;
  virtual void BeginTriangleStrip(int count) = 0;
  virtual void BeginTriangleFan(int count) = 0;
  virtual void PrimVertex(const CVector3f& vertex) = 0;
  virtual void PrimNormal(const CVector3f& normal) = 0;
  virtual void PrimColor(float r, float g, float b, float a) = 0;
  virtual void PrimColor(const CColor& color) = 0;
  virtual void EndPrimitive() = 0;
  virtual void SetAmbientColor(const CColor& color) = 0;
  virtual void DrawString(const char* text, int x, int y) = 0;
  virtual float GetFPS() = 0;
  virtual void CacheReflection(void (*callback)(void*, const CVector3f&), void* context,
                               bool clear) = 0;
  virtual void DrawSpaceWarp(const CVector3f& point, float strength) = 0;
  virtual void DrawModelDisintegrate(const CModel& model, const CTexture& texture,
                                     const CColor& color, float amount) = 0;
  virtual void DrawModelFlat(const CModel& model, const CModelFlags& flags, bool unsortedOnly) = 0;
  // Guessed name
  virtual void DrawModelProjectedShadow(const CModel& model, const CTexture& texture,
                                        const CVector3f& direction, const CColor& color,
                                        float scale) = 0;
  // Guessed name
  virtual void DrawModelNoise(const CModel& model, const CColor& color, bool additive) = 0;
  virtual bool EnableSilhouetteRender() = 0;
  // TODO: identify the unused argument and original method name.
  virtual void fn_802679DC(const void* unused, const CModel& model, const CModelFlags& flags) = 0;
  // Guessed name
  virtual void DrawSilhouetteNoise(const SSilhouetteNoise& noise) = 0;
  virtual void SetWireframeFlags(int flags) = 0;
  virtual void SetWorldFog(ERglFogMode mode, float start, float end, const CColor& color) = 0;
  virtual void RenderFogVolume(const CColor& color, const CAABox& bounds,
                               const TLockedToken< CModel >* model,
                               const CSkinnedModel* skinnedModel) = 0;
  // Guessed name
  virtual void SetRequestedMaterialMode(int mode) = 0;
  // Guessed name; corroborated by the Wii dark-world sphere caller.
  virtual void DrawDarkWorldVolume(const CVector3f& pos, const CVector3f& scale, uchar mix,
                                   uchar alpha, bool inside, float lod, const CVector2f& scroll1,
                                   const CVector2f& scroll2, const CVector2f& texScale1,
                                   const CVector2f& texScale2, const CTexture& environment,
                                   const CTexture& cloud1, const CTexture& cloud2, CColor color,
                                   CColor additiveColor, bool cylinder, bool additive) = 0;
  // Guessed name
  virtual void DrawDarkWorldFilter(float amount) = 0;
  // Guessed name
  virtual void DrawScanVisor(float scanTime, float width, float height, const CColor& color,
                             const CColor& scanColor, const CColor& maskColor,
                             const CColor* palette, int paletteSize,
                             const CVector3f& scanRange) = 0;
  // Guessed name
  virtual void DrawScreenFilter(const CColor& color0, const CColor& color1,
                                const CColor& color2) = 0;
  virtual int GetStaticWorldDataSize() = 0;
  virtual void SetGXRegister1Color(const CColor& color) = 0;
  virtual void SetWorldLightFadeLevel(float level) = 0;
  virtual CAABox GetAreaModelBounds(int areaId, int modelId) = 0;
  virtual void SetDestinationAlpha(int alpha) = 0;
  virtual void DisableDestinationAlpha() = 0;
  virtual bool IsRGBA6Current() const = 0;
  // Guessed name
  virtual void DrawDarkWorldTransition(const CColor& color0, const CColor& color1,
                                       const CColor& color2, const CColor& color3,
                                       const CVector2i& offset, const CVector2i& sourceSize,
                                       const CVector2i& targetSize) = 0;
  // Guessed name
  virtual void CopyTextureRegion(void* dest, int format, int left, int top, int width,
                                 int height) = 0;
  // Guessed name
  virtual void DrawDarkWorldCloud(float time, const CVector3f& scale, const CColor& color) = 0;
};

inline IRenderer::~IRenderer() {}

IRenderer* AllocateRenderer(IObjectStore& store, COsContext& context, CMemorySys& memory,
                            IFactory& factory);

// Guessed names for the renderer's external bucket-workspace hooks.
void ReleaseRendererWorkspace();
void SetRendererWorkspace(void* workspace);
uint GetRendererWorkspaceSize();

#endif // _IRENDERER
