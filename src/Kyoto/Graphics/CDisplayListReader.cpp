#include "Kyoto/Graphics/CDisplayListReader.hpp"

#include "Kyoto/Basics/CBasics.hpp"

#include <string.h>

namespace {
// Like CCubeModel's wireframe reader: an unaligned native-endian load on our big-endian target.
inline const ushort ReadDisplayListShort(const uchar* data) {
  uchar bytes[2];
  bytes[0] = data[0];
  bytes[1] = data[1];
#ifdef __MWERKS__
  return CBasics::SwapBytes(*reinterpret_cast< const ushort* >(bytes));
#else
  ushort value;
  memcpy(&value, bytes, sizeof(value));
  return CBasics::SwapBytes(value);
#endif
}
} // namespace

CDisplayListReader::CDisplayListReader(const void* displayList, uint size, uint vertexDesc)
: mDisplayList(static_cast< const uchar* >(displayList))
, mSize(size)
, mVertexDesc(vertexDesc)
, mVertexStride(GetVertexStride(vertexDesc)) {}

uint CDisplayListReader::GetVertexStride(uint vertexDesc) {
  uint stride = 0;
  for (int i = 0; i < 12; ++i) {
    switch (static_cast< GXAttrType >(vertexDesc >> (i * 2) & 3)) {
    case GX_NONE:
    case GX_DIRECT:
      break;
    case GX_INDEX8:
      ++stride;
      break;
    case GX_INDEX16:
      stride += 2;
      break;
    }
  }
  for (int i = 0; i < 8; ++i) {
    if (static_cast< GXAttrType >(vertexDesc >> (i + 24) & 1) == GX_DIRECT) {
      ++stride;
    }
  }
  return stride;
}

GXAttrType CDisplayListReader::GetAttributeType(uint vertexDesc, GXAttr attribute) {
  if (attribute >= GX_VA_POS) {
    return static_cast< GXAttrType >(vertexDesc >> ((attribute - GX_VA_POS) * 2) & 3);
  }
  GXAttrType type = GX_NONE;
  if (attribute <= GX_VA_TEX6MTXIDX) {
    type = static_cast< GXAttrType >(vertexDesc >> (attribute + 24) & 1);
  }
  return type;
}

uint CDisplayListReader::GetVertexIndex(const uchar* vertex, GXAttr attribute) const {
  const GXAttrType type = GetAttributeType(mVertexDesc, attribute);
  const uchar* cursor = vertex;
  if (type == GX_NONE) {
    return uint(-1);
  }

  for (int i = 0; i < attribute; ++i) {
    switch (GetAttributeType(mVertexDesc, static_cast< GXAttr >(i))) {
    case GX_NONE:
      break;
    case GX_DIRECT:
    case GX_INDEX8:
      ++cursor;
      break;
    case GX_INDEX16:
      cursor += 2;
      break;
    }
  }
  switch (type) {
  case GX_DIRECT:
  case GX_INDEX8:
    return *cursor;
  case GX_INDEX16:
    return ReadDisplayListShort(cursor);
  default:
    return uint(-1);
  }
}

bool CDisplayListReader::EnumerateTriangles(IDisplayListTriangleCallback& callback) const {
  const uchar* displayList = mDisplayList;
  const int size = mSize;
  int offset = 0;
  while (offset < size) {
    const GXPrimitive primitive = static_cast< GXPrimitive >(displayList[offset++] & 0xfc);
    if (primitive == GX_NOP) {
      continue;
    }
    const ushort count = ReadDisplayListShort(displayList + offset);
    offset += 2;

    switch (primitive) {
    case GX_TRIANGLES: {
      const int stride = mVertexStride;
      int vertexOffset = offset;
      for (int i = 0; i < count; i += 3, vertexOffset += stride * 3) {
        const uchar* vertex = displayList + vertexOffset;
        if (!callback.OnTriangle(*this, vertex, vertex + stride, vertex + stride * 2)) {
          return false;
        }
      }
      break;
    }
    case GX_TRIANGLEFAN: {
      const uchar* first = displayList + offset;
      const uchar* previous = first + mVertexStride;
      const uchar* current = previous + mVertexStride;
      for (int i = 2; i < count; ++i) {
        if (!callback.OnTriangle(*this, first, previous, current)) {
          return false;
        }
        previous = current;
        current += mVertexStride;
      }
      break;
    }
    case GX_TRIANGLESTRIP: {
      const uchar* vertices = displayList + offset;
      for (int i = 2; i < count; ++i) {
        if (i & 1) {
          if (!callback.OnTriangle(*this, vertices + mVertexStride * (i - 1),
                                   vertices + mVertexStride * (i - 2),
                                   vertices + mVertexStride * i)) {
            return false;
          }
        } else {
          if (!callback.OnTriangle(*this, vertices + mVertexStride * (i - 2),
                                   vertices + mVertexStride * (i - 1),
                                   vertices + mVertexStride * i)) {
            return false;
          }
        }
      }
      break;
    }
    }
    offset += count * mVertexStride;
  }
  return true;
}
