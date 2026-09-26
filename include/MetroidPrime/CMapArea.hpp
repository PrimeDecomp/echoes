#ifndef _CMAPAREA
#define _CMAPAREA

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "rstl/single_ptr.hpp"

class CInputStream;
class CMappableObject;
class IWorld;

class CMapArea {
public:
  class CMapAreaSurface {
    friend class CMapArea;

  public:
    void PostConstruct(const void* buf);
    CVector3f GetNormal() const { return mNormal; }
    CVector3f GetCenterPosition() const { return mCentroid; }

  private:
    CVector3f mNormal;
    CVector3f mCentroid;
    const int* mSurfOffset;
    const int* mOutlineOffset;
  };

  enum EVisMode { kVM_Always, kVM_MapStationOrVisit, kVM_Visit, kVM_Never };

  CMapArea(CInputStream& in, uint size);
  ~CMapArea();

  void PostConstruct();
  bool GetIsVisibleToAutoMapper(bool worldVis, bool areaVis) const;
  bool IsInDarkWorld() const; // Guessed name
  CVector3f GetAreaCenterPoint() const;
  CTransform4f GetAreaPostTransform(const IWorld& world, int areaId);
  const CAABox& GetBoundingBox() const { return mBox; }
  const CVector3f& GetMapAdjustment() const { return mMapAdjustment; } // Guessed name
  int GetNumMappableObjects() const { return mMappableObjCount; }
  int GetNumSurfaces() const { return mSurfaceCount; }
  const CMapAreaSurface& GetSurface(int idx) const { return mSurfaceStart[idx]; }
  const CVector3f* GetVertices() const { return mVertexStart; }

private:
  uint mMagic;
  uint mVersion;
  uint mFlags; // Guessed name; bit 0 identifies a Dark Aether map area.
  EVisMode mVisibilityMode;
  CAABox mBox;
  CVector3f mMapAdjustment; // Guessed name
  int mMappableObjCount;
  int mVertexCount;
  int mSurfaceCount;
  uint mSize;
  CMappableObject* mMoStart;
  CVector3f* mVertexStart;
  CMapAreaSurface* mSurfaceStart;
  rstl::single_ptr< uchar > mBuf;

  // Guessed names: prepared rendering storage and its non-owning sections.
  rstl::single_ptr< uchar > mRenderBuf;
  uint mRenderBufSize;
  CVector3f* mDoorVertices;
  uint mDoorVerticesSize;
  CVector3f* mSurfaceNormals;
  uint mSurfaceNormalsSize;
  uchar* mSurfaceDisplayList;
  uint mSurfaceDisplayListSize;
  uchar* mOutlineDisplayList;
  uint mOutlineDisplayListSize;
};
NESTED_CHECK_SIZEOF(CMapArea, CMapAreaSurface, 0x20)
CHECK_SIZEOF(CMapArea, 0x7c)

#endif // _CMAPAREA
