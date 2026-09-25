#ifndef _SOBJECTTAG
#define _SOBJECTTAG

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/construct.hpp"
#include "types.h"

#define kInvalidAssetId 0xFFFFFFFFu

typedef uint CAssetId;
typedef uint FourCC;

struct SObjectTag {
  FourCC type;
  CAssetId id;

  SObjectTag() {}
  SObjectTag(FourCC type, CAssetId id) : type(type), id(id) {}
  SObjectTag(const SObjectTag& other) : type(other.type), id(other.id) {}
  SObjectTag(CInputStream& in) : type(in.ReadInt32()), id(in.ReadInt32()) {}

  static const char* Type2Text(FourCC type);
};

namespace rstl {
template <>
inline void construct< SObjectTag >(void* dest, const SObjectTag& src) {
  *static_cast< SObjectTag* >(dest) = src;
}
} // namespace rstl

#endif // _SOBJECTTAG
