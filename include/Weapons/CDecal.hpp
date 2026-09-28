#ifndef _CDECAL
#define _CDECAL

#include "Weapons/CDecalDescription.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/vector.hpp"

class CColor;

class CDecal {
public:
  // Guessed name: a world-space polygon vertex and its texture coordinates.
  struct SDecalVertex {
    SDecalVertex(const CVector3f& position, const CVector2f& uv) : mPosition(position), mUV(uv) {}

    CVector3f mPosition;
    CVector2f mUV;
  };

  // Guessed name: one clipped polygon, rendered as a triangle fan.
  class CDecalPolygon {
  public:
    explicit CDecalPolygon(const rstl::vector< SDecalVertex >& vertices) : mVertices(vertices) {}
    void Render(const CColor& color, const CVector2f& uvOffset) const;

  private:
    rstl::vector< SDecalVertex > mVertices;
  };

  class CQuadDecal {
  public:
    explicit CQuadDecal(int lifetime = 0);

    rstl::vector< CDecalPolygon > mPolygons;
    bool mUseClippedGeometry : 1; // Guessed name
    int mLifetime;
    float mHalfSize;
    float mRotation;
    CVector3f mOffset;
    CVector2f mInitialUV;
  };

  CDecal(const TToken< CDecalDescription >& description, const CTransform4f& transform,
         const CUnitVector3f& direction, const rstl::vector< CCollisionSurface >& surfaces);

  static void SetGlobalSeed(ushort seed);
  static void SetMoveRedToAlphaBuffer(bool move) { mMoveRedToAlphaBuffer = move; }
  static void SetDisableAlphaUpdate(bool disable) { mDisableAlphaUpdate = disable; } // Guessed name

  void Update(float dt);
  void Render() const;
  void RenderMdl() const;
  void RenderQuad(CQuadDecal& quad, const CDecalDescription::SQuadDescr& description) const;

  bool IsDone() const { return mFlags == 7; }
  CVector3f GetTranslation() const { return mTransform.GetTranslation(); }

private:
  void InitQuad(CQuadDecal& quad, const CDecalDescription::SQuadDescr& description, int flag,
                const CUnitVector3f& direction, const rstl::vector< CCollisionSurface >& surfaces);
  // Guessed name for the Echoes-only projection and triangle clipping method.
  void BuildClippedGeometry(CQuadDecal& quad, const CDecalDescription::SQuadDescr& description,
                            const CUnitVector3f& direction,
                            const rstl::vector< CCollisionSurface >& surfaces);

  TLockedToken< CDecalDescription > mDescription;
  CTransform4f mTransform;
  mutable CQuadDecal mQuad1;
  mutable CQuadDecal mQuad2;
  int mModelLifetime;
  int mFrameIdx;
  int mFlags;
  mutable CVector3f mRotation;

  static CRandom16 mDecalRandom;
  static bool mMoveRedToAlphaBuffer;
  static bool mDisableAlphaUpdate;    // Guessed name
  static bool mEnableClippedGeometry; // Guessed name
};
NESTED_CHECK_SIZEOF(CDecal, SDecalVertex, 0x14)
NESTED_CHECK_SIZEOF(CDecal, CDecalPolygon, 0x10)
NESTED_CHECK_SIZEOF(CDecal, CQuadDecal, 0x34)
CHECK_SIZEOF(CDecal, 0xbc)

#endif // _CDECAL
