#ifndef _CCUBEMODEL
#define _CCUBEMODEL

#include "types.h"

#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/vector.hpp"

class CCubeSurface;
class CModelFlags;
class CStopwatch;
class CTexture;
class CTransform4f;
class CVector3f;
class IObjectStore;

enum ESurfaceSelection {
  kSS_Unsorted,
  kSS_Sorted,
  kSS_All,
};

class CCubeModel {
public:
  class ModelInstance {
  public:
    ModelInstance(rstl::vector< void* >& surfaces, const void* materialData, const void* positions,
                  const void* normals, const void* colors, const void* uvs, const void* packedTexCoords)
    : mSurfacePtrs(surfaces)
    , mMaterialData(materialData)
    , mPositions(positions)
    , mNormals(normals)
    , mColors(colors)
    , mTexCoords(uvs)
    , mPackedTexCoords(packedTexCoords) {}

    rstl::vector< void* >& Surfaces() { return mSurfacePtrs; }
    const void* GetMaterialPointer() const { return mMaterialData; }
    void SetMaterialPointer(const void* data) { mMaterialData = data; }
    const void* GetVertexPointer() const { return mPositions; }
    const void* GetNormalPointer() const { return mNormals; }
    const void* GetColorPointer() const { return mColors; }
    const void* GetTCPointer() const { return mTexCoords; }
    const void* GetPackedTCPointer() const { return mPackedTexCoords; }

  private:
    rstl::vector< void* >& mSurfacePtrs;
    const void* mMaterialData;
    const void* mPositions;
    const void* mNormals;
    const void* mColors;
    const void* mTexCoords;
    const void* mPackedTexCoords;
  };

  // Echoes method names below are inferred from Prime and call sites.
  CCubeModel(rstl::vector< void* >* surfaces, rstl::vector< TCachedToken< CTexture > >* textures,
             const void* materialData, const void* positions, const void* normals,
             const void* colors, const void* uvs, const void* compressedUvs, const CAABox& bounds,
             uchar visorFlags, bool texturesLoaded, uint idx);

  static void SetRenderModelBlack(bool v);
  static void SetModelWireframe(bool v);
  static void DisableShadowMaps();
  static void EnableShadowMaps(const CTexture*, const CTransform4f&, unsigned char, unsigned char);
  static void SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch);
  static void SetDrawingOccluders(bool);
  static void MakeTexturesFromMats(const void* data,
                                   rstl::vector< TCachedToken< CTexture > >& textures,
                                   IObjectStore& store, bool cache);

  void UnlockTextures() const;
  void RemapMaterialData(const void* data, rstl::vector< TCachedToken< CTexture > >* textures);
  bool TryLockTextures() const;
  void DrawNormal(ESurfaceSelection which) const;
  void DrawFlat(int flags) const;
  void Draw(const CModelFlags& flags) const;
  void Draw(u64 mask, const CModelFlags& flags) const;
  void DrawNormal(const CModelFlags& flags) const;
  void DrawAlpha(const CModelFlags& flags) const;
  void SetArraysCurrent() const;
  void SetStaticArraysCurrent() const;
  void DrawSurface(const CCubeSurface& surface, const CModelFlags& flags) const;
  void DrawSurfaceWireframe(const CCubeSurface& surface) const;
  void DrawSurfaceFlat(const CCubeSurface& surface) const; // Guessed name.
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawSurfaces(u64 mask, const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawAlphaSurfaces(const CModelFlags& flags) const;

  void SetUsingPackedLightmaps(bool v) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }

  const ModelInstance& GetModelInstance() const { return mInstance; }
  const void* GetPositions() const { return mInstance.GetVertexPointer(); }
  const void* GetNormals() const { return mInstance.GetNormalPointer(); }
  bool AreTexturesLoaded() const { return !mLoadTextures; }
  uchar GetModelFlags() const { return mVisorFlags; }
  int GetModelIndex() const { return mIdx; }

  const rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *mTextures; }
  const rstl::vector< TCachedToken< CTexture > >* GetTexturesPtr() const { return mTextures; }
  const CAABox& GetBoundingBox() const { return mBounds; }
  const CCubeSurface& GetNormalSurfaces() const { return mFirstUnsorted; }
  const CCubeSurface& GetAlphaSurfaces() const { return mFirstSorted; }

  // Guessed name; the target passes a surface handle by value for material lookup.
  CCubeMaterial GetMaterial(CCubeSurface surface) const {
    return GetMaterialByIndex(surface.GetMaterialIndex());
  }

  CCubeMaterial GetMaterialByIndex(int index) const {
    const uint offset = index != 0 ? CBasics::SwapBytes(mMaterialOffsets[index - 1]) : 0;
    return CCubeMaterial(mMaterialData + offset);
  }

private:
  void CacheMaterialData(); // Guessed name; caches the offsets and material payload.

  ModelInstance mInstance;
  rstl::vector< TCachedToken< CTexture > >* mTextures;
  CAABox mBounds;
  CCubeSurface mFirstUnsorted;
  CCubeSurface mFirstSorted;
  mutable bool mLoadTextures : 1;
  bool mVisible : 1;
  uchar x40_ : 6;
  uchar mVisorFlags;
  int mIdx;
  const uint* mMaterialOffsets;
  const uchar* mMaterialData;

  static bool sUsingPackedLightmaps;
};
CHECK_SIZEOF(CCubeModel, 0x50)

#endif // _CCUBEMODEL
