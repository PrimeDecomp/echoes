#include "rstl/rmemory_allocator.hpp"

void* rstl::rmemory_allocator::allocate(int size) {
  return size == 0 ? nullptr : rs_new uchar[size];
}
