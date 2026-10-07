#include "WorldFormat/CMetroidModelInstance.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetaRender/IRenderer.hpp"

static const CTransform4f& TransformFromData(const void* data) {
  return *static_cast< const CTransform4f* >(data);
}

static CAABox BoundingBoxFromData(const void* data) {
  float values[6];
  const float* source = static_cast< const float* >(data);
  for (int i = 0; i < 6; ++i) {
    values[i] = CBasics::SwapBytes(source[i]);
  }
  return *reinterpret_cast< const CAABox* >(values);
}

CMetroidModelInstance::CMetroidModelInstance(
    const void* header, const void* materials, const void* positions, const void* normals,
    const void* colors, const void* texCoords, const void* packedTexCoords,
    const rstl::vector< void* >& surfaces, const void* const& section1, const void* const& section2)
: mVisorFlags(*static_cast< const uint* >(header))
, mWorldTransform(TransformFromData(static_cast< const uchar* >(header) + sizeof(uint)))
, mWorldBounds(BoundingBoxFromData(static_cast< const uchar* >(header) + sizeof(CTransform4f) +
                                   sizeof(uint)))
, mMaterialData(materials)
, mSurfaces(surfaces)
, mPositions(positions)
, mNormals(normals)
, mColors(colors)
, mTexCoords(texCoords)
, mPackedTexCoords(packedTexCoords)
, x74_(section1)
, mSurfaceGroups(section2) {}

SAreaSurface::SAreaSurface(CInputStream& in)
: mBounds(in)
, mModelIndex(in.ReadInt16())
, mSurfaceGroupIndex(in.ReadInt16())
, x1c_(in.ReadInt16())
, x1e_(in.ReadInt16()) {}

ushort CMetroidModelInstance::CSurfaceGroups::GetSurfaceCount(int group) const {
  const ushort count = mData[group + 1];
  if (group == 0) {
    return count;
  }
  return count - mData[group];
}

const ushort* CMetroidModelInstance::CSurfaceGroups::GetSurfaceIndices(int group) const {
  const int start = group != 0 ? mData[group] : 0;
  return mData + (mData[0] + 1) + start;
}
