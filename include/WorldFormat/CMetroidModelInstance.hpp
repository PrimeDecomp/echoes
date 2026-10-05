#ifndef _CMETROIDMODELINSTANCE
#define _CMETROIDMODELINSTANCE

#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "rstl/vector.hpp"

class CMetroidModelInstance {
public:
  // Reconstructed name for the borrowed serialized group-count/index table.
  class CSurfaceGroups {
  public:
    explicit CSurfaceGroups(const void* data) : mData(static_cast< const ushort* >(data)) {}
    ushort GetSurfaceCount(int group) const;
    const ushort* GetSurfaceIndices(int group) const;

  private:
    const ushort* mData;
  };

  CMetroidModelInstance(const void* header, const void* materials, const void* positions,
                        const void* normals, const void* colors, const void* texCoords,
                        const void* packedTexCoords, const rstl::vector< void* >& surfaces,
                        const void* const& section1, const void* const& section2);

  const rstl::vector< void* >& GetSurfaces() const { return mSurfaces; }
  const void* GetVertexPointer() const { return mPositions; }
  const void* GetMaterialPointer() const { return mMaterialData; }
  const void* GetNormalPointer() const { return mNormals; }
  const void* GetColorPointer() const { return mColors; }
  const void* GetTCPointer() const { return mTexCoords; }
  const void* GetPackedTCPointer() const { return mPackedTexCoords; }
  const CAABox& GetBoundingBox() const { return mWorldBounds; }
  int GetFlags() const { return mVisorFlags; }
  CSurfaceGroups GetSurfaceGroups() const { return CSurfaceGroups(mSurfaceGroups); }

  // Guessed name. Each serialized surface record is two halfwords after the header.
  ushort GetSurfaceAreaIndex(int surface) const {
    return static_cast< const ushort* >(x74_)[surface * 2 + 2];
  }

  // Guessed names. The resource stores cumulative surface counts followed by indices.
  ushort GetSurfaceCountInGroup(int group) const {
    const ushort* table = static_cast< const ushort* >(mSurfaceGroups);
    return group == 0 ? table[group + 1] : table[group + 1] - table[group];
  }
  const ushort* GetSurfaceIndices(int group) const {
    const ushort* table = static_cast< const ushort* >(mSurfaceGroups);
    const ushort start = group == 0 ? 0 : table[group];
    return table + table[0] + 1 + start;
  }

  CCubeMaterial GetMaterialByIndex(int index) const {
    const uint* data = static_cast< const uint* >(mMaterialData);
    const uint* offsets = data + data[0] + 1;
    const uint offset = index == 0 ? 0 : offsets[index];
    const uchar* materials = reinterpret_cast< const uchar* >(offsets + offsets[0] + 1);
    return CCubeMaterial(materials + offset);
  }

private:
  int mVisorFlags;
  CTransform4f mWorldTransform;
  CAABox mWorldBounds;
  const void* mMaterialData;
  rstl::vector< void* > mSurfaces;
  const void* mPositions;
  const void* mNormals;
  const void* mColors;
  const void* mTexCoords;
  const void* mPackedTexCoords;
  const void* x74_;
  const void* mSurfaceGroups; // Guessed name.
};
CHECK_SIZEOF(CMetroidModelInstance, 0x7c)
NESTED_CHECK_SIZEOF(CMetroidModelInstance, CSurfaceGroups, 4)

#endif // _CMETROIDMODELINSTANCE
