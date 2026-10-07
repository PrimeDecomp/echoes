#ifndef _CDISPLAYLISTREADER
#define _CDISPLAYLISTREADER

#include "types.h"
#include <dolphin/gx.h>

class CDisplayListReader;

// Guessed name. Borrowed callback; the native interface has no virtual destructor.
class IDisplayListTriangleCallback {
public:
  virtual bool OnTriangle(const CDisplayListReader& reader, const uchar* a, const uchar* b,
                          const uchar* c) = 0;
};

// Guessed name for the shared GX display-list triangle reader.
class CDisplayListReader {
public:
  CDisplayListReader(const void* displayList, uint size, uint vertexDesc);
  bool EnumerateTriangles(IDisplayListTriangleCallback& callback) const;
  uint GetVertexIndex(const uchar* vertex, GXAttr attribute) const;

  // Guessed names for the native packed-descriptor decoding helpers.
  static uint GetVertexStride(uint vertexDesc);
  static GXAttrType GetAttributeType(uint vertexDesc, GXAttr attribute);

private:
  const uchar* mDisplayList;
  int mSize;
  uint mVertexDesc;
  uint mVertexStride;
};
CHECK_SIZEOF(CDisplayListReader, 0x10)

#endif // _CDISPLAYLISTREADER
