#ifndef _CSEGID
#define _CSEGID

#include <types.h>

#include "Kyoto/Streams/CInputStream.hpp"

class CSegId {
public:
  CSegId() : mId(-1) {}

  CSegId(int id) : mId(id) {}

  CSegId(uint id) : mId(id) {}

  CSegId(const char& id) : mId(id) {}

  CSegId(CInputStream& in);

  static CSegId Null() { return CSegId(99); }

  static CSegId Invalid() { return CSegId(255); }

  bool operator==(const CSegId& other) const { return mId == other.mId; }

  bool operator!=(const CSegId& other) const { return mId != other.mId; }

  const uchar val() const { return mId; }

private:
  uchar mId;
};
CHECK_SIZEOF(CSegId, 0x1)

namespace rstl {
template <>
struct is_trivially_destructible< CSegId > {
  enum { value = true };
};

template <>
inline void construct< CSegId >(void* dest, const CSegId& src) {
  *static_cast< CSegId* >(dest) = src;
}

template <>
struct is_trivially_destructible< pair< CSegId, CSegId > > {
  enum { value = true };
};

template <>
inline void construct< pair< CSegId, CSegId > >(void* dest, const pair< CSegId, CSegId >& src) {
  *static_cast< pair< CSegId, CSegId >* >(dest) = src;
}
} // namespace rstl

#endif // _CSEGID
