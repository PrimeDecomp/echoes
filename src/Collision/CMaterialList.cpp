#include "Collision/CMaterialList.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CMaterialList::CMaterialList(CInputStream& in) : mValue(in.ReadInt64()) {}

int CMaterialList::BitPosition(u64 flags) {
  for (int ret = 0, i = 0; i < 32; ++i) {
    if ((flags & 1) != 0) {
      return ret;
    }
    flags >>= 1;
    ++ret;
  }
  return -1;
}
