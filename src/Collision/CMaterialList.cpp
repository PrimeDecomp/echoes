#include "Collision/CMaterialList.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CMaterialList::CMaterialList(CInputStream& in) : mValue(in.ReadInt64()) {}

int CMaterialList::BitPosition(u64 flags) {
  for (int bit = 0; bit < 32; ++bit) {
    if ((flags & 1) != 0) {
      return bit;
    }
    flags >>= 1;
  }
  return -1;
}
