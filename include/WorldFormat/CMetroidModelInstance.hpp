#ifndef _CMETROIDMODELINSTANCE
#define _CMETROIDMODELINSTANCE

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "rstl/vector.hpp"

class CMetroidModelInstance {
public:
  CMetroidModelInstance(const void* header, const void* materials, const void* positions,
                        const void* normals, const void* colors, const void* texCoords,
                        const void* packedTexCoords, const rstl::vector< void* >& surfaces,
                        const void* const& section1, const void* const& section2);

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
  const void* x78_;
};
CHECK_SIZEOF(CMetroidModelInstance, 0x7c)

#endif // _CMETROIDMODELINSTANCE
